

#include "stdafx.h"
#include "annetwork.h"
#include "annlayer.h"
#include "vec2d.h"



//////////////////////////////////////////////////constructor/destructor////////////////////////////////////////////////////////
ANNetwork::ANNetwork(const wchar_t* fname) : m_status(-1),
                m_nrule(0.2f), m_alpha(0.7f),
                m_input_vec(0), m_add_vec(0), m_mul_vec(0)
{
        int res = 0;
        unsigned int neurons_num = 0;
        float w = 0.0f;
        unsigned int layers_num = 0;

        FILE* fp = _wfopen(fname, L"rt");
        if (fp != 0) {

                //get num of layers////////////////////////////////////
                if ((res = fwscanf(fp, L"%d", &layers_num)) != 1) {
                        fclose(fp);
                        m_status = -1;
                        return;
                }

                m_neurons.resize(layers_num);

                //get num of Neurons per layer/////////////////////////
                for (unsigned int l = 0; l < layers_num; l++) {
                        if ((res = fwscanf(fp, L"%d", &neurons_num)) != 1) {
                                fclose(fp);
                                m_status = -2;
                                return;
                        } else
                                m_neurons[l] = neurons_num;
                }
                for (unsigned int l = 0; l < layers_num - 1; l++)
                        m_layers.push_back(new AnnLayer(m_neurons[l] + 1, m_neurons[l+1]));


                //activation function for hidden/output layers/////////////////////
                if ((res = fwscanf(fp, L"%d %d", &m_actfun[0], &m_actfun[1])) != 2) {
                        m_actfun[0] = AnnLayer::LINEAR;
                        m_actfun[1] = AnnLayer::SIGMOID;
                }


                //normalization params////////////////////////////////////////////
                m_add_vec = new vec2D(1, m_neurons[0]);
                m_mul_vec = new vec2D(1, m_neurons[0]);
                m_input_vec = new vec2D(1, m_neurons[0]);

                for (unsigned int n = 0; n < m_neurons[0]; n++) {
                        float add, mul;
                        if ((res = fwscanf(fp, L"%f %f", &add, &mul)) != 2) {       //blank network file?
                                for (unsigned int m = 0; m < m_neurons[0]; m++) {
                                        (*m_add_vec)[m] = 0.0f;   //default add = 0
                                        (*m_mul_vec)[m] = 1.0f;   //default mult = 1
                                }
                                break;
                        }
                        (*m_add_vec)[n] = add;
                        (*m_mul_vec)[n] = mul;
                }


                //load weights/////////////////////////////////////////////////////
                for (unsigned int l = 0; l < m_layers.size(); l++) {         //load all weights except input layer
                        vec2D& tW = *m_layers[l]->m_tW;
                        for (unsigned int n = 0; n < tW.height(); n++) {       //num of Neurons in layer
                                for (unsigned int i = 0; i < tW.width(); i++) {      //num of inputs in Neuron
                                        if ((res = fwscanf(fp, L"%f", &w)) != 1) {        //blank network file?
                                                fclose(fp);
                                                m_status = 1;
                                                init_weights((unsigned int)time(0));   //init to random values
                                                return;
                                        } else {
                                                tW(n, i) = w;                                                
                                        }
                                }
                        }
                }

                fclose(fp);
                m_status = 0;
        } else
                m_status = -1;
}

ANNetwork::~ANNetwork()
{
        if (m_input_vec)
                delete m_input_vec;
        if (m_add_vec)
                delete m_add_vec;
        if (m_mul_vec)
                delete m_mul_vec;

        for (unsigned int l = 0; l < m_layers.size(); l++)        //delete layers
                delete m_layers[l];
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////init neuron weights///////////////////////////////////////////////////////
void ANNetwork::init_weights(unsigned int rseed) const
{
        int w;
        srand(rseed);

        //input layer remains with w=1.0
        for (unsigned int l = 0; l < m_layers.size(); l++) {
                vec2D& tW = *m_layers[l]->m_tW;
                for (unsigned int n = 0; n < tW.height(); n++) {
                        for (unsigned int i = 0; i < tW.width(); i++) {
                                w = 0xFFF & rand();
                                w -= 0x800;
                                tW(n, i) = (float)w / 2048.0f;                                
                        }
                }
        }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////run network////////////////////////////////////////////////////////////////
void ANNetwork::classify(const float* ivec, float* ovec) const
{
        //normalize input Xout = (Xin + add) * mul
        *m_input_vec = ivec;
        m_input_vec->add(*m_input_vec, *m_add_vec);                 //input = input .+ add
        m_input_vec->mule(*m_input_vec, *m_mul_vec);                //input = input .* mul
        m_layers[0]->set_input(*m_input_vec, m_actfun[0]);          //copy m_input_vec to layer[0]->inputs[1:size-1],  0-bias

        //m_layers[0]->get_input().print();
        //hidden layers
        for (unsigned int l = 0; l < m_layers.size(); l++) {
                AnnLayer* pnext = 0;
                if (l + 1 < m_layers.size())
                        pnext = m_layers[l+1];
                m_layers[l]->run(m_actfun[1], pnext);
        }

        //output layer
        m_layers[m_layers.size()-1]->get_output(ovec, m_actfun[1]);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ANNetwork::saveWs(bool file) const
{
        FILE* fp;
        wchar_t fname[256] = L"";

        for (unsigned int i = 0; i < m_layers.size(); i++) {
                swprintf(fname, L"W%d.txt", i + 1);
                if (file && (fp = _wfopen(fname, L"wt"))) {
                        const vec2D &tW = *m_layers[i]->m_tW;
                        for (unsigned int x = 0; x < tW.width(); x++) {
                                for (unsigned int y = 0; y < tW.height(); y++)
                                        fwprintf(fp, L" %f", tW(y, x));
                                fwprintf(fp, L"\n");
                        }
                        fclose(fp);
                } else {
                        const vec2D& tW = *m_layers[i]->m_tW;
                        wprintf(L"W%d %dx%d\n", i + 1, tW.width(), tW.height());
                        for (unsigned int x = 0; x < tW.width(); x++) {
                                for (unsigned int y = 0; y < tW.height(); y++)
                                        wprintf(L" %8.4f", tW(y, x));
                                wprintf(L"\n");
                        }
                }
        }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

