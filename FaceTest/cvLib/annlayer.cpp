

#include "stdafx.h"
#include "annlayer.h"
#include "vec2D.h"




//////////////////////////////////////////////////constructor/destructor////////////////////////////////////////////////////////
AnnLayer::AnnLayer(unsigned int n, unsigned int m)
{
        m_tW = new vec2D(m, n);         //W = W' NxM  for mmx optimized mult
        m_inputs = new vec2D(1, n);
        m_outputs = new vec2D(1, m);

        (*m_inputs)[0] = 1.0f;              //bias input multiplier
}
AnnLayer::~AnnLayer()
{
        delete m_tW;
        delete m_inputs;
        delete m_outputs;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void AnnLayer::run(int fun, AnnLayer* pnext)
{
        const vec2D& tW = *m_tW;
        const vec2D& inputs = *m_inputs;
        vec2D& outputs = *m_outputs;

        //Xout = Xin * W'
        outputs.mult(inputs, tW);
        //outputs.print();

        if (pnext != 0)
                pnext->set_input(outputs, fun);
        //pnext->get_input().print();
}

void AnnLayer::set_input(const vec2D& x, int fun)
{
        vec2D& inputs = *m_inputs;
        switch (fun) {
        case LINEAR:
                for (unsigned int i = 0; i < x.length(); i++)
                        inputs[i+1] = x[i];
                break;

        default:
        case SIGMOID:
                for (unsigned int i = 0; i < x.length(); i++)
                        inputs[i+1] = 1.0f / (1.0f + exp(-x[i]));
                break;

        case TANH:
                for (unsigned int i = 0; i < x.length(); i++)
                        inputs[i+1] = tanh(x[i]);
                break;
        }
}

void AnnLayer::get_output(float* ovec, int fun) const
{
        const vec2D& outputs = *m_outputs;
        switch (fun) {
        case LINEAR:
                for (unsigned int i = 0; i < outputs.length(); i++)
                        ovec[i] = outputs[i];
                break;

        default:
        case SIGMOID:
                for (unsigned int i = 0; i < outputs.length(); i++)
                        ovec[i] = 1.0f / (1.0f + exp(-outputs[i]));
                break;

        case TANH:
                for (unsigned int i = 0; i < outputs.length(); i++)
                        ovec[i] = tanh(outputs[i]);
                break;
        }
}

