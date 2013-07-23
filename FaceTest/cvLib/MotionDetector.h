

#ifndef MotionDetector_h
#define MotionDetector_h


class vec2D;
class vec2Dc;
class FaceDetector;


class MotionDetector
{
public:
        MotionDetector();
        //MotionDetector(const MotionDetector& md);
        ~MotionDetector();

// Operators
        //const MotionDetector& operator=(const MotionDetector& md);
        void init(unsigned int image_width, unsigned int image_height);
        void close();

        const vec2Dc* detect(const vec2D* frame, const FaceDetector* fdetect);        //frame 0...255
        void clear_last_frame();

// Operations
// Access
        inline int status() const;
        inline const vec2Dc* get_motion_vector() const;
        inline float threshold() const;
        inline void threshold(float th);


// Inquiry

protected:
private:
        MotionDetector(const MotionDetector& md);
        const MotionDetector& operator=(const MotionDetector& md);

        int m_status;
        vec2D* m_last_frame;
        vec2Dc* m_motion_vector;
        float m_TH;

};

// Inlines
inline int MotionDetector::status() const
{
        return m_status;
}

inline const vec2Dc* MotionDetector::get_motion_vector() const
{
        return m_motion_vector;
}

inline float MotionDetector::threshold() const
{
        return m_TH;
}

inline void MotionDetector::threshold(float th)
{
        if (th < 0.0f)
                th = 0.0f;
        if (th > 255.0f)
                th = 255.0f;
        m_TH = th;
}

#endif MotionDetector_h

