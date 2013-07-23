
#ifndef VEC2DC_H
#define VEC2DC_H


class vec2D;

class vec2Dc
{
public:
        vec2Dc(unsigned int ysize = 1, unsigned int xsize = 1, const unsigned char* data = 0);
        vec2Dc(const vec2Dc& v);
        ~vec2Dc();

        inline unsigned int width() const;
        inline unsigned int height() const;
        inline const unsigned char* data(int y, int x) const;
        void set(unsigned char scalar);                    //set all contents to scalar
        void print(const wchar_t* file = 0) const;         //dump contents
        void copy(const vec2Dc& v);                        //copy v to this
        void and(const vec2Dc& v);                         //this = this AND v
        void and(const vec2Dc& a, const vec2Dc& b);        //this = a AND b

        inline unsigned char& operator()(int y, int x);
        inline unsigned char operator()(int y, int x) const;
        inline const vec2Dc& operator=(const vec2Dc& v);
        inline bool operator==(const vec2Dc& v) const;
        inline bool operator!=(const vec2Dc& v) const;

        void inter2(const vec2Dc& src, vec2D& dst_grdx, vec2D& dst_grdy);          //biliniar 2d interpolation, grd temp buffers width = this->size height=2
        int dilate(const vec2Dc& src, unsigned int w = 3, unsigned int h = 3);
        int erode(const vec2Dc& src, unsigned int w = 3, unsigned int h = 3);

        bool is_data_equal(const vec2Dc& v) const;
        bool is_size_equal(const vec2Dc& v) const;


private:
        unsigned int m_width;
        unsigned int m_height;
        unsigned char** m_data;

        void init(unsigned int ysize, unsigned int xsize);
        void close();

};


inline unsigned int vec2Dc::width() const
{
        return m_width;
}

inline unsigned int vec2Dc::height() const
{
        return m_height;
}

inline const unsigned char* vec2Dc::data(int y, int x) const
{
        return m_data[y] + x;
}

inline unsigned char& vec2Dc::operator()(int y, int x)
{
        return m_data[y][x];
}

inline unsigned char vec2Dc::operator()(int y, int x) const
{
        return m_data[y][x];
}

inline const vec2Dc& vec2Dc::operator=(const vec2Dc& v)
{
        if (this != &v) {
                close();
                init(v.height(), v.width());
                copy(v);
        }
        return *this;
}

inline bool vec2Dc::operator==(const vec2Dc& v) const
{
        return is_data_equal(v);
}
inline bool vec2Dc::operator!=(const vec2Dc& v) const
{
        return !is_data_equal(v);
}

#endif