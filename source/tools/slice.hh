//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef SLICE_HH
#define SLICE_HH

#include "tArray.hh"
//#include "b_struct2.hh"
#include "weighted_index.hh"

class cMerit;

/**
 * This class is the base object to handle time-slicing. All other schedulers
 * are derived from this class.  This is a pure virtual class.
 *
 **/

class cSchedule {
protected:
  int item_count;
public:
  cSchedule(int _item_count);
  virtual ~cSchedule();

  virtual bool OK() { return true; }
  virtual void Adjust(int item_id, const cMerit & merit) { ; }
  virtual int GetNextID() = 0;
  virtual double GetStatus(int id) { return 0.0; }

  inline void SetSize(int _item_count)  { item_count = _item_count; }
};


/**
 * This class rotates between all items to schedule, giving each equal time.
 **/

class cConstSchedule : public cSchedule {
private:
  int last_id;
public:
  cConstSchedule(int _item_count);
  ~cConstSchedule();

  bool OK();
  void Adjust(int item_id, const cMerit & merit) { ; }

  int GetNextID();
};


/**
 * The Probiblistic Schedule has the chance for an item to
 * be scheduled proportional to the merit of that item.
 **/

class cProbSchedule : public cSchedule {
private:
  cWeightedIndex chart;
public:
  cProbSchedule(int num_cells);
  ~cProbSchedule();

  void Adjust(int item_id, const cMerit & merit);
  int GetNextID();
};


/**
 * The cIntegratedScheduleNode object manages bundlings of item's for the
 * integrated time slicing object (cIntegratedSchedule).  When GetNextID()
 * is called on one of these nodes, it must either choose from itself, or
 * pass the call down to the nodes below it (by running the GetNextID()
 * method of the next node).  If the node alternates between everything in
 * its own list the next node's list, then we have a perfect Logrithmic
 * (base 2) decrease in activity.  Sometimes a merit will be skipped in the
 * list, so the next node should only be called one out of every four times,
 * etc.
 *
 * This allows binary representations of merits to determine which nodes
 * each item should be included in.
 **/

class cIntegratedScheduleNode {
private:
  tArray<int> active_array; // Each cell in this array corressponds to the
                      //  item with the same ID.  If creature is not in the
                      //  list, its value in the array will be 0. If it is in
                      //  the list, it will  point to the cell of the next
                      //  included creature.  The last included creature has
                      //  a -1 in its cell.
  int first_entry;    // ID of first active creature.
  int active_entry;   // ID of next scheduled entry.
  int node_id;        // A unique id (representing the relative merit bit).

  int size;           // Number of active items in this node.
  int process_size;   // Number of times this node should be executed before
                      //   the next node is.
  int process_count;  // Number of times this node has been executed.
  bool execute;       // Should this node execute or pass?

  cIntegratedScheduleNode * next;
  cIntegratedScheduleNode * prev;
public:
  cIntegratedScheduleNode(int _item_count = 0, int in_id = -1);
  ~cIntegratedScheduleNode();

  void Insert(int item_id);
  void Remove(int item_id);
  int GetNextID();

  bool OK();

  inline void SetProcessSize(int in_p_size) { process_size = in_p_size; }
  inline void SetNext(cIntegratedScheduleNode * in_next) { next = in_next; }
  inline void SetPrev(cIntegratedScheduleNode * in_prev) { prev = in_prev; }

  inline int GetID() { return node_id; }
  inline int GetSize() { return size; }
  inline int GetProcessSize() { return process_size; }
  inline int GetProcessCount() { return process_count; }
  inline cIntegratedScheduleNode * GetNext() { return next; }
  inline cIntegratedScheduleNode * GetPrev() { return prev; }
};


/**
 * The cIntegratedSchedule method relies on breaking up all merits into
 * sums of powers of 2 (i.e. using the binary representation of the merit).
 * All items with merits in the highest power of two will get the most
 * time, and subsequent merit components will have time divided,
 * continuing recursively.  The simplest way of doing this while maximizing
 * evenness of distribution of time slices is to simply alternate executing
 * the best, and everything else (where in everything else we again alternate
 * with the best of this sub-list recursively).
 **/

class cIntegratedSchedule : public cSchedule {
private:
  tArray<cIntegratedScheduleNode *> node_array;
  int num_active_nodes;
  cMerit * merit_chart;

  void InsertNode(int node_id);
  void RemoveNode(int node_id);
  void ResizeNodes(int new_size);
public:
  cIntegratedSchedule(int _item_count);
  ~cIntegratedSchedule();

  void Adjust(int item_id, const cMerit & new_merit);
  int GetNextID();
  double GetStatus(int id);

  bool OK();
};


#endif
