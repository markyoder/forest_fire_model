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
#include "percolation.h"
#include "/home/myoder/Documents/Source/yodacode.h"
//#include </home/myoder/Documents/Source/glebsutils.h>

// mysql elements:
//#include <mysql.h>
//#include <mysql++.h>
// #include "/usr/local/include/mysql++/mysql++.h"


#include <stdlib.h>
#include <cmath>
#include <cassert>
#include <iomanip>

const unsigned int bigScale = 1;
//const unsigned int xmax0 = 1024;
//const unsigned int ymax0 = 1024;
//const unsigned int xmax = bigScale*xmax0;
//const unsigned int ymax = bigScale*ymax0;
const unsigned int xmax=512;
const unsigned int ymax=512;
const int rockAge = 123456789;
const int sheepAge = rockAge + 1;

// Declarations:
int *scrambleList(int *inList, int *outList, int listLen);
int *randomSequence(int *outList, int listLen);
bool testQuench(int k, int Pimmune, int kImmune);
bool IsOccupied(int x);


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
// ::: looks like 7c (note, there are two versions of the thinning algorithm) doesn't do anything. moving on to sheep...
//
// VERSION 7.3(7d) (starting from 7b, casting aside 7c):
// introduce sheep. we have nSheep sheep heards. let's start them at a random spot. they wander one or more turns (maybe walk two, eat 1,
// or either walk or eat, or maybe walk and eat at the same time). stastically, more sheep should be equivalent to fast moving sheep.
// eventually, sheep might be incorporate stand-age. sheep can eat young stands but not older stands. this might introduce a spark frequency
// dependence.
// let's start with simple sheep. they move 1 square at random. when they encounter a stand, they graze (gridVal->0).
// probably the best variation is that at each turn, they either graze or move.
// 
// 7d results: this is probably working, but our sheep are not scale invariant; we get a wieblel distribution, which is maybe ok.
// a curious result, that we may examine more closely later, is that we the slope actually becomes more shallow for
// sheepSpeed=1, then much steeper for sheepSpeed>=2.
//
// VERSION 7e:
// more with the sheep. let's try scale invariant sheep. 7d is flawed possibly because the sheep wander in planting time-steps
// rather than cluster growing time. instead of sheep wandering sheepSpeed every turn, plant nSheep randomly on the grid and 
// use the tree-planting probility/time step engine to move them. when we land on a sheep, it moves to a random square and eats
// the vegitation. variations might be, if we land next to sheep, the sheep moves there, this is programatically messy and equivalent
// results can be achieved by adding more sheep.
// these sheep will move and eat in lines, so their statistic should be different from clusters, but as per the percolation frequency of
// hitting sheep, we may get a null result unless the sheep move fundamentally more quickly than the clusters.
// and right, the sheep move too damn slow. maybe we increase their cross-section some way other than just adding more (which is percolation
// statistics).
//
// so we try something different... again. how 'bout sheep that instantly traverse the grid, for starters right before a fire, eventually 
// now and again. aka, every N turns, we random walk n sheep across the grid, or maybe from a random position for xmax*a (some factor of xmax)
// steps. doing this right before a fire, will help to separate this effect from the percolation of tree elements.
//
// VERSION 7h:
// are sheep ergotic? instead of one sheep moving 10 steps, 10 sheep move 1 step. sheep are randomly distributed.
//
//	VERSION 7i:
//	we have bigger problems. it would appear that a minor mistake may have facilitated the sheep effect of 7e.
// you might not find the effect in 7e any more because it has been corrected. a typo did two things:
// 1) it defined the torroid "skip the other side" line as a diagonal from TR to LL (aka, SheepPos%(xmax+1)==0). this, by itself, had little effect.
//	2) there was no rule to skip from far right to far left. sheep wander onto the border "rock" freely. when they wander one more to the right,
//		however, they encounter the SheepPos%(xmax+2)==0, as if they had come from the righ, and get sent to the leftmost square on the grid.
//		consequently, hungry sheep spent quite a bit of time on the right side of the grid. only when the tree clusters got thick around the right side
//		did the sheep venture into the grid for thinning. thus did sheep, in essence, beget sheep. i think.
//		SO, we want trapping regions along at least one border. set sheep to migrate east (right), eliminate right-side torroid rule.
//
//	also, with this error in place, sheep do NOT appear to be ergodic. fast sheep clear out their trapping region very quickly and so remain trapped.
// so, with 79, we have random-walking sheep with a trapping region on the right side.
//
// VERSION 7j:
// sheep only move if they are eating. when we roll a "move sheep," we try a move. if they are standing on or next to food, they eat/move to that food.
// they continue to munch the cluster until they wanter out of it. they then stop and wait for a new cluster to approach them. sheep are still circumference
// dependent. now the whole grid, when absent nearby clusters, acts as a trapping region for sheep.
//
// VERSION 7k:
//	same as j but we remove the trapping region on the right side...
//
// VERSION 7L:
// this is not relly an extension of 7k; we'll probably run with 0 sheep. we use a cool-frot propagation fire:
// - the fire burns in concentric boxes, but only the outside elements light new fires (one or two passes on the
//   outside ring). this will prohibit fires from propagating backwards. if we want to, we can introduce "wind"
//   to make the fires more realistic (aka, a wind driven cool flame-front), aka fires only propagate left to right,
//   and i suppose up or down, but i don't think it will be necessary.
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
	// maybe the random walk is not appropriate for sheep. remove backwards steps. for now,
	// just right and down...
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
	int foodDirections[5] = {0,0,0,0,0};
	int nFoodDirs=0, sheepDir=0;
	//
	// are we standing on food? do we put this in the mix or always eat where we're standing?
//	if (*(theGrid+sheepPos)==1) {
//		foodDirections[nFoodDirs]=0;
//		nFoodDirs++;
//		};
	if (*(theGrid+sheepPos)==1) { return 0; };
	if (*(theGrid+sheepPos+1)==1) {
		foodDirections[nFoodDirs]=1;
		nFoodDirs++;
		};
	if (*(theGrid+sheepPos-1)==1) {
		foodDirections[nFoodDirs]=-1;
		nFoodDirs++;
		};
	if (*(theGrid+sheepPos+gridX)==1) {
		foodDirections[nFoodDirs] = gridX;
		nFoodDirs++;
		};
	if (*(theGrid+sheepPos-gridX)==1) {
		foodDirections[nFoodDirs] = -gridX;
		nFoodDirs++;
		};
	//
	// debug:
	// printf("from getSheepDir: %d\n", getRandDir(gridX, thisRand));
	//printf("sheepPos: %d,%d\n", sheepPos/gridX, sheepPos%gridX);
	//printf("nFoodDirs: %d:: %d, %d, %d, %d\n", nFoodDirs, foodDirections[0], foodDirections[1], foodDirections[2], foodDirections[3] );
	//printf("neighbors(l, u, r, d): %d, %d, %d, %d\n", *(theGrid-1), *(theGrid+gridX), *(theGrid+1), *(theGrid-gridX));
	//
	if (nFoodDirs>0) {
		// there is something we can eat...
		sheepDir = foodDirections[thisRand->nextInt(nFoodDirs)];
		}
	else {
		sheepDir = getRandDir(gridX, thisRand);
		};
	//
	// if (sheepDir<0) sheepDir=-sheepDir;
	return sheepDir;
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
	
bool IsOccupied(int x) { 
	// return x>=1;
	return x == 1;
	};

bool inGrid (int gridPos, int gridX, int gridY) {
	// is this new position in a grid with border width/height 1?
	bool isIn=1;
	//if (gridPos<(gridX+2)) isIn=0;
	if (gridPos/(gridX+2)==0 or gridPos/(gridX+2)>=(gridY+1)) isIn=0;	// top or bottom row
	if (gridPos%(xmax+2)==0 or gridPos%(xmax+1)==0) isIn=0;
	//
	return isIn;
	};

// int doffire(unsigned int xmax=128, unsigned int ymax=128, unsigned int fmatch=125 unsigned short burnPropAge=400) {
// use simpler float nRand, nClust probabilities...
// int doffire(unsigned tmax=1000000, int doSQL=1, unsigned int fmatch=200, float nRand=1, float nClust=0, int kmax=4, int sheepSpeed=1, int nSheep=0) {
int doffire(unsigned tmax=1000000, int doSQL=1, unsigned int fmatch=200, int nSheep=0, std::string statFileName=std::string("tmpffoutput.dat")) {
	//
	std::string immCode=std::string("000");
	bool doImPrim=0, doImFlameFront=0, doImFireElement=0;
	//if (immCode.at(2)==std::string("1")[0] ) doImPrim=1;
	//if (immCode.at(1)==std::string("1")[0] ) doImFlameFront=1;
	//if (immCode.at(0)==std::string("1")[0] ) doImFireElement=1;
	float pImmune=0;
	float kImmune=5;
	float frandImmune=0;
	int randImmune = 0;
	// 
	// allow for no-fire simulation (aka, sheep only):
	if (fmatch==0) fmatch=tmax;
	int kmax=4;
	//
//	std::string simName("ForestFire7k");
//	std::string simPramComment("");
	//int fmatch = 250;
	//
	unsigned short rFire = 1;
	unsigned int nBurning = 0;
	unsigned short dnBurning = 0;
	int nTreesOnGrid=0;
	int thisX, thisY, xFire, yFire;
	//int *fireSquare = new int;
	int *fireSquare = NULL;
	unsigned int nBurnedTotal=0, nFires=0;  // number of elements burning, new Elements burning...
	//
	int burnPropAge=1;
	//
	//int *bigClustMaps = new int[(ymax+2)*(xmax+2)];
	int *grids = new int[(ymax+2)*(xmax+2)];
	int *sheepPoops = new int[(ymax+2)*(xmax+2)];
	int *thisSheepPoop = new int[nSheep];
	int *newSheepPoop = new int[nSheep];
	bool sheepIsEating=0;
	//
	// 20081219yoder: what is the square area footprint of the fire? for now, use a rectangle footprint: xmax, xmin, ymax, ymin.
	unsigned * fireFprints = new unsigned[ymax*xmax];
	unsigned * sheepFprints = new unsigned[ymax*xmax];
	unsigned firefpxmin, firefpxmax, firefpymin, firefpymax, fireArea;		// footprint prams.
	unsigned sheepfpxmin, sheepfpxmax, sheepfpymin, sheepfpymax, sheepArea;
	//
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
	Rand rfmatch(rSeed + drSeed);
		drSeed ++;
	Rand rcPlant(rSeed + drSeed);
		drSeed ++;
	Rand walkDir(rSeed + drSeed);
		drSeed ++;
	Rand sheepRand(rSeed + drSeed);
		drSeed ++;
	Rand sheepRand2(rSeed + drSeed);
		drSeed ++;

	// initialize the grid/cluster-grid with "rocks" around the border and in otherwise empty cells.
	// "cluster 0" will, for the time being, not participate in the simulation. just for posterity,
	// we will place "rocks" around the borders. initialize the interior of the grid with 0; we will
	// map the excluded areas via the clusters grid.
	unsigned int * sheepPos = new unsigned int[nSheep];
	for (int iNsheep=0; iNsheep<nSheep; iNsheep++) {
		*(sheepPos+iNsheep) = (xmax+2) + sheepRand.nextInt(ymax)*(xmax+2) + sheepRand.nextInt(xmax)+1; 
		*(thisSheepPoop+iNsheep) = 0;
		*(newSheepPoop+iNsheep)=0;
		};
	//unsigned int sheepPos = (xmax+2) + sheepRand.nextInt(ymax)*(xmax+2) + sheepRand.nextInt(xmax)+1; 
	//printf("initial sheepPos: %d, %d\n", sheepPos, sheepSpeed);
	for (unsigned int i = (xmax+2); i<(xmax+2)*(ymax+1); i++) {
		// initialize here with tree-seeds and sheep:		
		*(grids+i) = 0;
		*(sheepPoops+i)=0;
		};
	// borders:
	for (unsigned int i=0; i<(xmax+2); i++) {
		//grids[i] = rockAge;
		//grids[(ymax+1)*(xmax+2) + i] = rockAge;
		*(grids+i) = rockAge;
		*(grids + (ymax+1)*(xmax+2) + i) = rockAge;
		//
		};
	// borders:
	for (unsigned int i=0; i<(ymax+2); i++) {
		//grids[(xmax+2)*i] = rockAge;
		//grids[(xmax+2)*i + xmax+1] = rockAge;
		*(grids + (xmax+2)*i) = rockAge;
		*(grids +(xmax+2)*i + xmax+1) = rockAge;
		//
		};
	ofstream fstats;
	int directions[4]={1,-1, xmax+2, -(xmax+2)};
	//int randDirs[4]={1,-1, xmax+2, -(xmax+2)};	// new array to be randomized
	//int *newOrder = new int[4];
	int newOrder[4] = {0, 1, 2, 3};
	int *fireList = new int[(ymax)*(xmax)];
	unsigned short newFireElements=0;		// [x, x, x, x, x, x[nBurning], x, x, x, x[dnBurning], x, x, x, x[newFireElements] ]: old-fire, current flame-front, new elements.
	unsigned int totalFireCount=0;
	// and now, gather explicit cluster count statistics and extinguished fire statistics (and a cluster-finder object):
	unsigned int *extinguishedFires = new unsigned int[(ymax)*(xmax)];
	int *clusterSizes = new int[(ymax)*(xmax)];
	ClusterFinder finder(IsOccupied, signed(xmax+2), signed(ymax+2), clusterSizes, grids);
	bool doQuench = 0;
	//	
	//	unsigned int biggestFire = getClustMaps1(bigClustMaps);
	unsigned int biggestFire = xmax*ymax;
	// now, the cluster map is established. as we plant trees, we will plant against this grid.
	//
	// seed the grid with some trees:
	Rand treeSeedx(rSeed + drSeed);
		drSeed ++;
	Rand treeSeedy(rSeed + drSeed);
		drSeed ++;
	// particularly for dendritic models, how many seeds to we want? in the purest form, we want 1, but that will take a LONG time to get started.
	for (unsigned int i=0; i < 5*xmax; i++) {
		// seed the grid with some trees
		//if (rand()%(xmax)==1 and i%(xmax+2)!=0 and (i+1)%(xmax+2)!=0 and i>(xmax+2) and i<((xmax+2)*(ymax+1)) ) grids[i]++;
		int thisX = treeSeedx.nextInt(xmax);
		int thisY = treeSeedy.nextInt(ymax);
		//thisClust = clustGrids[(xmax+2)*(thisY+1)+thisX+1];
		//grids[(xmax+2)*(thisY+1)+thisX+1] = thisClust;
		//thisClust = *(bigClustMaps + (xmax+2)*(thisY+1)+thisX+1);		
		*(grids + (xmax+2)*(thisY+1)+thisX+1) = 1;
		nTreesOnGrid ++;
		};
		//printf("initialized: %d\n", nTreesOnGrid);
		//yodapause();


	//printf("random grid established...\n");
	// printGrid(&grids[0], 1, xmax, ymax);
	//
	//unsigned int fcounts[xmax*ymax];
	unsigned int *fcounts = new unsigned int[xmax*ymax];
	//unsigned int *fcounts = new unsigned int[biggestFire];
		for (unsigned i=0; i<xmax*ymax; i++) {
			fcounts[i]=0;
			//*(fcounts + i) = 0;
			fireFprints[i]=0;
			sheepFprints[i]=0;
			};
			//printf("fcounts etc. initialized.\n");

	//
	unsigned i=0;		// use "while" to increment i until we have fmax->tmax size 1 fires...
	//for (unsigned int i=0; i<=tmax; i++) {
	while (fcounts[100]<tmax) {
	//while (i<=tmax) {
		i++;	// note: time starts at 1...
		//
		// first do the sheep:
		// note, we can remove sheep by setting sheepSpeed=0
		// use smarter sheep; sheep move to (and eat) an occupied site or at random.
		// printf("sheepSpeed/pos: %d/%d,%d\n", sheepSpeed, sheepPos/(xmax+2), sheepPos%(xmax+2));
		// for (int irand=0; irand<25; irand++) {printf("a random number: %d\n", sheepRand.nextInt(100));};
		// for (int irand=0; irand<25; irand++) {printf("a random number: %d\n", getRandDir((xmax+2), &sheepRand));};
		// yodapause();
		// printf("a sheep-loop (%d)\n", nSheep);
	//	bool sheepIsEating=0;
		for (int insheep=0; insheep<nSheep; insheep++) {
			//for (int isheep = 0; isheep<sheepSpeed; isheep++) {
			//if (sheepRand.nextInt(sheepSpeed)==0 or sheepIsEating==1) {
			//sheepfpxmin=sheepPos[insheep]%(xmax+2); sheepfpxmax=sheepfpxmin; sheepfpymin=sheepPos[insheep]/(xmax+2); sheepfpymax=sheepfpymin;
			sheepfpxmin=0; sheepfpxmax=0; sheepfpymin=0; sheepfpymax=0;
			//if (sheepRand.nextInt(sheepSpeed)==0) {
			if (1==1) {
				// printf("a random number: %d\n", getRandDir((xmax+2), &sheepRand));
				//printf("a sheep dir(%d, %d): %d\n", sheepPos, *(grids+sheepPos), getSheepDir(sheepPos, grids, (xmax+2), &sheepRand));
				//
				sheepIsEating=1;		// not really (necessarily), but it's how we control our loop:
				while (sheepIsEating==1) {
					//printf("a sheep dir(%d, %d): %d\n", sheepPos[insheep], *(grids+sheepPos[insheep]), getSheepDir(sheepPos[insheep], grids, (xmax+2), &sheepRand));
					// move the sheep. if he's standing on food, he will either treat the square equally with its occupied neighbors or
					// always eat there (see getSheepDir() ).
					//printf("still eating (%d)\n", i);
					// footprint:
					sheepPos[insheep] = sheepPos[insheep] + getSheepDir(sheepPos[insheep], grids, (xmax+2), &sheepRand);
						if (sheepPos[insheep]/(xmax+2)==0) sheepPos[insheep]=sheepPos[insheep] + (ymax+1)*(xmax+2);
						if (sheepPos[insheep]/(xmax+2)==(ymax+1)) sheepPos[insheep]=sheepPos[insheep]-ymax*(xmax+2);	// wanders off the bottom. 
						if (sheepPos[insheep]%(xmax+2)==0) sheepPos[insheep]=sheepPos[insheep]+xmax;	// off left side
						if (sheepPos[insheep]%(xmax+2)==(xmax+1)) sheepPos[insheep]=sheepPos[insheep]-xmax;	// off right side
					switch (*(grids+sheepPos[insheep])) {
						case 0: case rockAge:
							// move:
							// sheepPos[insheep] = sheepPos[insheep] + getSheepDir(sheepPos[insheep], grids, (xmax+2), &sheepRand);
							//
							// we might break this part in version j...
							newSheepPoop[insheep]++;	// this tag will tell us when to stop the loop.
							if (newSheepPoop[insheep]>1) {
								// we've finished a sheep-cluster
								//sheepArea=0;
								if (thisSheepPoop[insheep]>0) {
									sheepPoops[thisSheepPoop[insheep]-1]++;
									if (sheepfpxmax!=0 and sheepfpxmin!=0 and sheepfpymin!=0 and sheepfpymax!=0) {
										sheepArea=(sheepfpxmax-sheepfpxmin+1)*(sheepfpymax-sheepfpymin+1);
										//printf("sheepArea(%d): %d/%d\n",i, sheepArea, thisSheepPoop[insheep]);
										if (sheepArea>0) {sheepFprints[sheepArea-1]++; sheepArea=0;}
										};
									};
								thisSheepPoop[insheep]=0;
								sheepIsEating=0;
								};

							//sheepIsEating=0;
							break;
						case 1:
							// eat:
							*(grids+sheepPos[insheep])=0;
							nTreesOnGrid=nTreesOnGrid-1;
							sheepIsEating=1;
							// printf("sheepEat...: %d\n", nTreesOnGrid);
							//sheepPos[insheep] = sheepPos[insheep] + getSheepDir(sheepPos[insheep], grids, (xmax+2), &sheepRand);
							//sheepIsEating=1;
							thisSheepPoop[insheep]++;
							newSheepPoop[insheep]=0;		//reset newSheepPoop...
							//
							// and if we eat something, expand the min/max bits:
							//printf("sheep are eating..\n");
							if (sheepfpxmax==0 and sheepfpxmin==0 and sheepfpymin==0 and sheepfpymax==0) {
								// new sheep-area:
								sheepfpxmax=sheepPos[insheep]%(xmax+2);
								sheepfpxmin=sheepPos[insheep]%(xmax+2);
								sheepfpymax=sheepPos[insheep]/(xmax+2);
								sheepfpymin=sheepPos[insheep]/(xmax+2);								
								};
							if (sheepPos[insheep]%(xmax+2) < sheepfpxmin) {sheepfpxmin=sheepPos[insheep]%(xmax+2);}
							if (sheepPos[insheep]%(xmax+2) > sheepfpxmax) {sheepfpxmax=sheepPos[insheep]%(xmax+2);}
							if (sheepPos[insheep]/(xmax+2) < sheepfpymin) {sheepfpymin=sheepPos[insheep]/(xmax+2);}
							if (sheepPos[insheep]/(xmax+2) > sheepfpymax) {sheepfpymax=sheepPos[insheep]/(xmax+2);}
							//printf("sheepArea bits(%d): %d, %d, %d, %d :: %d, %d, (%d/%d)\n", i, sheepfpxmin, sheepfpxmax, sheepfpymin, sheepfpymax, sheepPos[insheep]%(xmax+2), sheepPos[insheep]/(xmax+2), (sheepfpxmax-sheepfpxmin+1)*(sheepfpymax-sheepfpymin+1), thisSheepPoop[insheep]);
							// sheepArea=(sheepfpxmax-sheepfpxmin+1)*(sheepfpymax-sheepfpymin+1);
							//printf("sheep finish eating..\n");
							break;
						default:
							// nothing.
							break;
						};
					// sheepIsEating=0;	// for display purposes, set this so we can watch the sheep eat the clusters.
					};
					//printf("sheep done eating.\n");
				}; //sheepSpeed
			}; // nSheep


		//
		//if (doSQL==5 or doSQL==6) if(i%1000000 == 0) printf("%d million\n", i/1000000);
		if (doSQL==6) if(i%1000000 == 0) printf("%d million\n", i/1000000);
		//
		// PLANT A TREE:
		// select a grid for tree planting:
		// yoder, v7: introduce dendritic growth. we have two prams, nRand, nClst. define a P(rand)-> Pr, P(clust)->Pc: Pr=nRand/(nRand+nClust), etc.
		//for (unsigned int irp=0; irp<randPlant; irp++) {
		thisX = plantRandx.nextInt(xmax);
		thisY = plantRandy.nextInt(ymax);
		gridPos = (xmax+2)*(thisY+1)+1+thisX;
		//
		if (hasNeighbor(grids, gridPos)<=kmax and *(grids+gridPos) == 0) {
			*(grids+gridPos) = 1;
			nTreesOnGrid=nTreesOnGrid+1;
			// printf("Plant...: %d\n", nTreesOnGrid);
			};
		//
// debug:
//		if (i%10000==0) {
//			plotGridSimple (grids, i, xmax, ymax);
//			};
		// check grid stats (but we only need to count the grid so many times (and if we have to be careful not to overrun
		// our integers. also, we might not get perfect power-laws, so it's hard to know exactly when to stop counting clusters.:
		if (rand()%(5*fmatch)==1 and clusterSizes[10]<pow(10,6.2)) {
			//printf("find cluster sizes...\n");
			finder.find();
			//printf("**found cluster sizes... %d, %d\n", totalFireCount, i);
			}
	// we've planted a tree. do we throw a match?
	// a 1 in fmatch chance (use any value between 0 and fmatch)
		if (rfmatch.nextInt(fmatch) == 1) {
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
			//if (*fireSquare >= burnAge and *fireSquare < rockAge) {				
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
					unsigned int currentElementIndex=nBurning;
					//printf("fire front burning. fireIndex=%d\n", fireIndex);
					// these sequence randomization bits are left over from MFI, basically we randomize the order in which new fire elements are evaluated
					// to mitigate directional bias. these bits are probably not necessary for SHEEP and can be removed for comp. efficiency.
					// for simplicity, let's leave the code in place, leave the option and just skip the randomization.
				// 10 dec 2010 yoder (randomized vs speed bit)
				//	int * randomFireSequence = new int[dnBurning];
				//	int * testSeq = new int[dnBurning];
				//	for (int itest=0; itest<dnBurning; itest++) {
				//		*(testSeq + itest) = itest;
				//		}
				//	scrambleList(testSeq, randomFireSequence, dnBurning);
				//	delete [] testSeq;
					
					//if (dnBurning>1) randomSequence(randomFireSequence, dnBurning);	// randomize order in which we evalueate fire propagation.
					//for (int ilist=0; ilist<dnBurning; ilist++) {
					//	printf("randomSequence[%d]=%d\n", ilist, randomFireSequence[ilist]);
					//	};
					//
					while (fireIndex < (nBurning+dnBurning) and doQuench==0) {
						//newFireIndex=fireIndex-nBurning;
						//
						// 10 dec 2010 yoder (randomized vs speed bit (comment next line; add second).
						//currentElementIndex=nBurning+randomFireSequence[fireIndex-nBurning];	// this is long-winded because i'm building it off the simpler non-randomized model.
						//
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
						// 10 dec 2010 yoder (randomized vs speed bit)
					//	randomSequence(newOrder,4);
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
							//if (grids[fireList[currentElementIndex] + directions[idir]]==1) {
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
								// 10 dec 2010 yoder:
								currentElementIndex++;
								};
							//
							// MFI (after each element is tested to the fire). this is only allowed at this step for type III propagation (above). if we apply immunity
							// as each element is added and elements are added in some geometrical sequence (aka, around the fire-front), we break the symmetry and break
							// SOC between integer values of L^2. by itself, using the list method (in particular when we randomize direction) might fix this problem,
							// since it breakes down the spiral geometry of our former concentric square propagation.
								
							};
						fireIndex++;

						// evaluate immunity (as each burning (fire-front) element propagates):
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
					//delete [] randomFireSequence;
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
				//
				if (doQuench==1) extinguishedFires[nBurning + dnBurning + newFireElements-1]++;
				//
				// write to file:
				if (nFires%1000==0) {
					printf("Ns: %d, %d\n", nFires, fcounts[100]);
					fstats.open(statFileName.c_str());
					fstats << "# immunity: doImPrim: " << doImPrim << ", doImFlameFront: " << doImFlameFront << ", doImElem: " << doImFireElement <<"\n";
					fstats << "# prams: tmax=" << tmax << ", fmatch=" << fmatch << ", doSQL=" << doSQL << ", pImmune=" << pImmune << ", kImmune=" << kImmune << ", immCode=" << immCode.c_str() << "nSheep:=" << nSheep << "\n";
					fstats << "#k\tN_fires\tN_extinguishedFires\tN_clusters\tN_sheepPoop\n";
					for (unsigned int i=0; i<(xmax*ymax); i++) {
						if (fcounts[i]!=0 or extinguishedFires[i]!=0 or clusterSizes[i]!=0) {
							fstats << i+1 << "\t" << fcounts[i] << "\t" << extinguishedFires[i] << "\t" << clusterSizes[i] << "\t" << sheepPoops[i] << "\n";
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
		};	// end sim-steps.
	// end simulation.
	//
	// write data file (this should be a function call...):
	fstats.open(statFileName.c_str());
	fstats << "# immunity: doImPrim: " << doImPrim << ", doImFlameFront: " << doImFlameFront << ", doImElem: " << doImFireElement <<"\n";
	fstats << "# prams: tmax=" << tmax << ", fmatch=" << fmatch << ", doSQL=" << doSQL << ", pImmune=" << pImmune << ", kImmune=" << kImmune << ", immCode=" << immCode.c_str() << "nSheep:=" << nSheep << "\n";
	fstats << "#k\tN_fires\tN_extinguishedFires\tN_clusters\tN_sheepPoop\n";
	for (unsigned int i=0; i<(xmax*ymax); i++) {
		if (fcounts[i]!=0 or extinguishedFires[i]!=0 or clusterSizes[i]!=0) {
			fstats << i+1 << "\t" << fcounts[i] << "\t" << extinguishedFires[i] << "\t" << clusterSizes[i] << "\t" << sheepPoops[i] << "\n";
			};
		};
	fstats.close();

	//printf("doSQL: %d\n", doSQL);
	// doSQL's:
	// 0: 'print-grid" fires
	// 1: full SQL: insert each forest fire data-set into ForestFires
	// 2: print summary to screen. use this for direct gnuplot calls, " plot '<./ffire4...'" 
	// 3: Print each fire to screen; "plotGrid" each fire nBurning>(25) print summary to screen at end
	// 4: plotGridImg each fire nBurning>(20); print summary to screen,
	// 5: SQL: insert just summary data to SQL; prints progress by million (will screw up plotting)
	// 25: 2 & 5: print summary to screen, insert summary to sql.
	// 6: report summary, progress by million.
	// 11: return to standard-output the last grid in full. use to make an image of the final grid.
	//
	// end-o-run summaries (print):
	if (doSQL==2 or doSQL==3 or doSQL==4 or doSQL==6 or doSQL==25) {
		for (unsigned int i=0; i<(biggestFire); i++) {
			//if (fcounts[i]!=0) printf("%d\t%d\n", i+1, fcounts[i]);
			// if (*(fcounts + i) != 0) printf("%d\t%d\n", i+1, *(fcounts + i));
			// if (*(fcounts + i) != 0 or sheepPoops[i]!=0) printf("%d\t%d\t%d\n", i+1, *(fcounts + i), sheepPoops[i]);
			if (*(fcounts + i) != 0 or sheepPoops[i]!=0 or fireFprints[i]!=0 or sheepFprints[i]!=0) printf("%d\t%d\t%d\t%d\t%d\n", i+1, *(fcounts + i), sheepPoops[i], fireFprints[i], sheepFprints[i]);
			//printf("%d,\t%d\n", i+1, fcounts[i]);
			};
		} else {
			printf("finished.\nfire size, nFires, totalBurned: (%d) (%d) (%d)\n", nBurning, nFires, nBurnedTotal);
			};
/*
	if (doSQL==5 or doSQL==25) { // no plots, just a summary -> SQL
		//
		mysqlpp::Connection myconn2(DB, HOST, USER, PASSWORD);
		mysqlpp::Query myquery2=myconn2.query();
		// mysqlpp::Result res2;
		//
		for (unsigned int i=0; i<(biggestFire); i++) {
			// if (fcounts[i]!=0) printf("%d\t%d\n", i+1, fcounts[i]);
		//	printf ("sql bits: %d, %d, %d, %d\n", intKey, tmax, i+1, fcounts[i]);
			myquery2.reset();
			myquery2 << "insert into ffcounts (simIndex, tmax, nBurned, nEvents) values (%0q, %1q, %2q, %3q)";
			myquery2.parse();
			myquery2.execute(intKey, tmax, i+1, fcounts[i]);
			//printf("%d,\t%d\n", i+1, fcounts[i]);
			};				
		};
*/
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
//	unsigned int kmax = 4;
	unsigned int nSheep=1;
	std::string foutputname="results/ffire10b/ffier10b-temp.dat";
//	unsigned int rhoRocks = 0;
//	float pImmune = 0;
//	int nnnAge = 0;
	//
	if (argv[1]) tmax=yodacode::StrToInt(argv[1]);
	if (argv[2]) doSQL=yodacode::StrToInt(argv[2]);
	if (argv[3]) fmatch=yodacode::StrToInt(argv[3]);
//	if (argv[7]) sheepSpeed=yodacode::StrToInt(argv[7]);
	if (argv[4]) nSheep=yodacode::StrToInt(argv[4]);
//	if (argv[9]) pImmune = yodacode::StrToFloat(argv[9]);
//	if (argv[10]) nnnAge = yodacode::StrToInt(argv[10]);
	if (argv[5]) foutputname = argv[5];
	
	// printf("args: %d, %d, %d, %d, %d, %d, %d, %d\n", tmax, doSQL, fmatch, nRand, nClust, kmax, sheepSpeed, nSheep);
	//printf("args: %d, %d, %d\n", tmax, doSQL, fmatch);
	//yodapause();
	doffire(tmax, doSQL, fmatch, nSheep, foutputname);
	return 0;
	};

