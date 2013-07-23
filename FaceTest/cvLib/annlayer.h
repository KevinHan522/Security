

#ifndef AnnLayer_h
#define AnnLayer_h


class vec2D;

class AnnLayer
{
        friend class ANNetwork;
public:

        AnnLayer(unsigned int n, unsigned int m);         //n - prev layer neuronsNumber, m - this layer neuronsNumber
        ~AnnLayer();

        enum ACTFUN { LINEAR, SIGMOID, TANH };
                        
        void set_input(const vec2D& x, int fun);                    //set this->inputs[1:size-1] = actfun( x );   inputs[0]-bias 1
        void get_output(float* ovec, int fun) const;                //ovec = actfun(this->outputs)

        void run(int fun, AnnLayer* pnext = 0);                       //outputs = inputs*W'
        //pnext->inputs = actfun( outputs )

private:

        vec2D* m_tW;                    //transposed weights matrix tW   W = tW'
        vec2D* m_inputs;                //inputs [1, x1,x2,x3, ... xN]
        vec2D* m_outputs;               //outputs [y1,y2,y3, ... yN] = inputs*W'

        //unintended functions
        AnnLayer(const AnnLayer& layer);
        const AnnLayer& operator=(const AnnLayer& layer);

};



#endif

/*
    W weights matrix


    N1  N2  N3    neurons

    w00 w01 w02   bias weights
    w10 w11 w12
    w20 w21 w22
    w30 w31 w32
    w40 w41 w42


    tW = W' for mmx multiplications with input vector
*/