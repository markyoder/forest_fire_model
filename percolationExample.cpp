/**
   Minimalistic percolation model for cluster identification in simple C++

   Very simple and very fast Hoshen and Kopelman algorithm implementation

   g++ -Wall -pedantic -O3 -o percolation percolation.cpp

   Gleb
*/

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <cassert>

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::ofstream;

//-------------------------------------------------------------------------
// 2D, nearest neighbors, non-periodic. If N = total grid size then:
// Worst run time ~ N*Log(N), in reality ~ N
// Memory ~ 0.5*N (statistics storage is not included), could be 0.2*N
class ClusterFinder {
public:
    ClusterFinder(bool (*isOn_) (int, int), int xL, int yL, int* stats_) {
        isOn  = isOn_;
        xSize = xL;
        ySize = yL;
        stats = stats_;
        totalSize = xSize*ySize;
        int labelCnt  = int(0.2*totalSize+1); // 0.2 should work too

        try {
            row = new unsigned[xSize + 1]; // for row[-1]
            map = new signed[labelCnt];
        }
        catch(std::bad_alloc) {
            cerr << "Error: out of memory in ClusterFinder" << endl;
            exit(1);
        }

        row++; // so we can use index -1: row[-1]
        for(int i=1; i<labelCnt; i++) map[i] = 0;
    }

    ~ClusterFinder() {
        delete[] --row;
        delete[] map;
    }

    void find() {
        for(int x=-1; x<xSize; x++) row[x] = 0;
        unsigned nextLbl = 1;  // index of the next single
        int emptyCnt = 0;

        for(int y=0; y<ySize; y++) {
            for(int x=0; x<xSize; x++) {
                if(isOn(x, y) == false) { // is empty
                    row[x] = 0;
                    emptyCnt++;
                    continue; // quick out
                }

                unsigned lbl  = nextLbl;   // init to no neighbors
                unsigned left = row[x-1]; // memory goes from left to right
                unsigned up   = row[x];   // and from top to bottom

                if(left != 0) lbl = left = getRootLabel(left);
                if(up   != 0) lbl = up   = getRootLabel(up); // keep the up

                if(lbl == nextLbl) nextLbl++; // no neighbors
                else if(left != 0 && up != 0 && left != up) {
                    // for simplicity we keep the up label
                    map[up] += map[left];   // add all from the left
                    map[left] = -up;        // and remap left to the up
                }

                row[x] = lbl;
                map[lbl]++;
            }
        }

        int siteCnt = 0;
        for(unsigned i=1; i<nextLbl; i++) { // i<nextLbl, correct
            int size = map[i];
            map[i] = 0;
            if(size > 0) {
                stats[size]++;
                siteCnt += size;
            }
        }
        assert(emptyCnt + siteCnt == totalSize); // simple check
    }

private:
    unsigned getRootLabel(unsigned lbl) {
        while(map[lbl] < 0) lbl = -map[lbl];
        return lbl;
    }

    bool (*isOn) (int, int);
    int xSize;
    int ySize;
    int totalSize;
    unsigned* row;      // >= 0
    signed* map;        // > 0 - size, < 0 - pointer to another label
    int* stats;
};

//-------------------------------------------------------------------------
char* Grid;
int LinearSize;
bool IsOn(int x, int y) { return Grid[LinearSize*y + x] > 0; }

//-------------------------------------------------------------------------
int main() {

    // USER INPUT, NOT CHECKED!
    cout << "Enter the following:" << endl;

    cout << "linear size: ";
    cin  >> LinearSize;

    double prob;
    cout << "probability: ";
    cin  >> prob;

    int sampleCnt;
    cout << "sample cnt : ";
    cin  >> sampleCnt;

    string fileName;
    cout << "file name  : ";
    cin  >> fileName;

    // INITIALIZATION
    int totalSize = LinearSize*LinearSize;
    int* stats    = 0;

    try {
        Grid  = new char[totalSize];
        stats = new int[totalSize+1]; // for cluster size = totalSize
    }
    catch(std::bad_alloc) {
        cerr << "Error: out of memory in main" << endl;
        exit(1);
    }

    for(int i=0; i<=totalSize; i++) stats[i] = 0;
    ClusterFinder finder(IsOn, LinearSize, LinearSize, stats);

    // RUNNING
    cout << endl;
    for(int s=0; s<sampleCnt; s++) {
        cout << s << endl;
        for(int i=0; i<totalSize; i++) Grid[i] = char(drand48() < prob);
        finder.find();
    }

    // WRITE STATISTICS
    ofstream file(fileName.c_str());
    assert(file.good());

    for(int i=1; i<=totalSize; i++) {
        if(stats[i]) file << i << "\t" << stats[i] << endl;
    }

    // CLEANUP
    file.close();
    delete[] Grid;
    delete[] stats;

    return 0;
}
//-------------------------------------------------------------------------
