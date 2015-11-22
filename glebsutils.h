//-------------------------------------------------------------------------
// File: utils.h
// utilities for debuging, printing, strings and some classes
//-------------------------------------------------------------------------

#ifndef GLEBSUTILS_H_
#define GLEBSUTILS_H_

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


//--------------------------------------------------------
#define DEBUG(NAME) cerr << "DEBUG: " << #NAME << ": " << NAME << endl;

#define DEBUG_MPI(NAME) \
cerr << "R:" << Rank << ", " << #NAME << ": " << NAME << endl

//-------------------------------------------------------------------------
#define ABORT(ERR_MSG) {\
    cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;   \
    cerr << "ABORT : " << endl; \
    cerr << "ERROR : " << (ERR_MSG) << endl; \
    cerr << "IN    : " << __FILE__ << " (" << __LINE__ << ")" << endl;\
    exit(1);\
}

//-------------------------------------------------------------------------
#define ASSERT(COND, ERR_MSG) \
if(!(COND)) {  \
    cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;   \
    cerr << "FAILED : " << (#COND) << endl; \
    cerr << "ERROR  : " << (ERR_MSG) << endl; \
    cerr << "IN     : " << __FILE__ << " (" << __LINE__ << ")" << endl; \
    exit(1);\
}

#define COUT cout << "# "

#define INPUT(PROMPT, VAR) \
COUT << PROMPT; \
cin  >> VAR; \
if(!isatty(0)) cout << VAR << endl
//if(!isKeyboard()) cout << VAR << endl

#define SQR(VAL) ((VAL)*(VAL))

//-------------------------------------------------------------------------
#define PRINT_ARRAY(ARR, CNT)\
    cout << #ARR << ", cnt=" << #CNT << "\n";\
    cout << std::scientific;\
    cout << std::setprecision(2);\
    for(int i=0; i<CNT; i++) cout << ARR[i] << endl;\
    cout << endl

//-------------------------------------------------------------------------
#define PRINT_ARRAY_F(ARR, FIELD, CNT)\
    cout << #ARR << "[i]." << #FIELD <<", cnt=" << #CNT << "\n";\
    cout << std::scientific;\
    cout << std::setprecision(2);\
    for(int i=0; i<CNT; i++) cout << ARR[i].FIELD << "\t";\
    cout << endl

//-------------------------------------------------------------------------
class ErrorMessages {
  public:
    static const string FileBad(string name) {
        return "FILE: Failed to open: " + name;
    }
};

//-------------------------------------------------------------------------
void** new2D(int xSize, int ySize, int elemSize);

// some string utils
string toString(int i);
string toString(unsigned i);
string toString(double v);
int    toInt(string);
double toDouble(string);


string trim(string);
void breakString(string what, char sep, string& first, string& second);

std::ostream& stop(std::ostream&);
bool isKeyboard();
void print_trace ();

//----------------------------------------------------------------------
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
//-------------------------------------------------------------------------
class Stack {
  public:
    Stack(int size_) : ptr(0), size(size_) {
        data = new int[size];
        ASSERT(data != NULL, "No space");
    }
    void push(int elem) { ASSERT(ptr < size, "overrun"); data[ptr++] = elem; }
    int pop()           { ASSERT(ptr > 0, "overrun"); return data[--ptr]; }
    bool hasMore()      { return ptr > 0; }
    int itemCnt()       { return ptr; }
    ~Stack()            { delete[] data; }

  private:
    int* data;
    int ptr;
    int size;
};

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
class VarStat {
  public:
    VarStat() {
        totalCnt   = 0;
        runningCnt = 0;
        totalSum   = 0;
        runningSum = 0;
    }

    void add(double val) {
        totalCnt   ++;
        runningCnt ++;
        totalSum   += val;
        runningSum += val;
    }

    // resets running avr
    double getRunningAvr() {
        double ret = 0.0;
        if(runningCnt > 0) ret = runningSum/runningCnt;
        runningSum = 0.0;
        runningCnt = 0;
        return ret;
    }

    double getGlobalAvr() {
        double ret = 0.0;
        if(totalCnt > 0) ret = totalSum/totalCnt;
        return ret;
    }
                                   
    unsigned getTotalCnt() { return totalCnt; }

  private:
    unsigned totalCnt;
    unsigned runningCnt;
    double   totalSum;
    double   runningSum;
};

//-------------------------------------------------------------------------
// class to handle parameters in the form:
// vc.sim.year.end   = 1224000.0 # this is the end year of simulation
// key  = value # comments
//-------------------------------------------------------------------------
class Properties {
    class Entry {
    public:
	string key;
	string value;
	string comment;
    };

public:
    static       string MAGIC_EOP;
    static const int    MAX_KEYS  = 50;

    Properties() : entryCnt(0), hasEOP(true), keyMaxLen(0), valMaxLen(0) {}

    // replaces the existing key, if there is one
    void   add(string key, string val, string comm="");

    bool   exists(const string& key) { unsigned tmp; return exists(key, tmp); }
    bool   exists(const string& key, unsigned& ind); //=0);
    unsigned getCnt() { return entryCnt; }

    bool   getBoolValue(const string& key);
    string getStringValue(const string& key);
    double getDoubleValue(const string& key);
    int    getIntValue(const string& key);
    unsigned getUnsignedValue(const string& key);

    string getComment(const string& key);

    // for output
    void   format(string prefix_, bool eop=true) {
        prefix = prefix_;
        hasEOP = eop;
    }

    friend std::ostream& operator<<(std::ostream& os, const Properties& p);
    friend std::istream& operator>>(std::istream& is, Properties& p);

protected:
    // try to parse, false if fails
    bool   parse(string line);

    unsigned entryCnt;
    Entry entries[MAX_KEYS];

    // these are for the nice output
    bool     hasEOP;
    string   prefix;
    unsigned keyMaxLen;
    unsigned valMaxLen;
    string file;
};

//-------------------------------------------------------------------------
#endif
