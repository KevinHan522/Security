

#include "stdafx.h"

#include "vec1d.h"
#include "basefwt.h"
#include "Haar.h"


//haar filter////////////////////////////////////////////////////////////////////////////////////////
//   0     1     0     1
const float Haar::tH[4] = { 0.5f, 0.5f, 0.5f, 0.5f };
//   0     1     0     1
const float Haar::tG[4] = { 0.5f, -0.5f, 0.5f, -0.5f };

//haar filter////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////constructors/destructors///////////////////////////////////////////////////////////////////
Haar::Haar() : BaseFWT2D(L"haar", tH, 4, 0, tG, 4, 0, tH, 2, 0, tG, 2, 0)
{
}
///////////////////////////////////constructors/destructors///////////////////////////////////////////////////////////////////



///////////////////////////////////////////////transforms/////////////////////////////////////////////////////////////////////
void Haar::transrows(char** dest, char** sour, unsigned int w, unsigned int h) const
{
        unsigned int w2 = w / 2;

        __m64 m00FF;
        m00FF.m64_u64 = 0x00FF00FF00FF00FF;

        for (unsigned int y = 0; y < h; y++) {

                __m64 *mlo = (__m64 *) & dest[y][0];
                __m64 *mhi = (__m64 *) & dest[y][w2];
                __m64 *msour = (__m64 *) & sour[y][0];

                for (unsigned int k = 0; k < w2 / 8; k++) {   //k<w2/8   k=8*k

                        __m64 even = _mm_packs_pu16(_mm_and_si64(*msour, m00FF), _mm_and_si64(*(msour + 1), m00FF));       //even coeffs
                        __m64 odd = _mm_packs_pu16(_mm_srli_pi16(*msour, 8), _mm_srli_pi16(*(msour + 1), 8));              //odd coeffs

                        addsub(even, odd, mlo++, mhi++);
                        msour += 2;
                }

                if (w2 % 8) {
                        for (unsigned int k = w2 - (w2 % 8); k < w2; k++) {
                                dest[y][k] = char(((int)sour[y][2*k] + (int)sour[y][2*k+1]) / 2);
                                dest[y][k+w2] = char(((int)sour[y][2*k] - (int)sour[y][2*k+1]) / 2);
                        }
                }
        }
        _mm_empty();
}
void Haar::transcols(char** dest, char** sour, unsigned int w, unsigned int h) const
{
        unsigned int h2 = h / 2;

        for (unsigned int k = 0; k < h2; k++) {

                __m64 *mlo = (__m64 *) & dest[k][0];
                __m64 *mhi = (__m64 *) & dest[k+h2][0];
                __m64 *even = (__m64 *) & sour[2*k][0];
                __m64 *odd = (__m64 *) & sour[2*k+1][0];

                for (unsigned int x = 0; x < w / 8; x++) {
                        
                        addsub(*even, *odd, mlo, mhi);                        

                        even++;
                        odd++;
                        mlo++;
                        mhi++;
                }
        }
        _mm_empty();

        //odd remainder
        for (unsigned int x = w - (w % 8); x < w; x++) {
                for (unsigned int k = 0; k < h2; k++) {                        
                        dest[k][x] = char(((int)sour[2*k][x] + (int)sour[2*k+1][x]) / 2);                        
                        dest[k+h2][x] = char(((int)sour[2*k][x] - (int)sour[2*k+1][x]) / 2);
                }
        }
}
///////////////////////////////////////////////transforms/////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////synths//////////////////////////////////////////////////////////////////////////
void Haar::synthrows(char** dest, char** sour, unsigned int w, unsigned int h) const      //w,h of the LO part
{
}
void Haar::synthcols(char** dest, char** sour, unsigned int w, unsigned int h) const     //w,h of the LO part
{
}
//////////////////////////////////////////////synths//////////////////////////////////////////////////////////////////////////