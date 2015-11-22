

class ffireModel {
	// this is the body of the ffire model. it will contain all the pertinent variables
	// and the doFfireStep() function, or whatever we call it.
	// most of the varialbes won't be declared here; they will be private non-member variables.
	//
	public:
		//ffireModel(int tmax, int doSQL, int fMatch, int nRand, int nClust, int kmax, int sheepSpeed, int nSheep);
		ffireModel();
		void init(int tmax, int doSQL, int fMatch, int nRand, int nClust, int kmax, int sheepSpeed, int nSheep);
		//
		void doFFstep();
		int * getfCounts();
		int * getSheepPoops();
		int * getGrids();
		int getSheepPos();
		int getTimeStep();
		void setTimeStep(int val);
		void setSimName(std::string strName);
		void setSimPramComment(std::string strComment);
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
		int tmax, doSQL, fMatch, nRand, nClust, kmax, sheepSpeed, nSheep;
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

