#include "ll.hpp"
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
