

#include "stdafx.h"
#include "imagepyramid.h"

#include "imageframe.h"


ImagePyramid::ImagePyramid()
{
}

ImagePyramid::~ImagePyramid()
{
        clear();
}

void ImagePyramid::init(unsigned int image_width, unsigned int image_height,
                        const float* scales, unsigned int nscales)
{
        clear();

        m_pyramid.push_back(new ImageFrame(image_width, image_height, 1.0f));
        for (unsigned int i = 0; i < nscales; i++)
                m_pyramid.push_back(new ImageFrame(image_width, image_height, scales[i]));
}

void ImagePyramid::clear()
{
        for (unsigned int i = 0; i < m_pyramid.size(); i++)
                delete m_pyramid[i];
        m_pyramid.clear();
}
