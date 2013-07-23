

#include "stdafx.h"
#include "svm.h"

#include "vec2D.h"




/////////////////////////constructors/destructors////////////////////////////////////////////////////
SVMachine::SVMachine(const wchar_t* fname): m_status(-1),
                m_svmParam(0), m_bias(0), m_dimension(0), m_svsNum(0),
                m_scalar(0), m_xInput(0), m_rbfSub(0)
{
        wchar_t type[TYPESIZE];
        int res = 0;

        FILE* fp = _wfopen(fname, L"rt");
        if (fp) {
                if ((res = fwscanf(fp, L"%d %d %256s", &m_dimension, &m_svsNum, type)) != 3) {
                        m_status = -2;
                        fclose(fp);
                        return;
                }

                if (wcscmp(type, L"linear") == 0) {
                        m_svmType = LINEAR;
                } else if (wcscmp(type, L"rbf") == 0) {
                        m_svmType = RBF;

                        if ((res = fwscanf(fp, L"%lg", &m_svmParam)) != 1) {
                                m_status = -3;
                                fclose(fp);
                                return;
                        }
                } else if (wcscmp(type, L"polynomial") == 0) {
                        m_svmType = POLY;

                        if ((res = fwscanf(fp, L"%lg", &m_svmParam)) != 1) {
                                m_status = -3;
                                fclose(fp);
                                return;
                        }
                }
                else {
                        m_status = -4;
                        return;
                }

                if ((res = fwscanf(fp, L"%lg", &m_bias)) != 1) {
                        m_status = -5;
                        fclose(fp);
                        return;
                }

                m_weights.resize(m_svsNum);

                //read support vectors
                for (unsigned int s = 0; s < m_svsNum; s++) {
                        //read w
                        if ((res = fwscanf(fp, L"%lg", &m_weights[s])) != 1) {
                                m_status = -6;
                                fclose(fp);
                                return;
                        }

                        class vec2D* sv = new vec2D(1, m_dimension);
                        m_svs.push_back(sv);
                        for (unsigned int i = 0; i < m_dimension; i++) {
                                if ((res = fwscanf(fp, L"%g", &(*sv)(0, i))) != 1) {
                                        m_status = -7;
                                        fclose(fp);
                                        return;
                                }
                        }
                }

                m_scalar = new vec2D(1, 1);
                m_xInput = new vec2D(1, m_dimension);
                m_rbfSub = new vec2D(1, m_dimension);

                m_status = 0;
                fclose(fp);
        }
}
SVMachine::~SVMachine()
{
        if (m_svs.size()) {
                for (unsigned int i = 0; i < m_svs.size(); i++)
                        delete m_svs[i];
        }

        if (m_scalar)
                delete m_scalar;
        if (m_xInput)
                delete m_xInput;
        if (m_rbfSub)
                delete m_rbfSub;
}
/////////////////////////constructors/destructors////////////////////////////////////////////////////












///////////////////////////////////////classify///////////////////////////////////////////////////////////
int SVMachine::classify(const float* x, double& y) const
{
        double sum = 0.0;
        const double* weights = &m_weights[0];

        for (unsigned int i = 0; i < m_xInput->width(); i++)
                (*m_xInput)(0, i) = x[i];

        switch (m_svmType) {

        case LINEAR:
                for (unsigned int i = 0; i < m_svsNum; i++) {
                        m_scalar->mult(*m_xInput, *m_svs[i]);
                        sum += weights[i] * (double)(*m_scalar)(0, 0);
                }
                y = sum + m_bias;
                return sign(y);

        case RBF:
                for (unsigned int i = 0; i < m_svsNum; i++) {
                        m_rbfSub->sub(*m_xInput, *m_svs[i]);
                        m_scalar->mult(*m_rbfSub, *m_rbfSub);
                        sum += weights[i] * exp(-m_svmParam * (double)(*m_scalar)(0, 0));
                }
                y = sum + m_bias;
                return sign(y);

        case POLY:
                for (unsigned int i = 0; i < m_svsNum; i++) {
                        m_scalar->mult(*m_xInput, *m_svs[i]);
                        sum += weights[i] * pow((double)((*m_scalar)(0, 0) + 1.0f), m_svmParam);
                }
                y = sum + m_bias;
                return sign(y);

        default:
                y = 0;
                return 0;

        }
}

int SVMachine::classify(const vec2D& x, double& y) const
{
        double sum = 0.0;
        const double* weights = &m_weights[0];

        for (unsigned int i = 0; i < m_xInput->width(); i++)
                (*m_xInput)(0, i) = x(0, i);

        switch (m_svmType) {

        case LINEAR:
                for (unsigned int i = 0; i < m_svsNum; i++) {
                        m_scalar->mult(*m_xInput, *m_svs[i]);
                        sum += weights[i] * (double)(*m_scalar)(0, 0);
                }
                y = sum + m_bias;
                return sign(y);

        case RBF:
                for (unsigned int i = 0; i < m_svsNum; i++) {
                        m_rbfSub->sub(*m_xInput, *m_svs[i]);
                        m_scalar->mult(*m_rbfSub, *m_rbfSub);
                        sum += weights[i] * exp(-m_svmParam * (double)(*m_scalar)(0, 0));
                }
                y = sum + m_bias;
                return sign(y);

        case POLY:
                for (unsigned int i = 0; i < m_svsNum; i++) {
                        m_scalar->mult(*m_xInput, *m_svs[i]);
                        sum += weights[i] * pow((double)((*m_scalar)(0, 0) + 1.0f), m_svmParam);
                }
                y = sum + m_bias;
                return sign(y);

        default:
                y = 0;
                return 0;

        }
}


///////////////////////////////////////classify///////////////////////////////////////////////////////////
