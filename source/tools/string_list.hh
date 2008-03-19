#ifndef STRING_LIST_HH
#define STRING_LIST_HH

#include "tList.hh"
#include "string.hh"

class cStringList;

class cStringIterator {
private:
  tConstListIterator<cString> list_it;
  static const cString null_str;
public:
  cStringIterator(const cStringList & in_list);

  void Reset() { list_it.Reset(); }
  const cString & Get() {
    if (list_it.Get() == NULL) return null_str;
    return *(list_it.Get());
  }
  const cString & Next() { list_it.Next(); return Get(); }
  const cString & Prev() { list_it.Prev(); return Get(); }
  bool AtRoot() const { return list_it.AtRoot(); }
  bool AtEnd() const { return list_it.AtEnd(); }
};

class cStringList {
private:
  tList<cString> string_list;

  inline cString ReturnString(cString * out_string) {
    cString tmp_string(*out_string);
    delete out_string;
    return tmp_string;
  }
public:
  cStringList() { ; }
  cStringList(const cString & _list, char seperator=' ');
  cStringList(const cStringList & _list);
  ~cStringList();

  cStringList & operator=(const cStringList & _list);

  int GetSize() const { return string_list.GetSize(); }
  cString GetLine(int line_num) { return *(string_list.GetPos(line_num)); }
  const tList<cString> & GetList() const { return string_list; }

  bool HasString(const cString & test_string) const;
  cString PopString(const cString & test_string);

  void Push(const cString & _in) { string_list.Push(new cString(_in));}
  void PushRear(const cString & _in) { string_list.PushRear(new cString(_in));}
  cString Pop() { return ReturnString(string_list.Pop()); }
  cString PopRear() { return ReturnString(string_list.PopRear()); }

  void Load(cString _list, char seperator=' ') {
    while (_list.GetSize() > 0) PushRear(_list.Pop(seperator));
  }
  void Clear() {
    while (string_list.GetSize() > 0) delete string_list.Pop(); 
  }
};

#endif

