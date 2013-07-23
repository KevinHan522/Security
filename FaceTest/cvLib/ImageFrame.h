

#ifndef ImageFrame_h
#define ImageFrame_h


class vec2D;
class vec2Dc;


class ImageFrame
{
public:
        ImageFrame(unsigned int width, unsigned int height, float zoom);
        //ImageFrame(const ImageFrame& imframe);
        ~ImageFrame();

// Operators
        //const ImageFrame& operator=(const ImageFrame& imframe);

// Operations
        void load_frame(const vec2D* y, const vec2Dc* motion = 0, const vec2Dc* skin = 0);

// Access
        inline float get_zoom() const;
        inline const vec2D* get_y() const;
        inline const vec2D* get_y_blured() const;
        inline const vec2Dc* get_search_map() const;
        inline const vec2Dc* get_skin_map() const;
        inline vec2D* get_face_map() const;
        inline vec2D* get_tmp_face_map() const;
        
// Inquiry

protected:
private:
        ImageFrame(const ImageFrame& imframe);
        const ImageFrame& operator=(const ImageFrame& imframe);

        float m_zoom;                  //zoom ration from original image

        vec2D* m_y;                    //Y image
        vec2D* m_y_blured;             //Y image gaus blured
        vec2D* grdx;                   //temporary grids for interpolation
        vec2D* grdy;                   //temporary grids
        
        vec2D* m_gaus_filter;

        vec2Dc* m_search_map;          //motion AND skin
        vec2Dc* m_motion_map;          //motion map
        vec2Dc* m_skin_map;            //skin detector output          
        vec2D* m_face_map;             //face detector output
        vec2D* m_tmp_face_map;         //copy of face_map for debugging

};

// Inlines
inline float ImageFrame::get_zoom() const
{
        return m_zoom;
}
inline const vec2D* ImageFrame::get_y() const
{
        return m_y;
}
inline const vec2D* ImageFrame::get_y_blured() const
{
        return m_y_blured;
}
inline const vec2Dc* ImageFrame::get_search_map() const
{
        return m_search_map;
}
inline const vec2Dc* ImageFrame::get_skin_map() const
{
        return m_skin_map;
}
inline vec2D* ImageFrame::get_face_map() const
{
        return m_face_map;
}
inline vec2D* ImageFrame::get_tmp_face_map() const
{
        return m_tmp_face_map;
}

#endif ImageFrame_h

