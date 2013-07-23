

#include "stdafx.h"
#include "ImageResize.h"

#include "vec2D.h"
#include "basefwt.h"
#include "haar.h"


/////////////////////////constructors/destructors////////////////////////////////////////////////////
ImageResize::ImageResize() : m_status(0), m_width(0), m_height(0), m_rdata(0), m_gdata(0), m_bdata(0),
                             m_zoom(1.0f), m_y(0)
{
        m_r = new Haar();
        m_g = new Haar();
        m_b = new Haar();
}
ImageResize::~ImageResize()
{
        delete m_r;
        delete m_g;
        delete m_b;
        if (m_y != 0) delete m_y;
        if (m_rdata != 0) delete[] m_rdata;
        if (m_gdata != 0) delete[] m_gdata;
        if (m_bdata != 0) delete[] m_bdata;
}
/////////////////////////constructors/destructors////////////////////////////////////////////////////


void ImageResize::init(unsigned int w, unsigned int h, float zoom)
{
        m_width = w;
        m_height = h;
        m_zoom = zoom;

        if (m_y != 0) delete m_y;
        m_y = new vec2D((unsigned int)((float)h*zoom), (unsigned int)((float)w*zoom));

        if (m_rdata != 0) delete[] m_rdata;
        m_rdata = new unsigned char[w*h];
        if (m_gdata != 0) delete[] m_gdata;
        m_gdata = new unsigned char[w*h];
        if (m_bdata != 0) delete[] m_bdata;
        m_bdata = new unsigned char[w*h];
        
        m_r->init(w, h);
        m_g->init(w, h);
        m_b->init(w, h);

        m_status = 1;
}

int ImageResize::resize(const unsigned char *pBGR)    //pBGR [bgr] triplets
{
        if (m_status == 0) return -1;

         //uchar to char
        for (unsigned int i = 0; i < m_width*m_height; i++) {
                m_bdata[i] = *pBGR++;
                m_gdata[i] = *pBGR++;
                m_rdata[i] = *pBGR++;                
        }        

        //transform///////////////////////////////////////////
        unsigned int scales = (unsigned int)log2(1.0f / m_zoom);
        m_r->trans(m_rdata, scales);
        m_g->trans(m_gdata, scales);
        m_b->trans(m_bdata, scales);        

        rgb2y();

        //debug
        //m_y->print(L"pic.txt");

        return 0;
}

int ImageResize::resize(const unsigned char* pR, 
                        const unsigned char* pG, 
                        const unsigned char* pB) const
{
        if (m_status == 0) return -1;

        //transform///////////////////////////////////////////
        int scales = (int)log2(1.0f / m_zoom);
        m_r->trans(pR, scales);
        m_g->trans(pG, scales);
        m_b->trans(pB, scales);

        rgb2y();

        //debug
        //m_y->print(L"pic.txt");

        return 0;
}

void ImageResize::rgb2y() const
{
        char** r2 = m_r->getspec2d();
        char** g2 = m_g->getspec2d();
        char** b2 = m_b->getspec2d();
        //gray scale convert
        for (unsigned int y = 0; y < m_y->height(); y++)
                for (unsigned int x = 0; x < m_y->width(); x++) {
                        int r = int(r2[y][x]) + 128;
                        int g = int(g2[y][x]) + 128;
                        int b = int(b2[y][x]) + 128;

                        (*m_y)(y, x) = 0.2989f * float(r) + 0.5870f * float(g) + 0.1140f * float(b);      //SSE on
                }
}

char** ImageResize::getr() const
{
        return m_r->getspec2d();
}

char** ImageResize::getg() const
{
        return m_g->getspec2d();
}

char** ImageResize::getb() const
{
        return m_b->getspec2d();
}
