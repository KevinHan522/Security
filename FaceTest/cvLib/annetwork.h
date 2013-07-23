

#ifndef ANNetwork_h
#define ANNetwork_h

#include "vec2d.h"
class AnnLayer;
class vec2D;

class ANNetwork
{
public:
        ANNetwork(const wchar_t* fname);
        ~ANNetwork();

        inline int status() const;                              //get loading status
        inline int activation_function() const;                 //for last layer
        inline unsigned int dimension() const;                  //input vec dimension
        inline unsigned int output_size() const;
        void saveWs(bool file = false) const;                   //save W matrices

        void classify(const float* ivec, float* ovec) const;            //run network


private:
        int m_status;            //0 OK; -1 file not found; 1 random weights

        float m_nrule;           //learning rule 0.2
        float m_alpha;           //momentum  0.7

        int m_actfun[2];                      //activation function for input and output layers
        vector<unsigned int> m_neurons;       //neurons per layer
        vector<AnnLayer *> m_layers;          //layers

        vec2D* m_input_vec;                       //input[1,:] = (input[1,:]+add[1,:]) * mul[1,:]
        vec2D* m_add_vec;                         //normalization vectors
        vec2D* m_mul_vec;                         //normalization vectors


        void init_weights(unsigned int rseed) const;         //randomize weights


        //unintended functions
        ANNetwork(const ANNetwork& ann);
        const ANNetwork& operator=(const ANNetwork& ann);

};

inline int ANNetwork::status() const
{
        return m_status;
}

inline int ANNetwork::activation_function() const
{
        return m_actfun[1];
}

inline unsigned int ANNetwork::dimension() const
{
        if (m_neurons.size() > 0)
                return m_neurons[0];
        else
                return 0;
}

inline unsigned int ANNetwork::output_size() const
{
        if (m_neurons.size() > 0)
                return m_neurons[m_neurons.size()-1];
        else
                return 0;
}


#endif


/*

  file format
  3
  40 10 1
  [defaults]
  1                activation function 0-linear, 1-sigmoid

  [input norm]

  weights
   ...


 */