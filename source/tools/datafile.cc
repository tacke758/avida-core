/* datafile.cc ****************************************************************
 cDataFile 
 cDataFileManager 

 charles@krl.caltech.edu & travc@ugcs.caltech.edu
 Time-stamp: <98/10/22 02:00:21 travc>

******************************************************************************/
  
#include "datafile.hh"
#include <stdio.h>
#include "../main/stats.hh"

cDataFile::cDataFile(cString _name, tOutputMethod _outmethod) :
 name(_name),
 outmethod(_outmethod) {
   if( fopen(name,"r") ){
     // Warn  -- TO ADD @TCC
   }
   fp.open(name);
   assert( fp.good() );
}

void cDataFile::Output(){ assert(fp.good()); outmethod(fp); }


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


cDataFile & cDataFileManager::Get(const cString & name, 
				  const tOutputMethod outmethod){
  cEntry * entry = InternalFind(name);
  if( entry == NULL ){
    entry = new cEntry(name, outmethod, head);
    head = entry;
  }else{
    assert( entry->name == name && entry->file.CheckOutputMethod(outmethod) );
  }
  return entry->file;
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

