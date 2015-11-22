//-------------------------------------------------------------------------
// File: utils.h
// Implementation of utilities for debuging, printing,
// strings, and some classes
//-------------------------------------------------------------------------

#include <cmath>
#include <cassert>
#include <iomanip>

#include "yutils.h"

using namespace std;

//----------------------------------------------------------------------
#define IM1 2147483563
#define IM2 2147483399

#define IA1 40014
#define IA2 40692

#define IQ1 53668
#define IQ2 52774

#define IR1 12211
#define IR2 3791

#define IMM1 (IM1-1)
#define NDIV (1+IMM1/NTAB)
#define NORM 1./IM1

void Rand::State::print() {
    cout << idum1 << endl << idum2 << endl << iy << endl;
    for(int i=0; i<Rand::NTAB; i++) cout << iv[i] << " ";
    cout << endl;
}

Rand::Rand(const Rand::State& state) {
    idum1 = state.idum1;
    idum2 = state.idum2;
    iy    = state.iy;
    for(int i=0; i<NTAB; i++) iv[i] = state.iv[i];
}

void Rand::save(Rand::State& state) {
    state.idum1 = idum1;
    state.idum2 = idum2;
    state.iy    = iy;
    for(int i=0; i<NTAB; i++) state.iv[i] = iv[i];
}

void Rand::init(int seed) {
    idum2 = idum1 = (seed < 1 ? 1 : seed);
    for (int j=NTAB+7; j>=0; j--) {
        int k = idum1/IQ1;
        idum1  = IA1*(idum1 - k*IQ1) - k*IR1;
        if(idum1 < 0) idum1 += IM1;
        if(j < NTAB) iv[j] = idum1;
    }
    iy=iv[0];
}

void Rand::save(int* state) {
    state[0] = idum1;
    state[1] = idum2;
    state[2] = iy;

    int shift = 3;
    for(int i=0; i<Rand::NTAB; i++) state[shift+i] = iv[i];
}

void Rand::init(int* state) {
    idum1 = state[0];
    idum2 = state[1];
    iy    = state[2];
    
    int shift = 3;
    for(int i=0; i<Rand::NTAB; i++) iv[i] = state[shift+i];
}

double Rand::nextDouble() {
    int k = idum1/IQ1;
    idum1  = IA1*(idum1 - k*IQ1) - k*IR1;
    if(idum1 < 0) idum1 += IM1;

    k = idum2/IQ2;
    idum2  = IA2*(idum2 - k*IQ2) - k*IR2;
    if(idum2 < 0) idum2 += IM2;

    int tmp = iy/NDIV;
    iy = iv[tmp] - idum2;
    iv[tmp] = idum1;

    if(iy < 1) iy += IMM1;

    if(iy > IMM1) return 0.99999999;
    else          return iy*NORM;
}

#undef IM1
#undef IM2
#undef IA1
#undef IA2
#undef IQ1
#undef IQ2
#undef IR1
#undef IR2
#undef IMM1
#undef NDIV
#undef NORM
