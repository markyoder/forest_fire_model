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

#include <yodacode.h>
//#include </home/myoder/Documents/Source/glebsutils.h>

// mysql elements:
#include <mysql.h>
#include <mysql++.h>

const unsigned int xmax = 1024;
const unsigned int ymax = 1024;
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
// 5c uses immunity like: pIm/(1+k/kIm)
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

void yodapause() {
	yodacode::yodapause();
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


//int doffire(unsigned tmax=1000000, unsigned short fmatch=200, unsigned int randPlant=1, unsigned int clustPlant=0, unsigned short burnAge=1, unsigned short burnPropAge=1, int rhoRocks=0, int doSQL=1, float pImmune=0, int nnnAge=0) {
int doffire(unsigned int tmax=1000000, unsigned int fmatch=125, int doSQL=2, float pImmune=0, float kImmune=1) {
	// burn toplolgy: do we nave nnn burning
//	if (nnnAge==0) nnnAge=54321;	// trees age>nnnAge will spread to next nearest neighbors. use "0" pram for no nnn connectivity; nothing should ever get this old.
	int aveRho = 0, aveVol=0;
	int burnAge=1, burnPropAge=1;
	int nTreesOnGrid=0;
	int nFireSteps=0;	// the number of steps the fire has propagated (the radius, or more correctly the maximum distance of a burning element from the flashpoint)

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
	float aveTreeAge=0;
	//signed int grids[(ymax+2)*(xmax+2)];
	int *grids = new int[(ymax+2)*(xmax+2)];
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
		//yodapause();
		};
	//printf("random grid established...\n");
	//printGrid(&grids[0], 1, xmax, ymax);
	//
	char DB[]="ForestFire";
	char HOST[]="localhost";
	char USER[]="myoder";
	char PASSWORD[]="yoda";
	mysqlpp::Connection myconn(DB, HOST, USER, PASSWORD);
	mysqlpp::Query myquery=myconn.query();
	//mysqlpp::Result res1;
	mysqlpp::StoreQueryResult res1;
	int intKey;
	unsigned int fcounts[xmax*ymax];
		for (unsigned int i=0; i<xmax*ymax; i++) {
			fcounts[i]=0;
			};
	if (doSQL==1 or doSQL==5 or doSQL==25) {	
		//
		// insert a new row for this sim-run:
		myquery.reset();
		printf("insert simprams.\n");
		myquery << "insert into ForestFire.SimPrams (`SimName`, `xmax`, `ymax`, `sparkInterval`, `burnAge`, `tmax`, `nRand`, `pImmune`, `kImmune`, `Comment`) values (%0q, %1q, %2q, %3q, %4q, %5q, %6q, %7q, %8q, %9q )";
		myquery.parse();
		// note: simIndex(auto-int) and dtTime (default TIMESTAMP) set automatically.
		//myquery.execute(simName.c_str(), simName.c_str(), xmax, ymax, fmatch, burnAge, tmax, randPlant, clustPlant, kmax, simPramComment.c_str());
		  myquery.execute(simName.c_str(), xmax, ymax, fmatch, 1, tmax, 1, pImmune, kImmune, simPramComment.c_str());
		//
		// now, get the integer key for this simulation:
		// note that this could be accomplished with one call (optimal if MySQL calls have high overhead)
		// by writing a SPROC on the MySQL side.
		// also see the mysql_insert_id() (C API) and LAST_INSERT_ID() (SQL) functions, by which we should be ablt to automatically retrieve the indexID.
		myquery.reset();
		printf("fetch simIndex.\n");
		myquery << "select max(simIndex) from ForestFire.SimPrams";
		myquery.parse();
		res1 = myquery.store(simName.c_str(), simName.c_str());
		intKey = res1.at(0).at(0);
		}; // doSQL
	//
	//printf("beginnn.\n");
	for (unsigned int i=0; i<=tmax; i++) {
		// here, i'm being sloppy with random numbers. really, we need four independent
		// random number generators for xTree, yTree, xMatch, yMatch
		// printf(" iteration %d\n", i);
		//
		//
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
		
		// 24 feb 2010 yoder:
		// let's apply immunity to the initial spark as well, just to see what happens...
		//randImmune = rand()%RAND_MAX;
		//frandImmune = float(randImmune)/RAND_MAX;
		if (rand()%fmatch==1) {
		//if (rand()%fmatch==1 and frandImmune > (pImmune/float(nBurning+dnBurning))) {
		//if (rand()%fmatch==1 and frandImmune > pImmune ) {
			//yodapause();
			// throw a match.
			xFire = rand()%xmax+1;
			yFire = rand()%ymax+1;
			fireSquare = &grids[0] + xFire + (xmax+2)*yFire;
			//printf("match: (%d, %d) :: %d\n", xFire, yFire, *fireSquare);
			//yodapause();
			//
			//if (getGridStatus(*fireSquare, i) >= burnAge and getGridStatus(*fireSquare, i) < rockAge) {
			if (*fireSquare >= burnAge and *fireSquare < rockAge) {				
				// initiate a new fire.
				// start from the epicenter and work out in concentric rectangles (squares)
				// until there are no new fires.
				// note: we make two passes over each circle. for now, we assume all squares
				// continue to burn until the fire is over. this is a subtle consideration that
				// will not matter for simpler versions of the model, but if we introduce burn probabilities,
				// we will have to be more careful.
				// alternatively, we can scan the entire grid every time, but the above method will
				// save CPU time.
				//*fireSquare=-1;	// the fire-square starts burning
				*fireSquare=-(*fireSquare);	// the fire-square starts burning
				//
				rFire = 1;
				//nBurning = 1;
				dnBurning = 1;
				nBurning = dnBurning;
				//
				int yFireMin = int(yodacode::greaterOf((yFire-rFire), 1));	// we always start with a 1 squar boundary. we might, however, encounter the edges.
				int yFireMax = int(yodacode::lesserOf((yFire+rFire), float(ymax)));
				int xFireMin = int(yodacode::greaterOf(float(xFire-rFire), 1));
				int xFireMax = int(yodacode::lesserOf(float(xFire+rFire), float(xmax)));
				//printf("fire range: %d, %d, %d, %d\n", yFireMin, yFireMax, xFireMin, xFireMax);
				//printf("preplot\n.");
				//
				//plotGrid(&grids[0],i, xmax, ymax);
				if (doSQL==0) {
					printGrid(&grids[0], i, xmax, ymax);
					};
				//while (dnBurning > 0) {
				doQuench = 0;
				nFireSteps=0;	// steps fire has propagated, max dist/radius from flash-point. effectively time it's been burning.
				while (dnBurning > 0 and doQuench==0) {
					dnBurning=0;
					nFireSteps++;
					// 30 june 2008 yoder:
					//for (char doTwice = 0; doTwice <=1; doTwice++) {	// "char" is a 1 byte integer. we could also use a boolean to count to 2.
					for (char doTwice = 0; doTwice <1; doTwice++) {	// retrospectively, we don't need to do two loops since we do the whole area. also, this creates
																					// inconsistency when evaluating immunity.
																					// note: 5c data before 30 june uses the "doTwice" algorithm. different immunity effect?
					for (int iy = (yFireMin); iy <= (yFireMax); iy++) {
						//if (doQuench==1) break;
						for (int ix = (xFireMin); ix <= (xFireMax); ix++) {
							//if (doQuench==1) break;	// this is a safety measure. the loop should be otherwise killed by setting ix>xFireMax
							//
							// note this loops down then across...
							// if a neighbor is burning and the element is old enough and the element is not alread burning
							// (note by using burning -> -Age
							// also note: Gelb is right. a recursive approach is better.
							// printf("try-burn: %d, %d\n", ix, iy);
							//plotGrid(&grids[0], xmax, ymax);
							
							//randImmune = rand()%RAND_MAX;
							//frandImmune = float(randImmune)/RAND_MAX;
							
							//printf("frandImmune: %f\n", frandImmune);
							//yodapause();
							//
							int * centerGrid = &grids[0] + ix + (xmax+2)*iy;
							//
							int cStat, uStat, dStat, lStat, rStat;
						//	int ulStat, urStat, llStat, lrStat;	// diagonal elements.
						//	cStat = getGridStatus(*(centerGrid), i);
							//
						//	uStat = getGridStatus(*(centerGrid + (xmax+2)), i);
						//	dStat = getGridStatus(*(centerGrid - (xmax+2)), i);
						//	lStat = getGridStatus(*(centerGrid - 1), i);
						//	rStat = getGridStatus(*(centerGrid + 1), i);
						//	cStat = getGridStatus(*(centerGrid), i);
							//
							cStat = *(centerGrid);
							//
							uStat = *(centerGrid + (xmax+2));
							dStat = *(centerGrid - (xmax+2));
							lStat = *(centerGrid - 1);
							rStat = *(centerGrid + 1);
							//
							// diagonal elements:
						//	ulStat = getGridStatus(*(centerGrid + (xmax+2) - 1), i);
						//	urStat = getGridStatus(*(centerGrid + (xmax+2) + 1), i);
						//	llStat = getGridStatus(*(centerGrid - (xmax+2) - 1), i);
						//	lrStat = getGridStatus(*(centerGrid - (xmax+2) + 1), i);
							//yodapause();
							//if (grids[iy][ix] >= burnAge && (grids[ix+1][iy]==-1 || grids[ix-1][iy]==-1 || grids[ix][iy+1]==-1 || grids[ix][iy-1]==-1) ) {
							//printf(" checking: cg=%d, lg=%d, rg=%d, ug=%d, dg=%d, anyburn=%d\n", *centerGrid, *leftGrid, *rightGrid, *upGrid, *downGrid, (*leftGrid==-1 or *rightGrid==-1 or *upGrid==-1 or *downGrid==-1) );
							//if (*centerGrid >= burnAge and (*leftGrid==-1 or *rightGrid==-1 or *upGrid==-1 or *downGrid==-1)) {
							//if (*centerGrid >= burnAge and (*leftGrid<=-burnPropAge or *rightGrid<=-burnPropAge or *upGrid<=-burnPropAge or *downGrid<=-burnPropAge)) {
							//if (cStat >= burnAge and cStat < rockAge and (lStat<=-burnPropAge or rStat<=-burnPropAge or uStat<=-burnPropAge or dStat<=-burnPropAge)) {
							//
							// no immunity:
							//if (cStat >= burnAge and cStat < rockAge and (lStat<=-burnPropAge or rStat<=-burnPropAge or uStat<=-burnPropAge or dStat<=-burnPropAge)) {
							// local immunity:
							//printf("immune factor: %f, %f\n", pImmune, frandImmune);
							//if (frandImmune > pImmune and cStat >= burnAge and cStat < rockAge and (lStat<=-burnPropAge or rStat<=-burnPropAge or uStat<=-burnPropAge or dStat<=-burnPropAge)) {
							// next nearest neighbors:
							/*	
							if (cStat >= burnAge and cStat < rockAge 
								and  ( (lStat<=-burnPropAge or rStat<=-burnPropAge or uStat<=-burnPropAge or dStat<=-burnPropAge)
									 or ( (ulStat<=-nnnAge or urStat<=-nnnAge  or llStat<=-nnnAge  or lrStat<=-nnnAge)
										 and (ulStat<=-burnPropAge or urStat<=-burnPropAge or llStat<=-burnPropAge  or lrStat<=-burnPropAge)
										 )
							 		)
								) {
							*/
							if (cStat >= burnAge and cStat < rockAge 
								and (lStat<=-burnPropAge or rStat<=-burnPropAge or uStat<=-burnPropAge or dStat<=-burnPropAge)
								) {
								*centerGrid = -(*centerGrid);
								// age -> number of trees in that grid...
								dnBurning ++;
								//
								// what if we evaluate immunity here? if quench, break the propagation loop. (can we break-break? how 'bout we just set the iterators over the max vals):
								// note: we've not yet updated nBurning, so use nBurning + dnBurning:
								//if (frandImmune < (pImmune/(1+(float(nBurning+dnBurning)/kImmune))) ) {
								/*
								randImmune = rand()%RAND_MAX;
								frandImmune = float(randImmune)/RAND_MAX;
								if (frandImmune < (pImmune/float(nBurning+dnBurning)) ) {
									// this seems to give us some funny log-periodic behavior for small Pimmune.
									// the next thing to try is: evaluate Pquench as each element is burned, but mitigate the introduction of new geometry (aka, the loop around the fire) by,
									// when frandImmune fails (fire quenches), we set doQuench=1, but let the fire continue through its current perimeter. maybe we need to just start testing
									// each individual element for burn?
									doQuench=1;
									ix=xFireMax+1;
									iy=yFireMax+1;
									};
								*/
								// printf("[%d, %d] catches from [%d, %d]\n", ix, iy, xFire, yFire);
								};
							}; // ix
						}; // iy
						}; // doTwice	
						nBurning = nBurning + dnBurning;
						//
						// mean-field immunities:
						// normalize formulae to extinguish size1 fires with probability pImmune
						//
						// inverse linear type (becareful of x/0):
						// if (frandImmune < (1+(1/kImmune))*(pImmune/(1+(float(nBurning)/kImmune))) ) {
						// if (frandImmune < pImmune/(1+(float(nBurning-1)/kImmune)) ) {
						// exponential:
						// if (frandImmune < pImmune*exp(-kImmune*(nBurning-1)) ) doQuench=1;
						// power law:
						// if (frandImmune < pImmune*pow(nBurning, -kImmune)) {
						//printf("kImmune norm: 1 + (1/kImmune): %f\n", 1 + (1/kImmune));
						//
				////////////////////////
				// temporarily move immunity inside the prop-loop.		
						// IMMUNITY (note different functions we play with):
						// do we quenche the fire?
						// 1/nFireSteps:
						//if (frandImmune < (pImmune/nFireSteps) ) {		// this is dumb way to make this work. stick to k
						// omori-type:
						randImmune = rand()%RAND_MAX;
						frandImmune = float(randImmune)/float(RAND_MAX);
				//		if (frandImmune < (pImmune/(1+(float(nBurning-1)/kImmune))) ) {
				//		// pure 1/k:
				//		if (frandImmune < (pImmune/nBurning) ) {
						if (frandImmune < (pImmune/pow(float(nBurning), float(kImmune))) ) {
				//		//if (frandImmune < pow(pImmune,(float(nBurning)/5) )) {
							doQuench=1;
							//printf("quenched %d.\n", nBurning);
							//yodapause();
							};
				/////////////////////////
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
				nTreesOnGrid = nTreesOnGrid-nBurning;
				fcounts[nBurning-1]++;
			//	plotGrid(&grids[0], xmax, ymax);
				if (doSQL==0) {
					printGrid(&grids[0], i, xmax, ymax);
					};
				// write fire to MySQL:
				if (doSQL==1) {
					// printf("fire size, nFires, totalBurned: (%d) (%d) (%d)\n", nBurning, nFires, nBurnedTotal);
					myquery.reset();
					myquery << "insert into ForestFire.ForestFires (simIndex, t, xSpark, ySpark, nBurned, nTrees) values (%0q, %1q, %2q, %3q, %4q, %5q) ";
					myquery.parse();
					myquery.execute(intKey, i, xFire, yFire, nBurning, nTreesOnGrid);
					//yodapause();
					};
				if (doSQL==3) {
					printf("fire at time %d\n", i);
					if (nBurning>=xmax/5) plotGrid (&grids[0], i, xmax, ymax, burnPropAge);
					};
				if (doSQL==4) {
					if (nBurning>=xmax/5) plotGridImg (&grids[0], i, xmax, ymax, burnPropAge);
					};
				//
				// fires finished burning; extinguish:
				for (int iy = (yFireMin); iy <= (yFireMax); iy++) {
					for (int ix = (xFireMin); ix <= (xFireMax); ix++) {
						// &grids[0] + ix + (xmax+2)*iy
						if (grids[ix + (xmax+2)*iy] < 0) grids[ix + (xmax+2)*iy]=0;
						};
					};
				}; //else printf("no tree at match point.\n");	// if match -> tree...
			}; // if match-time

		// do we initialize with 0?
		//printf("ary element 0,i: %d", grids[0][i]);
		};	// end sim-steps.

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
//	if (myconn.connected()==0) myconn.connect(DB, HOST, USER, PASSWORD);
	mysqlpp::Connection newconn(DB, HOST, USER, PASSWORD);
	mysqlpp::Query newquery=myconn.query();
	//mysqlpp::Result res1;

	//mysqlpp::Query myquery=myconn.query();
	//mysqlpp::Result res1;
	// end-o-run summaries (print):
	if (doSQL==2  or doSQL==25 or doSQL==3 or doSQL==4 or doSQL==6) {
		for (unsigned int i=0; i<(xmax*ymax); i++) {
			if (fcounts[i]!=0) printf("%d\t%d\n", i+1, fcounts[i]);
			//printf("%d,\t%d\n", i+1, fcounts[i]);
			};
		} else {
			//printf("finished.\nfire size, nFires, totalBurned: (%d) (%d) (%d)\n", nBurning, nFires, nBurnedTotal);
			};
	if (doSQL==5 or doSQL==25 ) { // no plots, just a summary -> SQL
		//
		for (unsigned int i=0; i<(xmax*ymax); i++) {
			// if (fcounts[i]!=0) printf("%d\t%d\n", i+1, fcounts[i]);
		//	printf ("sql bits: %d, %d, %d, %d\n", intKey, tmax, i+1, fcounts[i]);
			newquery.reset();
			newquery << "insert into ffcounts (simIndex, tmax, nBurned, nEvents) values (%0q, %1q, %2q, %3q)";
			newquery.parse();
			newquery.execute(intKey, tmax, i+1, fcounts[i]);
			//printf("%d,\t%d\n", i+1, fcounts[i]);
			};				
		};
	// return the final grid in full and give an average density at the end...?
	if (doSQL==11) {
		for (unsigned int i=0; i<(xmax+2)*(ymax+2); i++) {
			printf ("%d,\t%d,\t%d\n", i-int(i/(xmax+2))*(xmax+2), i/(xmax+2), getGridStatus(*(grids+i), tmax));
			};
		};
	//g1.reset_plot();
	//g1.plot_xyz(vx, vy, vGridStat, "xyz plot of TreesPlanted");

	//yodacode::yodapause();
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
//	if (argv[10]) nnnAge = yodacode::StrToInt(argv[10]);
	
	//printf("args: %d, %d, %d, %d, %d, %d, %d\n", tmax, fmatch, randPlant, clustPlant, burnAge, burnPropAge, doSQL);
	//yodapause();
	//doffire(300000,1,2000,3);
	//doffire(tmax, fmatch, randPlant, clustPlant, burnAge, burnPropAge, rhoRocks, doSQL, pImmune, nnnAge);
	doffire(tmax, fmatch, doSQL, pImmune, kImmune);

	return 0;
	};

