#ifndef STRING_LIST_HH
#define STRING_LIST_HH

#include "tList.hh"
#include "string.hh"

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

  bool HasString(const cString & test_string) const;
  cString PopString(const cString & test_string);

  void Push(const cString & _in) { string_list.Push(new cString(_in));}
  void PushRear(const cString & _in) { string_list.PushRear(new cString(_in));}
  cString Pop() { return ReturnString(string_list.Pop()); }
  cString PopRear() { return ReturnString(string_list.PopRear()); }

  void Load(cString _list, char seperator=' ') {
    while (_list.GetSize() > 0) PushRear(_list.Pop(seperator));
  }
};

#endif

