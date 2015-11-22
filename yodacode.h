#ifndef YODACODE_H
#define YODACODE_H

#include <sstream>
#include <cassert>
#include <cstdlib>

//-------------------------------------------------------------------------
// Gleb's random numbers:
/*
class yRand {
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
*/

namespace yodacode {

int int1if0(int a) {
		// use for dividing by something that might be zero to avoid core dump. you'll have to trap the original value.
		if (a==0) a=1;
		return a;
		};

float float1if0(float a){
		// use for dividing by something that might be zero to avoid core dump. you'll have to trap the original value.
		if (a==0) a=1;
		return a;
		};

void yodaPause() {
	char text[20];
	printf("yodapausing...\n");
	fgets(text, sizeof text, stdin);
	};
void yodapause() {
	yodaPause();
	};
// this site shows standard C++ string class and functions:
// http://www.bgsu.edu/departments/compsci/docs/string.html
std::string intToStr(int myInt) {
	std::stringstream ss;
	std::string str;
	ss << myInt;
	ss>>str;
	//printf("intToStr: %s\n", str.c_str());
	return str;
	};

std::string longToStr(long myInt) {
	std::stringstream ss;
	std::string str;
	ss << myInt;
	ss>>str;
	//printf("intToStr: %s\n", str.c_str());
	return str;
	};

std::string floatToStr(float myFloat) {
	std::stringstream ss;
	std::string str;
	ss << myFloat;
	ss>>str;
	//printf("intToStr: %s\n", str.c_str());
	return str;
	};

std::string zeroTrimmed(std::string nn, int fullLen=2) {
	nn = nn.insert(0, "0000000000");
	//printf("string size: %d, %d\n", nn.size(), nn.length());
	nn = nn.substr(nn.length()-fullLen, nn.length()-1);
	//printf("zeroT: %s\n", nn.c_str());
	return nn;
	};

std::string zeroTrimmedn(std::string nn, int fullLen=2) {
	nn = nn.insert(0, "000000000000000000");
	//printf("string size: %d, %d\n", nn.size(), nn.length());
	nn = nn.substr(nn.length()-int(fullLen), nn.length()-1);
	//printf("zeroT: %s\n", nn.c_str());
	return nn;
	};

std::string getNowString() {
	// return a string like YYYYMMDDHHMMSS
	// requires <string>, <time>, <stdio>
	//http://www.cplusplus.com/reference/clibrary/ctime/
	time_t rawtime;
	struct tm * timeinfo;
	time (&rawtime);
	timeinfo = localtime(&rawtime);
	//
	std::string nowstring("");
	//printf("year: %s\n", intToStr(timeinfo->tm_year+1900).c_str());
	nowstring.append(intToStr(timeinfo->tm_year+1900));
	nowstring.append(yodacode::zeroTrimmed(intToStr(timeinfo->tm_mon+1)));
	nowstring.append(yodacode::zeroTrimmed(intToStr(timeinfo->tm_mday)));
	nowstring.append(yodacode::zeroTrimmed(intToStr(timeinfo->tm_hour)));
	nowstring.append(yodacode::zeroTrimmed(intToStr(timeinfo->tm_min)));
	nowstring.append(yodacode::zeroTrimmed(intToStr(timeinfo->tm_sec)));

	return nowstring;
	};

int StrToInt(std::string mystr) {
	std::istringstream stm;
	int mynum;
	stm.str(mystr);
	stm>>mynum;
	//
	return mynum;
	};

int StrToLong(std::string mystr) {
	std::istringstream stm;
	long mynum;
	stm.str(mystr);
	stm>>mynum;
	//
	return mynum;
	};

float StrToFloat(std::string mystr) {
	std::istringstream stm;
	float mynum;
	stm.str(mystr);
	stm>>mynum;
	//
	return mynum;
	};

float lesserOf(float a, float b) {
	// return the lesser of two floats.
	// note, we return a by default.
	float returnVal;
	//printf("a,b: %f, %f\n", a,b);
	if (a<=b) returnVal=a;
	if (b<a) returnVal=b;
	//
	return returnVal;
	};

float greaterOf(float a, float b) {
	// return the lesser of two floats.
	// note, we return a by default.
	float returnVal;
	if (a>=b) returnVal=a;
	if (b>a) returnVal=b;
	//
	return returnVal;
	};

/*
float sumAry(float thisAry[]) {
	int myLen = sizeof(thisAry);
	printf("size(myAry[]): %d\n", myLen);
	printf("size(an int): %d\n",sizeof(int));
//	printf("sArray/sArray[0]: %d\n", sizeof(thisAry)/sizeof(thisAry[0]));
	int nElements = sizeof(thisAry)/sizeof(thisAry[0]);
	printf("nElements: %d\n", nElements);
	float mySum = 0;
//	for (int i=0; i<nElements; i++) {
//		mySum = mySum + thisAry[i];
//		};
	return mySum;

	};
*/


}
#endif
