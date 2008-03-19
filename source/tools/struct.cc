//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
#include "struct.hh"

////////////////////////
//  cList
////////////////////////

cDataElement * cList::InsertFront(void * new_data)
{
  cDataElement * new_data_element;
  
  size++;
  new_data_element = new cDataElement(new_data, root.GetNext(), &root);
  root.GetNext()->SetPrev(new_data_element);
  root.SetNext(new_data_element);
  current = new_data_element;

  return new_data_element;
}

cDataElement * cList::InsertRear(void * new_data)
{
  cDataElement * new_data_element;
  
  size++;
  new_data_element = new cDataElement(new_data, &root, root.GetPrev());
  root.GetPrev()->SetNext(new_data_element);
  root.SetPrev(new_data_element);
  current = new_data_element;

  return new_data_element;
}

int cList::OK()
{
  int result = TRUE;
  int current_found = FALSE; // Make sure current is in the list.

  if (current == &root) current_found = TRUE;

  if (!size) {
    if (root.GetNext() != &root || root.GetPrev() != &root) {
      // g_debug.Error("empty cList improperly threaded!");
      result = FALSE;
    }
  }
  else {
    int count = 0;

    for (cDataElement * test_element = root.GetNext();
	 test_element != &root;
	 test_element = test_element->GetNext()) {
      if (current == test_element) current_found = TRUE;
      if (test_element->GetNext()->GetPrev() != test_element) {
	// g_debug.Error("Incorrectly threaded cList");
	result = FALSE;
      }
      count++;
      if (count > size) {
	// g_debug.Error("cList size is incorrect!");
	result = FALSE;
	break;
      }
    }
  }

  if (!current_found) result = FALSE;

  return result;
}

void * cList::RemoveFront()
{
  void * out_data;
  cDataElement * out_element;

  out_element = root.GetNext();
  out_data = out_element->GetData();
  Remove(out_element);

  return out_data;
}

void * cList::RemoveRear()
{
  void * out_data;
  cDataElement * out_element;

  out_element = root.GetPrev();
  out_data = out_element->GetData();
  Remove(out_element);

  return out_data;
}

void * cList::Remove(void * old_data)
{
  cDataElement * cur_element = root.GetNext();

  while (cur_element != &root) {
    if (cur_element->GetData() == old_data) {
      Remove(cur_element);
      return old_data;
    }
    cur_element = cur_element->GetNext();
  }

  return NULL;
}

void cList::Remove(cDataElement * old_element)
{
  old_element->GetNext()->SetPrev(old_element->GetPrev());
  old_element->GetPrev()->SetNext(old_element->GetNext());
  delete old_element;
  size--;
}

void * cList::operator[](int position)
{
  cDataElement * cur_element = root.GetNext();

  for (int i = 0; i < position; i++) cur_element = cur_element->GetNext();

  return cur_element->GetData();
}

int cList::Has(void * test_data)
{
  for (cDataElement * cur_element = root.GetNext();
       cur_element != &root;
       cur_element = cur_element->GetNext()) {
    if (cur_element->GetData() == test_data) {
      return TRUE;
    }
  }

  return FALSE;
}

void * cList::Find(void * test_data, int (*in_Compare)(void *, void *))
{
  cDataElement * cur_element;

  for (cur_element = root.GetNext();
       cur_element != &root;
       cur_element = cur_element->GetNext()) {
    if (!in_Compare(cur_element->GetData(), test_data)) {
      return cur_element->GetData();
    }
  }

  return NULL;
}

void cList::operator=(const cList & in_list)
{
  Clear();
  cDataElement * cur_element = in_list.root.GetNext();
  
  while (cur_element != &(in_list.root)) {
    InsertRear(cur_element->GetData());
    cur_element = cur_element->GetNext();
  }
}

////////////////////////
//  cMultiList
////////////////////////

cMultiList::cMultiList(int num_ptrs)
{
  num_pointers = num_ptrs;
  p_array = new cDataElement * [num_pointers];
  for (int i = 0; i < num_pointers; i++) {
    p_array[i] = root.GetNext();
  }
}

cMultiList::~cMultiList()
{
}


//////////////////////////
//	cQueue
//////////////////////////

cQueue::cQueue()
{
  // Constructor. Top of queue is cdTop. Add elements at cdRoot.GetNext().
  // When cdTop == NULL there is nothing there (also iSize == 0).

  iSize = 0;       // No elements yet
  cdTop = &cdRoot; // cDataElement constructor sets data/next/prev to NULL
}

cQueue::~cQueue() 
{
  // Destructor. FLush queue and leave
  QueueFlush();
}

void cQueue::QueueFlush()
{
  // Remove all allocated elements and release the storage
  // that they had. THIS COULD BE BAD... if each queue item was not allocated
  // by the caller or any queue items point at static storage areas!
  
  void	*vItem;
  while (iSize) {
    vItem = QueueGet();		// Pull top queue item off
    delete vItem;			// Free up storage we were tracking
  }
}

int cQueue::QueueAdd (void *vItem)
{
  // Add an item onto the front of the queue. Return 0 if we failed (likely
  // cause: could not allocate space), else size of queue with new item
  // added. This call IS NOT thread-safe, because the new queue item is
  // inserted, then the the pointer fix-up occurs, then iSize is
  // incremented. An interruption in the middle of this with another
  // QueueAdd could corrupt the queue!!

  cDataElement * cdNew;
  
  // The new item points to the bottom of the queue as its next item, and
  // the root as it's previous item
  cdNew = new cDataElement (vItem, cdRoot.GetNext(), &cdRoot);
  if (cdNew == NULL) {
    // We could not allocate space for the queue entry
    return 0;
  }
  
  // Fixup the pointer entries
  cdRoot.SetNext (cdNew); // Now this is the last queue entry

  if (iSize) {
    // One of many items on the queue
    cdNew->GetNext()->SetPrev (cdNew);	// Fixup backpointer
  } else {
    // First item on the queue
    cdTop = cdNew; // First item on queue resets the top!!
  }

  iSize++; // up the counter

  return iSize;
}

void * cQueue::QueueGet  (void)
{
  // Pull the topmost item off the queue and return it, or return NULL if
  // the queue is empty. It is up to the caller to release any storage that
  // we return.  We will take care of cleaning up the queue structures. As
  // with QueueAdd, this IS NOT THREAD SAFE!!!

  void * vItem; // Data from the top of our queue

  if (iSize) {
    // We have an item, so do the work to remove it from the queue
    cDataElement * cdItem = cdTop;

    vItem = cdItem->GetData(); // save pointer to the data
    cdTop = cdItem->GetPrev(); // Previous item is now the top
    cdTop->SetNext (NULL);     // Nothing comes after this now
    iSize--;
    delete cdItem;             // Release the storage now
  } else {
    vItem = NULL;              // Nothing in the queue
  }
  
  return vItem;
}

void * cQueue::QueuePeek (int iElement) // Header file defaults iElement = 0
{
  // Look at an item in the queue without pulling it off. Default is to
  // peek at the top item, but this will also let you look at any-old queue
  // item by passing in a zero-based index (0 = top of queue,
  // iSize-1 = bottom of queue). And guess what? It is not THREAD SAFE
  // either! If iElement is out of range we return NULL.

  cDataElement * cdItem = cdTop;
  int i = Min(iSize, Max(0, iElement));

  while (i--) {
    // Walk backwards as far as we must to get the requested element
    cdItem = cdItem->GetPrev();
  }

  // Return the data from the desired queue element
  return cdItem->GetData();
}
