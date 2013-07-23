

#include "stdafx.h"
#include "vec2dc.h"
#include "vec2d.h"




/////////////////////////constructors/destructors////////////////////////////////////////////////////
vec2Dc::vec2Dc(unsigned int ysize, unsigned int xsize, const unsigned char* data) : m_width(xsize), m_height(ysize)
{
        init(ysize, xsize);

        for (unsigned int y = 0; y < height(); y++) {
                for (unsigned int x = 0; x < width(); x++) {
                        if (data != 0)
                                m_data[y][x] = *data++;
                        else
                                m_data[y][x] = 0;
                }
        }
}
vec2Dc::vec2Dc(const vec2Dc& v)
{
        init(v.height(), v.width());
        copy(v);
}
vec2Dc::~vec2Dc()
{
        close();
}
/////////////////////////constructors/destructors////////////////////////////////////////////////////


void vec2Dc::init(unsigned int ysize, unsigned int xsize)
{
        m_width = xsize;
        m_height = ysize;
        m_data = (unsigned char**) malloc(m_height * sizeof(unsigned char*));     //setup rows
        for (unsigned int j = 0; j < m_height; j++)
                m_data[j] = (unsigned char *) malloc(m_width * sizeof(unsigned char));    //setup cols
}
void vec2Dc::close()
{
        for (unsigned int j = 0; j < m_height; j++)
                free(m_data[j]);      //delete colums
        free(m_data);
}

void vec2Dc::print(const wchar_t* file) const
{
        const vec2Dc& v = *this;
        if (file) {
                FILE *fp = _wfopen(file, L"wt");
                if (fp != 0) {
                        fwprintf(fp, L"\n vec: %p\n", this);
                        for (unsigned int y = 0; y < height(); y++) {
                                for (unsigned int x = 0; x < width(); x++)
                                        fwprintf(fp, L"%4d", v(y, x));
                                fwprintf(fp, L"\n");
                        }
                        fclose(fp);
                }
        } else {
                wprintf(L"\n vec: %p\n", this);
                for (unsigned int y = 0; y < height(); y++) {
                        for (unsigned int x = 0; x < width(); x++)
                                wprintf(L"%4d", v(y, x));
                        wprintf(L"\n");
                }
        }
}

void vec2Dc::copy(const vec2Dc& v)
{
        vec2Dc &pv = *this;

        for (unsigned int y = 0; y < height(); y++)
                for (unsigned int x = 0; x < width(); x++)
                        pv(y, x) = v(y, x);
}
void vec2Dc::and(const vec2Dc& v)
{
        vec2Dc &pv = *this;

        for (unsigned int y = 0; y < height(); y++)
                for (unsigned int x = 0; x < width(); x++)
                        pv(y, x) &= v(y, x);
}
void vec2Dc::and(const vec2Dc& a, const vec2Dc& b)        //this = a AND b
{
        vec2Dc &pv = *this;

        for (unsigned int y = 0; y < height(); y++)
                for (unsigned int x = 0; x < width(); x++)
                        pv(y, x) = a(y, x) & b(y, x);
}
void vec2Dc::set(unsigned char scalar)
{
        vec2Dc &v = *this;
        for (unsigned int y = 0; y < height(); y++)
                for (unsigned int x = 0; x < width(); x++)
                        v(y, x) = scalar;
}

void vec2Dc::inter2(const vec2Dc& src, vec2D& dst_grdx, vec2D& dst_grdy)
{
        vec2Dc& trg = *this;

        float xrto = float(width() - 1) / float(src.width() - 1);       //trg/src
        float yrto = float(height() - 1) / float(src.height() - 1);

        //arrange before srcx,srcy,frcx,frcy to speed calcs
        for (unsigned int y = 0; y < height(); y++) {
                float srcy = (float)y / yrto;
                float frcy = srcy - float((int)srcy);
                dst_grdy(0, y) = srcy;
                dst_grdy(1, y) = frcy;
        }

        for (unsigned int x = 0; x < width(); x++) {
                float srcx = (float)x / xrto;               //position to take from src vec2D
                float frcx = srcx - float((int)srcx);       //srcx=1.34  frcx=.34
                dst_grdx(0, x) = srcx;
                dst_grdx(1, x) = frcx;
        }
        //arrange before srcx,srcy,frcx,frcy to speed calcs

        for (unsigned int y = 0; y < height(); y++) {
                for (unsigned int x = 0; x < width(); x++) {

                        unsigned int sx = (unsigned int)dst_grdx(0, x);         //x index to source
                        unsigned int sy = (unsigned int)dst_grdy(0, y);         //y index to source
                        float tmp = src(sy, sx) * (1.0f - dst_grdy(1, y)) * (1.0f - dst_grdx(1, x));         //1-frcy 1-frcx

                        if (dst_grdx(1, x) > 0.0f && sx + 1 < src.width())
                                tmp += src(sy, sx + 1) * (1.0f - dst_grdy(1, y)) * dst_grdx(1, x);        //1-frcy frcx
                        if (dst_grdy(1, y) > 0.0f && sy + 1 < src.height())
                                tmp += src(sy + 1, sx) * dst_grdy(1, y) * (1.0f - dst_grdx(1, x));        //frcy 1-frcx
                        if ((dst_grdx(1, x) > 0.0f && dst_grdy(1, y) > 0.0f) && (sx + 1 < src.width() && sy + 1 < src.height()))
                                tmp += src(sy + 1, sx + 1) * dst_grdy(1, y) * dst_grdx(1, x);             //frcy frcx

                        trg(y, x) = (int)tmp;
                }
        }
}

int vec2Dc::dilate(const vec2Dc& src, unsigned int w, unsigned int h)
{
        if (w < 2) w = 2;
        if (h < 2) h = 2;

        int strel_w = (int)w;
        int strel_h = (int)h;

        vec2Dc& trg = *this;
        unsigned char val;

        if (trg.is_size_equal(src) == false)
                return -1;

        for (unsigned int y = 0; y < height(); y++) {
                for (unsigned int x = 0; x < width(); x++) {
                        val = 0;
                        for (int j = -strel_h / 2; j <= strel_h / 2; j++) {
                                for (int i = -strel_w / 2; i <= strel_w / 2; i++) {
                                        if (y + j < 0 || y + j > height() - 1)
                                                continue;
                                        if (x + i < 0 || x + i > width() - 1)
                                                continue;
                                        if (val < src(y + j, x + i))
                                                val = src(y + j, x + i);
                                }
                        }
                        trg(y, x) = val;
                }
        }

        return 0;
}

int vec2Dc::erode(const vec2Dc& src, unsigned int w, unsigned int h)
{
        if (w < 2) w = 2;
        if (h < 2) h = 2;

        int strel_w = (int)w;
        int strel_h = (int)h;

        vec2Dc& trg = *this;
        unsigned char val;

        if (trg.is_size_equal(src) == false)
                return -1;

        for (unsigned int y = 0; y < height(); y++) {
                for (unsigned int x = 0; x < width(); x++) {
                        val = 255;
                        for (int j = -strel_h / 2; j <= strel_h / 2; j++) {
                                for (int i = -strel_w / 2; i <= strel_w / 2; i++) {
                                        if (y + j < 0 || y + j > height() - 1)
                                                continue;
                                        if (x + i < 0 || x + i > width() - 1)
                                                continue;
                                        if (val > src(y + j, x + i))
                                                val = src(y + j, x + i);
                                }
                        }
                        trg(y, x) = val;
                }
        }

        return 0;
}

bool vec2Dc::is_data_equal(const vec2Dc& v) const
{
        const vec2Dc& src = *this;
        if (is_size_equal(v) == true) {
                for (unsigned int y = 0; y < height(); y++) {
                        for (unsigned int x = 0; x < width(); x++) {
                                if (src(y, x) != v(y, x))
                                        return false;
                        }
                }
                return true;
        } else
                return false;
}

bool vec2Dc::is_size_equal(const vec2Dc& v) const
{
        const vec2Dc& src = *this;
        if (src.width() == v.width() && src.height() == v.height())
                return true;
        else
                return false;
}