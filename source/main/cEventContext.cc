/*
 *  cEventContext.cc
 *  Avida
 *
 *  Created by Matthew Rupp on 2/4/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#include "cEventContext.h"
#include "tList.h"

cEventContext::~cEventContext()
{
  tList<cString> keys;
  cCntxEntry* event;
  cString* key;
  m_state_info.GetKeys(keys);
  while(keys.GetSize() > 0){
    key = keys.Pop();
    event = m_state_info.Remove(*key);
    delete event;
  }
}

const cFlexVar* cEventContext::operator[](const cString& name) 
{
  cCntxEntry* entry ;
  bool found = m_state_info.Find(name, entry);
  return (found) ? &entry->GetValue() : NULL; 
}



cEventContext& cEventContext::operator<<(const cCntxEntry& entry)
{
  m_state_info.Add(entry.GetName(), new cCntxEntry(entry));
  return *this;
}
