

#include "stdafx.h"
#include "motiondetector.h"

#include "vec2d.h"
#include "vec2dc.h"
#include "facedetector.h"


MotionDetector::MotionDetector() : m_status(-1), m_last_frame(0), m_motion_vector(0), m_TH(10.0)
{
}

MotionDetector::~MotionDetector()
{
        close();
}

void MotionDetector::init(unsigned int image_width, unsigned int image_height)
{
        close();
        m_last_frame = new vec2D(image_height, image_width);
        m_motion_vector = new vec2Dc(image_height, image_width);
        m_status = 0;
}

void MotionDetector::close()
{
        if (m_last_frame != 0) {
                delete m_last_frame;
                m_last_frame = 0;
        }
        if (m_motion_vector != 0) {
                delete m_motion_vector;
                m_motion_vector = 0;
        }
        m_status = -1;
}

const vec2Dc* MotionDetector::detect(const vec2D* frame, const FaceDetector* fdetect)
{
        if (status() < 0)
                return 0;

        //m_last = frame - m_last       
        m_last_frame->sub(*frame, *m_last_frame);    

        //set to 1.0 faces rects
        RECT r0;
        for (unsigned int i = 0; i < fdetect->get_faces_number(); i++) {
                const RECT* r = fdetect->get_face_rect(i);                                
                r0.left = r->left;
                r0.top = r->top;
                r0.right = r->right;
                r0.bottom = r->bottom;
                m_last_frame->set(255.0f, r0);
        }

        for (unsigned int y = fdetect->get_dy(); y < m_motion_vector->height() - fdetect->get_dy(); y++) {
                for (unsigned int x = fdetect->get_dx(); x < m_motion_vector->width() - fdetect->get_dx(); x++) {
                        if (fabs((*m_last_frame)(y, x)) > m_TH)
                                (*m_motion_vector)(y, x) = 1;
                        else
                                (*m_motion_vector)(y, x) = 0;
                }
        }
        *m_last_frame = *frame;        

        return m_motion_vector;
}

void MotionDetector::clear_last_frame()
{
        if (m_last_frame != 0)
                m_last_frame->set(0.0f);
}