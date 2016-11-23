#include <fstream.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "iostream.h"
#include "hgraph.hpp"

//#define debug

void error(char* s, char* s2="")
{
   //cerr<<s<<' '<<s2<<'\n';
   exit(1);
}

cell::cell(int num,int side)
{
   number=num;
   block=side;
   gainbucket=NULL;
   gain=0;
   gainfromlock=LOCKED;
}

net::net(int num)
{
   number=num;
   unlock[0]=unlock[1]=0;
   lock[0]=lock[1]=0;
}


void parthgraph::getgraph(char *filename)
{
   int side=0, count, padoffset, temp;
   node *nnode;
   char type, state, line[80];
   FILE *fr;

   srand(SEED);
   balance[0]=0;
   balance[1]=0;

   fr = fopen(filename, "r");
   if(!fr)  error("cannot open input file ",filename);
   fscanf(fr, "%*d %*d %d %d %d", &numnets, &numcells, &padoffset); 

   cells=(cell*)new cell[numcells];
   if(!cells) error("memory allocation error");
   minsize[0]=minsize[1]=int(ceil(numcells*RATIO));
   int size=numcells>>1;
   int max[2]={size, numcells - size};  // max size of each part
   // ***read in nodes
   for(count=0;count<numcells;count++)
   {
	  cells[count].number=count;
	  side=(rand() & 128)?1:0;
	  if(balance[side]==max[side]) side=other(side);
	  cells[count].block=side;
	  cells[count].gainbucket=NULL;
	  cells[count].gain=0;
	  cells[count].gainfromlock=LOCKED;
	  balance[side]++;
   }
   // ***read in nets
   nets=(net*)new net[numnets];
   if(!nets) error("memory allocation error");
   count = -1;
   while (fgets(line, 80, fr) != NULL) 
   {
	  if (sscanf(line, "%c%d %c", &type, &temp, &state) != 3) 
		 continue;
	  if (type == 'p')
		 temp += padoffset;
	  if (state == 's') {
		 count++;
		 nets[count].number=0;
		 nets[count].lock[0]=0;
		 nets[count].lock[1]=0;
		 nets[count].unlock[0]=0;
		 nets[count].unlock[1]=0;
	  }
	  nnode=new node(count);
	  if(!nnode) error("memory allocation error");
	  cells[temp].first.addhead(nnode);
	  nnode=new node(temp);
	  if(!nnode) error("memory allocation error");
	  nets[count].first.addhead(nnode);
	  nets[count].unlock[cells[temp].block]++;
	  nets[count].number++;
   }
   pmax=0;
   for(count=0;count<numcells;count++)
   {
	  if(cells[count].first.length>pmax)
		 pmax=cells[count].first.length;
   }
   buckets[0]=(ll) new LL[2*pmax+1];
   buckets[1]=(ll) new LL[2*pmax+1];
   if(!buckets[0] || !buckets[1]) error("memory allocation error");
}

void parthgraph::initgains(void)
{

   int count;
   int from,to;

   // ****calc gain of unlocked cells
   for (count=0;count<numcells;count++)
   {
	  if(cells[count].gainfromlock==LOCKED)
	  {
		 cells[count].first.reset();
		 from=cells[count].block;
		 to=other(from);
		 while(cells[count].first.current!=NULL)
		 {
			int nnum=cells[count].first.current->number;
			if( (nets[nnum].unlock[from]+nets[nnum].lock[from])==1)
			   cells[count].gain++;
			if( (nets[nnum].unlock[to]+nets[nnum].lock[to])==0)
			   cells[count].gain--;
			++cells[count].first;
		 }
	  }
   }

   maxgain[0]=-pmax;
   maxgain[1]=-pmax;

   node *nnode;
   for(count=0;count<numcells;count++)
   {
	  if(cells[count].gainbucket==NULL)
	  {
		 nnode=(node*)new node(count);
		 if(!nnode) error("memory allocation error");
		 cells[count].gainbucket=nnode;
	  }
	  else
		 nnode=cells[count].gainbucket;
	  buckets[cells[count].block][cells[count].gain+pmax].addhead(nnode);
	  if(cells[count].gain>maxgain[cells[count].block])
		 maxgain[cells[count].block]=cells[count].gain;
   }
#ifdef debug1
   printCells();
   exit(0);
#endif
}


void parthgraph::part(void)
{
   int prefix=0; // holds prefix sum of gains
   int done=0;
   //int joe=1;
   passes=0;   // num of passes

   while(!done)
   {
	  passes++;
	  swapall();
	  getprefix(prefix);
	  if(prefix<=0)
		 done=1;
	  reinit(prefix);
   }
}

void parthgraph::swapall(void)
{
   int from,to;
   cell *bestcell;
   net *curnet;
   int done=0;

   bestcell=gethighest();

   while(!done)
   {
#ifdef debug
	  printBucket();
	  printf("base cell=%d(%d) gain=%d\n", bestcell->number, bestcell->block, 
			 bestcell->gain);
	  char go;
	  scanf(" %c", &go);
#endif
	  from=bestcell->block;
	  to=other(from);
	  bestcell->block=to;
	  bestcell->gainfromlock=bestcell->gain;
	  bestcell->first.reset();
	  balance[from]--;
	  balance[to]++;
	  while(bestcell->first.current != NULL)
	  {
		 curnet=&nets[bestcell->first.current->number];

		 if(curnet->lock[to]==0)
		 {
			if(curnet->unlock[to]==0)
			   fixgain(curnet->first,1);
			else if(curnet->unlock[to]==1)
			   fixgain(curnet->first,-1,1,to);
		 }

		 curnet->unlock[from]--;
		 curnet->lock[to]++;

		 if(curnet->lock[from]==0)
		 {
			if(curnet->unlock[from]==0)
			   fixgain(curnet->first,-1);
			else if(curnet->unlock[from]==1)
			   fixgain(curnet->first,1,1,from);
		 }
		 ++(bestcell->first);
	  }
	  bestcell=gethighest();
	  if(bestcell==NULL)
		 done=1;
   }
}

void parthgraph::fixgain(LL &first,int operation,int single,int sameside)
{
   cell *cellptr;
   int block;

   first.reset();
   if(operation==1)
   {
	  while(first.current!=NULL)
	  {
		 cellptr=&cells[first.current->number];
		 if(cellptr->gainfromlock==LOCKED)
		 {
			block=cellptr->block;
			if( !single || (block==sameside) )
			{
			   if( cellptr->gain==maxgain[block] )
				  maxgain[block]++;
			   buckets[block][cellptr->gain+pmax].removenode(cellptr->gainbucket);
			   cellptr->gain++;
			   buckets[block][cellptr->gain+pmax].addhead(cellptr->gainbucket);
			}
		 }
		 ++first;
	  }
   }
   else
   {
	  while(first.current!=NULL)
	  {
		 cellptr=&cells[first.current->number];
		 if(cellptr->gainfromlock==LOCKED)
		 {
			block=cellptr->block;
			if( !single || (block==sameside) )
			{
			   if( (cellptr->gain==maxgain[block]) && (buckets[block][cellptr->gain+pmax].length==1) )
				  maxgain[block]--;
			   buckets[block][cellptr->gain+pmax].removenode(cellptr->gainbucket);
			   cellptr->gain--;
			   buckets[block][cellptr->gain+pmax].addhead(cellptr->gainbucket);
			}
		 }
		 ++first;
	  }
   }
}

cell* parthgraph::gethighest(void)
{
   cell *tmpcell=NULL;
   node *tmpnode=NULL;

   if( (maxgain[0]>=maxgain[1]) )
   {
	  if(balance[0]>=minsize[0])
	  {
		 tmpnode=buckets[0][maxgain[0]+pmax].removehead();
		 if(tmpnode!=NULL)
		 {
			tmpcell=&cells[tmpnode->number];
			free.addtail(tmpnode);
		 }
		 while( (!buckets[0][maxgain[0]+pmax].length) && (maxgain[0]>-pmax) )
			maxgain[0]--;
	  }
   }
   if(tmpcell==NULL)
   {
	  if(balance[1]>=minsize[1])
	  {
		 tmpnode=buckets[1][maxgain[1]+pmax].removehead();
		 if(tmpnode!=NULL)
		 {
			tmpcell=&cells[tmpnode->number];
			free.addtail(tmpnode);
		 }
		 while( (!buckets[1][maxgain[1]+pmax].length) && (maxgain[1]>-pmax) )
			maxgain[1]--;
	  }
		 else
		 {
			tmpnode=buckets[0][maxgain[0]+pmax].removehead();
			if(tmpnode!=NULL)
			{
			   tmpcell=&cells[tmpnode->number];
			   free.addtail(tmpnode);
			}
			while( (!buckets[0][maxgain[0]+pmax].length) && (maxgain[0]>-pmax) )
			   maxgain[0]--;
		 }
   }
   return tmpcell;
}

void parthgraph::getprefix(int& prenum)
{
   int max=-pmax,current=0;
   int curnum=0;

   free.reset();
   while(free.current!=NULL)
   {
	  current+=cells[free.current->number].gainfromlock;
	  curnum++;
	  if(max<current)
	  {
		 max=current;
		 prenum=curnum;
	  }
	  ++free;
   }
   if(max<=0)
	  prenum=0;
}

void parthgraph::reinit(int prenum)
{
   int count=0;
   cell *cellptr;

   if(free.length!=numcells)
   {
	  for(count=0;count<numcells;count++)
	  {
		 if(cells[count].gainfromlock==LOCKED)
		 {
			buckets[cells[count].block][cells[count].gain+pmax].removenode(cells[count].gainbucket);
			cells[count].gain = 0;  // reset to 0 --WD
		 }
	  }
   }
   free.reset();
   while(free.current!=NULL)
   {
	  count++;
	  cellptr=&cells[free.current->number];
	  if(count>prenum)
	  {
		 cellptr->block=other(cellptr->block);
		 balance[cellptr->block]++;
		 balance[other(cellptr->block)]--;
	  }
	  cellptr->gainfromlock=LOCKED;
	  cellptr->gain=0;
	  free.removenode(cellptr->gainbucket);
	  ++free;
   }

   for(count=0;count<numnets;count++)
   {
	  nets[count].lock[0]=0;
	  nets[count].lock[1]=0;
	  nets[count].unlock[0]=0;
	  nets[count].unlock[1]=0;
	  // ***build cell list for each net and initialize net
	  nets[count].first.reset();
	  while(nets[count].first.current!=NULL)
	  {
		 nets[count].unlock[ cells[nets[count].first.current->number].block ]++;
		 ++nets[count].first;
	  }
   }

   initgains();
}

int parthgraph::cutset(void)
{
   int cutset=0;
   int count;
   for(count=0;count<numnets;count++)
   {
	  nets[count].first.reset();
	  int sidezero=0;
	  int sideone=0;
	  while(nets[count].first.current!=NULL)
	  {
		 if(cells[nets[count].first.current->number].block==0)
			sidezero=1;
		 else
			sideone=1;
		 ++nets[count].first;
	  }
	  if( (sideone==1) && (sidezero==1) )
		 cutset++;
   }
   return cutset;
}

void parthgraph::print(char *str)
{
   sprintf(str,"%i,%i,passes %i",balance[0],balance[1], passes);
}

void parthgraph::printCells()
{
   printf("Node\tPart\tGain\n");
   for(int i=0; i<numcells; i++)
   {
      printf("%d\t%d\t%d\n", cells[i].number, cells[i].block, cells[i].gain);
   }
}

void parthgraph::printBucket()
{
   for(int part=0; part<2; part++)
   {
      printf("****** Top Bucket in part %d *****\n", part);
      buckets[part][maxgain[part]+pmax].reset();
      while(buckets[part][maxgain[part]+pmax].current!=NULL)
      {
		 printf("%d\t", 
				buckets[part][maxgain[part]+pmax].current->number);
		 ++buckets[part][maxgain[part]+pmax];
      }
      printf("\n");
   }
}

