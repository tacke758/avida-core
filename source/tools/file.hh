#ifndef FILE_HH
#define FILE_HH

#include <fstream.h>
#include "string.hh"

class cFile {
private:
  fstream fp;
  cString filename;
public:
  inline cFile() : filename("") { ; }
  inline cFile(cString _filename) : filename("") { Open(_filename); }
  ~cFile() { fp.close(); filename = ""; }

  inline const cString & GetFilename() { return filename; }

  // File interactions
  int Open(cString _filename, int mode=(ios::in|ios::nocreate));
  int Close();
  int ReadLine(cString & in_string);
  //int CountLines();
  //inline void Rewind() { fp.rdbuf()->seekpos(ios::beg,0); }

  // Tests
  inline int IsOpen() { return (filename != ""); }
  inline int Fail() { return (fp.fail()); }
  inline int Good() { return (fp.good()); }
  inline int Eof() { return (fp.eof()); }
  // int AtStart();
  // int AtEnd();
};


// A string Single Linked-List Element.

class cStringSLLE {
private:
  cString string;
  cStringSLLE * next;
public:
  inline cStringSLLE(cStringSLLE * in_next = NULL) { next = in_next; }

  inline cString & String() { return string; }
  inline cStringSLLE * GetNext() { return next; }

  inline void SetNext(cStringSLLE * in_next) { next = in_next; }
};


// A class to handle initialization files.

class cInitFile : public cFile {
private:
  int num_lines;
  cStringSLLE * line_list;

private:
  cInitFile(){ ; } // No default constructor

public:
  cInitFile(cString in_filename);
  ~cInitFile();

  void Load();
  //void Save(char * in_filename = NULL);
  void Compress();
  void AddLine(cString & in_string);
  cString GetLine(int line_num=0);
  cString RemoveLine();
  void PrintLines();
  int Find(cString & in_string, cString keyword, int col) const;

  inline int GetNumLines() { return num_lines; }
};


// A specialized initialization file for setting values.

class cGenesis : public cInitFile {
public:
  cGenesis(const char * filename = "genesis");

  void AddInput(char * in_name, int in_value);
  void AddInput(char * in_name, char * in_value);

  int     ReadInt   (char * name, int base     = 0) const;
  double  ReadFloat (char * name, float base   = 0.0) const;
  cString ReadString(char * name, cString base) const;
  cString ReadString(char * name) const;
};


// A specialized initiaization for schedules.

class cEvent {
private:
  int update;
  cString name;
  cString arg_list;
  cEvent * next;
public:
  inline cEvent(int in_update, const cString & in_name,
		const cString & in_arg_list) {
    update = in_update;
    name = in_name;
    arg_list = in_arg_list;
    next = NULL;
  }
  inline cEvent(const cEvent & in_event) {
    update = in_event.update;
    name = in_event.name;
    arg_list = in_event.arg_list;
    next = NULL;
  }

  inline cEvent & operator=(const cEvent & in_event) {
    update = in_event.update;
    name = in_event.name;
    arg_list = in_event.arg_list;
    return *this;
  }

  inline int GetUpdate() const { return update; }
  inline const cString & GetName() const { return name; }
  inline const cString & GetArgs() const { return arg_list; }
  inline cEvent * GetNext() const { return next; }

  inline void SetNext(cEvent * in_next) { next = in_next; }
};


class cEventList : public cInitFile {
private:
  cEvent * event_list;
  int num_events;

  void BuildEventList();
public:
  cEventList(char * filename = "event_list");
  inline ~cEventList() { while (event_list) Next(); }

  void Next();  // Delete current event and move to the next one.
  void AddEvent(int update, const cString & name, const cString & arg_list);


  inline int GetNextUpdate() { return event_list->GetUpdate(); }
  inline cEvent * GetEvent() { return event_list; }
  inline int GetNumEvents()  { return num_events; }
};

#endif
