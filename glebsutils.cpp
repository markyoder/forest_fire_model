//-------------------------------------------------------------------------
// File: utils.h
// Implementation of utilities for debuging, printing,
// strings, and some classes
//-------------------------------------------------------------------------

#include <cmath>
#include <cassert>
#include <iomanip>
#include <cstring>

#include "glebsutils.h"

using namespace std;

void** new2D(int xSize, int ySize, int elemSize) {
    int offset = ySize*sizeof(void*);
    int volume = xSize*ySize;
    char* data = NULL;
    try {
        data = new char[offset + volume*elemSize];
    }
    catch(std::bad_alloc) {
        throw "no space in new2D";
    }
    memset(data, 0, offset + volume*elemSize);
    void** ret = (void**) data;
    for(int y=0; y<ySize; y++)
        ret[y] = (void*) (data + offset + y*xSize*elemSize);
    return ret;
}

//-------------------------------------------------------------------------
std::string toString(int i) {
    std::ostringstream os;
    os << i;
    return os.str();
}

//-------------------------------------------------------------------------
std::string toString(unsigned u) {
    std::ostringstream os;
    os << u;
    return os.str();
}

//-------------------------------------------------------------------------
std::string toString(double d) {
    assert(finite(d));
    std::ostringstream os;
    os << std::setprecision(12);
    os << d;
    return os.str();
}

//-------------------------------------------------------------------------
int toInt(string val) {
    std::istringstream is(val);
    int ret = 0;
    is >> ret;
    return ret;
}

//-------------------------------------------------------------------------
double toDouble(string val) {
    std::istringstream is(val);
    double ret = 0;
    is >> ret;
    return ret;
}

//-------------------------------------------------------------------------
std::ostream& stop(std::ostream& os) {
    os << std::flush;
    cerr << endl << "STOP!" << endl;
    exit(1);
    return os;
}

//#include <unistd.h> // isatty
int isatty(int);
//-------------------------------------------------------------------------
bool isKeyboard() { return isatty(0); }



/*
#include <execinfo.h>
#include <cstdio>

// Obtain a backtrace and print it to `stdout'
void print_trace () {
    void *array[10];
    size_t size;
    char **strings;
    size_t i;

    size = backtrace(array, 10);
    strings = backtrace_symbols(array, size);

    printf ("Obtained %zd stack frames.\n", size);

    for (i = 0; i < size; i++)
	printf("%s\n", strings[i]);

    free(strings);
}
*/

//-------------------------------------------------------------------------
string trim(string str) {
/*
  void trim( string& s ) {
  static const char whitespace[] = " \n\t\v\r\f";
  s.erase( 0, s.find_first_not_of(whitespace) );
  s.erase( s.find_last_not_of(whitespace) + 1U );
*/

    static const char whites[] = " \n\t";
    string tmp = "";
    if(str.empty()) return tmp;

    string ret(str);
    int startInd = str.find_first_not_of(whites);
    int indLast  = str.find_last_not_of(whites);
    
    return ret.substr(startInd, (indLast-startInd+1));
}

//-------------------------------------------------------------------------
// if sep is not found, in first returns what
void breakString(string what, char sep, string& first, string& second) {
    string src(trim(what));
    first = what;
    second = "";
    if(src.size() == 1) return;

    int ind = src.find(sep);
    if(ind < 0) return; // no separator
    if(ind == 0) {      // src[0] == sep
        first = "";
        // belg: change return to src[1 ... size-1]
        return;
    }

    first  = trim(src.substr(0, ind));
    if(src.size() > unsigned(ind + 1))
        second = trim(src.substr(ind+1));

    return;
}
    
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

//-------------------------------------------------------------------------
// Properties implementation
//-------------------------------------------------------------------------
string Properties::MAGIC_EOP = "#$EOP";

bool Properties::parse(string line) {
    string src(trim(line));

    string key;
    string tail;
    string value = "";
    string comment = "";
    
    breakString(src, '=', key, tail);
//     if(key.empty() || tail.empty()) return false;
//     breakString(tail, '#', value, comment);
//     if(value.empty()) return false; // tail starts with #
//     add(key, value, comment);

    if(key.empty()) return false;
    if(tail.empty()) { add(key, value, comment); return true; }

    breakString(tail, '#', value, comment);
    //if(value.empty()) return false; // tail starts with #

    add(key, value, comment);
    return true;
}

void Properties::add(string key, string val, string comm) {
    unsigned ind = 0;
    if(exists(key, ind) == false) {
        ASSERT(entryCnt < MAX_KEYS - 2, "Too many entries");
        ind = entryCnt;
    }
    if(key.size() > keyMaxLen) keyMaxLen = key.size();
    if(val.size() > valMaxLen) valMaxLen = val.size();

    entries[ind].key     = key;
    entries[ind].value   = val;
    entries[ind].comment = comm;
    
    if(ind == entryCnt) entryCnt++;
}

bool Properties::exists(const string& key, unsigned& ind) {
    for(ind=0; ind<entryCnt; ind++) {
	if(key == entries[ind].key) return true;
    }
    return false;
}

string Properties::getStringValue(const string& key) {
    unsigned ind = 0;
    string ret = "";
    if(exists(key, ind)) ret = entries[ind].value;
    else ABORT("key not found: " + key);
    return ret;
}

double Properties::getDoubleValue(const string& key) {
    return toDouble(getStringValue(key));
}

bool Properties::getBoolValue(const string& key) {
    return "y" == getStringValue(key);
}

int  Properties::getIntValue(const string& key) {
    return toInt(getStringValue(key));
}
unsigned Properties::getUnsignedValue(const string& key) {
    return unsigned(toInt(getStringValue(key)));
}

string Properties::getComment(const string& key) {
    unsigned ind = 0;
    string ret = "";
    if(exists(key, ind)) ret = entries[ind].comment;
    return ret;
}

ostream& operator<<(ostream& os, const Properties& p) {

    for(unsigned i=0; i<p.entryCnt; i++) {
        string key(35, ' ');
        //string key(p.keyMaxLen, ' ');
        string val(p.valMaxLen, ' ');
        key.replace(0, p.entries[i].key.size(), p.entries[i].key);
        val.replace(0, p.entries[i].value.size(), p.entries[i].value);
        os << p.prefix << key << " = " << val;
        if(!p.entries[i].comment.empty()) 
            os << " # " << p.entries[i].comment;
        os << endl;
    }
    if(p.hasEOP) os << p.MAGIC_EOP << endl;

    return os;
}

istream& operator>>(istream& is, Properties& p) {
    while(is != 0) {
        string line;
	getline(is, line);
        line = trim(line);
        if(line.size() == 0) continue;
        if(line.substr(0, p.MAGIC_EOP.size()) == p.MAGIC_EOP) break;
        if(line.find_first_of("#") == 0) continue;
        p.parse(line);
    }
    return is;
}

//-------------------------------------------------------------------------
/* copy the following into the props.test file

   # State file
    asdfasdf=32

AAA=BBB#CCC

   #asdf3223=412341234
       32j2j2l2j2l2=
    vc.test.key  = 23lj2j3l2 # here we go

    vc.test.key3  = "23452345" #
   vc.file.this                   = state-v1.0-n0.d
   vc.sim.version                 = 1.0
   vc.size                        = 768
    vc.test.key2  = $$$$$\\// # here we go 2
   vc.sim.file.properties         =
   vc.sim.file.state.previous     = VC_2001v9_NV_12.d

#####
sadfjsdfa;ja=1112
##asdfasdfadsf
   vc.sim.file.system             = VC_FAULTS_2001v9.d
   vc.sim.file.green              = VC_2001v9_out.d
   vc.sim.year.end                = 1224000.0
   vc.sim.dt                      = 
   vc.sim.dynTrigger              = 
#$EOP
theendprops=theend

*/


//-------------------------------------------------------------------------
// TESTING!
//-------------------------------------------------------------------------
#ifdef TESTING

void propsTest(string fn) {
    /*
    string test("\t  a  b\n z  ");

    cout << ">>" << test << "<<" << endl
         << ">>" << trim(test) << "<<" << endl;

    */
    ifstream f;
    f.open(fn.c_str());
    ASSERT(f.good(), "Failed to open parameters file");

    Properties p;
    f >> p;
    f.close();

    p.format("AAA ", false);
    cout << p;
    cout << "..............................." << endl;
    p.format("", true);
    cout << p;

    cout << "Parameters count = " << p.getCnt() << endl;
}


int main(int argc, char* argv[]) {
    assert(argc == 2);
    propsTest(argv[1]);

    return 0;
}

#endif // TESTING

//-------------------------------------------------------------------------
