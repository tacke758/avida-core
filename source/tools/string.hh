/* string.hh ******************************************************************
 String classes 

 charles@krl.caltech.edu & travc@ugcs.caltech.edu
 Time-stamp: <97/07/08 16:51:56 travc>

 cString: basic string class
******************************************************************************/
  
#ifndef STRING_HH
#define STRING_HH

#include <stdlib.h>
#include <iostream.h>


// if boolean is not a type....
#define boolean char
#ifndef TRUE
#define FALSE 0
#define TRUE !FALSE
#endif


// {{{ ** class cString **
class cString {

  // {{{ -- Class Functions (static) --

public:
  static cString Stringf(const char * fmt, ...);
  static cString ToRomanNumeral(const int in_value);

  static int StrLength(const char * _in);

  // }}}
  

  // {{{ -- Constructors & Destructor --

public:
  cString();
  cString(const cString & in_string);
  cString(const char * in_string);
  //cString(const int size);
  ~cString(){ delete [] data; }

private:
  cString(char * in_string, boolean allocated); // Pre-Allocated in_string 

  // }}}


  // {{{ -- Interface --

public:

  // Set
  cString & Set(const char * fmt, ...);

  // Char * converstion
  const char * ToCharPtr() const { return data; }      // Const 
  const char * GetData() const { return ToCharPtr(); } // Const
  char * GetData() { return data; }                    // Non-Const
  const char * operator()() const { return GetData(); }  // Const
  char * operator()() { return GetData(); }              // Non-Const


  // Assignment from char *
  cString & Equals(const char * _in);  
  cString & operator = (const char * _in){ return Equals(_in); }


  // Assingment from cString
  cString & Equals(const cString & _in){ return Equals(_in.ToCharPtr()); }
  cString & operator = (const cString & _in){ return Equals(_in); }


  // Individal Char Access
  char GetChar(const int index) const { return data[index]; }  // Const
  char & GetChar(const int index) { return data[index]; }      // Non-Const
  char operator[](int index) const { return GetChar(index); }  // Const
  char & operator[](int index) { return GetChar(index); }      // Non-Const


  // Concatination
  cString operator+ (char in_char);
  cString operator+ (const char * in_string);
  cString operator+ (const cString & _in){ return *this+_in.ToCharPtr(); }
  cString & operator+= (char in_char){ Equals(*this+in_char); return *this; }
  cString & operator+= (const char * _in){ Equals(*this+_in); return *this; }
  cString & operator+= (const cString & _in){ return *this+=_in.ToCharPtr(); }


  // Accessors & Information (const)
  int GetSize() const { return StrLength(data); } 
  int CountWhitespace(int start=0) const; // Counts whitespace from 'start'.
  int CountWordsize(int start=0) const; // Counts non-whitespace from 'start'.
  int CountLinesize(int start=0) const; // Counts until first \n from start.
  int CountNumLines() const;            // Count number of lines in string.
  int GetNumWords() const;              // Get the numbers of words in string.
  cString GetWord(int word_id=0) const; // Get a specific word from the string.
  cString GetWordAt(int start=0) const; // Get the first word after start.


  // Searching (Find)
  int Find(char in_char, int start_pos=0) const;
  // Find first occurence after start_pos.
  int Find(const char * in_string, int start_pos=0) const;
  // Find first occurence after start_pos.
  int Find(const cString & in_string, int start_pos=0) const{
    return Find( in_string.ToCharPtr(), start_pos ); }


  // Modifiers
  cString & Insert(const char * in_string, const int pos=0);
  cString & Insert(const cString & _in, const int pos=0){ 
    return Insert(_in.ToCharPtr(),pos); }


  // Conversion operators
  int AsInt() const { return atoi(ToCharPtr()); }
  double AsDouble() const { return atof(ToCharPtr()); } 
  

  // Miscelaneous
  void SetRoman(const int in_value){ Equals(ToRomanNumeral(in_value)); }


  // -- Old Interface --
public:
  int Replace(const cString & old_string, const cString & new_string,
	      int start_pos=0); // Replace first old_string with new_string.
  // Return position it was found.

  int Clip(int clip_pos);            // Clip off anything past 'clip_pos'.
  int ClipFront(int clip_size);      // Clip off first 'clip_size' chars.
  int ClipEnd(int clip_size);        // Clip off last 'clip_size' chars.
  cString Substring(int start, int size); // return substring.
  int LeftJustify();                 // Remove begining whitespace.
  int RightJustify();                // Remove ending whitespace.  
  cString PopWord();                 // Remove the first word and return it.
  cString PopLine();                 // Remove the first line and return it.
  void CompressWhitespace();         // Change whitespace blocks to 1 space.
  void RemoveWhitespace();           // Get rid of ALL whitespace.
  void Reverse();                    // Reverse the character order in string.

  // A few functions to determine how similar strings are.
  int Distance(const cString & in_string, int offset=0) const; // Hamming Dist.
  int SDistance(const cString & in_string) const; // Sliding Hamming Distance
  int LDistance(const cString & in_string) const; // Levenschtein Distance

  // stdio output fucctions
  //inline void Print() const { FPrint(stdout); }
  //inline void FPrint(FILE * fp) const { fprintf(fp, "%s", data); }
  //inline void Println() const { FPrintln(stdout); }
  //inline void FPrintln(FILE * fp) const { fprintf(fp, "%s\n", data); }

  inline void Clear() { delete [] data; data = new char[1]; data[0]='\0'; }
  inline cString & Wrap(char * in_data)
  { if (data) delete [] data; data = in_data; return *this; }

  // Boolean operators
  boolean operator==(const char * _in)    const;
  boolean operator==(const cString & _in) const{return (*this==_in.ToCharPtr());}
  boolean operator!=(const char * _in)    const{return !(*this==_in); }
  boolean operator!=(const cString & _in) const{return !(*this==_in); }

  boolean IsWhitespace(int pos) const {
    return (GetChar(pos) == ' ' || GetChar(pos) == '\t' ||
	    GetChar(pos) == '\r' || GetChar(pos) == '\n'); }
  boolean IsEmpty() const { return GetChar(0) == '\0'; }
  boolean HasWord(const cString & in_word) const;


  // Internal conversions
  void ToLower(); // convert the string to lowercase.
  void ToUpper(); // convert the string to uppercase.

  // }}}


  // {{{ -- Constants -- 
  // assinged in .cc file

public:
  static const int MAX_LENGTH;

  // }}}


  // {{{ -- Internal Data --
protected:
  char * data;

  // }}}

    
  // {{{ -- Internal Methods --
protected:
  void CharPtrConstruct(const char * _in); // called from constructors

  // }}}

};
// }}}


// {{{ ** External cString Functions **

// iostream output functions
ostream & operator << (ostream & out, const cString & string);
istream & operator >> (istream & in, cString & string);

// }}}







/*
class cStringBlock {
private:
  int num_lines;
  cString * block;
public:
  cStringBlock(int in_lines=1);
  cStringBlock(const cStringBlock & in_block);
  ~cStringBlock();

  void Print();
  //void FPrint(FILE * fp);

  int WordGrep(cString in_string) const;  // Find first line num with word.

  const cStringBlock & operator=(const cStringBlock & in_block);
  inline cString & operator[](int line_num) { return block[line_num]; }
  inline const cString & GetLine(int line_num) const
    { return block[line_num]; }
  inline const cString & GetLastLine() const { return block[num_lines - 1]; }
  inline cString & EditLastLine() { return block[num_lines - 1]; }

  inline int GetNumLines() const { return num_lines; }
};
*/

class cStringEntry {
private:
  cString string;
  cStringEntry * next;
  cStringEntry * prev;
public:
  inline cStringEntry(const cString & in_string, cStringEntry * in_next=NULL,
		      cStringEntry * in_prev=NULL)
  { string = in_string; next = in_next; prev = in_prev; }
  inline cStringEntry() { next = NULL; prev = NULL; }
  inline ~cStringEntry() { ; }

  inline cString & GetString() { return string; }
  inline cStringEntry * GetNext() { return next; }
  inline cStringEntry * GetPrev() { return prev; }

  inline void SetNext(cStringEntry * in_next) { next = in_next; }
  inline void SetPrev(cStringEntry * in_prev) { prev = in_prev; }
};

class cStringList {
private:
  cStringEntry root;
  cStringEntry * current;
  int size;

  void Remove(cStringEntry * in_entry);
public:
  inline cStringList() {
    root.SetNext(&root);
    root.SetPrev(&root);
    current = &root;
    size = 0;
  }
  ~cStringList();

  int OK();
  
  cString & InsertFront(const cString & in_string);
  cString & InsertRear(const cString & in_string);
  cString & InsertCurrent(const cString & in_string);
  void InsertCurrent(cStringList & in_list);
  inline cString & InsertFront(char * in_string)
  { cString temp_string; return InsertFront(temp_string.Wrap(in_string)); }
  inline cString & InsertRear(char * in_string)
  { cString temp_string; return InsertRear(temp_string.Wrap(in_string)); }
  inline cString & InsertCurrent(char * in_string)
  { cString temp_string; return InsertCurrent(temp_string.Wrap(in_string));}

  inline void RemoveFront()
  { cStringEntry * out_entry = root.GetNext(); Remove(out_entry); }
  inline void RemoveRear()
  { cStringEntry * out_entry = root.GetPrev(); Remove(out_entry); }
  inline void RemoveCurrent() { Remove(current); }

  void Clear();
  void Print();
  
  // The following are several compression techniques, mostly to make
  // the strings look nicer/be easier to read.

  void CompressWhitespace();
  void RemoveWhitespace();
  void CompressLinefeeds();
  void ToUpper();
  void ToLower();
  void ReverseLines();

  // The following methods search through the list of strings for particular
  // words.  They start at the 'current' line, and then reset current if a
  // match is found.
  int FindKeyword(const cString & keyword, int col);
  int Grep(const cString & grepword);

  inline int AtFront() { return (current == root.GetNext()); }
  inline int AtEnd()   { return (current == root.GetPrev()); }
  inline int AtRoot()  { return (current == &root); }

  inline cString & Reset()
  { current = root.GetNext(); return current->GetString(); }
  inline cString & ResetTail()
  { current = root.GetPrev(); return current->GetString(); }
  inline cString & GetCurrent() const { return current->GetString(); }
  inline cString & Next()
  { current = current->GetNext();  return current->GetString(); }
  inline cString & Prev()
  { current = current->GetPrev();  return current->GetString(); }
  cString & SetCurrent(int pos);

  inline int GetSize() { return size; }
  inline cString & GetFirst() { return root.GetNext()->GetString(); }
  inline cString & GetLast() { return root.GetPrev()->GetString(); }
};


// Comment this class later
class cBuffer {
private:
  char * data;
  int data_size;
  int buf_size;
  int start_pos;
public:
  cBuffer(int size=8);
  ~cBuffer();

  void Add(char in_char);
  void Add(const cString & in_string);

  char operator[](int pos);

  cString AsString();

  inline int GetSize() { return data_size; }
};


#endif
