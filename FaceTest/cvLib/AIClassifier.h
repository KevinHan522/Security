

#ifndef AIClassifier_h
#define AIClassifier_h


#include "annlayer.h"
#include "annetwork.h"
#include "svm.h"
class ANNetwork;
class SVMachine;


class AIClassifier
{
public:
        AIClassifier(const wchar_t* fname);
        //AIClassifier(const AIClassifier& classifier);
        ~AIClassifier();

// Operators
        //const AIClassifier& operator=(const AIClassifier& classifier);

// Operations
        inline int classify(const float* x, float* y);

// Access        
// Inquiry
        inline int status() const;
        inline unsigned int input_dimension() const;

protected:
private:
        AIClassifier(const AIClassifier& classifier);
        const AIClassifier& operator=(const AIClassifier& classifier);

        int m_status;

        ANNetwork* m_ann;
        SVMachine* m_svm;  

        inline int sign(float x) const;
};

// Inlines
inline int AIClassifier::status() const
{
        return m_status;
}

inline unsigned int AIClassifier::input_dimension() const
{
        if (m_svm != 0 && m_svm->status() == 0)
                return m_svm->dimension();
        else if(m_ann != 0 && m_ann->status() == 0)
                return m_ann->dimension();
        else
                return 0;
}

inline int AIClassifier::sign(float x) const
{
        return (x >= 0.0f) ? 1 : -1;
}

inline int AIClassifier::classify(const float* x, float* y)
{
        double dy;
        int s = 0;        
        
        if (m_ann != 0 && m_ann->status() == 0) {
                m_ann->classify(x, y);
                if (m_ann->activation_function() == AnnLayer::SIGMOID)
                        s = sign(y[0] - 0.5f);
                else
                        s = sign(y[0]);
        }
        else if (m_svm != 0 && m_svm->status() == 0) {
                s = m_svm->classify(x, dy);
                y[0] = float(dy);
        }
        else {
                y[0] = 0.0f;                
        }

        return s;
}



#endif AIClassifier_h

