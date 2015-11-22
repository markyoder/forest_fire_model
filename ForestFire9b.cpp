#include <unistd.h>
#include "gnuplot_i.hpp"

#include <stdio.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>

#include <math.h>
#include <stdlib.h>

#include <time.h>
#include <string>
#include <vector>
#include <sstream>

#include "glebsutils.h"
#include "yodacode.h"
#include "percolation.h"
//#include </home/myoder/Documents/Source/glebsutils.h>


// Declarations:
int *scrambleList(int *inList, int *outList, int listLen);
int *randomSequence(int *outList, int listLen);
bool testQuench(int k, int Pimmune, int kImmune);
bool IsOccupied(int x);

class immCode{
	public:
		immCode();
		void setFullStep(bool b);
		void setFlameFronts(bool b);
		void setNewElements(bool b);
		//
		bool getFullStep();
		bool getFlameFronts();
		bool getNewElements();
	private:
		bool atFullStep;
		bool atFlameFronts;
		bool atNewElements;
        };
	


///////////////////////////////
///////////////////////////////

//const unsigned int xmax = 1024;
//const unsigned int ymax = 1024;
// or use notaion:
// pow(2,18)  etc...
const unsigned int xmax = 1024;	// 65536 2**16 //16384 (2**14) //(13->) 8192 (2**13)
const unsigned int ymax =1024;	//64 (2**6) (7->128) 2**7

//const unsigned int xmax = 32;
//const unsigned int ymax = 32;
const int rockAge = 7654321;

// Make a forest fire model based on Turcotte, 1999 "Self-organized criticality"
// grid status: {0,{int},-1} = {bare, tree of age {int}, burning} respectively
// we may not need the "burning" status since nothing will be burning for more than one turn... for the time being.
// also, how will we propagate the burn? maybe we scan the whole grid each time and set burning each square next to
// a burning square.
//
// VERSION 2.0:
// in version 2, we age each existing tree with each time-step (each attempted tree-planting).
// before each round of tree-planting, scan the grid and : if grid[x,y]>1 then grid[x,y]++
//
// Version 3.0:
// in version 3, we age each existing tree, like 2. all trees are flamible but we change the burn propagation rules.
// only older trees propagate when they burn. the idea, of course, is to simulate the growth of  scrub and accumulation
// of fuel in older, unburned regions of forest.
// a modification might be to use a more complex metric: if (age1*age2) > ageSqr0 then burn.
// ---> we see little or no effect. if we set the propagation edge really high (like 10+ * spark interval), we
// get a substantial rise in fires size 1 (duh) but the linear part of the power-law distribution becomes shallower,
// not steeper as desired (showing relatively more large fires). unless propAge is very large, we don't see a substantial
// number of non-propagating trees, let alone clusers thereof. then, of course, those clusters merge into the greater
// cluster and the whole forest explodes.
//	---> think about: poissanian temporal distribution for tree planting
// ---> think about: preferential tree clustering: plant one tree spatially randomly; plant a second on the edge of a
//      a cluster.
// ---> age the trees differently. the trees and clusters grow/age on different time scales. instead, age trees by
//      incrementing if when we drop a tree on top of them again. aka, tree-plant=>age+1 always.
//
// VERSION 4.0:
// try a different age process.
// always, squareValue= squareValue+1 to plant and age trees. set a propagation threshhold as in version 3.0
// also, establish calling parameters, preferential clustering parameters, and age-burn/age-propagate ratios.
//
// version 5.0:
// 1)add permanently "rocky" or baren sites (cannot grow, cannot burn). will this reduce average connectivity and
// shrink clusters? if P(burn) ~ <k>/k0, this should have at least a linear effect. preferential geometry could
// amplify the effect.
// 2) add these rocky elements or plant trees against some fractal substrate or perhaps add lines or other geometries
// of plants or rocks, or whatever.
// 3) also, we may include some sort of cluster-shape parameterization. long clusters (from Diffusion Limited growth)
// have higher connectivity potential to randomly arriving nodes; maybe we can use this to mitigate the development
// of large clusters.
// 5c uses immunity like: pIm/(1+k/kIm) or pIm/k. be careful; there are several variations in comment.
// most pointedly, two versions: immunity is checked at each radial propagation step vs immunity checked as each element is added to the fire.
// the first method produces the "bump" for small fires; the second method produces beautiful PL for .5<Pim<1.0, but for smaller Pim
// we get some shape artifacts (big flat-spots spanning integer L areas, aka ~10^2 - 14^2 for Pim=.4. this appears to be associated with 
// the introduction of preferential direction/geometry. basically, we introduce a spiral propagation of fires that is not compatible with their accumulation dynamics.
//
// 5c1: put the immunity function into the combustion of each individual element. remove the Pquench bit. as the fire propagates,
// each element fails to burn with P=Pim/k. when no more elements burn, the fire goes out; we'll leave some bits behind. 
//
// 9a: new fire propagation. 5cSmoothe breaks symetry by introducing a spiral-like propagation. now, fires will propagate by a list of burning elements:
// 1) as a fire propagates, grids[i]->0 (we clean up as the fire burns).
// 2) the position of each element is registered in fireList[], an array size L^2. nBurning, dnBurning, newdnBurning are recorded:
//    firelist: [x0, x1, x2, x3, x4 (nBurning), x5, x6, x7, x8 (+dnBurning), x9, x10, x11, x12, x13 (newdnBurning), NULL, NULL...]
//     so, already burned and tested elements -> nBurning, active flame-frone elements nBurning-dnBurning (number of elements that lit up last turn)
//     new elements added during this propagation step -> + + newdnBurning.
// 3) we only need to test elements +1, -1, +L, -L from the dnBurning steps.
// 4) after a propagation step, nBurning+=dnBurning, dnBurning=newdnBurning, newdnBurning=0
//
// 9b: 9a is the standard list-based model. 9b introduces MFI.

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

// immCode class:
immCode::immCode() {
	atFullStep=1;
	atFlameFronts=0;
	atNewElements=0;
	};
	
bool atFullStep;
		bool atFlameFronts;
		bool atNewElements;
	
void immCode::setFullStep(bool b) {atFullStep=b;};
void immCode::setFlameFronts(bool b) {atFlameFronts=b;};
void immCode::setNewElements(bool b){atNewElements=b;};
//
bool immCode::getFullStep() {return atFullStep;};
bool immCode::getFlameFronts() {return atFlameFronts;};
bool immCode::getNewElements() {return atNewElements;};
/////////////////
void yodapause() {
	yodacode::yodapause();
	};

bool IsOccupied(int x) { 
	// return x>=1;
	return x == 1;
	};

int getGridStatus(int gridVal, unsigned int t) {
	int treeAge=0;
	//if (gridVal>0) treeAge = t-gridVal;
	//if (gridVal<0) treeAge = -t-gridVal;
	treeAge = gridVal;
	//
	return treeAge;
	};

int printGrid(signed int *grids, unsigned int t, int X, int Y) {
	// gnuPlot is being a pain in the ass, so for now, let's just print the stupid grid.
	printf("printGrid (%d: %d, %d)\n", t, X, Y);
	int myX, myY;
	int imax = (X+2)*(Y+2);
	int i = 0;
	int treeAge;
	float aveTreeAge;
	int nTrees=0;
	int aveRho = 0;	// sum(isTree)
	int aveVol = 0;	// sum(vals)

	while (i < imax) {
		//printf("starting i: %d\n", i);
		//if (int(i/20)==float(i)/20) yodapause();
		myY = int(float(i)/float(X+2));
		myX = i-(X+2)*myY;
		treeAge = getGridStatus(*(grids+i),t);
		//printf("treeAge, gridStat::() (%d, %d::*%s*)\n", treeAge, *(grids+i), yodacode::zeroTrimmed(yodacode::intToStr(*(grids+i)).c_str(), 3).c_str() );
		//
		if (treeAge > 0 and treeAge < rockAge) {
			printf("(%s)", yodacode::zeroTrimmed(yodacode::intToStr(treeAge).c_str(), 3).c_str());
			aveTreeAge = aveTreeAge + float(treeAge);
			nTrees++;
			}; 
		if (treeAge < 0) {
			printf("*%s*", yodacode::zeroTrimmed(yodacode::intToStr(-treeAge).c_str(), 3).c_str());
			}; 
		if (treeAge==0) printf("  O  ");
		if (treeAge>=rockAge) {
			printf(" XxX ");
			}; 
		if (float(i+1)/float(X+2) == int((i+1)/(X+2))) printf("\n");
		//
		//printf("!!: %d, %d, :: %f, %d, %d\n", i, X, float(i)/float(X), int(i/X), (float(i+1)/float(X) == int((i+1)/X)) );
		//yodapause();
		i++;
		};
	if (nTrees !=0) aveTreeAge = aveTreeAge/nTrees; else aveTreeAge=0;
	printf("average tree age: %f\n", aveTreeAge);

	for (unsigned int k=0; k<(xmax+2)*(ymax+2); k++) {
		if (*(grids+k)>0 and *(grids+k)<rockAge) {
			aveRho = aveRho + 1;
			aveVol = aveVol + *(grids+k);
			};
		};
	printf("Rho, Vol: %f, %f\n", float(aveRho)/float(xmax*ymax), float(aveVol)/float(xmax*ymax));


	yodapause();
	
	return 0;
	
	};
int plotGridImg(signed int * grids, unsigned int t, int X, int Y, signed burnPropAge) {
	// gnuplot has a "with image" option...
	int myX, myY;
	Gnuplot g1=Gnuplot();
	//
	g1.cmd("set xrange[0 : %d]", X+1);
	g1.cmd("set yrange[0 : %d]", Y+1);
	g1.cmd("plot '-' with image"); 
	//
	int imax = (X+2)*(Y+2);
	int treeAge;
	int i = 0;
	while (i < imax) {
		//printf("starting i: %d\n", i);
		//if (int(i/20)==float(i)/20) yodapause();
		myY = i/(X+2);
		myX = i-myY*(X+2);
		//myVal = *(grids + i);
		treeAge=getGridStatus(*(grids+i), t);
		if (treeAge==0) treeAge=0;
		if (treeAge>=burnPropAge) treeAge=8;
		if (treeAge>0 and treeAge < burnPropAge) treeAge=4;
		if (treeAge<0 and treeAge>-burnPropAge) treeAge=12;
		if (treeAge<= -burnPropAge) treeAge=16;
		if (i<(int(xmax)+2)) treeAge=int(i/16);	// this to get some pallet normalization...
		g1.cmd("%d,\t%d,\t%d", myX, myY, treeAge);
		i++;
		};
	g1.cmd("e");
	yodapause();
	return 0;
	};

int plotGrid(signed int * grids, unsigned int t, int X, int Y, signed burnPropAge) {
	//printf("we'll plot the grid here...");
	// yodapause();
	//std::string strArray("");
	//std::string strNewRow("");
	int myX, myY, treeAge;
	Gnuplot g1=Gnuplot();
	//
	// note: grids[x][y] (maybe we should just make our own 1d array) is like: grids->[x=0][{y}],[x=1][{y}],etc.
	// so we have to transpose this. get the 1st of every row, the second of every row, etc.
	//g1.cmd("plot '-' using 1:2 with points 8 ");
	// g1.cmd("set multiplot");
	g1.cmd("set xrange[0 : %d]", X+1);
	g1.cmd("set yrange[0 : %d]", Y+1);
	//g1.cmd("plot '-' title 'young' with points 3, '-' title 'older' with points, '-' title 'young-burn' with points 7, '-' title 'old-burn' with points  9");
	g1.cmd("plot '-' title 'young' with points 3, '-' title 'older' with points, '-' title 'young-burn' with points 7, '-' title 'old-burn' with points  9, '-' title 'rocks' with points");
	//g1.cmd("3 4");
	//g1.cmd("e");
	//g1.cmd("plot '-' with points 4");
	int imax = (X+2)*(Y+2);
	int i = 0;
	while (i < imax) {
		myY = i/(X+2);
		myX = i-myY*(X+2);
		treeAge=0;
		//treeAge = getGridStatus(*(grids+i), t);
		g1.cmd("0,\t0");
		treeAge = *(grids+i);
		if (treeAge > 0 and treeAge < burnPropAge) {
		//if (treeAge > burnAge and treeAge < burnPropAge) {
			g1.cmd("%d,\t%d", myX, myY);
			};
		i++;
		};
	g1.cmd("e");
	//
	i = 0;
	while (i < imax) {
		myY = i/(X+2);
		myX = i-myY*(X+2);
		treeAge=0;
		//treeAge = getGridStatus(*(grids+i), t);
		g1.cmd("0,\t0");
		treeAge = *(grids+i);
		if (treeAge >= burnPropAge and treeAge<rockAge) {
			g1.cmd("%d,\t%d", myX, myY);
			};
		i++;
		};
	g1.cmd("e");
	//
	//// Burning Fires:
	i = 0;
	while (i < imax) {
		myY = i/(X+2);
		myX = i-myY*(X+2);
		treeAge=0;
		g1.cmd("0,\t0");
		//treeAge = getGridStatus(*(grids+i), t);
		treeAge = *(grids+i);
		if (treeAge<=-1 and treeAge>-burnPropAge) {
			g1.cmd("%d,\t%d", myX, myY);
			};
		i++;
		};
	g1.cmd("e");
	i = 0;
	//
	while (i < imax) {
		myY = i/(X+2);
		myX = i-myY*(X+2);
		treeAge=0;
		g1.cmd("0,\t0");
		//treeAge = getGridStatus(*(grids+i), t);
		treeAge = *(grids+i);
		if (treeAge <= -burnPropAge) {
			g1.cmd("%d,\t%d", myX, myY);
			};
		i++;
		};
	g1.cmd("e");

	//

	// Rocks:
	i = 0;
	while (i < imax) {
		myY = i/(X+2);
		myX = i-myY*(X+2);
		treeAge=0;
		g1.cmd("0,\t0");
		//treeAge = getGridStatus(*(grids+i), t);
		treeAge = *(grids+i);
		if (treeAge >= rockAge) {
			g1.cmd("%d,\t%d", myX, myY);
			};
		i++;
		};
	g1.cmd("e");

//
	yodapause();

	return 0;
	};
	
int *randomSequence(int *outList, int listLen) {
	int *tempList=new int[listLen];
	for (int i=0; i<listLen; i++) {
		tempList[i]=i;
		};
	scrambleList(tempList, outList, listLen);
	
	delete [] tempList;
	return outList;
	};
int *scrambleList(int *inList, int *outList, int listLen) {
	//
	int newPos=0;
	//srand(time(NULL));
	//
	//int *outList = new int[listLen];
	for (int i=0; i<listLen; i++) {
		outList[i]=NULL;
		};
	//
	// select each element from inList, give it a new location in tempList:
	for (int i=0; i<listLen; i++) {
		newPos=rand()%listLen;
		while (outList[newPos] != NULL) {
			newPos=(newPos+1)%listLen;
			};
		outList[newPos]=inList[i];
		};
	return outList;
	};

bool testQuench(int k=1, float Pimmune=1, float kImmune=1) {
	int randImmune = rand()%RAND_MAX;
	float frandImmune = float(randImmune)/float(RAND_MAX);
	//
	// now, functional distribution of your liking:
	// kImmune may not be used...
	return frandImmune<(Pimmune/(pow(float(k),kImmune)));
	};

//int doffire(unsigned tmax=1000000, unsigned short fmatch=200, unsigned int randPlant=1, unsigned int clustPlant=0, unsigned short burnAge=1, unsigned short burnPropAge=1, int rhoRocks=0, int doSQL=1, float pImmune=0, int nnnAge=0) {
int doffire(unsigned int tmax=1000000, unsigned int fmatch=125, int doSQL=2, float pImmune=0, float kImmune=1, std::string immCode=std::string("100"), std::string statFileName=std::string("tmpffoutput.dat") ) {
	// immCode: [1]: cluster level, [2]: fire-front elements, [3]: each new element. (evaluate immunity at these times).
	//printf("test immCode: %s\n", immCode.c_str());
	//if (immCode.at(2)==std::string("1")[0] ) printf("and we are not doing perim-level immunity.\n");
	bool doImPrim=0, doImFlameFront=0, doImFireElement=0;
	if (immCode.at(2)==std::string("1")[0] ) doImPrim=1;
	if (immCode.at(1)==std::string("1")[0] ) doImFlameFront=1;
	if (immCode.at(0)==std::string("1")[0] ) doImFireElement=1;
	
	ofstream fstats;
	//
	printf("# immunity: doImPrim: %d, doImFlameFront: %d, doImFireElement: %d\n", doImPrim, doImFlameFront, doImFireElement);
	printf("# prams: tmax=%d, fmatch=%d, doSQL=%d, pImmune=%f, kImmune=%f, immCode=%s\n", tmax, fmatch, doSQL, pImmune, kImmune, immCode.c_str());
	
	int aveRho = 0, aveVol=0;
	int burnAge=1, burnPropAge=1;
	int nTreesOnGrid=0;
	
	std::string simName("ForestFire5c");
	std::string simPramComment("single-pass fire propagation/immunity evaluation");
	//
	unsigned short rFire = 1;
	unsigned int nBurning = 1;
	unsigned short dnBurning = 1;
	int thisXY, xFire, yFire;
//	int thisStatus;
	int * thisSquare, * fireSquare;
	unsigned int nBurnedTotal=0, nFires=0;  // number of elements burning, new Elements burning...
	//float aveTreeAge=0;
	//signed int grids[(ymax+2)*(xmax+2)];
	int *grids = new int[(ymax+2)*(xmax+2)];
	//int *directions = new int[4];
	//*directions={1,-1, xmax+2, -(xmax+2)};
	int directions[4]={1,-1, xmax+2, -(xmax+2)};
	//int randDirs[4]={1,-1, xmax+2, -(xmax+2)};	// new array to be randomized
	int *newOrder = new int[4];
	// v9:
	int *fireList = new int[(ymax)*(xmax)];
	unsigned int *fcounts = new unsigned int[(ymax)*(xmax)];
	unsigned short newFireElements=0;		// [x, x, x, x, x, x[nBurning], x, x, x, x[dnBurning], x, x, x, x[newFireElements] ]: old-fire, current flame-front, new elements.
	unsigned int totalFireCount=0;
        //
        // dimension tracking bits:
        unsigned int *meann = new unsigned int[ymax*xmax];
        unsigned int *meannsq = new unsigned int[ymax*xmax];
        unsigned int fclustern = 0;     // number of iterations (circumferences) of cluster (radius).
	//
	// and now, gather explicit cluster count statistics and extinguished fire statistics (and a cluster-finder object):
	unsigned int *extinguishedFires = new unsigned int[(ymax)*(xmax)];
	int *clusterSizes = new int[(ymax)*(xmax)];
	ClusterFinder finder(IsOccupied, signed(xmax+2), signed(ymax+2), clusterSizes, grids);
	
	//
	bool doQuench = 0;
	//
	float frandImmune=0;
	int randImmune = 0;
	//
	//unsigned int rhoRocks = 10;	// of 100
	// initialize?
	srand(time(NULL));	// eventually, we have to be smarter about this and use independent random sets. see Gleb's random number gens.
	for (unsigned int i=0; i<(xmax+2)*(ymax+2); i++) {
		grids[i]=0;
		// seed the grid with some trees
		if (rand()%(xmax/2)==1 and i%(xmax+2)!=0 and i%(xmax+2)!=(xmax+1) and i>(xmax+2) and i<((xmax+2)*(ymax+1)) ) {
			grids[i]=1;
			nTreesOnGrid ++;
			};
		//if (i>(xmax+2)*(ymax+1)) printf("initializing grid %d\n", i);
		//yodapause();
		};
	//printf("random grid established...\n");
	//printGrid(&grids[0], 1, xmax, ymax);
	//
	
	//int intKey;
		
	//unsigned int fcounts[xmax*ymax];
		for (unsigned int i=0; i<xmax*ymax; i++) {
			//fcounts[i]=0;
			//fireList[i]=0;
			*(fcounts + i)=0;
			*(fireList + i)=0;
			//
			*(extinguishedFires+i)=0;
			*(clusterSizes+i)=0;
                        //
                        *(meann+i)=0;
                        *(meannsq+i)=0;
			};
	
	//
	//printf("beginnn.\n");
	//for (unsigned int i=0; i<=tmax; i++) {
	unsigned int i=0;		// just so other bits don't break...
	//while (totalFireCount<=tmax) {
	while (fcounts[99]<=tmax){
		i++;		// this might overflow i guess. if so... figure out something.
		//if (doSQL==5 or doSQL==6) if(i%1000000 == 0) printf("%d million\n", i/1000000);
		if (doSQL==6) if(i%1000000 == 0) printf("%d million\n", i/1000000);

		if (doSQL==16) {
			if(i%100000 == 0) {
				// printf("%d million\n", i/1000000);
				aveRho=0;
				aveVol=0;
				for (unsigned int k=0; k<(xmax+2)*(ymax+2); k++) {
					if (*(grids+k)>0 and *(grids+k)<rockAge) {
						aveRho = aveRho + 1;
						aveVol = aveVol + *(grids+k);
						};
					};
				//printf("mils,\t Rho,\t Vol,\t %d, %f, %f\n", i/1000000, float(aveRho)/float(xmax*ymax), float(aveVol)/float(xmax*ymax));
				printf("%d, %f, %f\n", i/1000000, float(aveRho)/float(xmax*ymax), float(aveVol)/float(xmax*ymax));
				};
			};
		//
		// PLANT A TREE:
		// select a grid for tree planting:
			//thisX = rand()%xmax+1;
			//thisY = rand()%ymax+1;
			//thisSquare = &grids[0] + thisX + (xmax+2)*thisY;	// point to the current square...
			// for speed, select one random number; if we land on a rock, then just skip. there aren't many rocks.
			thisXY = xmax+3 + rand()%((xmax+2)*ymax-1);
			thisSquare = grids + thisXY;
			// if (*thisSquare<rockAge) {
			if (*thisSquare==0) {
				*thisSquare = *thisSquare + 1;
				nTreesOnGrid ++;
				};

	// we've planted a tree. do we throw a match?
		// we can do this two ways. we can trow a match every M steps or set a 
		// 1/M probability every time.
		// for now, we're being super simple; throw a match every 'fmatch' steps:
		// throw a match with 1/fmatch probability. generate a random number beteen 1 and fmatch; each number appears with freq. 1/fmatch
		//ffmatch = float(i+1)/float(fmatch);
		//if (float(int(ffmatch))==ffmatch and i>xmax) {
		//if (rand()%((1+prefPlant)*fmatch)==1) {
		
		// check grid stats (but we only need to count the grid so many times. we seem to be running over the max integer size):
		if (rand()%(5*fmatch)==1 and clusterSizes[10]<pow(10,7)) {
			//printf("find cluster sizes...\n");
			finder.find();
			//printf("**found cluster sizes... %d, %d\n", totalFireCount, i);
			}
		
		
		if (rand()%fmatch==1) {
			//yodapause();
			// throw a match.
			xFire = rand()%xmax+1;
			yFire = rand()%ymax+1;
			fireSquare = &grids[0] + xFire + (xmax+2)*yFire;
			//printf("match: (%d, %d) :: %d/%d\n", xFire, yFire, *fireSquare, *(&grids[0] + (xFire + (xmax+2)*yFire)));
			//yodapause();
			//
			//printf("now evaluate *fireSquare, etc. %d\n", *fireSquare);
			//if (getGridStatus(*fireSquare, i) >= burnAge and getGridStatus(*fireSquare, i) < rockAge) {
			if (*fireSquare >= burnAge and *fireSquare < rockAge) {				
				// initiate a new fire.
				// now, we have three places to test quench-immunity:
				// 1) after each new step of front propagation
				// 2) after each burning element is tested against its NN
				// 3) after each element is added.
				// when the sequence of testing is fully randomized, we can test at any level.
				//
				// use the list-method (see v9a comments at the top) to propagate the fire.
				//printf("set 0-grid...\n");
				//*fireSquare = -(*fireSquare);	// we could remove the square; we track the fire in a list (=0), but we want to be able to plot the grid...
				//*fireSquare=0;
				//printf("set fireSquare... %d, %d, %d: %d\n",xFire, yFire, xmax, (xFire + (xmax+2)*yFire));
				totalFireCount++;
				*fireSquare=-1;
				fireList[0]=(xFire + (xmax+2)*yFire);	// or should we use an array of addresses?
                                fclustern=1; // tracking cluster dimension.
                                 //
				//printf("set fireSquare done\n");
				//
				rFire = 1;
				dnBurning = 1;
				nBurning = 0;
				newFireElements=0;
				//
				//int yFireMin = int(yodacode::greaterOf((yFire-rFire), 1));	// we always start with a 1 squar boundary. we might, however, encounter the edges.
				//int yFireMax = int(yodacode::lesserOf((yFire+rFire), float(ymax)));
				//int xFireMin = int(yodacode::greaterOf(float(xFire-rFire), 1));
				//int xFireMax = int(yodacode::lesserOf(float(xFire+rFire), float(xmax)));
				//
				//printf("fire range: %d, %d, %d, %d\n", yFireMin, yFireMax, xFireMin, xFireMax);
				//printf("preplot\n.");
				//
				//plotGrid(&grids[0],i, xmax, ymax);
				if (doSQL==0) {
					printGrid(&grids[0], i, xmax, ymax);
					};
				//while (dnBurning > 0) {
				doQuench = 0;
				//nFireSteps=0;	// steps fire has propagated, max dist/radius from flash-point. effectively time it's been burning.
				//printf("fire started at %d. now propagate.\n", xFire + (xmax+2)*yFire);
				while (dnBurning > 0 and doQuench==0) {
					//dnBurning=0;
					// evaluate neighbors of current burn front (nBurning-1 < i < (nBurning-1)+dnBurning
					unsigned int fireIndex=nBurning;
					//unsigned int newFireIndex=0;
					unsigned int currentElementIndex;
					//printf("fire front burning. fireIndex=%d\n", fireIndex);
					int * randomFireSequence = new int[dnBurning];
					int * testSeq = new int[dnBurning];
					for (int itest=0; itest<dnBurning; itest++) {
						*(testSeq + itest) = itest;
						}
					scrambleList(testSeq, randomFireSequence, dnBurning);
					delete [] testSeq;
					//if (dnBurning>1) randomSequence(randomFireSequence, dnBurning);	// randomize order in which we evalueate fire propagation.
					//for (int ilist=0; ilist<dnBurning; ilist++) {
					//	printf("randomSequence[%d]=%d\n", ilist, randomFireSequence[ilist]);
					//	};
					//
					while (fireIndex < (nBurning+dnBurning) and doQuench==0) {
						//newFireIndex=fireIndex-nBurning;
						currentElementIndex=nBurning+randomFireSequence[fireIndex-nBurning];	// this is long-winded because i'm building it off the simpler non-randomized model.
						//printf("fire index: %d (%d, %d, %d, %d)\n", fireIndex-nBurning, fireIndex, nBurning, dnBurning, currentElementIndex);
						//printf("failing here? %d, %d, %d\n", nBurning, dnBurning, fireIndex-nBurning);
					
						//currentElementIndex=nBurning+randomFireSequence[fireIndex-nBurning];
					//	printf("nope. ");
						
						
						// check for fire:
						// to maintain symmetry, randomize directions:
						// randDirs
						//scrambleList(directions, randDirs, 4);
						//int ordr[4]={0,1,2,3};
						//int newOrder[4];
						randomSequence(newOrder,4);
						//scrambleList(ordr, newOrder, 4);	// i think this is killing the random number generator.
						//printf("newOrder: %d, %d, %d, %d\n" , newOrder[0], newOrder[1],newOrder[2],newOrder[3]);
						
						for (int idir=0; idir<4; idir++) {
							// note: this loop format may facilitate randomization of direction later on...
							// nothing randomized:
							/*
							// I.
							if (grids[fireList[fireIndex] + directions[idir]]==1) {
								grids[fireList[fireIndex] + directions[idir]]=-1;
								fireList[nBurning + dnBurning + newFireElements]=fireList[fireIndex]+directions[idir];
								newFireElements++;
								};
							*/
							// order of each element (NN) randomized:
							/*
							// II.
							if (grids[fireList[fireIndex] + directions[newOrder[idir]]]==1) {
								grids[fireList[fireIndex] + directions[newOrder[idir]]]=-1;
								fireList[nBurning + dnBurning + newFireElements]=fireList[fireIndex]+directions[newOrder[idir]];
								newFireElements++;
								};
							*/
							// III.
							// each element (NN) and order of flame-front randomized:
							//printf("currentElement: %d\n", currentElementIndex);
							//printf("grid index: %d, %d, %d\n" , fireList[currentElementIndex], directions[newOrder[idir]], newOrder[idir]);
							//printf("segmentation fault test: grid-val (%d)\n", grids[fireList[currentElementIndex] + directions[newOrder[idir]]]);
							if (grids[fireList[currentElementIndex] + directions[newOrder[idir]]]==1) {
								//
								// add an entry to fireList. at the end of fireList [nBurning+dnBurning+newFireElements], add the fire location value,
								// fireList[currentElementIndex] + direction[]
								if (doQuench==0) {
									grids[fireList[currentElementIndex] + directions[newOrder[idir]]]=-1;
									fireList[nBurning + dnBurning + newFireElements]=fireList[currentElementIndex]+directions[newOrder[idir]];
									newFireElements++;
									};
								//if (immCode[2]=="1") {
								// so, do we do this before or after we evaluate the element? by doing this after we propagate the first step (so
								// fires are always k>1; alternatively we could use 1/(k+1)^p), we can use Pimmune>1 .
								if (doImFireElement) {
									// evaluate immunity (as each new element burns):
									//if (testQuench((nBurning+dnBurning+newFireElements), pImmune, kImmune)) {
									randImmune = rand()%RAND_MAX;
									frandImmune = float(randImmune)/float(RAND_MAX);
									if (frandImmune < (pImmune/pow(float(nBurning+dnBurning+newFireElements), kImmune)) ) {
										idir=4;
										doQuench=1;
										
										//printf("fire quenched during element-propagation: %d/%d\n", nBurning+dnBurning+newFireElements, fireIndex+1);
										//printGrid(&grids[0], i, xmax, ymax);
										//continue;
										};
									};
								};
							//
							// MFI (after each element is tested to the fire). this is only allowed at this step for type III propagation (above). if we apply immunity
							// as each element is added and elements are added in some geometrical sequence (aka, around the fire-front), we break the symmetry and break
							// SOC between integer values of L^2. by itself, using the list method (in particular when we randomize direction) might fix this problem,
							// since it breakes down the spiral geometry of our former concentric square propagation.
								
							};
						fireIndex++;

                                                // evaluate immunity (as each burning (fire-front) element propagates; aka, each new element has propagated to all its NN):
						//if (testQuench((nBurning+dnBurning+newFireElements), pImmune, kImmune)) {
                                                //if (immCode[1]=="1") {
						if (doImFlameFront) {
							randImmune = rand()%RAND_MAX;
							frandImmune = float(randImmune)/float(RAND_MAX);
							if (frandImmune < (pImmune/pow(float(nBurning+dnBurning+newFireElements), kImmune)) ) {
								//printf("quenched a fire at k=%d, Pq=%f/%f\n", (nBurning+dnBurning+newFireElements), pImmune/pow((nBurning+dnBurning+newFireElements), kImmune), frandImmune);
								doQuench=1;
								};
							};
					 	//
						//
						
						};
					// this round of propagation is over (new elements have propagated to NN).
					nBurning = nBurning + dnBurning;
					dnBurning=newFireElements;
					newFireElements=0;
                                        fclustern++;        // new "radius" for dimension tracking.
					delete [] randomFireSequence;
					//
					// evaluate immunity (after each full propagation step):
					//if (immCode[0]=="1") {
					if (doImPrim) {
						// at this point, the first step of propagation has occurred, so nominally we can use pImmune>1.
						// arguably, this creates a new characteristic size; maybe the omori-type immunity is a better idea?
						//if (testQuench((nBurning+dnBurning+newFireElements), pImmune, kImmune)) {
						randImmune = rand()%RAND_MAX;
						frandImmune = float(randImmune)/float(RAND_MAX);
						if (frandImmune < (pImmune/pow(float(nBurning+dnBurning+newFireElements), kImmune)) ) {
						//if (frandImmune < (pImmune/(1+float(nBurning+dnBurning+newFireElements)/kImmune) ) ){
							//idir=4;
							doQuench=1;
							};
						};
					//printGrid(&grids[0], i, xmax, ymax);
					// g1.plot_xy(vfireX, vfireY, "");
					};	// end fire still burining
				//
				// fire is over:
				nFires++;
				nBurnedTotal = nBurnedTotal + nBurning;
				nTreesOnGrid = nTreesOnGrid-nBurning;
				//printf("fire over; %d burned.\n", nBurning);
				//fcounts[nBurning-1]++;
				fcounts[nBurning + dnBurning + newFireElements-1]++;	//note, if we quench, we must count dnBurning. if we burn through the whole cluster, dnBurning=0.
                                // and dimension tracking:
                                meann[nBurning + dnBurning + newFireElements-1]+=fclustern;
                                meannsq[nBurning + dnBurning + newFireElements-1]+=fclustern*fclustern;
				//
				if (doQuench==1) extinguishedFires[nBurning + dnBurning + newFireElements-1]++;
				//
				// write to file:
                                //if (nFires%1000000==0) {
                                if (nFires%1000==0) {
					fstats.open(statFileName.c_str());
					fstats << "# xmax:\t" << xmax << "\tymax:\t" << ymax << "\n";
					fstats << "# immunity: doImPrim: " << doImPrim << ", doImFlameFront: " << doImFlameFront << ", doImElem: " << doImFireElement <<"\n";
					fstats << "# prams: tmax=" << tmax << ", fmatch=" << fmatch << ", doSQL=" << doSQL << ", pImmune=" << pImmune << ", kImmune=" << kImmune << ", immCode=" << immCode.c_str() << "\n";
					for (unsigned int i=0; i<(xmax*ymax); i++) {
						if (fcounts[i]!=0 or extinguishedFires[i]!=0 or clusterSizes[i]!=0) {
                                                    //fstats << i+1 << "\t" << fcounts[i] << "\t" << extinguishedFires[i] << "\t" << clusterSizes[i] << "\n";
                                                    fstats << i+1 << "\t" << fcounts[i] << "\t" << extinguishedFires[i] << "\t" << clusterSizes[i] <<"\t" << meann[i] <<"\t"<< meannsq[i]<<"\n";
							};
						};
					fstats.close();
					};
				////	
				// printf("fcounts[%d]: %d\n", nBurning-1, fcounts[nBurning-1]);
				//plotGrid(&grids[0], xmax, ymax);
				//printGrid(&grids[0], i, xmax, ymax);
				if (doSQL==0) {
					printGrid(&grids[0], i, xmax, ymax);
					};
				// write fire to MySQL:
				
				if (doSQL==3) {
					printf("fire at time %d\n", i);
					if (nBurning>=xmax/5) plotGrid (&grids[0], i, xmax, ymax, burnPropAge);
					};
				if (doSQL==4) {
					if (nBurning>=xmax/5) plotGridImg (&grids[0], i, xmax, ymax, burnPropAge);
					};
				//
				// fires finished burning; extinguish:
				unsigned int icleanup=0;
				while (fireList[icleanup]!=0) {
				//while (icleanup<nBurning+dnBurning+newFireElements){
				//while (icleanup<(xmax*ymax)){
					grids[fireList[icleanup]]=0;
					fireList[icleanup]=0;
					icleanup++;
					};
				nBurning = 0;
				dnBurning=0;
				newFireElements=0;
				//printGrid(&grids[0], i, xmax, ymax);
				//fireIndex=0;
				}; //else printf("no tree at match point.\n");	// if match -> tree...
			}; // if match-time

		// do we initialize with 0?
		//printf("ary element 0,i: %d", grids[0][i]);
		};	// end sim-steps.
	//printf("sim finished.\n");
	// end simulation.
	//printf("doSQL: %d\n", doSQL);
	// doSQL's:
	// 0: 'print-grid" fires
	// 1: full SQL: insert each forest fire data-set into ForestFires
	// 2: print summary to screen. use this for direct gnuplot calls, " plot '<./ffire4...'" 
	// 3: Print each fire to screen; "plotGrid" each fire nBurning>(25) print summary to screen at end
	// 4: plotGridImg each fire nBurning>(20); print summary to screen,
	// 5: SQL: insert just summary data to SQL; prints progress by million (will screw up plotting)
	// 6: report summary, progress by million.
	// 11: return to standard-output the last grid in full. use to make an image of the final grid.
	//
	// for super long runs (10^9 steps), it looks like the mysql connection times out, so all is lost.
	// renew the connection. 
	//

	// end-o-run summaries (print):
	
	//printf("xmax*ymax = %d; %d, %d\n", xmax*ymax, ymax, xmax);
	if (doSQL==2  or doSQL==25 or doSQL==3 or doSQL==4 or doSQL==6) {
		//printf("dosql=2 or something. we should get a summary.");
		for (unsigned int i=0; i<(xmax*ymax); i++) {
			//if (fcounts[i]!=0) {
			if (fcounts[i]!=0 or extinguishedFires[i]!=0 or clusterSizes[i]!=0) {
				//printf("%d\t%d\n", i+1, fcounts[i]);
				printf("%d\t%d\t%d\t%d\n", i+1, fcounts[i], extinguishedFires[i], clusterSizes[i]);
				//printf("%d\t%d\n", i+1, i);
				//printf("%d,\t%d\n", i+1, fcounts[i]);
				}
			else {
				//printf("finished.\nfire size, nFires, totalBurned: (%d) (%d) (%d)\n", nBurning, nFires, nBurnedTotal);
				};
			};
		};
	fstats.open(statFileName.c_str());
        fstats << "# xmax:\t" << xmax << "\tymax:\t" << ymax << "\n";
	fstats << "# immunity: doImPrim: " << doImPrim << ", doImFlameFront: " << doImFlameFront << ", doImElem: " << doImFireElement <<"\n";
	fstats << "# prams: tmax=" << tmax << ", fmatch=" << fmatch << ", doSQL=" << doSQL << ", pImmune=" << pImmune << ", kImmune=" << kImmune << ", immCode=" << immCode.c_str() << "\n";
	for (unsigned int i=0; i<(xmax*ymax); i++) {
		if (fcounts[i]!=0 or extinguishedFires[i]!=0 or clusterSizes[i]!=0) {
                    //fstats << i+1 << "\t" << fcounts[i] << "\t" << extinguishedFires[i] << "\t" << clusterSizes[i] << "\n";
                    fstats << i+1 << "\t" << fcounts[i] << "\t" << extinguishedFires[i] << "\t" << clusterSizes[i] <<"\t" << meann[i] <<"\t"<< meannsq[i]<<"\n";
                    };
		};
	fstats.close();
					
	//printf("moving past dosql5,25\n");
	// return the final grid in full and give an average density at the end...?
	if (doSQL==11) {
		for (unsigned int i=0; i<(xmax+2)*(ymax+2); i++) {
			printf ("%d,\t%d,\t%d\n", i-int(i/(xmax+2))*(xmax+2), i/(xmax+2), getGridStatus(*(grids+i), tmax));
			};
		};
	//g1.reset_plot();
	//g1.plot_xyz(vx, vy, vGridStat, "xyz plot of TreesPlanted");

	//yodacode::yodapause();
	//
	// clean up memory?
	delete [] grids;
	delete [] fireList;
	delete [] fcounts;
	//delete [] directions;
	return 0;
	//return &grids[0];
	};


int main(int argc, char *argv[]) {
	//ffPlots();
	unsigned int tmax=1000000;
	int doSQL = 4;
	unsigned int fmatch=200;
//	unsigned int randPlant = 1;
//	unsigned int clustPlant = 0;
//	unsigned int burnAge = 1;
//	unsigned int burnPropAge=5;
//	unsigned int rhoRocks = 0;
	float pImmune = 0;
	float kImmune = 5;	// a parameter to use on k for immunity. aka, r=p/(1+k/kImmune) or r=p*k^(kImmune)
	std::string immCode="100";
	std::string foutputname="ffier9b-temp.dat";
	//char immCode[]="100"
//	int nnnAge = 0;
	//
	if (argv[1]) tmax=yodacode::StrToInt(argv[1]);
	if (argv[2]) doSQL=yodacode::StrToInt(argv[2]);
	if (argv[3]) fmatch=yodacode::StrToInt(argv[3]);
//	if (argv[4]) randPlant=yodacode::StrToInt(argv[4]);
//	if (argv[5]) clustPlant=yodacode::StrToInt(argv[5]);
//	if (argv[5]) burnAge=yodacode::StrToInt(argv[5]);
//	if (argv[7]) burnPropAge=yodacode::StrToInt(argv[7]);
//	if (argv[8]) rhoRocks=yodacode::StrToInt(argv[8]);
	if (argv[4]) pImmune = yodacode::StrToFloat(argv[4]);
	if (argv[5]) kImmune = yodacode::StrToFloat(argv[5]);
	if (argv[6]) immCode = argv[6];
	if (argv[7]) foutputname = argv[7];
	
	//
	//printf("args: %d, %d, %d, %d, %d, %d, %d\n", tmax, fmatch, randPlant, clustPlant, burnAge, burnPropAge, doSQL);
	//yodapause();
	//doffire(300000,1,2000,3);
	//doffire(tmax, fmatch, randPlant, clustPlant, burnAge, burnPropAge, rhoRocks, doSQL, pImmune, nnnAge);
	doffire(tmax, fmatch, doSQL, pImmune, kImmune, immCode, foutputname);
	//printf("returned.\n");
	
	//int littleArray[6]={0,1,2,3,4,5};
	//int littleScramble[6];
	//scrambleList(littleArray, littleScramble, 6);
	//for (int i=0; i<6; i++) {
	//	printf("little and scramble: %d, %d\n", littleArray[i], littleScramble[i]);
	//	};
		
	
	return 0;
	};

