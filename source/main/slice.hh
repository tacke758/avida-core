//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          //
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef SLICE_HH
#define SLICE_HH

// The cSchedule class is a generic object to handle time-slicing.

#include "population.hh"

class cScheduleNode {
private:
  cMerit merit_pos;
  int id_num;
public:
  virtual void SingleProcess() = 0;
  
  inline int GetSchedID() const { return id_num; }
  inline cMerit & GetSchedMerit() { return merit_pos; }

  virtual cMerit GetCurMerit() = 0;
};

class cSchedule {
protected:
  cAutoAdaptivePopulation * population;
public:
  cSchedule();
  virtual ~cSchedule();

  virtual int OK() { return TRUE; }  // @CAO FIX NOW!!!!!!
  virtual void Adjust(cBaseCPU * /* in_cpu */ ) { ; }
  virtual void Process(int total_slices);

  inline void SetPopulation(cAutoAdaptivePopulation * in_population)
    { population = in_population; }
};

// No changes here... everything is default for now...
class cConstSchedule : public cSchedule {
private:
public:
  cConstSchedule();
  ~cConstSchedule();
};


// The block schedule processes multiple slices of a cpu at once.  The number
// of slices pressessed is proportional to its merit.

class cBlockSchedule : public cSchedule {
private:
public:
  cBlockSchedule();
  ~cBlockSchedule();

  void Process(int total_slices);
};


// The Probiblistic Schedule has the chance for an instruction in a CPU to
// be processed proportional to the merit of that CPU.

class cProbSchedule : public cSchedule {
private:
  cIndexedBlockStruct * merit_bs;
public:
  cProbSchedule(int num_cells);
  ~cProbSchedule();
  
  void Adjust(cBaseCPU * in_cpu);
  void Process(int total_slices);
};


// The cLogScheduleNode object manages bundlings of CPU's for the logrithmic
// time slicing object (cLogSchedule).  When Process() is called on one of
// these nodes, it must eighter execute itself, or pass process down to the
// nodes below it (by running the Process() method of the next node).  If
// The node alternates between itself and the next, then we have a perfect
// Logrithmic (base 2) decrease in activity.  Sometimes a merit will be
// skipped in the list, so the next node should only be called one out of
// everyo four times.  Etc.

class cLogScheduleNode {
private:
  cBaseCPU * first;   // CPU list off of this node.
  cBaseCPU * current; // Position in CPU list being executed.
  int size;           // Number of CPUs in this node.
  int process_size;   // Number of times CPUs in this node should be executed
                      //   before the next node is.
  int process_count;  // Number of times this node has been executed.
  int merit_level;    // Merit for the creatures in this node.

  cLogScheduleNode * next;
  cLogScheduleNode * prev;
public:
  inline cLogScheduleNode(int node_id = -1);
  inline ~cLogScheduleNode();

  inline int OK();

  inline void Insert(cBaseCPU * in_cpu);
  inline void Remove(cBaseCPU * in_cpu);
  int Process();   // execute self or pass processing to next node.
  void Adjust();   // Adjust stats based on new position in list.

  inline void SetNext(cLogScheduleNode * in_next) { next = in_next; }
  inline void SetPrev(cLogScheduleNode * in_prev) { prev = in_prev; }
  inline void SetMerit(int in_merit) { merit_level = in_merit; }

  inline cBaseCPU * GetFirst() { return first; }
  inline int GetSize() { return size; }
  inline cLogScheduleNode * GetNext() { return next; }
  inline cLogScheduleNode * GetPrev() { return prev; }
  inline int GetProcessSize() { return process_size; }
  inline int GetProcessCount() { return process_count; }
  inline int GetMeritLevel() { return merit_level; }
};

// The cLogSchedule method relies on forcing all merits to the nearest
// power of 2, starting from the highest, and giving it half the time
// slices, and continuing recursively.  The simpliest way of doing this
// while maximizing evenness of distribution of time slices is to simply
// alternate executing the best, and everything else (where in everything
// else we again alternate with the best of this sub-list recursively)

class cLogSchedule : public cSchedule {
private:
  cLogScheduleNode ** node_array;
  cLogScheduleNode * first_node;
  int array_size;
  int list_size;
  int max_node;
  int * cpu_chart;

  void Resize(int new_size);
  void CreateNode(int node_id);
  void DestroyNode(int node_id);
public:
  cLogSchedule(int num_cells);
  ~cLogSchedule();

  int OK();

  void Adjust(cBaseCPU * in_cpu);
  void Process(int total_slices);

  inline int GetSize() { return array_size; }
};


////////////////////////////
// cIntegratedScheduleNode
////////////////////////////

class cIntegratedScheduleNode {
private:
  int * active_array; // Each cell in this array corressponds to the CPU with
                      //   the same ID.  If creature is not in the list, its
                      //   value in the array will be 0. If it is in the list,
                      //   it will  point to the cell of the next included
                      //   creature.  The last included creature has a -1 in
                      //   its cell.
  int first_entry;    // ID of first active creature.
  int node_id;        // A unique id (representing the relative merit bit).

  int size;           // Number of active CPUs in this node.
  int process_size;   // Number of times this node should be executed before
                      //   the next node is.
  int process_count;  // Number of times this node has been executed.
  cAutoAdaptivePopulation * population;

  cIntegratedScheduleNode * next;
  cIntegratedScheduleNode * prev;
public:
  cIntegratedScheduleNode(cAutoAdaptivePopulation * in_population = NULL,
			  int in_id = -1);
  ~cIntegratedScheduleNode();

  void Insert(int cpu_id);
  void Remove(int cpu_id);
  int Process();

  int OK();

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

class cIntegratedSchedule : public cSchedule {
private:
  cIntegratedScheduleNode ** node_array;
  int array_size;
  int num_active_nodes;
  cMerit * merit_chart;

  void InsertNode(int node_id);
  void RemoveNode(int node_id);
  void ResizeNodes(int new_size);
public:
  cIntegratedSchedule(cAutoAdaptivePopulation * in_population);
  ~cIntegratedSchedule();

  void Adjust(cBaseCPU * in_cpu);
  void Process(int total_slices);

  int OK();
};

//////////////
//  cLogScheduleNode
//////////////

inline int cLogScheduleNode::OK()
{
  int result = TRUE;

#ifdef DEBUG
  cBaseCPU * temp_cpu = NULL, * prev_cpu = NULL;
  
  // Check to make sure list is correct...

  int temp_size = 0;
  for (temp_cpu = first; temp_cpu != NULL; temp_cpu = temp_cpu->GetNext()) {
    if (temp_cpu->GetPrev() != prev_cpu) {
      g_debug.Error("Mis-threaded cLogScheduleNode.cpu list! Error in 'prev'");
      result = FALSE;
    }
    prev_cpu = temp_cpu;
    temp_size++;
    if (temp_size > size) break;
  }

  if (temp_size != size) {
    g_debug.Error("Mis-Threaded CPU-list merit %d with size %d [%d]",
		  merit_level, size, temp_size);

    result = FALSE;
  }
#endif

  return result;
}

inline void cLogScheduleNode::Insert(cBaseCPU * in_cpu)
{
  in_cpu->SetNext(first);
  if (first) first->SetPrev(in_cpu);
  first = in_cpu;
  size++;
}

inline void cLogScheduleNode::Remove(cBaseCPU * in_cpu)
{
  size--;
  if (in_cpu == first) first = in_cpu->GetNext();
  if (in_cpu == current) current = current->GetNext();

  if (in_cpu->GetNext()) in_cpu->GetNext()->SetPrev(in_cpu->GetPrev());
  if (in_cpu->GetPrev()) in_cpu->GetPrev()->SetNext(in_cpu->GetNext());

  in_cpu->SetNext(NULL);
  in_cpu->SetPrev(NULL);
}

inline cLogScheduleNode::cLogScheduleNode(int node_id)
{
  g_memory.Add(C_LOG_SCHEDULE_NODE);

  first = NULL;
  size = 0;
  process_size = 1;
  process_count = 0;
  merit_level = node_id;

  next = NULL;
  prev = NULL;
}

inline cLogScheduleNode::~cLogScheduleNode()
{
  g_memory.Remove(C_LOG_SCHEDULE_NODE);
}

#endif
