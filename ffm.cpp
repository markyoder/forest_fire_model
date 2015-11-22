/**
   g++ -Wall -O3 -o ffm ffm.cpp
*/

#include <fstream>
#include <iostream>
#include <cassert>
#include <cmath>

using namespace std;

# define MBIG 1000000000
# define MSEED 161803398
# define MZ 0
# define FAC (1.0/MBIG)

long SeedKnuth = -123456;

double rand3 () {
    static int inext, inextp;
    static long ma[56];
    static int iff=0;
    long mj, mk;
    int i, ii, k;

    if(SeedKnuth < 0 || iff == 0) {
        iff = 1;
        mj = MSEED-(SeedKnuth < 0 ? -SeedKnuth : SeedKnuth);
        mj %= MBIG;
        ma[55] = mj;
        mk = 1;
        for(i=1; i<=54; i++) {
            ii = (21*i) % 55;
            ma[ii] = mk;
            mk = mj-mk;
            if(mk < MZ) mk += MBIG;
            mj = ma[ii];
        }
        for(k=1; k<=4; k++)
            for(i=1; i<=55; i++) {
                ma[i] -= ma[1+(i+30) % 55];
                if(ma[i] < MZ) ma[i] += MBIG;
            }
        inext = 0;
        inextp = 31;
        SeedKnuth = 1;
    }
    if(++inext == 56) inext = 1;
    if(++inextp == 56) inextp = 1;
    mj  = ma[inext]-ma[inextp];
    if(mj < MZ) mj += MBIG;
    ma[inext] = mj;
    if(mj == MBIG) return 0.0; // to avoid returing 1.0 exactly
    else return mj*FAC;
    //return mj*FAC;
}

unsigned LinearSize;
unsigned Volume;
unsigned* Grid;
unsigned* Stat;
unsigned const EmptyInd = 0;

unsigned inline getRandom() { return 1 + unsigned(Volume*rand3()); }
double   inline getTime(long long step) { return step*1.0/Volume; }

void init(char* fname) {
    try {
        Grid = new unsigned[Volume+1];
        Stat = new unsigned[Volume+1];
    }
    catch(bad_alloc) {
        throw "no space for data";
    }
    for(unsigned i=0; i<=Volume; i++) Grid[i] = Stat[i] = 0;
}

unsigned EventSize;
// memory grows to the right and down
void visitPeriodic(unsigned ind) {
    Grid[ind] = 0;
    EventSize++;

    unsigned indR = ind < Volume ? ind+1 : 1;
    if(Grid[indR]) visitPeriodic(indR);

    unsigned indD = ind <= Volume-LinearSize ? ind+LinearSize : ind%LinearSize;
    if(Grid[indD]) visitPeriodic(indD);

    unsigned indL = ind > 1 ? ind-1 : Volume;
    if(Grid[indL]) visitPeriodic(indL);

    unsigned indU = ind > LinearSize ? ind-LinearSize : Volume-LinearSize+ind;
    if(Grid[indU]) visitPeriodic(indU);
}

// non periodic (free)
void visit(unsigned ind) {
    Grid[ind] = 0;
    EventSize++;

    unsigned indR = ind < Volume ? ind+1 : EmptyInd;
    if(Grid[indR]) visit(indR);

    unsigned indD = ind <= Volume-LinearSize ? ind+LinearSize : EmptyInd;
    if(Grid[indD]) visit(indD);

    unsigned indL = ind > 1 ? ind-1 : EmptyInd;
    if(Grid[indL]) visit(indL);

    unsigned indU = ind > LinearSize ? ind-LinearSize : EmptyInd;
    if(Grid[indU]) visit(indU);
}

unsigned runEvent(unsigned ind0) {
    EventSize = 0;
    visit(ind0);
    return EventSize;
}

int main(int argc, char** argv) {
    if(argc != 5) {
        cout << "Usage:\nLinearSize FiringRate EventCnt StatFile" << endl;
        exit(1);
    }

    LinearSize = atoi(argv[1]);
    assert(LinearSize > 1);
    
    Volume = LinearSize*LinearSize;

    double rate = atof(argv[2]);
    assert(rate > 0);
    assert(rate < 1);

    unsigned eventCntMax = atoi(argv[3]);
    assert(eventCntMax > 0);

    init(argv[4]);
    unsigned eventCnt = 0;
    unsigned filledCnt = 0;
    long long sinceLastGlobalEvent = 0;

    while(1) {
        // FIND STEP FOR THE NEXT TRIAL EVENT
        double rand = rand3();
        while(rand == 0.0) rand = rand3(); // to avoid log(0.0)
        unsigned nextTrialEvent = 1 + unsigned(log(rand)/log(1.0 - rate));
        sinceLastGlobalEvent += nextTrialEvent;

        for(unsigned sc=0; sc<nextTrialEvent; sc++) {
            unsigned ind = getRandom();
            if(Grid[ind] == 0) {
                Grid[ind] = 1;
                filledCnt++;
            }
        }

        // SEE IF WE SHOULD RUN THE EVENT
        unsigned ind0 = getRandom();
        if(Grid[ind0]) {
            eventCnt++;
            unsigned eventSize = runEvent(ind0);
            filledCnt -= eventSize;
            Stat[eventSize]++;
            if(eventSize == Volume) {
                cout << getTime(sinceLastGlobalEvent) << endl;
                sinceLastGlobalEvent = 0;
            }
            if(eventCnt == eventCntMax) break;
        }
    }

    unsigned sum = 0;
    for(unsigned i=1; i<=Volume; i++) sum += Grid[i];
    assert(sum == filledCnt);

    ofstream statFile(argv[4]);
    assert(statFile.good());

    for(unsigned s=1; s<=Volume; s++) {
        if(Stat[s]) statFile << s << '\t' << Stat[s] << endl;
    }

    statFile.close();
    return 0;
}


