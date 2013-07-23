

#include "stdafx.h"
#include "facedetector.h"

#include "vec2D.h"
#include "vec2Dc.h"
#include "annetwork.h"
#include "aiclassifier.h"
#include "imageframe.h"


FaceDetector::FaceDetector() : m_status(-1), m_dx(0), m_dy(0), m_status_of_classifiers(0),
                               m_face_rectangle(0), m_face_vector(0), m_projected_face(0),
                               m_motion_amount(-1.0f), m_skin_amount(-1.0f), 
                               m_skin_mask(0), m_tmp_skin_mask(0), m_skin_filter(0),
                               m_preface_filter(0), m_projection_matrix(0), m_face_classifier(0) 
{
        m_histogram = new vec2D(1, 256);
}

FaceDetector::~FaceDetector()
{
        delete m_histogram;

        if (m_face_rectangle != 0) delete m_face_rectangle;
        if (m_face_vector != 0) delete m_face_vector;        
        if (m_skin_mask != 0) {
                delete m_skin_mask;
                delete m_tmp_skin_mask;
        }

        unload_skin_filter();
        unload_preface_filter();
        unload_face_classifier();
        unload_projection_matrix();

        clear_faces();
}

void FaceDetector::init(unsigned int image_width, unsigned int image_height,
                        unsigned int face_width, unsigned int face_height, 
                        const float* scales, unsigned int nscales)
{
        if (m_face_rectangle != 0) delete m_face_rectangle;
        m_face_rectangle = new vec2D(face_height, face_width);
        if (m_face_vector != 0) delete m_face_vector;
        m_face_vector = new vec2D(1, face_height * face_width);
        if (m_skin_mask != 0) {
                delete m_skin_mask;
                delete m_tmp_skin_mask;
        }
        m_skin_mask = new vec2Dc(image_height, image_width);
        m_tmp_skin_mask = new vec2Dc(image_height, image_width);

        m_dx = (face_width - 1) / 2;
        m_dy = (face_height - 1) / 2;

        m_pyramid.init(image_width, image_height, scales, nscales);
        
        m_status = 0;
}

int FaceDetector::load_skin_filter(const wchar_t* fname)
{
        unload_skin_filter();
        m_skin_filter = new AIClassifier(fname);
        if (m_skin_filter->status() == 0)
                m_status_of_classifiers |= SKINAI_BIT;
        return m_skin_filter->status();
}

void FaceDetector::unload_skin_filter()
{
        if (m_skin_filter != 0) {
                delete m_skin_filter;
                m_skin_filter = 0;
                m_status_of_classifiers &= ~SKINAI_BIT;
        }
}

int FaceDetector::load_preface_filter(const wchar_t* fname)
{
        unload_preface_filter();
        m_preface_filter = new AIClassifier(fname);
        if (m_preface_filter->status() == 0)
                m_status_of_classifiers |= PREFACEAI_BIT;
        return m_preface_filter->status();
}

void FaceDetector::unload_preface_filter()
{
        if (m_preface_filter != 0) {
                delete m_preface_filter;
                m_preface_filter = 0;
                m_status_of_classifiers &= ~PREFACEAI_BIT;
        }
}

int FaceDetector::load_projection_matrix(const wchar_t* fname)
{
        unload_projection_matrix();
        m_projection_matrix = new ANNetwork(fname);
        if (m_projection_matrix->status() == 0) {
                m_projected_face = new vec2D(1, m_projection_matrix->output_size());
                m_status_of_classifiers |= PRJMAT_BIT;
        }
        return m_projection_matrix->status();
}

void FaceDetector::unload_projection_matrix()
{
        if (m_projection_matrix != 0) {
                delete m_projected_face;
                m_projected_face = 0;
                delete m_projection_matrix;
                m_projection_matrix = 0;
                m_status_of_classifiers &= ~PRJMAT_BIT;
        }
}
        
int FaceDetector::load_face_classifier(const wchar_t* fname)
{
        unload_face_classifier();
        m_face_classifier = new AIClassifier(fname);
        if (m_face_classifier->status() == 0)
                m_status_of_classifiers |= FACEAI_BIT;
        return m_face_classifier->status();
}

void FaceDetector::unload_face_classifier()
{
        if (m_face_classifier != 0) {
                delete m_face_classifier;
                m_face_classifier = 0;
                m_status_of_classifiers &= ~FACEAI_BIT;
        }
}

void FaceDetector::clear_faces()
{
        for (unsigned int i = 0; i < get_faces_number(); i++)
                delete m_faces[i].face;
        m_faces.clear();
}


/*
    y - Gray image
    r,g,b - R,G,B components for skinfilter    
*/
int FaceDetector::detect(const vec2D* y, char** r, char** g, char** b, const vec2Dc* search_mask)
{
        if (m_status < 0)
                return m_status;        

        estimate_motion_percent(search_mask);

        m_skin_amount = -1.0f;
        const vec2Dc* pskin_mask = 0;
        //optional skin filter
        if ((r != 0 && g != 0 && b != 0) && m_skin_filter != 0 && 
             m_skin_filter->status() == 0 && m_skin_filter->input_dimension() == 3) {
                skin_filter(r, g, b, search_mask);
                pskin_mask = m_skin_mask;
        }                
                
        //build pyramid of images
        for (unsigned int i = 0; i < m_pyramid.get_frames_number(); i++)
                m_pyramid.get_frame(i)->load_frame(y, search_mask, pskin_mask);        
        
        if (m_projection_matrix == 0 || m_face_classifier == 0)
                return -2;
        if (m_face_classifier->input_dimension() != m_projection_matrix->output_size())
                return -3;

        run_classifiers();
        return search_faces();
}

void FaceDetector::run_classifiers()
{
        float ovec;

        int w2 = m_face_rectangle->width() / 2;
        int h2 = m_face_rectangle->height() / 2;

        bool prefilter = false;
        if (m_preface_filter != 0 && m_preface_filter->input_dimension() == m_face_rectangle->length())
                prefilter = true;

        for (unsigned int i = 0; i < m_pyramid.get_frames_number(); i++) {
                const vec2D& my = *m_pyramid.get_frame(i)->get_y_blured();
                vec2D& face_map = *m_pyramid.get_frame(i)->get_face_map();
                const vec2Dc& search_map = *m_pyramid.get_frame(i)->get_search_map();

                face_map.set(0.0f);
                for (unsigned int y = get_dy(); y < my.height() - get_dy(); y++) {
                        for (unsigned int x = get_dx(); x < my.width() - get_dx(); x++) {                                                                
                                //negotiate with [motion & skin detector out]
                                if (search_map(y, x) == 0)   
                                        continue;

                                if (m_face_rectangle->copy(my, x - w2, y - h2)) {  //copy from left,top offsets
                                        m_face_rectangle->histeq(*m_histogram);

                                        for (unsigned int m = 0; m < m_face_rectangle->length(); m++)
                                                (*m_face_vector)(0, m) = (*m_face_rectangle)[m];

                                        if (prefilter == true) {
                                                if (m_preface_filter->classify(&(*m_face_vector)(0, 0), &ovec) < 0)  //non-face                                                        
                                                        continue;                                                
                                        }

                                        //project to low dimension
                                        m_projection_matrix->classify(&(*m_face_vector)(0, 0), &(*m_projected_face)(0, 0));
                                        //classify projecte data
                                        m_face_classifier->classify(&(*m_projected_face)(0, 0), &face_map(y, x));                                        
                                }
                                //else failed to copy
                                // ovec=0.0f;
                        }
                }
                //debug
                //face_map.print(L"face_map.txt");
        }
}

unsigned int FaceDetector::search_faces()
{
        ImageFrame* pframe = 0;

        RECT rect;
        FACERECT face_rect;        

        clear_faces();

        float max = 0.0f, tmpmx = 0.0f, zoom = 0.0f;
        unsigned int x = 0, y = 0, index = 0;
        int tmpx = 0, tmpy = 0;
        unsigned int w2 = m_face_rectangle->width() / 2;
        unsigned int h2 = m_face_rectangle->height() / 2;

        //debug out only
        for (unsigned int i = 0; i < m_pyramid.get_frames_number(); i++)
                m_pyramid.get_frame(i)->get_tmp_face_map()->copy(*m_pyramid.get_frame(i)->get_face_map());
        //debug out only

        while (true) {

                max = -FLT_MAX;
                index = 0;
                for (unsigned int i = 0; i < m_pyramid.get_frames_number(); i++) {
                        pframe = m_pyramid.get_frame(i);
                        pframe->get_face_map()->maxval(tmpmx, tmpx, tmpy, 3, 3, m_dx, m_dy);
                        if (tmpmx > max) {
                                index = i;
                                max = tmpmx;
                                x = tmpx;
                                y = tmpy;
                        }
                }

                pframe = m_pyramid.get_frame(index);
                zoom = pframe->get_zoom(); 
                if (max > 6.0f*0.8f + 3.0f*0.15f) { //3x3 positives             //6 positives + 3 negatives in 3x3 box

                        rect.left = (int)(float(x - w2) / zoom);
                        rect.top = (int)(float(y - h2) / zoom);
                        rect.right = (int)(float(x + w2) / zoom);
                        rect.bottom = (int)(float(y + h2) / zoom);
                        face_rect.rect = rect;
                        face_rect.x = (unsigned int)(float(x) / zoom);
                        face_rect.y = (unsigned int)(float(y) / zoom);
                        face_rect.diag = sqrt(pow((float(w2) / zoom), 2.0f) + pow((float(h2) / zoom), 2.0f));
                        face_rect.face = new vec2D(m_face_rectangle->height(), m_face_rectangle->width());
                        face_rect.face->copy(*pframe->get_y(), x - w2, y - h2);
                        face_rect.face->normalize(0.0f, 1.0f);

                        if (is_foundface_overlaps(face_rect) == false) {
                                m_faces.push_back(face_rect);
                                erase_face_rect(rect);            //erase found face region  [left, right)
                        } else {
                                erase_face_rect((float)x / zoom, (float)y / zoom);
                        }
                } else
                        break;
        }

        return (unsigned int)m_faces.size();
}

void FaceDetector::erase_face_rect(const RECT& r)
{
        RECT zr;
        for (unsigned int i = 0; i < m_pyramid.get_frames_number(); i++) {
                ImageFrame* pframe = m_pyramid.get_frame(i);
                float zoom = pframe->get_zoom();

                zr.left = int((float)r.left * zoom);
                zr.top = int((float)r.top * zoom);
                zr.right = int((float)r.right * zoom) + 1;
                zr.bottom = int((float)r.bottom * zoom) + 1;
                
                pframe->get_face_map()->set(0.0f, zr);   //[left, right)                              
        }
}
void FaceDetector::erase_face_rect(unsigned int x, unsigned int y)
{
        RECT zr;
        int tmpx, tmpy;
        for (unsigned int i = 0; i < m_pyramid.get_frames_number(); i++) {
                ImageFrame* pframe = m_pyramid.get_frame(i);
                float zoom = pframe->get_zoom();

                tmpx = int((float)x * zoom);
                tmpy = int((float)y * zoom);
                zr.left = tmpx - 2;
                zr.top = tmpy - 2;
                zr.right = (tmpx + 2) + 1;
                zr.bottom = (tmpy + 2) + 1;

                pframe->get_face_map()->set(0.0f, zr);   //[left, right)                
        }
}

bool FaceDetector::is_foundface_overlaps(const FACERECT& fr) const
{
        for (unsigned int i = 0; i < (unsigned int)m_faces.size(); i++) {
                const FACERECT* pfr = &m_faces[i];
                //euclid distanse between centers
                float dist = sqrt(pow((float(pfr->x) - float(fr.x)), 2)
                                   + pow((float(pfr->y) - float(fr.y)), 2));
                if (dist <= 0.90f * (fr.diag + pfr->diag))
                        return true;
        }
        return false;
}





void FaceDetector::estimate_motion_percent(const vec2Dc* search_mask)
{
        if (search_mask == 0)
                m_motion_amount = -1.0f;
        else {
                unsigned int motion_pixels = 0;
                unsigned int total_pixels = 0;
                for (unsigned int y = get_dy(); y < search_mask->height() - get_dy(); y++) {
                        for (unsigned int x = get_dx(); x < search_mask->width() - get_dx(); x++) {
                                total_pixels++;
                                if ((*search_mask)(y, x) == 1)
                                        motion_pixels++;
                        }
                }
                m_motion_amount = float(motion_pixels) / float(total_pixels);
        }
}
/*
        check if (m_skin_filter != 0 && m_skin_filter->status() == 0) before call
*/
void FaceDetector::skin_filter(char** r, char** g, char** b, const vec2Dc* search_mask)
{
        float ivec[3] = {0.0f, 0.0f, 0.0f};    //0.0 ... 1.0f range
        float ovec = 0.0f;

        m_skin_mask->set(0);

        unsigned int skin_pixels = 0;
        unsigned int total_pixels = 0;
        for (unsigned int y = get_dy(); y < m_skin_mask->height() - get_dy(); y++) {
                for (unsigned int x = get_dx(); x < m_skin_mask->width() - get_dx(); x++) {
                        total_pixels++;

                        if (search_mask != 0 && ((*search_mask)(y, x) == 0))                                 
                                continue;

                        ivec[0] = (float)((int)r[y][x] + 128) / 255.0f;
                        ivec[1] = (float)((int)g[y][x] + 128) / 255.0f;
                        ivec[2] = (float)((int)b[y][x] + 128) / 255.0f;
                        if (m_skin_filter->classify(ivec, &ovec) >= 0) {
                                (*m_skin_mask)(y, x) = 1;
                                skin_pixels++;
                        }
                }
        }
        m_skin_amount = float(skin_pixels) / float(total_pixels);

        m_tmp_skin_mask->dilate(*m_skin_mask, 5, 5);
        m_skin_mask->erode(*m_tmp_skin_mask, 5, 5);
}

