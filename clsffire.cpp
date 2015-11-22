/*
include "/home/myoder/Documents/Source/yodacode.h"
#include "glebsutils.h"
#include "clsffire.h"
#include "ffirebits.h"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <stdlib.h>

#include <time.h>
#include <string>
#include <vector>
#include <sstream>
*/

//ffireModel::ffireModel (int tmax, int doSQL, int fMatch, int nRand, int nClust, int kmax, int sheepSpeed, int nSheep) {
ffireModel::ffireModel() {
	//init(tmax, doSQL, fMatch, nRand, nClust, kmax, sheepSpeed, nSheep);
	init(1000, 0, 10, 1, 0, 4, 3, 1);
	};

void ffireModel::init(int tmax, int doSQL, int fMatch, int nRand, int nClust, int kmax, int sheepSpeed, int nSheep) {
	timeStep=0;
	if (fMatch==0) fMatch=tmax;
	//
	simName="ForestFire7e";
	simPramComment="";
	//
	fireSquare = NULL;
	nBurnedTotal=0;
	nFires=0;  // number of elements burning, new Elements burning...
	int biggestFire = xmax0*ymax0;
	aveRho = 0;
	//
	burnPropAge=1;
	//
	grids = new int[(ymax+2)*(xmax+2)];
	gridPos=0;
	//
	rSeed=int(time(NULL));
	drSeed = 0;
	srand(time(NULL));	
	plantRandx.init(rSeed + drSeed);
	plantRandy.init(rSeed + drSeed);
	fireRandx.init(rSeed + drSeed);
	fireRandy.init(rSeed + drSeed);
	rfMatch.init(rSeed + drSeed);
	rcPlant.init(rSeed + drSeed);
	walkDir.init(rSeed + drSeed);
	sheepRand.init(rSeed + drSeed);
	sheepRand2.init(rSeed + drSeed);
	treeSeedx.init(rSeed + drSeed);
	treeSeedy.init(rSeed + drSeed);

	// initialize the grid/cluster-grid with "rocks" around the border and in otherwise empty cells.
	// "cluster 0" will, for the time being, not participate in the simulation. just for posterity,
	// we will place "rocks" around the borders. initialize the interior of the grid with 0; we will
	// map the excluded areas via the clusters grid.
	for (int i = (xmax+2); i<(xmax+2)*(ymax+1); i++) {
		// initialize here with tree-seeds and sheep:		
		*(grids+i) = 0;
		if (treeSeedx.nextInt(xmax*xmax)<=xmax) *(grids+i)=1;
		};
	for (unsigned int i=0; i<(xmax+2); i++) {
		//grids[i] = rockAge;
		//grids[(ymax+1)*(xmax+2) + i] = rockAge;
		*(grids+i) = rockAge;
		*(grids + (ymax+1)*(xmax+2) + i) = rockAge;
		//
		};
	for (unsigned int i=0; i<(ymax+2); i++) {
		//grids[(xmax+2)*i] = rockAge;
		//grids[(xmax+2)*i + xmax+1] = rockAge;
		*(grids + (xmax+2)*i) = rockAge;
		*(grids +(xmax+2)*i + xmax+1) = rockAge;
		//
		};
	//
	//
	int *fCountsInternal = new int[biggestFire];
	int *sheepPoopsInternal = new int[biggestFire];
	sheepPoops = sheepPoopsInternal;
	fCounts = fCountsInternal;		// how exacty does C++ handle the pointer/integer array, etc.? here, we're at wasting only a pointer variable...
		for (int i=0; i<biggestFire; i++) {
			//fcounts[i]=0;
			*(fCounts + i) = 0;
			};
	};

int * ffireModel::getfCounts() {
	return fCounts;
	};
//		void doFFstep();
int * ffireModel::getSheepPoops() {
	return sheepPoops;
	};
int ffireModel::getSheepPos() {return sheepPos; };
int ffireModel::getTimeStep() {return timeStep; };
void ffireModel::setSimName(std::string strName) { simName = strName; };

int * ffireModel::getGrids() {
	return grids;
	};

void ffireModel::setSimPramComment(std::string strComment) {
	simPramComment = strComment;
	};

void ffireModel::setSQLPrams(std::string sDB, std::string sUsr, std::string sPwrd, std::string sHost) {
	sqlDB = sDB;
	sqlUSER = sUsr;
	sqlPASSWORD = sPwrd;
	sqlHOST = sHost;
	};

void ffireModel::setTimeStep(int val) {
	timeStep=val;
	};

/*
int ffireModel::plotGrid(signed int * grids, unsigned int t, int X, int Y, signed burnPropAge) {
	plotGrid(grids, t, X, Y, burnPropAge);
	};
int ffireModel::plotGridImg(signed int * grids, unsigned int t, int X, int Y, signed burnPropAge) {
	plotGridImg(grids, t, X, Y, burnPropAge);
	};
int ffireModel::plotGridSimple(signed int * grids, unsigned int t, int X, int Y) {
	plotGridSimple(grids, t, X, Y);
	};
*/

////////////////////////////////////////////
///////////////////////////////////////////
void ffireModel::doFFstep() {
		int xFire, yFire, rFire, dnBurning, nBurning;
		int aveVol, aveRho;
		timeStep++;	// note: time starts at 1...
		//
		// first do the sheep:
		// note, we can remove sheep by setting sheepSpeed=0
		// use smarter sheep; sheep move to (and eat) an occupied site or at random.
		// printf("sheepSpeed: %d\n", sheepSpeed);
		// yodapause();
		//
		for (unsigned short isheep = 0; isheep<sheepSpeed; isheep++) {
			switch (*(grids+sheepPos)) {
				case 0: case rockAge:
					// move:
					//sheepPos = sheepPos + getRandDir((xmax+2), sheepRand);
					sheepPos = sheepPos + getSheepDir(sheepPos, grids, (xmax+2), sheepRand);
					break;
				case 1:
					// eat:
					*(grids+sheepPos)=0;
					break;
				default:
					// nothing.
					break;
				};
			
			//
			// 28 april 2008 yoder:
			// put sheep on torroid gemoetry. in principle, fire and sheep differer in that a cluster's probability of
			// catching fire is proportional to area. its probability of being eaten by sheep is related to its circumference,
			// specifically, C-loops. except where we drop sheep from space. the sheep always wander...
			// note: we use a series of if statements but are careful that none maps the test value to a subsequent test range...
			if (sheepPos<(xmax+2)) sheepPos=sheepPos + ymax*(xmax+2);	// if he wanders off the top...
			if (sheepPos>((xmax+2)*(ymax+1))) sheepPos=sheepPos-ymax*(xmax+2);	// wanders off the bottom.  also, sheepPos^(xmax+2)??
			if (sheepPos%(xmax+2)==0) sheepPos=sheepPos+xmax;	// off left side
			if (sheepPos%(xmax+1)==0) sheepPos=sheepPos-xmax;	// off right side
			}; // sheep-speed

		//
		// PLANT A TREE:
		// select a grid for tree planting:
		// yoder, v7: introduce dendritic growth. we have two prams, nRand, nClst. define a P(rand)-> Pr, P(clust)->Pc: Pr=nRand/(nRand+nClust), etc.
		//for (unsigned int irp=0; irp<randPlant; irp++) {
		thisX = plantRandx.nextInt(xmax);
		thisY = plantRandy.nextInt(ymax);
		gridPos = (xmax+2)*(thisY+1)+1+thisX;
		//
		*(grids+gridPos) = 1;
		
	// we've planted a tree. do we throw a match?
	// a 1 in fMatch chance (use any value between 0 and fMatch)
		if (rfMatch.nextInt(fMatch) == 1) {
			//yodapause();
			// throw a match.
			xFire = fireRandx.nextInt(xmax) + 1;
			yFire = fireRandy.nextInt(ymax) + 1;
			//
			fireSquare = grids + (xmax+2)*yFire + xFire;
			//printf("match: (%d, %d) :: %d\n", xFire, yFire, *fireSquare);
			//yodapause();
			//
			if (*fireSquare > 0 and *fireSquare<rockAge) {
				// initiate a new fire.
				// start from the epicenter and work out in concentric rectangles (squares)
				// until there are no new fires.
				// note: we make two passes over each circle. for now, we assume all squares
				// continue to burn until the fire is over. this is a subtle consideration that
				// will not matter for simpler versions of the model, but if we introduce burn probabilities,
				// we will have to be more careful.

				*fireSquare=-(*fireSquare);	// the fire-square starts burning
				//
				rFire = 1;
				dnBurning = 1;
				//
				nBurning = dnBurning;
				int yFireMin = int(yodacode::greaterOf((yFire-rFire), 1));	// we always start with a 1 squar boundary. we might, however, encounter the edges.
				int yFireMax = int(yodacode::lesserOf((yFire+rFire), float(ymax)));
				int xFireMin = int(yodacode::greaterOf(float(xFire-rFire), 1));
				int xFireMax = int(yodacode::lesserOf(float(xFire+rFire), float(xmax)));
				if (doSQL==0) {
					printGrid(grids, timeStep, xmax, ymax);
					};
				//while (dnBurning > 0) {
				while (dnBurning > 0) {
					dnBurning=0;
					for (char doTwice = 0; doTwice <=1; doTwice++) {	// "char" is a 1 byte integer. we could also use a boolean to count to 2.
					for (int iy = (yFireMin); iy <= (yFireMax); iy++) {
						for (int ix = (xFireMin); ix <= (xFireMax); ix++) {
							// also note: Gelb is right. a recursive approach is faster, though this bredth-first appraoch is more like real fire propagation.
							// printf("try-burn: %d, %d\n", ix, iy);
							//plotGrid(&grids[0], xmax, ymax);
							//yodapause();
							//
							int * centerGrid = (grids + ix + (xmax+2)*iy);
							int cStat, uStat, dStat, lStat, rStat;
							//int ulStat, urStat, llStat, lrStat;	// diagonal elements.
							cStat = *centerGrid;
							//
							uStat = *(centerGrid + (xmax+2));
							dStat = *(centerGrid - (xmax+2));
							lStat = *(centerGrid - 1);
							rStat = *(centerGrid + 1);
							//
							// diagonal elements:
							//ulStat = getGridStatus(*(centerGrid + (xmax+2) - 1), i);
							//urStat = getGridStatus(*(centerGrid + (xmax+2) + 1), i);
							//llStat = getGridStatus(*(centerGrid - (xmax+2) - 1), i);
							//lrStat = getGridStatus(*(centerGrid - (xmax+2) + 1), i);
							//yodapause();
								//if (*centerGrid >= burnAge and (*leftGrid==-1 or *rightGrid==-1 or *upGrid==-1 or *downGrid==-1)) {
								//if (*centerGrid >= burnAge and (*leftGrid<=-burnPropAge or *rightGrid<=-burnPropAge or *upGrid<=-burnPropAge or *downGrid<=-burnPropAge)) {
								//if (cStat >= burnAge and cStat < rockAge and (lStat<=-burnPropAge or rStat<=-burnPropAge or uStat<=-burnPropAge or dStat<=-burnPropAge)) {
								//
							// no immunity:
							if (cStat >= 1 and cStat < rockAge 
								and (lStat<=-1 or rStat<=-1 or uStat<=-1 or dStat<=-1) ) {
		//						// next nearest neighbors:
		//						if (cStat >= burnAge and cStat < rockAge 
		//							and  ( (lStat<=-burnPropAge or rStat<=-burnPropAge or uStat<=-burnPropAge or dStat<=-burnPropAge)
		//								 or ( (ulStat<=-nnnAge or urStat<=-nnnAge  or llStat<=-nnnAge  or lrStat<=-nnnAge)
		//									 and (ulStat<=-burnPropAge or urStat<=-burnPropAge or llStat<=-burnPropAge  or lrStat<=-burnPropAge)
		//									 )
		//						 		)
		//							) {
								*centerGrid = -(*centerGrid);
								// age -> number of trees in that grid...
								dnBurning ++;
								//printf("[%d, %d] catches from [%d, %d]\n", ix, iy, xFire, yFire);
								};
							}; // ix
						}; // iy
						}; // doTwice	
						// plotGridSimple (grids, i, xmax, ymax);
						nBurning = nBurning + dnBurning;
						//yodapause();
					xFireMin = int(yodacode::greaterOf(1, float(xFireMin-1)));
					xFireMax = int(yodacode::lesserOf(float(xmax), xFireMax+1));
					yFireMin = int(yodacode::greaterOf(1, float(yFireMin-1)));
					yFireMax = int(yodacode::lesserOf(float(ymax), yFireMax+1));
					// g1.plot_xy(vfireX, vfireY, "");
					};	// end "while" fire still burining
				//
				nFires++;
				nBurnedTotal = nBurnedTotal + nBurning;
				fCounts[nBurning-1]++;
				//
				// fires finished burning; extinguish:
				for (int iy = (yFireMin); iy <= (yFireMax); iy++) {
					for (int ix = (xFireMin); ix <= (xFireMax); ix++) {
						// &grids[0] + ix + (xmax+2)*iy
						//if (grids[ix + (xmax+2)*iy] < 0) grids[ix + (xmax+2)*iy]=0;
						if (*(grids +ix + (xmax+2)*iy) < 0) *(grids +ix + (xmax+2)*iy)=0;
						};
					};

				}; //else printf("no tree at match point.\n");	// if match -> tree...
			}; // if match-time
		};
////////////////////////////////
///////////////////////////////



