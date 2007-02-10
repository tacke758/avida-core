/*
 *  cStringList.cc
 *  Avida
 *
 *  Called "string_list.cc" prior to 12/7/05.
 *  Copyright 2005-2006 Michigan State University. All rights reserved.
 *  Copyright 1993-2003 California Institute of Technology
 *
 */

#include "cStringList.h"

#include "functions.h"
#include "cStringIterator.h"

using namespace std;


cStringList::cStringList(const cString & _list, char seperator)
{
  Load(_list, seperator);
}

cStringList::cStringList(const cStringList & _list)
{
  tConstListIterator<cString> list_it(_list.string_list);
  while (list_it.Next() != NULL) {
    string_list.PushRear(new cString( *(list_it.Get()) ));
  }
}

cStringList::~cStringList()
{
  while (string_list.GetSize() > 0) delete string_list.Pop();
}


cStringList & cStringList::operator=(const cStringList & _list)
{
  // If this list already as anything in it, make sure to delete it.
  Clear();

  // Now add the new list into this one.
  tConstListIterator<cString> list_it(_list.string_list);
  while (list_it.Next() != NULL) {
    string_list.PushRear(new cString( *(list_it.Get()) ));
  }
  return *this;
}

bool cStringList::HasString(const cString & test_string) const
{
  tConstListIterator<cString> string_it(string_list);
  const cString * cur_string = NULL;
  while ((cur_string = string_it.Next()) != NULL) {
    if (test_string == *cur_string) {
      return true;
    }
  }
  return false;
}

cString cStringList::PopString(const cString & test_string)
{
  tListIterator<cString> string_it(string_list);
  const cString * cur_string = NULL;
  while ((cur_string = string_it.Next()) != NULL) {
    if (test_string == *cur_string) {
      return ReturnString(string_list.Remove(string_it));
    }
  }
  return "";
}
