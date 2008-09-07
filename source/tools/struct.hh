#ifndef STRUCT_HH
#define STRUCT_HH

#include "include.hh"
#include "functions.hh"

// The following class is used within linked lists.  It should be overloaded
// to contain the specific information for the list in question.

class cDataElement {
protected:
  void * data;
  cDataElement * next;
  cDataElement * prev;
public:
  inline cDataElement(void * in_data = NULL, cDataElement * in_next = NULL,
		      cDataElement * in_prev = NULL) {
    data = in_data;
    next = in_next;
    prev = in_prev;
  }
  ~cDataElement() {;}

  inline void * GetData() const { return data; }
  inline cDataElement * GetNext() const { return next; }
  inline cDataElement * GetPrev() const { return prev; }

  inline void SetNext(cDataElement * in_next) { next = in_next; }
  inline void SetPrev(cDataElement * in_prev) { prev = in_prev; }
};

class cList {
protected:
  cDataElement root;
  cDataElement * current;
  int size;
public:
  inline cList() {
    size = 0;
    root.SetNext(&root);
    root.SetPrev(&root);
    current = &root;
  }
  ~cList() {
    // clean up all the data elements in the list
    while (size) RemoveFront();
  }

  int OK();
  inline int GetSize() const { return size; }
  inline void * GetFirst()   { return root.GetNext()->GetData(); }
  inline void * GetLast()    { return root.GetPrev()->GetData(); }

  cDataElement * InsertFront (void * new_data);
  cDataElement * InsertRear (void * new_data);
  void * RemoveFront();
  void * RemoveRear();

  void * Remove(void * old_data);
  void Remove(cDataElement * old_element);

  void * operator[](int position);
  void * Get(int position) { return operator[](position); }
  void * Find(void * test_data, int (*in_Compare)(void *, void *));

  inline void Clear() { while (size) RemoveFront(); }

  inline int AtFront() { return (current == root.GetNext()); }
  inline int AtEnd()   { return (current == root.GetPrev()); }
  inline int AtRoot()  { return (current == &root); }

  inline void * Reset() { current = root.GetNext(); return current; }
  inline void * GetCurrent() { return current->GetData(); }
  inline void * Next()
    { current = current->GetNext();  return current->GetData(); }
  inline void * Prev()
    { current = current->GetPrev(); return current->GetData(); }

  inline void * CircNext() {
    current = current->GetNext();
    if (current == &root) current = current->GetNext();
    return current->GetData();
  }
  inline void * CircPrev() {
    current = current->GetPrev();
    if (current == &root) current = current->GetPrev();
    return current->GetData();
  }

  void operator=(const cList & in_list);
};

class cMultiList : public cList {
private:
  cDataElement ** p_array;
  int num_pointers;
public:
  cMultiList(int num_ptrs);
  ~cMultiList();

  inline void * Remove(void * old_data) { return cList::Remove(old_data); }
  inline void Remove(cDataElement * old_element) {
    cList::Remove(old_element);
  }

  inline void * Next(int p_num) {
    p_array[p_num] = p_array[p_num]->GetNext();
    return p_array[p_num]->GetData();
  }
  inline void Reset(int p_num) { p_array[p_num] = root.GetNext(); }
  inline void * Get(int p_num) { return p_array[p_num]->GetData(); }
  inline void Remove(int p_num) {
    Remove(p_array[p_num]);
    p_array[p_num] = root.GetNext();
  }
};

// Implement a standard FIFO queue
class cQueue {
private:
	cDataElement	cdRoot;		// root/start of Queue
	cDataElement	*cdTop;		// top of Queue 
	int				iSize;		// # of elements in queue

public:
	cQueue();
	~cQueue();

	int		QueueAdd  (void *vItem);
	void *	QueueGet  (void);
	void *	QueuePeek (int iElement = 0);
	void	QueueFlush();

	inline int	QueueGetSize (void)		{return iSize;}
};

#endif
