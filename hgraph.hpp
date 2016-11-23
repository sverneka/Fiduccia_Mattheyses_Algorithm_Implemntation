#ifndef HGRAPH_HPP
#define HGRAPH_HPP

#ifndef NULL
#define NULL 0
#endif

#define other(num) ((num)?0:1)
#define LOCKED -10000
#include "ll.hpp"

extern int SEED; // random seed for initial partitioning
extern double RATIO; // ratio of balance between partitions

typedef LL *ll;   // ptr to a linked list
struct net;       // predefine net

struct cell
{
int number;       // index # of cell in cells array
LL first;         // ll of nets on cell
node *gainbucket; // ptr to cell in the bucket lists
int block;        // which block (partition) the cell is in
int gain;         // gain of cell
int gainfromlock; // gain from moving and locking cell
cell(int num,int side); // constructor
cell() {}         // default constructor
};

struct net
{
int number;     // number of cells on net
LL first;       // ll of cells on net
int unlock[2];  // # of unlocked cells in each block
int lock[2];    // # of locked cells in each block
net(int num);   // constructor, assigns num to number
net() {}        // default constructor
};

class parthgraph
{
private:
int numcells;   // number of cells in hgraph
int numnets;    // number of nets in hgraph
double ratio;   // ratio of balance between the blocks
int minsize[2]; //
cell *cells;    // array of cells in hgraph
net *nets;      // array of nets in hgraph
int pmax;       // max number of pins per cell
ll buckets[2];  // buckets for gain updates
LL free;        // ll of free cells
int maxgain[2]; // max gain of any cell in each block
int balance[2];
int passes;     // number of passes

public:
parthgraph(void){}  // constructor
//void getgraph(char* filename); // loads hgraph filename from disk
void getgraph();
void part(void);               // performs partitioning
void initgains(void);          // initializes gains and buckets
void swapall(void);
void getprefix(int& prenum);
void reinit(int prenum);
cell* gethighest(void);
void fixgain(LL &first,int operation,int single=0,int sameside=0);
int cutset(void);
void print(char *str);
// debug functions
void printCells(); // printout cell info
void printBucket(); // printout top buckets
};

#endif
