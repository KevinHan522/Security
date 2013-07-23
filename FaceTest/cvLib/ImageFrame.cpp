

#include "stdafx.h"
#include "imageframe.h"

#include "vec2D.h"
#include "vec2Dc.h"


//gaus blur filter
static float gblur[] = { 0.0625f,  0.125f,  0.0625f,
                         0.125f,   0.25f,   0.125f,
                         0.0625f,  0.125f,  0.0625f };


ImageFrame::ImageFrame(unsigned int width, unsigned int height, float zoom) : m_zoom(zoom)
{
        unsigned int w = unsigned int(zoom * (float)width);
        unsigned int h = unsigned int(zoom * (float)height);

        m_y = new vec2D(h, w);
        m_y_blured = new vec2D(h, w);        

        m_gaus_filter = new vec2D(3, 3, -1, -1, gblur);

        m_face_map = new vec2D(h, w);
        m_tmp_face_map = new vec2D(h, w);
        m_search_map = new vec2Dc(h, w);
        m_motion_map = new vec2Dc(h, w);
        m_skin_map = new vec2Dc(h, w);        
        
        grdx = new vec2D(2, w);
        grdy = new vec2D(2, h);
}
ImageFrame::~ImageFrame()
{
        delete m_y;
        delete m_y_blured;
        delete m_gaus_filter;
        delete m_face_map;
        delete m_tmp_face_map;
        delete m_search_map;
        delete m_motion_map;
        delete m_skin_map;
        delete grdx;
        delete grdy;
}

void ImageFrame::load_frame(const vec2D* y, const vec2Dc* motion, const vec2Dc* skin)
{
        if (m_zoom == 1.0f) {
                m_y->copy(*y);
                m_search_map->set(1);                

                if (skin != 0)
                        m_skin_map->copy(*skin);
                else
                        m_skin_map->set(1);
                if (motion != 0)
                        m_motion_map->copy(*motion);
                else
                        m_motion_map->set(1);                
        } else {
                m_y->inter2(*y, *grdx, *grdy);
                if (skin != 0)
                        m_skin_map->inter2(*skin, *grdx, *grdy);
                else
                        m_skin_map->set(1);
                if (motion != 0)
                        m_motion_map->inter2(*motion, *grdx, *grdy);
                else
                        m_motion_map->set(1);                
        }
        m_search_map->and(*m_motion_map, *m_skin_map);                
        m_y_blured->conv2D(*m_y, *m_gaus_filter);
}