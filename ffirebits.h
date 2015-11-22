

#ifndef FFIREBITS_H
#define FFIREBITS_H

#include "/home/myoder/Documents/Source/yodacode.h"
#include "glebsutils.h"
// mysql elements:
#include <mysql.h>
#include <mysql++.h>

#include "gnuplot_i.hpp"
#include "glebsutils.h"



const int bigScale = 1;
const int xmax0 = 256;
const int ymax0 = 256;
const int xmax = bigScale*xmax0;
const int ymax = bigScale*ymax0;
const int rockAge = 123456789;
const int sheepAge = rockAge + 1;

int getGridStatus(int gridVal, unsigned int t) {
	int treeAge=0;
	//if (gridVal>0) treeAge = t-gridVal;
	//if (gridVal<0) treeAge = -t-gridVal;
	treeAge = gridVal;
	//
	return treeAge;
	};

int yodapause() {
	yodacode::yodapause();
	return 0;
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
		if (treeAge==rockAge) {
			printf(" XxX ");
			}; 
		if (treeAge==sheepAge) {
			printf(" SsS ");
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


int getClustMaps1(int *bigClustMaps) {
	//
	// define the cluster substrate:
	//	N		k		P(k)		P(k)norm.	L		N*L		#rows
	//	1		1024	1024		0.0156		32		32			0.13
	//	8		256	2048		0.0313		16		128		0.5
	//	64		64		4096		0.0625		8		512		2
	//	512	16		8192		0.1250		4		2048		8
	//	4096	4		16384		0.2500		2		8192		32
	// 32768	1		32768		0.5000		1		32768		128
	//
	// note: xmax, ymax, bigscale are declared as global constants.
	int xmax1=xmax/bigScale, ymax1=ymax/bigScale;
	int cgSize = (ymax1+2)*(xmax1+2);
	int *clustGrids = new int[cgSize];	// primary map of sub-clusters;
	// initialize.
	for (int i=0; i<cgSize; i++) {
		*(clustGrids+i) = 0;
		};
	////////////////////////////
	// now, set up clusters. i think we might as well do this semi-manually. we have square clusters, so this should not be too bad.
	int thisClust = 1;
	// note: 2L = xmax;
	//
	// try the sim without the size 1 clusters; use two copies of the larger half...
	int dpos = 64;
	for (unsigned short i2 = 0; i2<=3; i2++) {
	// start with size-1 clusters:
	/*
		for (unsigned int iy=0; iy < 128; iy++) {
			for (unsigned int ix=0; ix < xmax1; ix++) {
				// //*clustGrids[(xmax+2)*(iy+1)+ix+1]=thisClust + ix/(xmax/pow(2,clustOrder)); // note: integer division
				//*clustGrids[(xmax1+2)*(iy+1)+ix+1]=thisClust;
				//*(clustGrids +(xmax+2)*(iy+1)+ix+1)=thisClust + ix/(xmax/pow(2,clustOrder)); // note: integer division
				*(clustGrids + (xmax1+2)*(iy+1)+ix+1)=thisClust;

				thisClust++;
				//printf("%d, ", thisClust);
				};
			};
	*/
		//
	/*
		// size 4, L=2 clusters
		for (unsigned int iy=128-i2*dpos; iy < 192-i2*dpos; iy++) {
			for (unsigned int ix=0; ix < xmax1; ix++) {
				//clustGrids[(xmax1+2)*(iy+1)+ix+1]=thisClust + ix/2;
				*(clustGrids +(xmax1+2)*(iy+1)+ix+1)=thisClust + ix/2;
				// thisClust = thisClust + 1;
				};
				if ( (iy+1)%2 == 0) thisClust = thisClust + xmax1/2;
			};
		//thisClust = thisClust + 4096;	// we could code this incrementation in the loop if we like...
	*/
		//
		// size 16, L=4 clusters
		for (int iy=192-i2*dpos; iy < 224-i2*dpos; iy++) {
			for (int ix=0; ix < xmax1; ix++) {
				//clustGrids[(xmax+2)*(iy+1)+ix+1]=thisClust + int(xmax*(iy-192)/4) + int(ix/4);
				//clustGrids[(xmax1+2)*(iy+1)+ix+1]=thisClust + ix/4;
				*(clustGrids + (xmax1+2)*(iy+1)+ix+1)=thisClust + ix/4;
				};
				if ( (iy+1)%4 == 0) thisClust = thisClust + xmax1/4;
			};
		//thisClust = thisClust + 512;	// we could code this incrementation in the loop if we like...
		//
		// size 64, L=8 clusters
		for (int iy=224-i2*dpos; iy < 240-i2*dpos; iy++) {
			for (int ix=0; ix < xmax1; ix++) {
				*(clustGrids + (xmax1+2)*(iy+1)+ix+1)=thisClust + int(ix/8);
				};
				if ( (iy+1)%8 == 0) thisClust = thisClust + xmax1/8;
			};
		thisClust = thisClust + 64;	// we could code this incrementation in the loop if we like...
		//
		// size 256, L=16
		for (int iy=240-i2*dpos; iy < 256-i2*dpos; iy++) {
			for (int ix=0; ix < 128; ix++) {
				*(clustGrids + (xmax1+2)*(iy+1)+ix+1)=thisClust + int(ix/16);
				};
				//if ( (iy+1)%16 == 0) thisClust = thisClust + xmax1/16;
			};
		thisClust = thisClust + 8;
		//
		// size 1024, L=32
		for (int iy=240-i2*dpos; iy < 256-i2*dpos; iy++) {
			for (int ix=128; ix < 192; ix++) {
				*(clustGrids + (xmax1+2)*(iy+1)+ix+1)=thisClust;
				};
			};
		thisClust = thisClust + 1;
		};
		//
	
	//printf("now map to the big cluster.\n");
	// now, map this to the large grid. we've chose 1->4 mapping for the time being. scan the cluster-map; expand each pixel to a 4x4 on the new grid.
	for (int i=1; i<(ymax1+1); i++) {
		for (int ii = 1; ii<=bigScale; ii++) {
			for (int iii=1; iii<(xmax1+1); iii++) {
				for (int iiii = 1; iiii<=bigScale; iiii++) {
					// write gridClusts[ii, i] 4 times
					//bigClustMaps[(bigScale*xmax+2)*(bigScale*(i-1)+ii) + (bigScale*(iii-1)+iiii)] = clustGrids[i*(xmax+2) + iii];
					//*(bigClustMaps + (bigScale*xmax+2)*(bigScale*(i-1)+ii) + (bigScale*(iii-1)+iiii)) = clustGrids[i*(xmax+2) + iii];

					//*(bigClustMaps + (xmax+2)*(bigScale*(i-1)+ii) + (bigScale*(iii-1)+iiii)) = clustGrids[i*(xmax1+2) + iii];
					*(bigClustMaps + (xmax+2)*(bigScale*(i-1)+ii) + (bigScale*(iii-1)+iiii)) = *(clustGrids + i*(xmax1+2) + iii);
					};
				};
			};
		};
	int biggestFire = (bigScale*32)*(bigScale*32);

	//printf("cluster map set.\n");
	//plotGridImg (clustGrids, 0, xmax/bigScale, ymax/bigScale, 1);
	//plotGridImg (bigClustMaps, 0, xmax, ymax, 1);

	delete clustGrids;

	return biggestFire;	
	};

int hasNeighbor (int *grids, int gridPos) {
	// look for rectilinear neighbors in grids around gridPos:
	int uStat, dStat, lStat, rStat;
	int nNeighbors=0;
	//
	uStat = *(grids + gridPos + (xmax+2));
	dStat = *(grids + gridPos - (xmax+2));
	lStat = *(grids + gridPos - 1);
	rStat = *(grids + gridPos + 1);
	//
	if (uStat!=0 and uStat<rockAge) nNeighbors++;
	if (dStat!=0 and uStat<rockAge) nNeighbors++;
	if (lStat!=0 and uStat<rockAge) nNeighbors++;
	if (rStat!=0 and uStat<rockAge) nNeighbors++;
	//
	return nNeighbors;
	};


int getRandDir (int gridSize, Rand *myRand) {
	int thisDir = myRand->nextInt(4);
	int dirVal = 0;
	switch (thisDir) {
		case 0:
			dirVal = 1;
			break;
		case 1:
			dirVal = -1;
			break;	
		case 2:
			dirVal = gridSize;
			break;
		case 3:
			dirVal = -gridSize;
			break;
		};
	return dirVal;
	};


int getSheepDir (int sheepPos, int *theGrid, int gridX, Rand *thisRand) {
	// smart sheep will move toward something to eat if it is adjacent.
	// eventually, we might re-introduce tree age; sheep can only eat young trees and
	// must move around old trees.
	// do fires kill sheep?
	//
	int foodDirections[4] = {0,0,0,0};
	int nFoodDirs=0, sheepDir=0;
	//
	if (*(theGrid+1)>=1) {
		foodDirections[nFoodDirs]=1;
		nFoodDirs++;
		};
	if (*(theGrid-1)>=1) {
		foodDirections[nFoodDirs]=-1;
		nFoodDirs++;
		};
	if (*(theGrid+gridX)>=1) {
		foodDirections[nFoodDirs] = gridX;
		nFoodDirs++;
		};
	if (*(theGrid-gridX)>=1) {
		foodDirections[nFoodDirs] = -gridX;
		nFoodDirs++;
		};
	//
	if (nFoodDirs>0) {
		// there is something we can eat...
		sheepDir = foodDirections[thisRand->nextInt(nFoodDirs)];
		}
	else {
		sheepDir = getRandDir(gridX, thisRand);
		};
	//
	return sheepDir;
	};

bool inGrid (int gridPos, int gridX, int gridY) {
	// is this new position in a grid with border width/height 1?
	bool isIn=1;
	//if (gridPos<(gridX+2)) isIn=0;
	if (gridPos/(gridX+2)==0 or gridPos/(gridX+2)>=(gridY+1)) isIn=0;	// top or bottom row
	if (gridPos%(gridX+2)==0 or gridPos%(gridX+1)==0) isIn=0;
	//
	return isIn;
	};

////////////////////////////////////////////
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
		if (treeAge!=0) {
			treeAge=getGridStatus(*(grids+i), t);
			srand(treeAge);
			treeAge = rand()%16777216;
			};
	//	if (treeAge==0) treeAge=0;
	//	if (treeAge>0 and treeAge<=32768) treeAge=100;
	//	if (treeAge>32768 and treeAge<=(32768+4096)) treeAge=200;
	//	if (treeAge>(32768+4096) and treeAge<=(32768+4096+512)) treeAge=400;
	//	if (treeAge>(32768+4096+512) and treeAge<=(32768+4096+512+64)) treeAge=600;
	//	if (treeAge>(32768+4096+512+64) and treeAge<=(32768+4096+512+64+8)) treeAge=800;
	//	if (treeAge>(32768+4096+512+64+8) and treeAge<=(32768+4096+512+64+8+1)) treeAge=1000;
//
//		if (treeAge==0) treeAge=0;
//		if (treeAge>=burnPropAge) treeAge=8;
//		if (treeAge>0 and treeAge < burnPropAge) treeAge=4;
//		if (treeAge<0 and treeAge>-burnPropAge) treeAge=12;
//		if (treeAge<= -burnPropAge) treeAge=16;
//		if (i<(int(xmax)+2)) treeAge=int(i/16);	// this to get some pallet normalization...
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

int plotGridSimple(signed int * grids, unsigned int t, int X, int Y) {
	//printf("we'll plot the grid here...");
	// yodapause();
	//std::string strArray("");
	//std::string strNewRow("");
	int myX, myY, treeAge;
	Gnuplot g1=Gnuplot();
	//
	g1.cmd("set xrange[0 : %d]", X+1);
	g1.cmd("set yrange[0 : %d]", Y+1);
	//g1.cmd("plot '-' title 'young' with points 3, '-' title 'older' with points, '-' title 'young-burn' with points 7, '-' title 'old-burn' with points  9");
	g1.cmd("plot '-' title 'trees' with points 3, '-' title 'burn' with points  9, '-' title 'rocks' with points 1");
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
		if (treeAge > 0 and treeAge < rockAge) {
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
		g1.cmd("0,\t0");
		treeAge = *(grids+i);
		if (treeAge < 0) {
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
		g1.cmd("0,\t0");
		treeAge = *(grids+i);
		if (treeAge == rockAge) {
		//if (treeAge > burnAge and treeAge < burnPropAge) {
			g1.cmd("%d,\t%d", myX, myY);
			};
		i++;
		};
	g1.cmd("e");	
	//
	yodapause();
	//
	return 0;
	};
//////////////////////////
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
class ffireModel {
	// this is the body of the ffire model. it will contain all the pertinent variables
	// and the doFfireStep() function, or whatever we call it.
	// most of the varialbes won't be declared here; they will be private non-member variables.
	//
	public:
		//ffireModel(int tmax, int doSQL, int fMatch, int nRand, int nClust, int kmax, int sheepSpeed, int nSheep);
		ffireModel();
		void init(int doSQL, int fMatch, int nRand, int nClust, int kmax, int sheepSpeed, int nSheep);
		//
		void doFFstep();
		int * getfCounts();
		int * getSheepPoops();
		int * getGrids();
		//
		int getSheepPos();
		int getTimeStep();
		int getNtrees();
		int getThisSheepPoop();
		int getBiggestFire();
		int gettmax();
		int getnEvents();
		//
		void setTimeStep(int val);
		void setSimName(std::string strName);
		void setSimPramComment(std::string strComment);
		void settmax(int thisTmax);
		//
		void doNsteps(int nSteps);
		void doNevents(int nevents);
		void doNsizeOne(int nevents);
		//
		/*
		int plotGrid(signed int * grids, unsigned int t, int X, int Y, int burnPropAge);
		int plotGridImg(signed int * grids, unsigned int t, int X, int Y, int burnPropAge);
		int plotGridSimple(signed int * grids, unsigned int t, int X, int Y);
		*/
		//
		void setSQLPrams(std::string sDB, std::string sUsr, std::string sPwrd, std::string sHost);
	private:
		int timeStep;
		int *grids;		// we either make this public, allow access, or really just make it a pointer to a pre-allocated int pointer.
		int tmax, doSQL, fMatch, nRand, nClust, kmax, sheepSpeed, nSheep, nEvents;
		int nTrees, thisSheepPoop, newSheepPoop;			// total number of trees on grid, current accumulating sheep-poop cluster, new SheepPoop?
		int biggestFire;
		int *fCounts, *sheepPoops;
		int sheepPos, nBurnedTotal, nFires, dNburning;
		//int rFire;		// range of fire; this will almost always be 1
		//int nBurning, dnBurning;
		int thisX, thisY, xfire, yfire;	// plant/spark positions
		int *fireSquare;
		//
		int burnPropAge;
		//
	
		int intKey;
		float aveRho;
		int gridPos;
		//
		// random number generators and elements:
		int rSeed, drSeed;	// set a seed value and increment for each new random number generator.
		Rand plantRandx;
		Rand plantRandy;
		Rand fireRandx;
		Rand fireRandy;
		Rand rfMatch;
		Rand rcPlant;
		Rand walkDir;
		Rand sheepRand;
		Rand sheepRand2;
		Rand treeSeedx;
		Rand treeSeedy;
		//
		// mysql elements:
		//mysqlpp::Connection myconn;
		//mysqlpp::Query myquery;
		//mysqlpp::Result res1;
		std::string sqlDB;
		std::string sqlUSER;
		std::string sqlPASSWORD;
		std::string sqlHOST;
		std::string simName;
		std::string simPramComment;
	//
	};

ffireModel::ffireModel() {
	//init(tmax, doSQL, fMatch, nRand, nClust, kmax, sheepSpeed, nSheep);
	init(0, 10, 1, 0, 4, 3, 1);
	};

void ffireModel::init(int thisdoSQL, int thisfMatch, int thisnRand, int thisnClust, int thiskmax, int thissheepSpeed, int thisnSheep) {
	timeStep=0;
	//
	tmax=0;
	doSQL=thisdoSQL;
	fMatch=thisfMatch;
		if (thisfMatch==0) fMatch=999999999;
	nRand = thisnRand;
	nClust = thisnClust;
	kmax = thiskmax;
	sheepSpeed = thissheepSpeed;
	nSheep = thisnSheep;
	
	//
	simName="ForestFire7e";
	simPramComment="";
	//
	fireSquare = NULL;
	nBurnedTotal=0;
	nEvents=0;
	nFires=0;  // number of elements burning, new Elements burning...
	biggestFire = xmax0*ymax0;
	aveRho = 0;
	//
	burnPropAge=1;
	nTrees = 0;
	thisSheepPoop = 0;
	newSheepPoop = 0;
	//
	grids = new int[(ymax+2)*(xmax+2)];
	gridPos=0;
	//
	rSeed=int(time(NULL));
	drSeed = 0;
	srand(time(NULL));	
	plantRandx.init(rSeed + drSeed);
		drSeed++;
	plantRandy.init(rSeed + drSeed);
		drSeed++;
	fireRandx.init(rSeed + drSeed);
		drSeed++;
	fireRandy.init(rSeed + drSeed);
		drSeed++;
	rfMatch.init(rSeed + drSeed);
		drSeed++;
	rcPlant.init(rSeed + drSeed);
		drSeed++;
	walkDir.init(rSeed + drSeed);
		drSeed++;
	sheepRand.init(rSeed + drSeed);
		drSeed++;
	sheepRand2.init(rSeed + drSeed);
		drSeed++;
	treeSeedx.init(rSeed + drSeed);
		drSeed++;
	treeSeedy.init(rSeed + drSeed);
		drSeed++;

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
	// and initialize the sheep position:
	sheepPos = (xmax+2)*(1+sheepRand.nextInt(ymax)) + sheepRand.nextInt(xmax) + 1;
	//
	//
	fCounts = new int[biggestFire];
	sheepPoops = new int[biggestFire];
	//int *fCountsInternal = new int[biggestFire];
	//int *sheepPoopsInternal = new int[biggestFire];
	//sheepPoops = sheepPoopsInternal;
	//fCounts = fCountsInternal;		// how exacty does C++ handle the pointer/integer array, etc.? here, we're at wasting only a pointer variable...
		for (int i=0; i<biggestFire; i++) {
			//fcounts[i]=0;
			//*(fCounts + i) = 0;
			//*(sheepPoops + i) = 0;
			fCounts[i]=0;
			sheepPoops[i]=0;
			};
	};

int * ffireModel::getfCounts() { return fCounts; };
int ffireModel::gettmax() { return tmax; };
int * ffireModel::getSheepPoops() { return sheepPoops; };
int ffireModel::getSheepPos() {return sheepPos; };
int ffireModel::getTimeStep() {return timeStep; };
int ffireModel::getNtrees() {return nTrees; };
int ffireModel::getBiggestFire() { return biggestFire; };
int ffireModel::getThisSheepPoop() {return thisSheepPoop; };
int * ffireModel::getGrids() { return grids; };
int ffireModel::getnEvents() { return nEvents; };

void ffireModel::setSimName(std::string strName) { simName = strName; };
void ffireModel::settmax(int thisTmax=0) { tmax=thisTmax; } ;

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

void ffireModel::doNsteps(int nSteps) {
	for (int isteps=0; isteps<nSteps; isteps++) {
		doFFstep();
		};
	};

void ffireModel::doNevents(int nevents) {
	while (nEvents<=nevents) {
		doFFstep();
		};
	};

void ffireModel::doNsizeOne(int nevents) {
	while (fCounts[0]<=nevents) {
		doFFstep();
		};
	};

void ffireModel::doFFstep() {
		int xFire, yFire, rFire, dnBurning, nBurning;
		timeStep++;	// note: time starts at 1...
		//
		// PLANT A TREE:
		// select a grid for tree planting:
		// yoder, v7: introduce dendritic growth. we have two prams, nRand, nClst. define a P(rand)-> Pr, P(clust)->Pc: Pr=nRand/(nRand+nClust), etc.
		//for (unsigned int irp=0; irp<randPlant; irp++) {
		thisX = plantRandx.nextInt(xmax);
		thisY = plantRandy.nextInt(ymax);
		gridPos = (xmax+2)*(thisY+1)+1+thisX;
		//printf("plant x,y: (%d, %d)\n", thisX, thisY);
		//
		if (*(grids+gridPos)==0) {
			*(grids+gridPos) = 1;
			nTrees++;		// keep track of number of trees on grid so we can make a lorenz map or something.
			};
		
	// we've planted a tree. do we throw a match?
	// a 1 in fMatch chance (use any value between 0 and fMatch)
		if (rfMatch.nextInt(fMatch) == 1 and fMatch!=0) {
			// first do the sheep:
			// note, we can remove sheep by setting sheepSpeed=0
			// use smarter sheep; sheep move to (and eat) an occupied site or at random.
			// printf("sheepSpeed: %d\n", sheepSpeed);
			// yodapause();
			//
			//printf("doing a ff step.\n");
			for (int ifsheep=0; ifsheep<fMatch; ifsheep++) {
				//for (int isheep = 0; isheep<sheepSpeed; isheep++) {
				if (sheepRand.nextInt(sheepSpeed+1)==1) {
					 //printf("doing a doFFstep step...(%d), [%d,%d::%d]\n", sheepSpeed, sheepPos/(xmax+2), sheepPos%(xmax+2), *(grids+sheepPos));
					switch (*(grids+sheepPos)) {
						case 0: case rockAge:
							// move:
							//sheepPos = sheepPos + getRandDir((xmax+2), &sheepRand);
							sheepPos = sheepPos + getSheepDir(sheepPos, grids, (xmax+2), &sheepRand);
							newSheepPoop++;
							if (newSheepPoop>1) {
								// we've finished a sheep-cluster
								if (thisSheepPoop>0) {sheepPoops[thisSheepPoop-1]++; };
								thisSheepPoop=0;
								};
							break;
						case 1:
							// eat:
							*(grids+sheepPos)=0;
							nTrees = nTrees-1;
							thisSheepPoop++;
							newSheepPoop=0;		//reset newSheepPoop...
							//yodapause();
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
					if (sheepPos%(xmax+2)==(xmax+1)) sheepPos=sheepPos-xmax;	// off right side
					}; // sheep-speed
				};

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
				//
				nEvents++;
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
//				if (doSQL==0) {
//					printGrid(grids, timeStep, xmax, ymax);
//					};
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
				nTrees = nTrees-nBurning;
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


///////////////////////////////////////////
///////////////////////////////////////////

//////////////////////////////////////////

#endif
