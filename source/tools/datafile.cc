//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
/* datafile.cc ****************************************************************
 cDataFile 
 cDataFileManager 

 charles@krl.caltech.edu & travc@ugcs.caltech.edu
 Time-stamp: <1999-01-07 10:03:39 travc>

******************************************************************************/
  
#include "datafile.hh"
#include <stdio.h>
#include "../main/stats.hh"

cDataFile::cDataFile(cString _name) : name(_name) {
  if( fopen(name,"r") ){
    // Warn  -- TO ADD @TCC
  }
  fp.open(name);
  assert( fp.good() );
}


cDataFileManager::~cDataFileManager(){
  while( head != NULL ){
    next = head->next;
    delete head;
    head = next;
  }
}


cDataFileManager::cEntry * cDataFileManager::InternalFind(const cString & name){
  prev = NULL;
  cEntry * current = head;
  while( current != NULL ){
    if( name == current->name ){
      next = current->next;
      return current;
    }
    current = current->next;
  }
  return NULL;
}


cDataFile & cDataFileManager::Get(const cString & name){ 
  cEntry * entry = InternalFind(name);
  if( entry == NULL ){
    entry = new cEntry(name, head);
    head = entry;
  }else{
    assert( entry->name == name );
  }
  return entry->file;
}    


ofstream & cDataFileManager::GetOFStream(const cString & name){ 
  cEntry * entry = InternalFind(name);
  if( entry == NULL ){
    entry = new cEntry(name, head);
    head = entry;
  }else{
    assert( entry->name == name );
  }
  return entry->file.GetOFStream();
}    


void cDataFileManager::FlushAll(){
  cEntry * current = head;
  while( current != NULL ){
    current->file.Flush();
    current = current->next;
  }
}


bool cDataFileManager::Remove(const cString & name){
  cEntry * entry = InternalFind(name);
  if( entry == NULL ){
    return FALSE;
  }else{
    if( prev == NULL ){
      head = entry->next;
    }else{
      prev->next = entry->next;
    }
    delete entry;
  }
  return TRUE;
}    

