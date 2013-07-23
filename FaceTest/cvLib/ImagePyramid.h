

#ifndef ImagePyramid_h
#define ImagePyramid_h

#include "imageframe.h"
class ImageFrame;


class ImagePyramid
{
public:
        ImagePyramid();
        //ImagePyramid(const ImagePyramid& impyramid);
        ~ImagePyramid();

// Operators
        //const ImagePyramid& operator=(const ImagePyramid& impyramid);

// Operations
        void init(unsigned int image_width, unsigned int image_height,
                  const float* scales = 0, unsigned int nscales = 0);

// Access
        inline ImageFrame* get_frame(unsigned int i) const;
        
// Inquiry
        inline unsigned int get_frames_number() const;

protected:
private:
        ImagePyramid(const ImagePyramid& impyramid);
        const ImagePyramid& operator=(const ImagePyramid& impyramid);

        vector<ImageFrame *> m_pyramid;

        void clear();

};

// Inlines
inline ImageFrame* ImagePyramid::get_frame(unsigned int i) const
{
        return m_pyramid[i];
}

inline unsigned int ImagePyramid::get_frames_number() const
{
        return (unsigned int)m_pyramid.size();
}


#endif ImagePyramid_h

