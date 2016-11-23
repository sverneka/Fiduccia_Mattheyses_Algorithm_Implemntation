#include <stdio.h>
#include "hgraph.hpp"
#include <stdlib.h>
#include <iostream>
#include <conio.h>

#include <fstream>
#include <math.h>
#include "ll.hpp"

using namespace std;
const int NUMNETS = 6;
const int NUMCELLS = 8;
int SEED =1,passes=0;
double RATIO =0.375;
int inputhg[][NUMCELLS] = {{0,2,4,100}, {1,2,3,100} , {2,5,4,100}, {6,5,7,100}, {3,5,100}, {6,4,100}};
//int inputhg[][NUMCELLS]={{1,3,5,0},{2,3,4,0},{3,6,5,0},{8,6,7,0},{4,6,0},{7,5,0}};
parthgraph hg;
main()
{
	char name[]="tresult";
	hg.getgraph();
	hg.initgains();
	hg.part();


	char string[100];
	hg.print(string);
  
	cout<<"seed, "<<SEED<<",";
	cout<<"final cutset, "<<hg.cutset()<<',';
	cout<<string;
	cout<<'\n';
	getch();
}



//#define debug
void error(char* s, char* s2="")
{
   printf("%s",s);
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


void parthgraph::getgraph()
{
   int side=0, count;
   node *nnode;

   srand(SEED);
   balance[0]=0;
   balance[1]=0;

   cells=(cell*)new cell[NUMCELLS];
   if(!cells) error("memory allocation error");
   minsize[0]=minsize[1]=3;
   int max[2] = {5,5};
   // ***read in nodes
   for(count=0;count<NUMCELLS;count++)
   {
	  cells[count].number=count;
	  side=(rand() & 128)?1:0;
	  if(balance[side]==max[side]) side=other(side);
	  switch(count)
      {
                   case 0: side = 0;
                   break;
                   case 1: side = 1;
                   break;
                   case 2: side = 0 ;
                   break;
                   case 3 :side = 0;
                   break;
                   case 4 :side = 1;
                   break;
                   case 5 : side = 1;
                   break;
                   case 6: side = 0;
                   break;
                   case 7: side = 1;
                   break;
                   }
                   cells[count].block=side;
	  cells[count].gainbucket=NULL;
	  cells[count].gain=0;
	  cells[count].gainfromlock=LOCKED;
	  balance[side]++;
   }
   // ***read in nets
   nets=(net*)new net[NUMNETS];
   if(!nets) error("memory allocation error");
   //count = -1;
    for(count=0;count<NUMNETS;count++) {
	int j = 0;
    nets[count].number=0;
		 nets[count].lock[0]=0;
		 nets[count].lock[1]=0;
		 nets[count].unlock[0]=0;
		 nets[count].unlock[1]=0;
    while(inputhg[count][j]!=100) {

	  nnode=new node(count);
	  if(!nnode) error("memory allocation error");
	  cells[inputhg[count][j]].first.addhead(nnode);
	  nnode=new node(inputhg[count][j]);
	  if(!nnode) error("memory allocation error");
	  nets[count].first.addhead(nnode);
	  nets[count].unlock[cells[inputhg[count][j]].block]++;
	  nets[count].number++;
	  j=j+1;
	  }
	}
    pmax=0;
    for(count=0;count<NUMCELLS;count++) {
		if(cells[count].first.length>pmax)
			pmax=cells[count].first.length;
    } 
    buckets[0]=(ll) new LL[2*pmax+1];
    buckets[1]=(ll) new LL[2*pmax+1];
    if(!buckets[0] || !buckets[1]) error("memory allocation error");
}

void parthgraph::initgains(void) {

   int count;
   int from,to;
   
	nets=(net*)new net[NUMNETS];
   	if(!nets) error("memory allocation error");
   	//count = -1;
    for(count=0;count<NUMNETS;count++) {
		int j = 0;
    	nets[count].number=0;
		nets[count].lock[0]=0;
		nets[count].lock[1]=0;
		nets[count].unlock[0]=0;
		nets[count].unlock[1]=0;
    	while(inputhg[count][j]!=100) {
	  		node* nnode=new node(inputhg[count][j]);
	  		if(!nnode) error("memory allocation error");
	  		nets[count].first.addhead(nnode);
            nets[count].unlock[cells[inputhg[count][j]].block]++;
	  		nets[count].number++;
   			j=j+1;
		}
	}

   
   buckets[0]=(ll) new LL[2*pmax+1];
   buckets[1]=(ll) new LL[2*pmax+1];
   if(!buckets[0] || !buckets[1]) error("memory allocation error");
   // ****calc gain of unlocked cells
   for (count=0;count<NUMCELLS;count++)
   {
	  if(cells[count].gainfromlock==LOCKED)
	  {
		 cells[count].first.reset();
		  cells[count].gain = 0;
		 from=cells[count].block;
		 to=other(from);
		 while(cells[count].first.current!=NULL)
		 {
			int nnum=cells[count].first.current->number;
			if( (nets[nnum].unlock[from]+nets[nnum].lock[from])==1)
			   cells[count].gain++;
			if( (nets[nnum].unlock[to]+nets[nnum].lock[to])==0)
			   cells[count].gain--;
			cells[count].first.operator1();
		 }
	  }
    }
	//printf("\nsachin vernekar1\n");
  	//printCells();
	//printf("\nsachin vernekar2\n");  
   	maxgain[0]=-pmax;
   	maxgain[1]=-pmax;

   	node *nnode;
   	for(count=0;count<NUMCELLS;count++)
   	{
		if(cells[count].gainbucket==NULL)
		{
		nnode=(node*)new node(count);
		if(!nnode) error("memory allocation error");
		cells[count].gainbucket = nnode;
	  	}	
	  	else
		nnode=cells[count].gainbucket;
	  	buckets[cells[count].block][cells[count].gain+pmax].addhead(nnode);
	  	//printf("cell no = %d, bucket no = %d\n ",count,cells[count].block);
	  	if(cells[count].gain>maxgain[cells[count].block])
			maxgain[cells[count].block]=cells[count].gain;
   	}
}


void parthgraph::part(void)
{
   int prefix=0; // holds prefix sum of gains
   int done=0;
   int noofzeros = 0;
   int prevcutsize=0,cutsizecount = 0;
   char c = 'N';

   while(!done)
   {
	  passes++;
	  swapall();
	  printf("cutset = %d\n",hg.cutset());
	  for(int count=0;count<NUMCELLS;count++)
	  cells[count].gainfromlock = LOCKED;
	 initgains();
	if(prevcutsize == hg.cutset())
    cutsizecount = cutsizecount+1;
    else cutsizecount = 0;
    if(cutsizecount>2)
    {
    printf("cutsize not changing = %d, Manasij, Do u want to stop?Y:N",hg.cutset());
    scanf("%c",&c);
	}
    if(c == 'Y' || c == 'y')
    done=1;
    prevcutsize = hg.cutset();
   	}
}

void parthgraph::swapall(void)
{
   int from,to;
   cell *bestcell;
   net *curnet;
   int done=0;

   bestcell=gethighest();
if(bestcell == NULL)
done = 1;
   while(!done)
   {
//#ifdef debug
//	  printBucket();
	  printf("base cell=%d(%d) gain=%d\n", bestcell->number, bestcell->block, bestcell->gain);
			 printCells();
	  char go;
	 // scanf(" %c", &go);
//#endif
	  from=bestcell->block;
	  to=other(from);
	  bestcell->block=to;
	  bestcell->gainfromlock=bestcell->gain;
	  //bestcell->gainbucket=NULL;
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
		 bestcell->first.operator1();
	  }
	  bestcell=gethighest();
	  printCells();
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
		 first.operator1();
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
		 first.operator1();
	  }
   }
}

cell* parthgraph::gethighest(void)
{
   cell *tmpcell=NULL;
   node *tmpnode=NULL;
   int maxgaincell[2];//cell to be finally selected for moving
   int blockno;//static int swapside=0;  //*******force toggle swap


if( (maxgain[0]>=maxgain[1]) && maxgain[0]>=0)
   {
	  if(balance[0]>minsize[0])
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
   if(tmpcell==NULL )
   {
	  if(balance[1]>minsize[1] && (maxgain[1]>=0))
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
		 else if(balance[0]>minsize[0] && (maxgain[0]>=0))
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





int parthgraph::cutset(void)
{
   int cutset=0;
   int count;
   for(count=0;count<NUMNETS;count++)
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
		 nets[count].first.operator1();
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
   for(int i=0; i<NUMCELLS; i++)
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
		 buckets[part][maxgain[part]+pmax].operator1();
      }
      printf("\n");
   }
}


// check if a node is in list before remove it
//#define Check

// ***overload ++ for LL***increments current to point to next node******
// **********************************************************************
void LL::operator1()
  {
//  curcount++;
//  if (curcount<=length)
  if(current->next->number!=-1)
    current=current->next;
  else
    current=NULL;
  }

// ******reset****resets current to point to head of list****************
// **********************************************************************
void LL::reset(void)
  {
  if(length!=0)
    {
    curcount=1;
    current=head->next;
    }
  else
    current=NULL;
  }


// *************constructor of linked list*******************************
// *********initializes variables****************************************
LL::LL(void)
  {
  head=&headdummy;
  tail=&taildummy;
  head->next=tail;
  tail->prev=head;
  head->prev=NULL;
  tail->next=NULL;
  current=head;
  length=0;
  curcount=0;
  }

// *************addhead*******adds a node to head of list****************
// **********************************************************************
void LL::addhead(node* nnode)
  {
  length++;
  nnode->next=head->next;
  nnode->prev=head;
  head->next=nnode;
  nnode->next->prev=nnode;
  }

// *************addtail*******adds a node to tail of list****************
// **********************************************************************
void LL::addtail(node* nnode)
  {
  length++;
  nnode->next=tail;
  nnode->prev=tail->prev;
  tail->prev=nnode;
  nnode->prev->next=nnode;
  }

// *************removehead******removes node at head of list and*********
// **********************************************************************
node* LL::removehead(void)
  {
  if(length)
    {
    length--;
    node* nnode;
    nnode=head->next;
    head->next=nnode->next;
    nnode->next->prev=head;
    nnode->next=NULL;
    nnode->prev=NULL;
    return nnode;
    }
  else return NULL;
  }

// *************removetail******removes node at end of list and *********
// ****returns a ptr to it***********************************************
node* LL::removetail(void)
  {
  if (length)
    {
    length--;
    node* nnode;
    nnode=tail->prev;
    tail->prev=nnode->prev;
    nnode->prev->next=tail;
    nnode->prev=NULL;
    nnode->next=NULL;
    return nnode;
    }
  else return NULL;
  }

// *************get********returns a ptr to the node index number from***
// ****the head**********************************************************
node* LL::get(int index)
  {
  if ( (index<=length) && length) // make sure index and length is valid
    {
    int count;
    node *temp;

    if( ((int)(length/2)) > index )  // count in from shortest side
      {
      temp=head;
      for(count=0;count<index;count++)
        temp=temp->next;
      }
    else
      {
      temp=tail;
      for(count=length+1;count>index;count--)
        temp=temp->prev;
      }
    return temp;
    }
  else return NULL;
  }

// *************search********search nnode in the ll********************
// **********************************************************************
int LL::search(node* nnode)
{
   if(length)
   {
      int index=0;
      node* temp=head->next;
      while(temp->next)
      {
	 index++;
	 if(temp == nnode)
	    return index;
	 temp=temp->next;
      }
   }
   return 0;
}

// *************removenode********removes nnode from the ll**************
// **********************************************************************
void LL::removenode(node* nnode)
{
#ifdef Check
  if(!search(nnode))  // node not in list
  {
     cout << "node" << nnode->number << " is not in list!!" << endl;
     exit(1);
  }
#endif
if(nnode!=NULL)
  {
  nnode->prev->next=nnode->next;
  nnode->next->prev=nnode->prev;
  length--;
  }
}


