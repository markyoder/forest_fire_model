#include <stdio.h>
//#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <iostream>
#include <fstream>
//#include </home/myoder/Documents/Source/yodacode.h>
#include <math.h>

#include <sstream>
#include <time.h>
#include <vector>

#include <cmath>
#include <cassert>
#include <iomanip>

#include <cctype>

#include <mysql.h>
#include <mysql++.h>



// note: ths part would go in the .h file; the actual function definitions (below) go in the .cpp part.
class myClass {
	public:
		myClass();
		void sayHello(std::string mystring);
		void sayHello2();
		void setHello2(std::string mystring);
		std::string getHello2();
	private:
		std::string hello2Str;
	};
	
std::vector<std::vector<float> > sortHvector(std::vector<std::vector<float> > vIn, int sortCol) {
	const unsigned nRows=vIn.size();
	unsigned rwidth=vIn[0].size();
	int minIndex=0;
	//unsigned vLen=nRows;
	//
	std::vector<std::vector<float> > vSorted(nRows, std::vector<float>(rwidth,0));
	//
	//for (unsigned ii=0; ii<nRows; ii++) {
	// unsigned ii=0;
	//while (vIn.size()>0) {
	for (unsigned ii=0; ii<nRows; ii++) {
		float minVal=vIn[0][sortCol], thisVal=0;
		for (unsigned i=0; i<vIn.size(); i++) {
		//for (unsigned i=0; i<vLen; i++) {
			thisVal=vIn[i][sortCol];
			
			if (thisVal<=minVal) {
				minVal=thisVal; minIndex=i;
				};
			// if (thisVal>=maxVal) maxVal=thisVal; maxIndex=i;
		//currentSize--;
			};
		vSorted[ii]=vIn[minIndex];
		vIn.erase(vIn.begin()+minIndex);
		//vIn[minIndex]=vIn[vLen-1];
		//vLen--;
		};
	//
	return vSorted;
	};
	
std::vector<std::vector<float> > sortHvector2(std::vector<std::vector<float> > vIn, int sortCol) {
	// note: horizontal vectors like:
	// < <H0, F0>, <H1, F1>, ... , <H(N-1), F(N-1)> >
	// we sort a vector-o-vectors by sortCol in ascending order.
	// we scan the vector for the max and min values and build the return vector from both ends.
	//		note: in retrospect this is an unnecessary level of complexity. it would be equivalent
	//		and maybe better to look for either the max OR min value, place said value appropriately,
	//		and then 'continue'. this way, we scan the minimal number of rows each time.
	// printf ("vIn.size(): %d\n", vIn[0].size());
	int vWidth=vIn[0].size();
	int nRows=vIn.size();
	// printf("assigning sort stuff...\n");
	std::vector<float> vRow(vWidth, 0);
	std::vector<std::vector<float> > vOut(vIn.size(), vRow);
	//
	int nStill=nRows;
	int outMaxRow=nRows-1, outMinRow=0;
	//
	// now, walk through the vIn; look for min and max values. watch for optimization tricks:
	int maxRow, minRow;
	// printf("declarations done. now, start many walks...\n");
	while (nStill>0) {
		// printf("walk number (%d)\n", nRows-nStill);
		float maxVal = vIn[0][sortCol], minVal = vIn[0][sortCol];
		// maxRow=nStill-1, minRow=0;
		maxRow=0;		// source row(s)
		minRow=0;
		for (int iRows=0; iRows<nStill; iRows++) {
			if (vIn[iRows][sortCol] >= maxVal) {
				maxVal=vIn[iRows][sortCol];
				maxRow=iRows;
				};
			if (vIn[iRows][sortCol] <= minVal) {
				minVal=vIn[iRows][sortCol];			
				minRow=iRows;
				};
			//
			// if (maxRow!=0 and minRow!=0) break;		// optimize: exit loop.
			};
			// printf("finished walk (%d)\n", nRows-nStill);
		// now, we've spun through the unsorted array and identified the max and min remaining values:
		// copy to sorted array:
		// printf("min,max: %f, %f\n", minVal, maxVal);
		vOut[outMaxRow]=vIn[maxRow];
		vOut[outMinRow]=vIn[minRow];
		outMaxRow--;
		outMinRow++;
		// printf("finished assigning to sorted vector.\n");
		//
		// remove these values; move last two values into their places:
		vIn[maxRow]=vIn[nStill-1];
		nStill--;
		if (maxRow!=minRow) {
			vIn[minRow]=vIn[nStill-1];	// note: odd count arrays have one element at the final step...
			nStill--;
			};
		// printf("finished compressing vector. nStill=%d\n", nStill);
		// now, we only use the top part of the parent array. we can also do this with v.erase, but this way should be faster 
		// since it does not require a resize (of course, the vector class might resize more intelligently...).
		};
	//
	return vOut;
	};	

myClass::myClass() 
	{
	//setHello2("default hello2");
	hello2Str = "default hello2";
	};

void myClass::sayHello(std::string mystring) {
	printf("hello: %s\n", mystring.c_str());
	};
void myClass::sayHello2() {
	printf("hello2: %s\n", hello2Str.c_str());
	};
void myClass::setHello2(std::string mystring) {
	hello2Str = mystring;
	};
std::string myClass::getHello2() {return hello2Str;};
//
//
int readArray(int *ary, int i) {
	printf("readArray (%d): %d\n", i, *(ary+i));
	return 0;
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
	
bool isnumeric(char *str, int slen) {
	bool isnum=1;
	for (int i=0; i<slen; i++) {
		if (isdigit(str[i])==0) {
			isnum=0;
			i = slen;
			};
		};
	return isnum;
	};



int main(int argc, char *argv[]) {

	std::string testString("this is a test.");
	printf("test string: %s\n", testString.c_str());
	printf("test string[2]: %c\n", testString[2]);
	
	size_t length;
	length=testString.size();
	char test[int(length)];
	testString.copy(test, 0, length);
	
	
	char a;
	a=testString[2];
	if (testString[2]=='i') printf("this is true\n");
		
	printf("this is also a string: %s\n", test);
		
	return 0;
	};

/*
int StrToInt(std::string mystr) {
	std::istringstream stm;
	int mynum;
	stm.str(mystr);
	stm>>mynum;
	//
	return mynum;
	};
*/


