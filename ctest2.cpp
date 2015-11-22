#include <stdio.h>
//#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <iostream>
#include <fstream>
//#include </home/myoder/Documents/Source/yodacode.h>
#include "glebsutils.h"
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
//	printf("startup integers:\n");
//	for (int i=1;i<argc;i++) {
//		printf("arg%d: %s\n", i, argv[i]);
//		};
//	for (int i=0; i<100; i++ ) {
//		printf("%d  %d\n", i, i*i);
//		}
	//Rand yrand1(time(NULL));
	//Rand yrand2(time(NULL)+1);
	//Rand yrand3(time(NULL)+2);
	Rand yrand1(42);
	Rand yrand2(11);
	Rand yrand3(7);
	//
	for (int i=0; i<25; i++) {
		printf("r1,r2,r3: %f, %f\n", yrand1.nextDouble(), yrand2.nextDouble());
		printf("**r1,r2,r3: %d, %d\n", yrand1.nextInt(42), yrand2.nextInt(1));
		};
	printf("-------------------------\n");
	//
	Rand yrand4(42);
	Rand yrand5(11);
	Rand yrand6(7);
	for (int ii=0; ii<5; ii++) {
		for (int i=0; i<5; i++) {
			printf("r1,r2,r3: %f, %f, %f\n", yrand4.nextDouble(), yrand5.nextDouble(), yrand6.nextDouble());
			};
		printf("next 5\n");
		};
//	printf("mod: 5%2 : %d, 4%2 : %d, 6%3 : %d, 6%5 : %d\n", 5%2, 4%2, 6%3, 6%5);

	printf("declare a big ass array:\n");
	// this will cause a segmentation error.
	//short myBigArray[5000000];
	int *arySize = new int;
		*arySize=1500000000;
	printf("array size declared [%d]...\n", *arySize);
	int *myBigArray = new int[*arySize];
	int *myRegArray;
	myRegArray = new int[10000];
	printf("array declared [%d]...\n", *arySize);
	//
	//int anArray[100];
	//printf("size of myBigArray: %d\n", int(sizeof(myRegArray))/int(sizeof(int)) ); 	// not useful for pointer-arrays.
	delete myBigArray;
	delete myRegArray;
/*
	for (int i=0; i<100; i++) {
		printf("rand %d: (%d)\n", i, yrand1.nextInt(4));
		};
	int a=5;
	int b=2;
	int c=4;
	printf("5%2: %d\n", 5%2);
	printf("4%2: %d\n", 4%2);
	printf("5/2: %d\n", a/b);
	printf("4/2: %d\n", c/b);
*/
	myClass myhello;
	myhello.setHello2("this is hello2.");
	myhello.sayHello("hello from primary hello");
	myhello.sayHello2();
	printf("hello2Str: %s\n", myhello.getHello2().c_str());
	//
	//
	for (int i=0; i<10; i++) {
		printf("nextInt(1): %d\n", yrand4.nextInt(1));
		// random of set of 1 numbers; (always 0)
		};
	//
	// can put a pipe/redirection in the place of a file?
	std::ifstream myfile;
	std::string line;
	myfile.open("ffdifference.gnu");
	//myfile.open("<mysql -umyoder -pyoda -DQuakeData -r -e""select concat(cast(year(eventDateTime) as char(4)), ' ' ,  cast(month(eventDateTime) as char(4)), ' ', cast(day(eventDateTime) as char(2)), ' ' , cast(lng as char(10)), '  ', cast(lat as char(10)), '  ', cast(mag as char(6)) ) as '' from earthquakes limit 100""");
	while (! myfile.eof() ) {
		getline(myfile,line);
		cout<<line<<endl;
		};
	myfile.close();
	
	char str1[]="4567";
	char str2[]="45a67";
	
	printf("digit tests:\n");
	printf("isdigit('a'): %d\n", isdigit(str1[2]));
	printf("isdigit('4'): %d\n", isdigit(str2[2]));
	printf("isnumeric('4567'): %d\n", isnumeric(str1, 4));
	printf("isnumeric('45a67'): %d\n", isnumeric(str2, 5));
	// printf("strlen(str1): %u\n", strlen(str2));

	char argString[] = "dataIndex, lat, lon, 2, 5, 0, 1a";
	int strLen = strlen(argString);
	char *strCols = argString;
	int nCols=1;
	for (int i=0; i < strLen; i++) {	
		if (strCols[i]==',') nCols++;		//eventually gives nCols-1
		printf("i: %d\n", i);
		};
	printf("ncols: %d\n", nCols);
	//
	std::string argStrings[nCols];
	int thisCol=0;
	argStrings[0]="";
	// scan the string again; assign values:
	for (int i=0; i <= strLen; i++) {
		if (strCols[i] != ',' && strCols[i] !=' ') {
			argStrings[thisCol] += strCols[i];
			};
		if (strCols[i] == ',') {
			// new element:
//			// if it is not numeric, wrap it in quotes:
//			if (isnumeric(argStrings[thisCol].c_str(), argStrings[thisCol].size() )==0) {
//				argStrings[thisCol].insert(argStrings[thisCol].begin()+0, "'");
//				argStrings[thisCol] += "'";
//				};
			thisCol++;
			// initialize next column:
			argStrings[thisCol]="";
			};
		};
		for (int i=0; i<nCols; i++) {
			char *thisStr = new char[argStrings[i].size()];
			for (unsigned int ii=0; ii<argStrings[i].size(); ii++) {
				*(thisStr + ii) = argStrings[i].c_str()[ii];
				};
			// use this to wrap a col-name in "`" (or something). these variables don't
			// seem to pass properly anyway, so we'll just get the indices from the row-data.
			/*	
			if (!isnumeric(thisStr, argStrings[i].size())) {
				std::string newstring="`";
				newstring.append(argStrings[i].c_str());
				newstring.append("`");
				//
				argStrings[i]=newstring;
				// delete &newstring;
				delete thisStr;
				char *thisStr = new char[argStrings[i].size()];
				for (unsigned int ii=0; ii<argStrings[i].size(); ii++) {
					*(thisStr + ii) = argStrings[i].c_str()[ii];
					};
				};
			*/				
			
			printf("col %d: %s, isnumeric(%d)\n", i, argStrings[i].c_str(), isnumeric(thisStr, argStrings[i].size()));
			// printf("col %d: %s\n", i, argStrings[i].c_str());
			delete thisStr;
			};
			
	mysqlpp::Connection mycon("QuakeData", "localhost", "myoder", "yoda");
	//mysqlpp::Query myquery=mycon.query();
	mysqlpp::Query myquery=mycon.query();
	mysqlpp::StoreQueryResult res1;
//	mysqlpp::UseQueryResult res1=query.use();	// this appears to simply not be part of modern MySQL++
	//
	myquery << "select * from PI where dataIndex=12 limit 15";
	// myquery.parse();
	res1 = myquery.store();
	//
	float myint = res1.at(2).at(2);
	printf("field 2,2: %f\n", myint);
	
	std::string strLat("lat");
//	mysqlpp::Row row = res1.fetch_row();
//	printf("lat: %s\n", row("lat"));

	printf("field-2-name: %s\n", res1.field_name(2).c_str() );
	printf("field-lat-num: %d\n", res1.field_num(strLat));
	
	
	//
	char str1a[]={"seismicity"};
	char str2a[]={"101"};
	// newLen1 = strlen(str1a);
	// newLen2 = strlen(str2a);
	char newstr[100];
	strcpy(newstr, str1a);
	strcat(newstr, str2a);
	strcat(newstr, ".xyz");
	
	printf("newstr: %s\n", newstr);
	// char newstr[]=str1a + str2a + ".xyz";
	
	std::ostringstream os;
	os << "str1" << 5 << "_" << 6.7;
	std::string teststring(os.str());
	printf("teststring: %s\n", teststring.c_str());
	std::string str1b="str1f";
	std::string str2b="str1abc";
	printf("compare %i\n", str1b==str2b);
	printf("compare %i\n", str1b.compare(str2b));
	
	time_t mytime;
	time (&mytime);
	printf("local time: %s\n", ctime(&mytime));
	
	std::vector<float> vRow(4,0);
	std::vector<std::vector<float> > myvec;
	std::vector<std::vector<float> > myvec0;
	std::vector<std::vector<float> > myvec1;
	std::vector<std::vector<float> > myvec2;		
	for (int i=0; i<10; i++) {
		for (int ii=0; ii<4; ii++) vRow[ii] = float(rand()%100)/100;
		myvec.push_back(vRow);
		};
	for (unsigned i=0; i<myvec.size(); i++) {
		printf("myvec: %f, %f, %f, %f (%d/%d)\n", myvec[i][0], myvec[i][1], myvec[i][2], myvec[i][3] , i, myvec.size());
		};

	printf ("===================\n");
	myvec0=myvec;
	myvec1=sortHvector(myvec0,2);
	//myvec1=sortHvector(myvec1,2);
	//myvec1=sortHvector(myvec1,2);
	// myvec.erase(myvec.begin()+4);
	for (unsigned i=0; i<myvec1.size(); i++) {
		printf("myvec: %f, %f, %f, %f (%d/%d)\n", myvec1[i][0], myvec1[i][1], myvec1[i][2], myvec1[i][3] , i, myvec1.size());
		};
	
	// funny assignments...
	printf("funny assignments\n");
	float f1, f2;
	int i1, i2;
	std::string stri("1234567");
	std::string strf("245.17");
	// std::ostringstream strBuffer;
	std::string strBuffer;
	//
	// string::iterator rit;
	for (std::string::iterator rit=stri.begin(); rit < stri.end(); rit++) {
		// strBuffer << *rit;
		strBuffer.push_back(*rit);
		// cout << *rit;
		};
	i1=atoi(strBuffer.c_str());
	printf("strBuffer: %s/%d\n", strBuffer.c_str(), i1);
	
	strBuffer.clear();
	
	// integer arithmetic:
	// int a=10,b=3,c=21,d=4;
	printf("integer operations:\n");
	printf("10/3=%d, 8/3=%d, 10pct3=%d, 10^3=%d\n", 10/3, 8/3, 10%3, 10^3);
	
	std::string testString("this is a test.")
	printf("test string: %s\n", testString.c_str());
	
//	for (rit=strf.begin(); rit < strf.end(); rit++) {
//		strBuffer << *rit;
//		};
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


