#ifndef LL_HPP
#define LL_HPP

#ifndef NULL
#define NULL 0
#endif

#include <fstream>

class node;

class node
  {
  public:
    int number;
    node *next, *prev;
    node(int num) {number=num;}
    node() {number=-1;}
  };

class LL
  {
  protected:
  node headdummy,taildummy;  // dummy nodes for the ends of the ll
  //unsigned int length;       // holds length of ll
  node* head;                // ptr to head of ll
  node* tail;                // ptr to tail of ll
  unsigned int curcount;     // keeps track of where current is pointing

  public:
  node* current;             // ptr to current point of ll

  LL(void);                  // constructor
  void addhead(node* nnode); // adds nnode to head of ll
  void addtail(node* nnode); // adds nnode to tail of ll
  node* removehead(void);    // removes head of ll, returns ptr to it
  node* removetail(void);    // removes tail of ll, returns ptr to it
  void removenode(node* nnode); // removes nnode from the ll
  node* get(int index);      // returns ptr to index number from head
  void reset(void);          // resets current to head
  void operator1();         // increments current
  int search(node* nnode);   // search if nnode is in list, return index
  unsigned int length;
  };


#endif
