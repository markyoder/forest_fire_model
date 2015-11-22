
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
//
int printGrid2(signed int *grids, unsigned int t, int X, int Y, int sheepPos) {
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
		if (i==sheepPos) {
			printf(" &&& ");
			}
		else {
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
		};
//		if (treeAge==sheepAge) {
//			printf(" SsS ");
//			};

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
//
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
//		if (treeAge==sheepAge) {
//			printf(" SsS ");
//			};
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

int getRandDir (int gridSize, Rand myRand) {	
	gridSize = xmax+2;
	//int thisDir = myRand.nextInt(4);
	int thisDir = rand()%4;
	int dirVal = 0;
	printf("thisDir: %d\n", thisDir);
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

bool inGrid (int gridPos, int gridX, int gridY) {
	// is this new position in a grid with border width/height 1?
	bool isIn=1;
	//if (gridPos<(gridX+2)) isIn=0;
	if (gridPos/(gridX+2)==0 or gridPos/(gridX+2)>=(gridY+1)) isIn=0;	// top or bottom row
	if (gridPos%(xmax+2)==0 or gridPos%(xmax+1)==0) isIn=0;
	//
	return isIn;
	};

int getSheepDir (unsigned int sheepPos, int *theGrid, int gridX, Rand thisRand) {
	// smart sheep will move toward something to eat if it is adjacent.
	// eventually, we might re-introduce tree age; sheep can only eat young trees and
	// must move around old trees.
	// do fires kill sheep?
	//
	int foodDirections[4] = {0,0,0,0};
	int nFoodDirs=0, sheepDir=0;
	//
	if (*(theGrid + sheepPos + 1)==1) {
		foodDirections[nFoodDirs]=1;
		nFoodDirs++;
		};
	if (*(theGrid + sheepPos - 1)==1) {
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
	if (nFoodDirs>0) {
		// there is something we can eat...
		sheepDir = foodDirections[thisRand.nextInt(nFoodDirs)];
		}
	else {
		sheepDir = getRandDir(gridX, thisRand);
		if (inGrid(sheepPos+sheepDir, gridX, gridX)==false) {
			//sheepDir = thisRand.nextInt(ymax)*(xmax+2) + thisRand.nextInt(xmax) + 1 - sheepPos;	// pick a point on the grid, subtract current pos; go that many...
			sheepDir = rand()%((ymax)*(xmax+2)) + rand()%xmax + 1 - sheepPos;	// pick a point on the grid, subtract current pos; go that many...
			};
		// space sheep: teleport and look again...
		// sheepDir = thisRand.nextInt(ymax)*(xmax+2) + thisRand.nextInt(xmax) + 1 - sheepPos;	// pick a point on the grid, subtract current pos; go that many...
		};
	//
	return sheepDir;
	};

