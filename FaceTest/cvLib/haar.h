

#ifndef HAAR_H
#define HAAR_H


class BaseFWT2D;

class Haar : public BaseFWT2D    //generic 2D fwt
{
public:
        Haar();

private:
        static const float tH[4];
        static const float tG[4];

        inline void addsub(__m64 even, __m64 odd,               //addsub for transform
                           __m64 *lo, __m64 *hi) const;        

        void transrows(char** dest, char** sour, unsigned int w, unsigned int h) const;
        void transcols(char** dest, char** sour, unsigned int w, unsigned int h) const;
        void synthrows(char** dest, char** sour, unsigned int w, unsigned int h) const;
        void synthcols(char** dest, char** sour, unsigned int w, unsigned int h) const;


};



/*
  usage:
   Haar fwt;
    fwt.init(width,height);    //init buffers
     fwt.trans(data,3,30);     //fwt transform from data buffer J=3,TH=30, data is intact after fwt
     fwt.synth(data);          //fwt synthesis to data buffer
     ...
     ...  //keep going multiple times transforming and synthesing
     ...
    fwt.close();


*/





inline void Haar::addsub(__m64 even, __m64 odd, __m64 *lo, __m64 *hi) const      //even,odd - chars,  lo,hi - chars
{
        char *clo = (char *)lo;
        char *chi = (char *)hi;

        __m64 ma, mb, m7F, m01, m128;
        m01.m64_u64 = 0x0101010101010101;
        m7F.m64_u64 = 0x7F7F7F7F7F7F7F7F;
        m128.m64_u64 = 0x8080808080808080;

        //char to uchar
        even = _mm_add_pi8(even, m128);
        odd = _mm_add_pi8(odd, m128);
        //char to uchar

        ma = _mm_and_si64(even, m01);
        mb = _mm_and_si64(odd, m01);
        //m01 = _mm_or_si64( ma, mb );            // x.5 = x+1
        m01 = _mm_and_si64(ma, mb);               // x.5 = x

        even = _mm_srli_si64(even, 1);            // add/2
        even = _mm_and_si64(even, m7F);
        odd = _mm_srli_si64(odd, 1);              // sub/2
        odd = _mm_and_si64(odd, m7F);

        ma = _mm_adds_pu8(even, odd);
        ma = _mm_adds_pu8(ma, m01);
        mb = _mm_subs_pi8(even, odd);

        //uchar to char
        *lo = _mm_sub_pi8(ma, m128);
        *hi = mb;

        _mm_empty();                
}

#endif