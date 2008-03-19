//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef HELP_HH
#define HELP_HH

#include <iostream.h>

#include "string.hh"
#include "file.hh"
#include "tList.hh"

class cHelpType;
class cHelpManager;

class cHelpEntry {
protected:
  cString name;
public:
  cHelpEntry(const cString & _name) : name(_name) { ; }
  virtual ~cHelpEntry() { ; }

  const cString & GetName() const { return name; }
  virtual const cString & GetKeyword() const = 0;
  virtual const cString & GetDesc() const = 0;
  virtual cHelpType * GetType() const = 0;

  virtual bool IsAlias() const = 0;

  cString GetHTMLFilename() const
    { return cString::Stringf("help.%s.html", GetKeyword()()); }
};

class cHelpFullEntry : public cHelpEntry {
private:
  cHelpType * type;
  cString desc;
public:
  cHelpFullEntry(const cString & _name, cHelpType * _type, const cString _desc)
    : cHelpEntry(_name), type(_type), desc(_desc) { ; }
  cHelpFullEntry() : cHelpEntry(""), type(NULL) { ; }
  ~cHelpFullEntry() { ; }

  const cHelpFullEntry & operator=(const cHelpEntry & in_entry) {
    name = in_entry.GetName();
    type = in_entry.GetType();
    desc = in_entry.GetDesc();
    return *this;
  }

  const cString & GetKeyword() const { return name; }
  const cString & GetDesc() const { return desc; }
  cHelpType * GetType() const { return type; }

  bool IsAlias() const { return false; }
};

class cHelpAlias : public cHelpEntry {
private:
  cHelpFullEntry * full_entry;
public:
  cHelpAlias(const cString & _alias, cHelpFullEntry * _full_entry)
    : cHelpEntry(_alias), full_entry(_full_entry) { ; }
  ~cHelpAlias() { ; }

  const cString & GetKeyword() const { return full_entry->GetKeyword(); }
  const cString & GetDesc() const { return full_entry->GetDesc(); }
  cHelpType * GetType() const { return full_entry->GetType(); }

  bool IsAlias() const { return true; }
};

class cHelpType {
private:
  cString name;
  tList<cHelpEntry> entry_list;
  cHelpManager * manager;
  int num_entries;
public:
  cHelpType(const cString & _name, cHelpManager * _manager)
    : name(_name), manager(_manager), num_entries(0) { ; }
  ~cHelpType() { while (entry_list.GetSize() > 0) delete entry_list.Pop(); }

  cHelpFullEntry * AddEntry(const cString & _name, const cString & _desc) {
    cHelpFullEntry * new_entry = new cHelpFullEntry(_name, this, _desc);
    entry_list.Push(new_entry);
    num_entries++;
    return new_entry;
  }

  cHelpAlias * AddAlias(const cString & alias_name, cHelpFullEntry * entry) {
    cHelpAlias * new_alias = new cHelpAlias(alias_name, entry);
    entry_list.Push(new_alias);
    return new_alias;
  }

  const cString & GetName() const { return name; }

  cHelpEntry * FindEntry(const cString & entry_name) {
    tListIterator<cHelpEntry> entry_it(entry_list);
    while (entry_it.Next() != NULL) {
      if (entry_it.Get()->GetName() == entry_name) {
	return entry_it.Get(); // Found!
      }
    }
    return NULL;  // Not found...
  }

  void PrintHTML();
};

class cHelpManager {
private:
  tList<cHelpType> type_list;
  cHelpFullEntry * last_entry;
  bool verbose;   // Should we print messages to the screen?

  // Private methods...
  cHelpType * GetType(const cString type_name);
public:
  cHelpManager() : last_entry(NULL), verbose (false) { ; }
  ~cHelpManager() { while (type_list.GetSize() > 0) delete type_list.Pop(); }

  void LoadFile(const cString & filename);
  void PrintHTML();
  cHelpEntry * FindEntry(cString entry_name) {
    entry_name.ToLower();

    tListIterator<cHelpType> type_it(type_list);
    while (type_it.Next() != NULL) {
      cHelpEntry * found_entry = type_it.Get()->FindEntry(entry_name);
      if (found_entry != NULL) return found_entry;
    }
    return NULL;
  }
  
  void SetVerbose(bool _verbose = true) { verbose = _verbose; }
  bool GetVerbose() const { return verbose; }
};

#endif

