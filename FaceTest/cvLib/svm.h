
#ifndef SVMachine_H
#define SVMachine_H


class vec2D;


#define TYPESIZE 256

class SVMachine
{
public:
        SVMachine(const wchar_t* fname);
        //SVMachine(const SVMachine& svm);
        ~SVMachine();

        enum SVMTYPE { LINEAR, RBF, POLY };

// Operators
        //const SVMachine& operator=(const SVMachine& svm);

// Operations
        int classify(const float* x, double& y) const;
        int classify(const vec2D& x, double& y) const;  //row vector (MMX wise)
        
// Access
        inline unsigned int dimension() const;                 //support vector dimensionality
        
// Inquiry
        inline int status() const;                             //0-OK, -1,-2,-3,... errs

protected:               
private:
        SVMachine(const SVMachine& svm);
        const SVMachine& operator=(const SVMachine& svm);

        int m_status;                   //1-OK, -1 file err

        unsigned int m_dimension;       //sv dimensionality
        unsigned int m_svsNum;          //sv's number
        enum SVMTYPE m_svmType;         //lin,rbf,poly

        double m_svmParam;              //ploynomial - d,  (x*sv + 1) .^ d
                                        //rbf - gamma,  exp(-gamma * norm(s-sv))

        vector<double> m_weights;     //col vector w = alpha * y
        vector<vec2D*> m_svs;         //support vectors, row vectors

        double m_bias;                //bias

        vec2D* m_scalar;          //temp scalar vector 1x1
        vec2D* m_xInput;          //temp x input vector 1xdim
        vec2D* m_rbfSub;          //rbf sub operation

        inline int sign(double x) const;

};


/*
   SVMachine svm(L"file.svm");
    y = svm.classify(x);    //1 or -1



   file format
    dimesion
    sv's num
    type (linear,rbf,...)

    bias

    w1
    sv1      //col vector

    w2
    sv2      //col vector

    ...



    f(x) = sign( sum(w(i)*kern(x,sv(i)) ) + bias )
*/


inline int SVMachine::status() const
{
        return m_status;
}

inline unsigned int SVMachine::dimension() const
{
        return m_dimension;
}

inline int SVMachine::sign(double x) const
{
        return (x >= 0.0) ? 1 : -1;
}


#endif