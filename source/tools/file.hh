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

  inline const cString & GetFilename() const { return filename; }

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

public:
  cInitFile();
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
  cGenesis();
  cGenesis(const char * filename);

  int Open(cString _filename, int mode=(ios::in|ios::nocreate));

  void AddInput(const char * in_name, int in_value);
  void AddInput(const char * in_name, const char * in_value);

  int     ReadInt   (char * name, int base     = 0) const;
  double  ReadFloat (char * name, float base   = 0.0) const;
  cString ReadString(char * name, cString base) const;
  cString ReadString(char * name) const;
};

#endif
