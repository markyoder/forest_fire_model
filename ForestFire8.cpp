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
#include "ffirebits.h"
//#include "clsffire.h"

// mysql elements:
//#include <mysql.h>
//#include <mysql++.h>
// #include "/usr/local/include/mysql++/mysql++.h"


#include <stdlib.h>
#include <cmath>
#include <cassert>
#include <iomanip>

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
// ::: looks like 7c (note, there are two versions of the thinning algorithm) don't do anything. moving on to sheep...
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
// VERSION 8:
//	build classes so we can use QT to visualize.
//
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

// int doffire(unsigned int xmax=128, unsigned int ymax=128, unsigned int fmatch=125 unsigned short burnPropAge=400) {
// use simpler float nRand, nClust probabilities...

int sumIntArray(int * thisArray, int minArray=0, int maxArray=0) {
	int thisTotal=0;
	for (int i=minArray; i<=maxArray; i++) {
		thisTotal = thisTotal + *(thisArray+i);
		};
	//
	return thisTotal;
	};


int main(int argc, char *argv[]) {
	//ffPlots();
	int tmax=1000000;
	int doSQL = 4;
	int fmatch=200;
	int nRand = 1;
	int nClust = 0;
	int kmax = 4;
	int sheepSpeed = 5;
	int nSheep = 1;
	//
	if (argv[1]) tmax=yodacode::StrToInt(argv[1]);
	if (argv[2]) doSQL=yodacode::StrToInt(argv[2]);
	if (argv[3]) fmatch=yodacode::StrToInt(argv[3]);
	if (argv[4]) nRand=yodacode::StrToInt(argv[4]);
	if (argv[5]) nClust=yodacode::StrToInt(argv[5]);
	if (argv[6]) kmax=yodacode::StrToInt(argv[6]);
	if (argv[7]) sheepSpeed=yodacode::StrToInt(argv[7]);
	if (argv[8]) nSheep=yodacode::StrToInt(argv[8]);
	//
	//myClass myhello;
	//myhello.setHello2("this is hello2.");
	//myhello.sayHello("hello from primary hello");
	//myhello.sayHello2();
	//printf("hello2Str: %s\n", myhello.getHello2().c_str());

	//doffire(tmax, doSQL, fmatch, nRand, nClust, kmax, sheepSpeed);
	//
	ffireModel thisFFire;
	//thisFFire.init(0, 1000, 1, 0, 4, 5, 1);
	thisFFire.init(doSQL, fmatch, nRand, nClust, kmax, sheepSpeed, nSheep);
	// there are less stupid ways to do this. grab the address of the grids array; use it to control
	int * myFireCounts = thisFFire.getfCounts();
	int * mySheepPoops = thisFFire.getSheepPoops();
	// while (*myGrids<=1000) {		//for total number of events, we track it in the class or make a sum function (ctest2.cpp???)
	//
	// debug:
	// printf("initialized myGrids, now start steps.\n");
	thisFFire.doNsteps(tmax);
	//thisFFire.doNsizeOne(tmax);
	//
	//printf("biggest fire: %d\n", thisFFire.getBiggestFire());
	// display summary results (we'll wrap this into an external or class function):
	for (int i=0; i<thisFFire.getBiggestFire(); i++) {
	//for (int i=0; i<100; i++) {
		//if (fcounts[i]!=0) printf("%d\t%d\n", i+1, fcounts[i]);
		//if (*(myFireCounts + i) != 0)
		 printf("%d\t%d\t%d\n", i+1, *(myFireCounts + i), *(mySheepPoops+i));
		//printf("%d,\t%d\n", i+1, fcounts[i]);
		};
	
	return 0;
	};

