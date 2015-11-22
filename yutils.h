//-------------------------------------------------------------------------
// File: utils.h
// utilities for debuging, printing, strings and some classes
//-------------------------------------------------------------------------

#ifndef YUTILS_H_
#define YUTILS_H_

#include <iostream>
#include <cassert>
#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

//-------------------------------------------------------------------------
class Rand {
  public:
    enum {NTAB=32, INT_SIZE = NTAB+3};
    struct State {
        int idum1;
        int idum2;
        int iy;
        int iv[NTAB];
        void print();
    };

    Rand(int seed = 17) { init(seed); }
    Rand(const Rand::State& state);
    void init(int seed);

    double nextDouble();
    int nextInt(int max) { return ((int)(max*nextDouble()))%max; }

    void save(Rand::State& state);
    void save(int* stream);
    
    void init(int* stream);
  private:
    int idum1;
    int idum2;
    int iy;
    int iv[NTAB];
};

//-------------------------------------------------------------------------

#endif
