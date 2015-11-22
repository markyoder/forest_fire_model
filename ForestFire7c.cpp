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

#include "glebsutils.h"
#include "/home/myoder/Documents/Source/yodacode.h"
//#include </home/myoder/Documents/Source/glebsutils.h>

// mysql elements:
#include <mysql.h>
#include <mysql++.h>


#include <stdlib.h>
#include <cmath>
#include <cassert>
#include <iomanip>

const unsigned int bigScale = 1;
const unsigned int xmax0 = 256;
const unsigned int ymax0 = 256;
const unsigned int xmax = bigScale*xmax0;
const unsigned int ymax = bigScale*ymax0;
const int rockAge = 123456789;

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
//
// VERSION 6.0:
// simplify and cheat a little bit. we've pretty much decided that tree age, etc. is not a factor, so let's get rid
// of it.
// impose a pseudo-fractal substrate. we create artificial cells of size and frequency according to the observed
// k^-.5 SCA fire distribution. we hypothesize that we are observing the burning of saturated cells of this dimension.
// we start with a simple, not terribly flexible model. we use two grids; one is a map of sub-clusters; the other is the
// actual fire grid. when we drop a tree, check the clusterGrid[] array to see which cluster a tree belongs to; assign to
// grids[] that index value (aka, if we drop a tree into cluster-5, we put a "5" into the grids[] cell). note, we are doing
// away with any aging, unburnable sites, etc. within a given cell, we're back to the most basic FFM.
//
// VERSION 7.0:
// version 6.0 was fun, but as it so happens, don was reading minnich's plot incorrectly, so the real slope of the line was about
// -1.4 in SCA, -2 in BCA, so we're still in the percolation regime.
// Let's try a DENDRITIC (aka DLA) model. this will be like our old enhanced clustering thing, but maybe we restrict the connectivity to 2 or 3
// so we get long skinny things.
// place nSeed seed sites, then place nWalker "trees" that walk to the nearest (within some timesteps of course) existing site with
// appropriate connectivity, then place nRand random sites.
//
// VERSION 7.1 (7b):
//	7.0 is certainly not scale invariant. is self-clustering scale variant or is it an artifact of the model? maybe both, but the model
// changes as clusters are added. basically, while the grid is sparse, a cluster has a high probability of growing by adsorbing a 
// "walking" tree. as the grid becomes more dense, a given cluster has a smaller probability of growing according to this mechanism, 
// so we eliminate small clusters and get a constant distribution of clusters up to some size related to nClust/nRand.
// let's continue with this approach but seek out a scale invariant approach. one way to do this is to scan the whole grid; each empty
// cell has some probability Pplant to grow a tree; any tree square has some probability Pprop of spreading to an adjacent square.
// this, besides addressing the cluster-density issue above, also facilitates density increases inside a cluster.
//rather than scanning the whole grid every time, which is computationally intractable or programmatically tricky, we employ our random
// plant method with the following variations:
//	- a tree landing on an empty cell has some probability P1 of growing
// - a tree landing on an occupied cell has some probability P2 of spreading to an adjacent cell, if one is available.
//		- further variations:
//			- try once in one/each direction?
//			- try once in one/each available direction?
//			- if we "plant" on an occupied site, does that count as a regular planting? (i think this will break scale invariance)
// 	* i think i'll start with try once in each direction and the buck stops there.
//		* use the same planting probability distribution. when a tree lands on an empty grid, Pgrow = nRand/(nRand + nClust),
//		* Pspread = nClust/(nRand + nClust)
//
//	VERSION 7.2 (7c):
// 7b works, more or less. by varying the probability with which we plant adjacent sites, we get a shallower slope (denser, clusters so
// larger fire volume; this does not address the actual hectar measurement by the forest services). another variation might be to vary
// the number of steps (random walks or some crazy expanding radius with diminishing probability) away from an occupied site at which we can
// plant a new tree. as this limit ->big, we expect a nearly constant distribution i think; maybe we actually get a positive slope?
//
// here, we try 7b in reverse. when we land on an occupied site, we look around that site and r with some probability P we remove a trees
// while k>k0. just removeing any tree in the vicinity, i think, will not be significant; it will be like dropping fewer trees, aka a longer
// spark frequency, so we'll push for thinner connectivity, k0=2, to make thinner clusters.
//
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
	unsigned int xmax1=xmax/bigScale, ymax1=ymax/bigScale;
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
	unsigned int dpos = 64;
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
		for (unsigned int iy=192-i2*dpos; iy < 224-i2*dpos; iy++) {
			for (unsigned int ix=0; ix < xmax1; ix++) {
				//clustGrids[(xmax+2)*(iy+1)+ix+1]=thisClust + int(xmax*(iy-192)/4) + int(ix/4);
				//clustGrids[(xmax1+2)*(iy+1)+ix+1]=thisClust + ix/4;
				*(clustGrids + (xmax1+2)*(iy+1)+ix+1)=thisClust + ix/4;
				};
				if ( (iy+1)%4 == 0) thisClust = thisClust + xmax1/4;
			};
		//thisClust = thisClust + 512;	// we could code this incrementation in the loop if we like...
		//
		// size 64, L=8 clusters
		for (unsigned int iy=224-i2*dpos; iy < 240-i2*dpos; iy++) {
			for (unsigned int ix=0; ix < xmax1; ix++) {
				*(clustGrids + (xmax1+2)*(iy+1)+ix+1)=thisClust + int(ix/8);
				};
				if ( (iy+1)%8 == 0) thisClust = thisClust + xmax1/8;
			};
		thisClust = thisClust + 64;	// we could code this incrementation in the loop if we like...
		//
		// size 256, L=16
		for (unsigned int iy=240-i2*dpos; iy < 256-i2*dpos; iy++) {
			for (unsigned int ix=0; ix < 128; ix++) {
				*(clustGrids + (xmax1+2)*(iy+1)+ix+1)=thisClust + int(ix/16);
				};
				//if ( (iy+1)%16 == 0) thisClust = thisClust + xmax1/16;
			};
		thisClust = thisClust + 8;
		//
		// size 1024, L=32
		for (unsigned int iy=240-i2*dpos; iy < 256-i2*dpos; iy++) {
			for (unsigned int ix=128; ix < 192; ix++) {
				*(clustGrids + (xmax1+2)*(iy+1)+ix+1)=thisClust;
				};
			};
		thisClust = thisClust + 1;
		};
		//
	
	//printf("now map to the big cluster.\n");
	// now, map this to the large grid. we've chose 1->4 mapping for the time being. scan the cluster-map; expand each pixel to a 4x4 on the new grid.
	for (unsigned int i=1; i<(ymax1+1); i++) {
		for (unsigned int ii = 1; ii<=bigScale; ii++) {
			for (unsigned int iii=1; iii<(xmax1+1); iii++) {
				for (unsigned int iiii = 1; iiii<=bigScale; iiii++) {
					// write gridClusts[ii, i] 4 times
					//bigClustMaps[(bigScale*xmax+2)*(bigScale*(i-1)+ii) + (bigScale*(iii-1)+iiii)] = clustGrids[i*(xmax+2) + iii];
					//*(bigClustMaps + (bigScale*xmax+2)*(bigScale*(i-1)+ii) + (bigScale*(iii-1)+iiii)) = clustGrids[i*(xmax+2) + iii];

					//*(bigClustMaps + (xmax+2)*(bigScale*(i-1)+ii) + (bigScale*(iii-1)+iiii)) = clustGrids[i*(xmax1+2) + iii];
					*(bigClustMaps + (xmax+2)*(bigScale*(i-1)+ii) + (bigScale*(iii-1)+iiii)) = *(clustGrids + i*(xmax1+2) + iii);
					};
				};
			};
		};
	unsigned int biggestFire = (bigScale*32)*(bigScale*32);

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

// int doffire(unsigned int xmax=128, unsigned int ymax=128, unsigned int fmatch=125 unsigned short burnPropAge=400) {
// use simpler float nRand, nClust probabilities...
int doffire(unsigned tmax=1000000, int doSQL=1, unsigned short fMatch=200, float nRand=1, float nClust=0, int kmax=4) {
	// burn toplolgy: do we nave nnn burning
	//
	std::string simName("ForestFire7");
	std::string simPramComment("");
	//int fMatch = 250;
	//
	unsigned short rFire = 1;
	unsigned int nBurning = 0;
	unsigned short dnBurning = 0;
	int thisX, thisY, xFire, yFire; 
	//int *fireSquare = new int;
	int *fireSquare = NULL;
	unsigned int nBurnedTotal=0, nFires=0;  // number of elements burning, new Elements burning...
	//
	int burnPropAge=1;
	//
	//int *bigClustMaps = new int[(ymax+2)*(xmax+2)];
	int *grids = new int[(ymax+2)*(xmax+2)];
	int intKey;
	float aveRho, aveVol;
	short sWalkDir = 0;
	int gridPos=0;
	//
	int rSeed=int(time(NULL));
	int drSeed = 0;
	srand(time(NULL));	
	Rand plantRandx(rSeed + drSeed);
		drSeed ++;
	Rand plantRandy(rSeed + drSeed);
		drSeed ++;
	Rand fireRandx(rSeed + drSeed);
		drSeed ++;
	Rand fireRandy(rSeed + drSeed);
		drSeed ++;
	Rand rfMatch(rSeed + drSeed);
		drSeed ++;
	Rand rcPlant(rSeed + drSeed);
		drSeed ++;
	Rand walkDir(rSeed + drSeed);
		drSeed ++;

	// initialize the grid/cluster-grid with "rocks" around the border and in otherwise empty cells.
	// "cluster 0" will, for the time being, not participate in the simulation. just for posterity,
	// we will place "rocks" around the borders. initialize the interior of the grid with 0; we will
	// map the excluded areas via the clusters grid.
	for (unsigned int i = (xmax+2); i<(xmax+2)*(ymax+1); i++) {
		grids[i] = 0;
		//clustGrids[i] = 0;
		};
	for (unsigned int i=0; i<(xmax+2); i++) {
		grids[i] = rockAge;
		grids[(ymax+1)*(xmax+2) + i] = rockAge;
		//
		};
	for (unsigned int i=0; i<(ymax+2); i++) {
		grids[(xmax+2)*i] = rockAge;
		grids[(xmax+2)*i + xmax+1] = rockAge;
		//
		};
	//printf("the grid:\n");
	//printGrid(grids, 1, xmax, ymax);
	//
	// cluster map moved to a function:
//	unsigned int biggestFire = getClustMaps1(bigClustMaps);
	unsigned int biggestFire = xmax0*ymax0;
	// now, the cluster map is established. as we plant trees, we will plant against this grid.
	//
	// seed the grid with some trees:
	Rand treeSeedx(rSeed + drSeed);
		drSeed ++;
	Rand treeSeedy(rSeed + drSeed);
		drSeed ++;
	// particularly for dendritic models, how many seeds to we want? in the purest form, we want 1, but that will take a LONG time to get started.
	for (unsigned int i=0; i < xmax; i++) {
		// seed the grid with some trees
		//if (rand()%(xmax)==1 and i%(xmax+2)!=0 and (i+1)%(xmax+2)!=0 and i>(xmax+2) and i<((xmax+2)*(ymax+1)) ) grids[i]++;
		int thisX = treeSeedx.nextInt(xmax);
		int thisY = treeSeedy.nextInt(ymax);
		//thisClust = clustGrids[(xmax+2)*(thisY+1)+thisX+1];
		//grids[(xmax+2)*(thisY+1)+thisX+1] = thisClust;
		//thisClust = *(bigClustMaps + (xmax+2)*(thisY+1)+thisX+1);
		*(grids + (xmax+2)*(thisY+1)+thisX+1) = 1;		
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
	mysqlpp::Result res1;
	//int intKey;
	//unsigned int fcounts[xmax*ymax];
	//unsigned int *fcounts = new unsigned int[xmax*ymax];
	unsigned int *fcounts = new unsigned int[biggestFire];
		for (unsigned int i=0; i<biggestFire; i++) {
			//fcounts[i]=0;
			*(fcounts + i) = 0;
			};
	if (doSQL==1 or doSQL==5) {	
		//
		// insert a new row for this sim-run:
		myquery.reset();
		printf("insert simprams.\n");
		//myquery << "insert into ForestFire.SimPrams (SimName, SimSW, xmax, ymax, sparkInterval, sparkProb, burnAge, propAge, tmax, wrapX, wrapY, Comment) values (%0q, %1q, %2q, %3q, %4q, %5q, %6q, %7q, %8q, %9q, %10q, %11q )";
		myquery << "insert into ForestFire.SimPrams (SimName, SimSW, xmax, ymax, sparkInterval, tmax, nRand, nClust, kmax, Comment) values (%0q, %1q, %2q, %3q, %4q, %5q, %6q, %7q, %8q, %9q)";
		myquery.parse();
		// note: simIndex(auto-int) and dtTime (default TIMESTAMP) set automatically.
		myquery.execute(simName.c_str(), simName.c_str(), xmax, ymax, fMatch, tmax, nRand, nClust, kmax, simPramComment.c_str());
		//
		// now, get the integer key for this simulation:
		// note that this could be accomplished with one call (optimal if MySQL calls have high overhead)
		// by writing a SPROC on the MySQL side.
		// also see the mysql_insert_id() (C API) and LAST_INSERT_ID() (SQL) functions, by which we should be ablt to automatically retrieve the indexID.
		myquery.reset();
		printf("fetch simIndex.\n");
		myquery << "select max(simIndex) from ForestFire.SimPrams where simName=%0q and simSW=%1q";
		myquery.parse();
		res1 = myquery.store(simName.c_str(), simName.c_str());
		intKey = res1.at(0).at(0);
		}; // doSQL

/*
	if (doSQL==1 or doSQL==5) {	
		//
		// insert a new row for this sim-run:
		myquery.reset();
		printf("insert simprams.\n");
		myquery << "insert into ForestFire.SimPrams (SimName, SimSW, xmax, ymax, sparkInterval, sparkProb, burnAge, propAge, tmax, wrapX, wrapY, Comment) values (%0q, %1q, %2q, %3q, %4q, %5q, %6q, %7q, %8q, %9q, %10q, %11q )";
		myquery.parse();
		// note: simIndex(auto-int) and dtTime (default TIMESTAMP) set automatically.
		myquery.execute(simName.c_str(), simName.c_str(), xmax, ymax, fmatch, pMatch, burnAge, burnPropAge, tmax, wrapx1, wrapy1, simPramComment.c_str());
		//
		// now, get the integer key for this simulation:
		// note that this could be accomplished with one call (optimal if MySQL calls have high overhead)
		// by writing a SPROC on the MySQL side.
		// also see the mysql_insert_id() (C API) and LAST_INSERT_ID() (SQL) functions, by which we should be ablt to automatically retrieve the indexID.
		myquery.reset();
		printf("fetch simIndex.\n");
		myquery << "select max(simIndex) from ForestFire.SimPrams where simName=%0q and simSW=%1q";
		myquery.parse();
		res1 = myquery.store(simName.c_str(), simName.c_str());
		intKey = res1.at(0).at(0);
		}; // doSQL
*/
	//
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
		// yoder, v7: introduce dendritic growth. we have two prams, nRand, nClst. define a P(rand)-> Pr, P(clust)->Pc: Pr=nRand/(nRand+nClust), etc.
		//for (unsigned int irp=0; irp<randPlant; irp++) {
		thisX = plantRandx.nextInt(xmax);
		thisY = plantRandy.nextInt(ymax);
		gridPos = (xmax+2)*(thisY+1)+1+thisX;
		//

/*
// this block: here, we make a rule that if we plant on top of a tree, we look around that tree to maybe eliminate trees where k>2.
// we can achieve the same results by modifying the random planting probability according to how many neighbors that site has.
// when we land on an empty site, sart with pPlant; look around the site. for each tree, pPlant -> pPlant*Pclust
		float thisProb=nRand;
		float pThin = 1-nClust;
		switch (*(grids+gridPos)) {
			case 0:
				// run around this square and look for planted sites:
				if (*(grids+gridPos +1) == 1) thisProb = thisProb*pThin;
				if (*(grids+gridPos -1) == 1) thisProb = thisProb*pThin;
				if (*(grids+gridPos + (xmax+2)) == 1)  thisProb = thisProb*pThin;
				if (*(grids+gridPos - (xmax+2)) == 1)  thisProb = thisProb*pThin;
				if (thisProb>(pThin*pThin)) thisProb = nRand;
				if (rcPlant.nextDouble() <= thisProb) *(grids+gridPos) = 1;
				// if we want, we can impose an if + algebraic rule to only affect some minimum connectivity.
				break;
			case 1:
				// in this case, we do nothing, though we might move back to 7b and include the thin-planting rule
				break;
			};
*/

		// what is the grid value? if it's empty, plant against P=nRand/(nRand+nClust); if it's a tree, plant each adjacent grid with nR/(nR+nC)
		unsigned short gridk=0;
		// save 4 dPos position adjustments. we'll randomly pick 1 or more of these as a thinning direction.
		int dPos[4] = {0, 0, 0, 0};
		switch (*(grids+gridPos)) {
			case 0:
				// planting stats:
				if (rcPlant.nextDouble() <= nRand) {
					*(grids+gridPos) = 1;
					};
				break;
			case 1:
				// what is the connectivity?
				if (*(grids+gridPos +1) == 1) {
					dPos[gridk]=1;
					gridk++;
					};
				if (*(grids+gridPos -1) == 1) {
					dPos[gridk]=-1;
					gridk++;
					};
				if (*(grids+gridPos + (xmax+2)) == 1) {
					dPos[gridk]=(xmax+2);
					gridk++;
					};
				if (*(grids+gridPos - (xmax+2)) == 1)  {
					dPos[gridk]=-(xmax+2);
					gridk++;
					};
				//
				// if k>k0, then attempt to thin the area by removing up to k-k0 adjacent trees.
				if (gridk>2) {
					for (int ik=0; ik<(gridk-2); ik++) {
						// we've filled the top gridk elements of dPos;
						int thinPos = gridPos + dPos[rcPlant.nextInt(gridk)];
						if (rcPlant.nextDouble() < nClust) {
							 *(grids+thinPos)=0;
							// printf("peacefully removing a tree (%d).", gridk);
							};
						};
					};

				break;
			};


		//
// debug:
//		if (i%10000==0) {
//			plotGridSimple (grids, i, xmax, ymax);
//			};

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

				//burningClust = clustGrids[(xmax+2)*yFire + xFire];
				//burningClust = *(bigClustMaps + (xmax+2)*yFire + xFire);
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
				//printf("fire range: %d, %d, %d, %d\n", yFireMin, yFireMax, xFireMin, xFireMax);
				//printf("preplot\n.");
				//
				//plotGrid(&grids[0],i, xmax, ymax);
				if (doSQL==0) {
					printGrid(grids, i, xmax, ymax);
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
								and (lStat<=-1 or rStat<=-1 or uStat<=-1 or dStat<=-1) )
								{
	//
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
				fcounts[nBurning-1]++;
			//	plotGrid(&grids[0], xmax, ymax);
				if (doSQL==0) {
					printGrid(&grids[0], i, xmax, ymax);
					printf("fire size, vol, nFires, totalBurned, totalVol: (%d) (%d) (%d)\n", nBurning, nFires, nBurnedTotal);
					};
				// write fire to MySQL:
				if (doSQL==1) {
					printf("fire size, nFires, totalBurned: (%d) (%d) (%d)\n", nBurning, nFires, nBurnedTotal);
	//				myquery.reset();
	//				myquery << "insert into ForestFire.ForestFires (simIndex, t, xSpark, ySpark, AveTreeAge, nBurned) values (%0q, %1q, %2q, %3q, %4q, %5q) ";
	//				myquery.parse();
	//				myquery.execute(intKey, i, xFire, yFire, aveTreeAge, nBurning);
					};
				if (doSQL==3) {
					printf("fire (%d) at time %d\n",nBurning, i);
					if (nBurning>=10) plotGridSimple (grids, i, xmax, ymax);
					};
				if (doSQL==4) {
					if (nBurning>=10) plotGridImg (grids, i, xmax, ymax, burnPropAge);
					};
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
	// end-o-run summaries (print):
	if (doSQL==2 or doSQL==3 or doSQL==4 or doSQL==6) {
		for (unsigned int i=0; i<(biggestFire); i++) {
			//if (fcounts[i]!=0) printf("%d\t%d\n", i+1, fcounts[i]);
			if (*(fcounts + i) != 0) printf("%d\t%d\n", i+1, *(fcounts + i));
			//printf("%d,\t%d\n", i+1, fcounts[i]);
			};
		} else {
			//printf("finished.\nfire size, nFires, totalBurned: (%d) (%d) (%d)\n", nBurning, nFires, nBurnedTotal);
			};
	if (doSQL==5 ) { // no plots, just a summary -> SQL
		//
		for (unsigned int i=0; i<(biggestFire); i++) {
			// if (fcounts[i]!=0) printf("%d\t%d\n", i+1, fcounts[i]);
		//	printf ("sql bits: %d, %d, %d, %d\n", intKey, tmax, i+1, fcounts[i]);
			myquery.reset();
			myquery << "insert into ffcounts (simIndex, tmax, nBurned, nEvents) values (%0q, %1q, %2q, %3q)";
			myquery.parse();
			myquery.execute(intKey, tmax, i+1, fcounts[i]);
			//printf("%d,\t%d\n", i+1, fcounts[i]);
			};				
		};
	// return the final grid in full and give an average density at the end...?
	if (doSQL==11) {
		for (unsigned int i=0; i<(xmax+2)*(ymax+2); i++) {
			printf ("%d,\t%d,\t%d\n", i-int(i/(xmax+2))*(xmax+2), i/(xmax+2), getGridStatus(*(grids+i), tmax));
			};
		};

	//yodacode::yodapause();
	return 0;
	//return &grids[0];
	};


int main(int argc, char *argv[]) {
	//ffPlots();
	unsigned int tmax=1000000;
	int doSQL = 4;
	unsigned int fmatch=200;
	unsigned int nRand = 1;
	unsigned int nClust = 0;
	unsigned int kmax = 4;
//	unsigned int burnPropAge=5;
//	unsigned int rhoRocks = 0;
//	float pImmune = 0;
//	int nnnAge = 0;
	//
	if (argv[1]) tmax=yodacode::StrToInt(argv[1]);
	if (argv[2]) doSQL=yodacode::StrToInt(argv[2]);
	if (argv[3]) fmatch=yodacode::StrToInt(argv[3]);
	if (argv[4]) nRand=yodacode::StrToInt(argv[4]);
	if (argv[5]) nClust=yodacode::StrToInt(argv[5]);
	if (argv[6]) kmax=yodacode::StrToInt(argv[6]);
//	if (argv[7]) burnPropAge=yodacode::StrToInt(argv[7]);
//	if (argv[8]) rhoRocks=yodacode::StrToInt(argv[8]);
//	if (argv[9]) pImmune = yodacode::StrToFloat(argv[9]);
//	if (argv[10]) nnnAge = yodacode::StrToInt(argv[10]);
	
	//printf("args: %d, %d, %d, %d, %d, %d, %d\n", tmax, fmatch, randPlant, clustPlant, burnAge, burnPropAge, doSQL);
	//printf("args: %d, %d, %d\n", tmax, doSQL, fmatch);
	//yodapause();
	//doffire(300000,1,2000,3);
	doffire(tmax, doSQL, fmatch, nRand, nClust, kmax);
	return 0;
	};

