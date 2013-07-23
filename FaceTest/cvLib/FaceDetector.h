

#ifndef FaceDetector_h
#define FaceDetector_h

#include "vec2Dc.h"
class vec2D;
class vec2Dc;
class ANNetwork;
class AIClassifier;
#include "imagepyramid.h"


class FaceDetector
{
public:
        FaceDetector();
        //FaceDetector(const FaceDetector& fdetect);
        ~FaceDetector();

// Operators
        //const FaceDetector& operator=(const FaceDetector& fdetect);

// Operations
        void init(unsigned int image_width, unsigned int image_height,
                  unsigned int face_width, unsigned int face_height, 
                  const float* scales = 0, unsigned int nscales = 0);
        
        int load_skin_filter(const wchar_t* fname);
        void unload_skin_filter();
        int load_preface_filter(const wchar_t* fname);
        void unload_preface_filter();
        int load_projection_matrix(const wchar_t* fname);
        void unload_projection_matrix();
        int load_face_classifier(const wchar_t* fname);
        void unload_face_classifier();


        //res<0 err, res>0 number of faces detected
        //-1: not init, -2, -3
        int detect(const vec2D* y,                         //Gray image
                   char** r, char** g, char** b,           //R,G,B components for skinfilter
                   const vec2Dc* search_mask = 0);         

// Access
        inline unsigned int get_dx() const;
        inline unsigned int get_dy() const;
        inline unsigned int get_faces_number() const;
        inline const RECT* get_face_rect(unsigned int i) const; 
        inline const vec2D* get_face(unsigned int i) const;
        inline float get_motion_amount() const;
        inline float get_skin_amount() const;

        //for debugging purposes        
        inline const vec2Dc* get_skin_mask() const;
        inline const ImageFrame* get_image_frame(unsigned int i) const;

// Inquiry
        inline int status() const;
        inline unsigned int status_of_classifiers() const;
        
        
        enum Config { PRJMAT_BIT = 0x1, 
                      FACEAI_BIT = 0x2, 
                      SKINAI_BIT = 0x4,
                      PREFACEAI_BIT = 0x8};

protected:
private:
        FaceDetector(const FaceDetector& fdetect);
        const FaceDetector& operator=(const FaceDetector& fdetect);

        int m_status;                           //-1 not init, 0 init                                    
        unsigned int m_status_of_classifiers;   //        

        unsigned int m_dx;          //offset to search from in the image 
        unsigned int m_dy;

        vec2D* m_histogram;         //temp array for histogram
        vec2D* m_face_rectangle;    //face_height x face_width vector  
        vec2D* m_face_vector;       //1 x (face_height * face_width) vector
        vec2D* m_projected_face;    //

        float m_skin_amount;        //-1.0f - skin filter absent; 0.0f - 1.0f skin amount
        float m_motion_amount;      //-1.0f - no motion estimation; 0.0f - 1.0f motion amount
        vec2Dc* m_skin_mask;
        vec2Dc* m_tmp_skin_mask;
        AIClassifier* m_skin_filter;
        void estimate_motion_percent(const vec2Dc* search_mask);
        void skin_filter(char** r, char** g, char** b, const vec2Dc* search_mask = 0);

        AIClassifier* m_preface_filter;
        AIClassifier* m_face_classifier;
        ANNetwork* m_projection_matrix;

        ImagePyramid m_pyramid;      //scales images
        
        vector<FACERECT> m_faces;    //found faces        
        void clear_faces();

        void run_classifiers();
        unsigned int search_faces();
        void erase_face_rect(const RECT& r);
        void erase_face_rect(unsigned int x, unsigned int y);
        bool is_foundface_overlaps(const FACERECT& fr) const;
        

};

// Inlines
inline int FaceDetector::status() const
{
        return m_status;
}

inline unsigned int FaceDetector::status_of_classifiers() const
{
        return m_status_of_classifiers;
}

inline unsigned int FaceDetector::get_dx() const
{
        return m_dx;
}

inline unsigned int FaceDetector::get_dy() const
{
        return m_dy;
}

inline unsigned int FaceDetector::get_faces_number() const
{
        return (unsigned int)m_faces.size();
}

inline const RECT* FaceDetector::get_face_rect(unsigned int i) const
{
        if (get_faces_number() > 0)
                return &m_faces[i % get_faces_number()].rect;
        else
                return 0;
}

inline const vec2D* FaceDetector::get_face(unsigned int i) const
{
        if (get_faces_number() > 0)
                return m_faces[i % get_faces_number()].face;
        else
                return 0;
}

inline float FaceDetector::get_motion_amount() const
{
        return m_motion_amount;
}

inline float FaceDetector::get_skin_amount() const
{
        return m_skin_amount;
}

inline const vec2Dc* FaceDetector::get_skin_mask() const
{
        return m_skin_mask;
}

inline const ImageFrame* FaceDetector::get_image_frame(unsigned int i) const
{       
        if (m_pyramid.get_frames_number() > 0)
                return m_pyramid.get_frame(i % m_pyramid.get_frames_number());
        else 
                return 0;
}

#endif FaceDetector_h

