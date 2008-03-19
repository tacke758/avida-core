#ifndef DATA_ENTRY_HH
#define DATA_ENTRY_HH

#include <iostream.h>

#include "../tools/string.hh"
#include "../tools/string_list.hh"

class cDataEntry {
private:
  cString name;            // Short Name
  cString desc;            // Descriptive Name
  cString null_value;      // Value when "off", such as "0", "Inf.", or "N/A"
  cString html_table_flags; // String to include in <td> entry in html mode.
public:
  cDataEntry(const cString & _name, const cString & _desc,
	     const cString & _null="0",
	     const cString & _html_cell="align=center")
    : name(_name), desc(_desc), null_value(_null),
      html_table_flags(_html_cell) { ; }
  virtual ~cDataEntry() { ; }

  const cString & GetName() const { return name; }
  const cString & GetDesc() const { return desc; }
  const cString & GetNull() const { return null_value; }
  const cString & GetHtmlCellFlags() const { return html_table_flags; }

  virtual bool Print(ostream & fp) const { (void) fp;  return false; }
};

template <class T> class tDataEntryBase : public cDataEntry {
protected:
  T * target;
public:
  tDataEntryBase(const cString & _name, const cString & _desc,
		 const cString & _null="0",
		 const cString & _html_cell="align=center")
    : cDataEntry(_name, _desc, _null, _html_cell), target(NULL) { ; }
  
  void SetTarget(T * _target) { target = _target; }

  virtual bool Print(ostream & fp) const { (void) fp;  return false; }
  virtual int Compare(T * other) const { (void) other; return 0; }

  void HTMLPrint(ostream & fp, int compare=0, bool print_text=true) {
    fp << "<td " << GetHtmlCellFlags() << " ";
    if (compare == -2) {
      fp << "bgcolor=\"#FF0000\">";
      if (print_text == true) fp << GetNull() << " ";
      else fp << "&nbsp; ";
      return;
    }

    if (compare == -1)     fp << "bgcolor=\"#FFCCCC\">";
    else if (compare == 0) fp << ">";
    else if (compare == 1) fp << "bgcolor=\"#CCFFCC\">";
    else if (compare == 2) fp << "bgcolor=\"#00FF00\">";
    else {
      cerr << "Error! Illegal case in Compare:" << compare << endl;
      exit(0);
    }

    if (print_text == true) fp << *this << " ";
    else fp << "&nbsp; ";
  }
};


template <class T, class OUT> class tDataEntry : public tDataEntryBase<T> {
protected:
  OUT (T::*DataRetrieval)() const;
  int (T::*DataCompare)(T*) const;
public:
  tDataEntry(const cString & _name, const cString & _desc,
	     OUT (T::*_funR)() const,
	     int (T::*_funC)(T * _o) const = &T::CompareNULL,
	     const cString & _null="0",
	     const cString & _html_cell="align=center")
    : tDataEntryBase<T>(_name, _desc, _null, _html_cell), DataRetrieval(_funR),
      DataCompare(_funC) { ; }

  bool Print(ostream & fp) const {
    if (target == NULL) return false;
    fp << (target->*DataRetrieval)();
    return true;
  }

  int Compare(T * other) const { return (target->*DataCompare)(other); }
};

template <class T, class OUT, class ARG> class tArgDataEntry
  : public tDataEntryBase<T> {
protected:
  OUT (T::*DataRetrieval)(ARG) const;
  int (T::*DataCompare)(T*,ARG) const;
  ARG arg;
public:
  tArgDataEntry(const cString & _name, const cString & _desc,
		OUT (T::*_funR)(ARG) const, ARG _arg,
		int (T::*_funC)(T * _o, ARG _i) const = &T::CompareArgNULL,
		const cString & _null="0",
		const cString & _html_cell="align=center")
    : tDataEntryBase<T>(_name, _desc, _null, _html_cell),
      DataRetrieval(_funR), DataCompare(_funC), arg(_arg) { ; }

  bool Print(ostream & fp) const {
    if (target == NULL) return false;
    fp << (target->*DataRetrieval)(arg);
    return true;
  }

  int Compare(T * other) const { return (target->*DataCompare)(other, arg); }
};

template <class T> class tDataEntryCommand {
private:
  tDataEntryBase<T> * data_entry;
  cStringList args;
public:
  tDataEntryCommand(tDataEntryBase<T> * _entry, const cString & _args="")
    : data_entry(_entry), args(_args, ':') { ; }
  
  tDataEntryBase<T> & GetEntry() { return *data_entry; }
  const cStringList & GetArgs() const { return args; }
  bool HasArg(const cString & test_arg) { return args.HasString(test_arg); }

  const cString & GetName() const { return data_entry->GetName(); }
  const cString & GetDesc() const { return data_entry->GetDesc(); }
  const cString & GetNull() const { return data_entry->GetNull(); }
  const cString & GetHtmlCellFlags() const
    { return data_entry->GetHtmlCellFlags(); }

  void SetTarget(T * _target) { data_entry->SetTarget(_target); }
  bool Print(ostream & fp) const { return data_entry->Print(fp); }
  int Compare(T * other) const { return data_entry->Compare(other); }
  void HTMLPrint(ostream & fp, int compare=0, bool print_text=true)
    { data_entry->HTMLPrint(fp, compare); }
};

ostream & operator << (ostream & out, cDataEntry & entry);

#endif

