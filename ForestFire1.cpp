#include <unistd.h>
#include "gnuplot_i.hpp"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <stdlib.h>

#include <time.h>
#include <string>
#include <vector>
#include <sstream>

#include </home/myoder/Documents/Source/yodacode.h>
//#include </home/myoder/Documents/Source/glebsutils.h>

// mysql elements:
#include <mysql.h>
#include <mysql++.h>

// Make a forest fire model based on Turcotte, 1999 "Self-organized criticality"
// grid status: {0,{int},-1} = {bare, tree of age {int}, burning} respectively
// we may not need the "burning" status since nothing will be burning for more than one turn... for the time being.
// also, how will we propagate the burn? maybe we scan the whole grid each time and set burning each square next to
// a burning square.
//
// 2009-12-22 yoder: add element level immunity with some function.
// in particular, we'll be interested in a p*1/k type immunity, maybe using
// the 5c (c??) p*1/(1-k/k0) where k0 basically mitigates small k integer effects and k is the number of currently
// burning elements.
//
// a few comments on variables:
// char: 1byte
// short: short integer (2 bytes)
// int (4 bytes)
// long (4 bytes??? 8 bytes?)
// etc.
// i've been specific about the variable types to optimze speed. if this sim grows beyond a few thousand elements
// on a side, some of these variables should be re-typed. also, i've been specific on signed/unsigned for the same
// reasons.
//
// declare grid dimensions as constants so we can put them into arrays?
const unsigned int xmax=128;
const unsigned int ymax = 128;
void yodapause() {
	yodacode::yodapause();
	};

int printGrid(signed int *grids, int X, int Y) {
	// gnuPlot is being a pain in the ass, so for now, let's just print the stupid grid.
	int myX, myY;
	int imax = (X+2)*(Y+2);
	int i = 0;
	while (i < imax) {
		//printf("starting i: %d\n", i);
		//if (int(i/20)==float(i)/20) yodapause();
		myY = int(float(i)/float(X+2));
		myX = i-(X+2)*myY;
		//myVal = *(grids + i);
		// note: the array is transposed...
		//printf("%d ", *(grids +i));
		if (*(grids+i)==0) printf("O");
		if (*(grids+i)==1) printf("A");
		if (*(grids+i)==-1) printf("*");
		if (float(i+1)/float(X+2) == int((i+1)/(X+2))) printf("\n");
		//printf("!!: %d, %d, :: %f, %d, %d\n", i, X, float(i)/float(X), int(i/X), (float(i+1)/float(X) == int((i+1)/X)) );
		yodapause();
		i++;
		};
	//yodapause();
	
	return 0;
	
	};

int plotGrid(signed int * grids, int X, int Y) {
	printf("we'll plot the grid here...");
	// yodapause();
	//std::string strArray("");
	//std::string strNewRow("");
	int myX, myY, myVal;
	Gnuplot g1=Gnuplot();
	//
	// note: grids[x][y] (maybe we should just make our own 1d array) is like: grids->[x=0][{y}],[x=1][{y}],etc.
	// so we have to transpose this. get the 1st of every row, the second of every row, etc.
	//g1.cmd("plot '-' using 1:2 with points 8 ");
	// g1.cmd("set multiplot");
	g1.cmd("set xrange[0 : %d]", X+1);
	g1.cmd("set yrange[0 : %d]", Y+1);
	g1.cmd("plot '-' with points 4, '-' with points 7");
	//g1.cmd("3 4");
	//g1.cmd("e");
	//g1.cmd("plot '-' with points 4");
	int imax = (X+2)*(Y+2);
	int i = 0;
	while (i < imax) {
		//printf("starting i: %d\n", i);
		//if (int(i/20)==float(i)/20) yodapause();
		myY = i/(X+2);
		myX = i-myY*(X+2);
		myVal = *(grids + i);
		// printf("thisX, thisY, thisVal: (%d, %d, %d)\n", thisX, thisY, thisVal);
		// yodacode::yodapause();
		if (myVal >= 1) {
			//printf("(%i) thisX, thisY, thisVal: (%d, %d, %d)\n", i, myX, myY, myVal);
			//if (int(i/20)==float(i)/20) yodapause();
			g1.cmd("%d %d", myX, myY);
			//g1.cmd("%s %s", yodacode::intToStr(thisX).c_str(), yodacode::intToStr(thisY).c_str());
			};
		i++;
		};
	g1.cmd("e");
	// yodapause();
	//
	//printf("now plot fires?\n");
	i = 0;
	//g1.cmd("plot '-' using 1:2 with linespoints 0 ");
	while (i < imax) {
		myY = i/(X+2);
		myX = i-myY*(X+2);
		myVal = *(grids + i);
		if (myVal==-1) {
			g1.cmd("%d %d", myX, myY);
			};
		i++;
		};
	g1.cmd("e");

	yodacode::yodapause();


	return 0;
	};

int main() {
//	int Xmax = 1000;
//	int Ymax = 1000;
	std::string simName("ForestFire1");
	unsigned short fmatch = 2000;
	float pMatch = 1;
	unsigned short burnAge=1;	// a tree will burn if it comes into contact with fire and age>=burnAge
	unsigned int tmax = 10000000;
	// unsigned int tmax = 1638 * int(pow(10,6));
	bool wrapx1 = 0;
	//bool wrapx2 = 0;
	bool wrapy1 = 0;
	//bool wrapy2 = 0;
	//
	unsigned short rFire = 1;
	unsigned int nBurning = 1;
	unsigned short dnBurning = 1;
	int thisX, thisY, thisStatus, xFire, yFire;
	int * thisSquare, * fireSquare;
	unsigned int nTreesPlanted = 0, nBurnedTotal=0, nFires=0;  // number of elements burning, new Elements burning...
	float ffmatch;
	signed int grids[(ymax+2)*(xmax+2)];
	// initialize?
	srand(time(NULL));	// eventually, we have to be smarter about this and use independent random sets. see Gleb's random number gens.
	for (unsigned int i=0; i<=(xmax+2)*(ymax+2); i++) {
		*((&grids[0])+i)=0;
		//yodapause();
		};
	srand(time(NULL));	// eventually, we have to be smarter about this and use independent random sets.
	//
	
	char DB[]="ForestFire";
	char HOST[]="localhost";
	char USER[]="myoder";
	char PASSWORD[]="yoda";
	mysqlpp::Connection myconn(DB, HOST, USER, PASSWORD);
	mysqlpp::Query myquery=myconn.query();
	mysqlpp::Result res1;
	//
	// insert a new row for this sim-run:
	myquery.reset();
	printf("insert simprams.\n");
	myquery << "insert into ForestFire.SimPrams (SimName, SimSW, xmax, ymax, sparkInterval, sparkProb, burnAge, tmax, wrapX, wrapY ) values (%0q, %1q, %2q, %3q, %4q, %5q, %6q, %7q, %8q, %9q )";
	myquery.parse();
	// note: simIndex(auto-int) and dtTime (default TIMESTAMP) set automatically.
	myquery.execute(simName.c_str(), "yodaff1", xmax, ymax, fmatch, pMatch, burnAge, tmax, wrapx1, wrapy1);
	//
	// now, get the integer key for this simulation:
	// note that this could be accomplished with one call (optimal if MySQL calls have high overhead)
	// by writing a SPROC on the MySQL side.
	// also see the mysql_insert_id() (C API) and LAST_INSERT_ID() (SQL) functions, by which we should be ablt to automatically retrieve the indexID.
	myquery.reset();
	printf("fetch simIndex.\n");
	myquery << "select max(simIndex) from ForestFire.SimPrams where simName=%0q and simSW=%1q";
	myquery.parse();
	res1 = myquery.store(simName.c_str(), "yodaff1");
	printf("fetched simIndex.\n");
	//res1 = myquery.store("VC", 0, "NED-1857_ALL");
	int intKey = res1.at(0).at(0);
	//	
	for (unsigned int i=0; i<=tmax; i++) {
		// here, i'm being sloppy with random numbers. really, we need four independent
		// random number generators for xTree, yTree, xMatch, yMatch
		// printf(" iteration %d\n", i);
		//
		// PLANT A TREE:
		// select a grid for tree planting:
		thisX = rand()%xmax+1;
		thisY = rand()%ymax+1;
		thisSquare = &grids[0] + thisX + (xmax+2)*thisY;	// point to the current square...
		// now check the status of the grid element. this code is semi-optional.
		// we assess the grid status so we can report
		// "trees planted" statistics. 
		//thisStatus = grids[thisY][thisX];
		thisStatus = *thisSquare;
		if (thisStatus <= 0) {
			nTreesPlanted++;
			//grids[thisY][thisX] = 1;
			//grids[thisSquare]=1;
			*thisSquare = 1;
			//printf("(%d)tree2: (%d, %d, %d)\n",i , thisX, thisY, grids[thisY][thisX]);
			// plotGrid(&grids[0][0], xmax, ymax);
			};
		//
		// we've planted a tree. do we throw a match?
		// we can do this two ways. we can trow a match every M steps or set a 
		// 1/M probability every time.
		// for now, we're being super simple; throw a match every 'fmatch' steps:
		ffmatch = float(i+1)/float(fmatch);
		//if (float(int(ffmatch))==ffmatch) printf("crazy true condition (%d)...\n", i);
		// skip the first xmax or so tree-plantings to mitigate a temporal, pre-critical edge effect
		if (float(int(ffmatch))==ffmatch and i>xmax) {
			//yodapause();
			// throw a match.
			xFire = rand()%xmax+1;
			yFire = rand()%ymax+1;
			fireSquare = &grids[0] + xFire + (xmax+2)*yFire;
			printf("match: (%d, %d) :: %d\n", xFire, yFire, *fireSquare);
			//yodapause();
			//
			if (*fireSquare >= burnAge) {
				// initiate a new fire.
				// start from the epicenter and work out in concentric rectangles (squares)
				// until there are no new fires.
				// note: we make two passes over each circle. for now, we assume all squares
				// continue to burn until the fire is over. this is a subtle consideration that
				// will not matter for simpler versions of the model, but if we introduce burn probabilities,
				// we will have to be more careful.
				// alternatively, we can scan the entire grid every time, but the above method will
				// save CPU time.
				*fireSquare=-1;	// the fire-square starts burning
				//
				rFire = 1;
				nBurning = 1;
				dnBurning = 1;
				int yFireMin = int(yodacode::greaterOf((yFire-rFire), 1));	// we always start with a 1 squar boundary. we might, however, encounter the edges.
				int yFireMax = int(yodacode::lesserOf((yFire+rFire), float(ymax)));
				int xFireMin = int(yodacode::greaterOf(float(xFire-rFire), 1));
				int xFireMax = int(yodacode::lesserOf(float(xFire+rFire), float(xmax)));
				//printf("fire range: %d, %d, %d, %d\n", yFireMin, yFireMax, xFireMin, xFireMax);
				//printf("preplot\n.");
				//plotGrid(&grids[0], xmax, ymax);
				//printGrid(&grids[0], xmax, ymax);
				while (dnBurning > 0) {
					dnBurning=0;
					for (char doTwice = 0; doTwice <=1; doTwice++) {	// "char" is a 1 byte integer. we could also use a boolean to count to 2.
					for (int iy = (yFireMin); iy <= (yFireMax); iy++) {
						for (int ix = (xFireMin); ix <= (xFireMax); ix++) {
							// note this loops down then across...
							// if a neighbor is burning and the element is old enough and the element is not alread burning
							// (note by using burning -> -1, the "burnAge" condition achieves (2) and (3)...
							// also note: Gelb is right. a recursive approach is better.
							// printf("try-burn: %d, %d\n", ix, iy);
							//plotGrid(&grids[0], xmax, ymax);
							int * centerGrid = &grids[0] + ix + (xmax+2)*iy;
							int * upGrid = centerGrid + (xmax+2);
							int * downGrid = centerGrid - (xmax+2);
							int * leftGrid = centerGrid-1;
							int * rightGrid = centerGrid+1;
							//yodapause();
							//if (grids[iy][ix] >= burnAge && (grids[ix+1][iy]==-1 || grids[ix-1][iy]==-1 || grids[ix][iy+1]==-1 || grids[ix][iy-1]==-1) ) {
							//printf(" checking: cg=%d, lg=%d, rg=%d, ug=%d, dg=%d, anyburn=%d\n", *centerGrid, *leftGrid, *rightGrid, *upGrid, *downGrid, (*leftGrid==-1 or *rightGrid==-1 or *upGrid==-1 or *downGrid==-1) );
							if (*centerGrid >= 1 and (*leftGrid==-1 or *rightGrid==-1 or *upGrid==-1 or *downGrid==-1)) {
							// if (grids[iy][ix] >= 1 && (grids[ix+1][iy]==-1 || grids[ix-1][iy]==-1 || grids[ix][iy+1]==-1 || grids[ix][iy-1]==-1) ) {
								*centerGrid = -1;
								dnBurning ++;
								// printf("[%d, %d] catches from [%d, %d]\n", ix, iy, xFire, yFire);
								};
							}; // ix
						}; // iy
						}; // doTwice	
						nBurning = nBurning + dnBurning;
					//rFire++;
					xFireMin = int(yodacode::greaterOf(1, float(xFireMin-1)));
					xFireMax = int(yodacode::lesserOf(float(xmax), xFireMax+1));
					yFireMin = int(yodacode::greaterOf(1, float(yFireMin-1)));
					yFireMax = int(yodacode::lesserOf(float(ymax), yFireMax+1));
					// g1.plot_xy(vfireX, vfireY, "");
					};	// end fire still burining
				//
				// recursive method:
				// if (grids[xFire][yFire] >= burnAge) {
				//	burn(xFire, yFire);	// burn() will look up, down, right, left and call itself recursively. we have to do the "pass array address" bit though...
				//	};
				//
				nFires++;
				nBurnedTotal = nBurnedTotal + nBurning;
				printf("fire size, nFires, totalBurned: (%d) (%d) (%d)\n", nBurning, nFires, nBurnedTotal);
			//	plotGrid(&grids[0], xmax, ymax);
				// write fire to MySQL:
				myquery.reset();
				myquery << "insert into ForestFire.ForestFires (simIndex, t, xSpark, ySpark, nBurned) values (%0q, %1q, %2q, %3q, %4q) ";
				myquery.parse();
				myquery.execute(intKey, i, xFire, yFire, nBurning);
				//			
				// printGrid(&grids[0], xmax, ymax);
				// clear grid of fire:
				for (unsigned int i=0; i<=((ymax+2)*(xmax+2)-1);i++) {
					if (grids[i]==-1) grids[i]=0;
					};
				// printGrid(&grids[0], xmax, ymax);
				//plotGrid(&grids[0], xmax, ymax);
				}; //else printf("no tree at match point.\n");	// if match -> tree...
			}; // if match-time

		// do we initialize with 0?
		//printf("ary element 0,i: %d", grids[0][i]);
		};	// end sim-steps.

	// end simulation.
	printf("finished.\nfire size, nFires, totalBurned: (%d) (%d) (%d)\n", nBurning, nFires, nBurnedTotal);
	//g1.reset_plot();
	//g1.plot_xyz(vx, vy, vGridStat, "xyz plot of TreesPlanted");

	//yodacode::yodapause();
	return 0;
	};
