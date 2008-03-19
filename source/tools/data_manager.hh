//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef DATA_MANAGER_HH
#define DATA_MANAGER_HH

//#include <iostream.h>
// porting to gcc 3.1 -- k
#include <iostream>

#include "tDictionary.hh"

#include "data_entry.hh"

class cDataFile;

// This template manages a collection of keywords and associates each with
// an accessor for a specific object.  When a list of keywords is then
// fed in with an output file, the associated data for that object will
// then be printed to the output file.

class cDataManager_Base {
private:
  cString filetype;
public:
  cDataManager_Base(const cString & in_filetype) : filetype(in_filetype) { ; }
  virtual ~cDataManager_Base();
  
  const cString & GetFiletype() const { return filetype; }

  virtual bool Print(const cString & name, std::ostream & fp) = 0;
  virtual bool GetDesc(const cString & name, cString & out_desc) = 0;

  bool PrintRow(cDataFile & data_file, cString row_entries, char sep=' ');
};

template <class T> class tDataManager : public cDataManager_Base {
private:
  T * target;
  tDictionary< tDataEntryBase<T> * > entry_dict;
public:
  tDataManager(T * _target, const cString & in_filetype="unknown")
    : cDataManager_Base(in_filetype), target(_target) { ; }
  ~tDataManager() { ; }

  template<class OUT> bool Add(const cString & name,  const cString & desc,
			       OUT (T::*_funR)() const,
			       void (T::*_funS)(OUT _val) = NULL,
			       int (T::*_funC)(T * _o) const = NULL,
			       const cString & null="0",
			       const cString & html_cell="align=center") {
    tDataEntryBase<T> * new_entry =
     new tDataEntry<T, OUT> (name, desc, _funR, _funS, _funC, null, html_cell);
    new_entry->SetTarget(target);
    entry_dict.Add(name, new_entry);
    return true;
  }

  bool Print(const cString & name, std::ostream & fp) {
    tDataEntryBase<T> * cur_entry = NULL;
    if (entry_dict.Find(name, cur_entry) == false) return false;
    cur_entry->Print(fp);
    return true;
  }

  bool GetDesc(const cString & name, cString & out_desc) {
    tDataEntryBase<T> * cur_entry = NULL;
    if (entry_dict.Find(name, cur_entry) == false) return false;
    out_desc = cur_entry->GetDesc();
    return true;
  }
};

#endif
