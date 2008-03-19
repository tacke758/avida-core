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
