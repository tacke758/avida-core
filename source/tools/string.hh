/* string.hh ******************************************************************
 String classes 

 charles@krl.caltech.edu & travc@ugcs.caltech.edu
 Time-stamp: <98/04/07 19:35:24 travc>

 cString: basic string class
******************************************************************************/
  
#ifndef STRING_HH
#define STRING_HH

#include <stdlib.h>
#include <iostream.h>
#include "assert.hh"
#include "../defs.hh"


// if Boolean is not a type....
#define Boolean char
#ifndef TRUE
#define FALSE 0
#define TRUE !FALSE
#endif


// ** class cString **********************************************************
class cString {

protected:
  void CopyOnWrite(){
    if( value->IsShared() ){  // if it is shared
      value->RemoveRef();     // remove our reference count
      value = new cStringData(*value);  // make own copy of value
    }
  }

  // -- Contained Classes -- 
private:
  // Declarations (only needed)
  class cStringData;

  // {{{ -- cCharProxy -- To detect rvalue vs lvalue --------------------- 

  class cCharProxy{
  private:
    cString & string; 
    unsigned short index;

  public:
    cCharProxy( cString & _string, unsigned short _index) :
     string(_string), index(_index) {;}

    inline cCharProxy & operator= (char c);     // lvalue
    inline cCharProxy & operator+= (char c);    // lvalue
    inline cCharProxy & operator-= (char c);    // lvalue
    inline cCharProxy & operator++ ();          // lvalue (prefix)
    inline char         operator++ (int dummy); // lvalue (postfix)
    inline cCharProxy & operator-- ();          // lvalue (prefix)
    inline char         operator-- (int dummy); // lvalue (postfix)
    inline operator char () const ;             // rvalue
  }; 

  friend cCharProxy;  // Telling rvalue vs lvalue ....

  // }}}  End cCharProxy 
  // {{{ -- cStringData -- Holds the actual data and is reference count -- 
  class cStringData{
    // NOTE: Terminating NULL is always there (you can't assign!!)
  
  private:
    unsigned short refs;   // Number of references 
    unsigned short size;   // size of data (NOT INCLUDING TRAILING NULL)
    char * data;
    
  public:
    explicit cStringData(unsigned short in_size);
    cStringData(unsigned short in_size, const char * in);
    cStringData(const cStringData & in);
    
    ~cStringData(){ 
      assert_error( refs == 0, "Deleting cStringData with References!!" );
      delete [] data; }

    
    cStringData & operator= (const cStringData & in) {
      delete [] data;
      size = in.GetSize();
      data = new char [size+1];
      assert_error(data!=NULL,"Memory Allocation Error: Out of Memory");
      for( unsigned short i=0; i<size; ++i )  data[i]=in[i];
      data[size] = '\0';
      return (*this);
    }


    unsigned short GetSize() const { return size; }
    
    const char * GetData() const { return data; }
    
    char operator[] (int index) const {
      assert_error(index >= 0, "Lower Bounds Error");
      assert_error(index <= size, "Upper Bounds Error");
      return data[index];
    }

    char & operator[](int index) {
      assert_error(index >= 0, "Lower Bounds Error");
      assert_error(index <= size, "Upper Bounds Error");
      assert_debug(index != size, "Cannot Change Terminating NULL");
      return data[index];
    }
    
    
    Boolean IsShared() { return( refs > 1 ); }
    
    unsigned short RemoveRef() { 
      assert_error( refs > 0, "Reference count corrupted");
      return(--refs);
    }
    
    cStringData * NewRef() { ++refs; return this; }

  };  
  // }}} End cStringData


  // -- Class Functions (static) --
public:
  static cString Stringf(const char * fmt, ...);
  static cString ToRomanNumeral(const int in_value);
  static int StrLength(const char * _in);


  // -- Constants -- 
public:
  static const int MAX_LENGTH;
  


  //  -- INTERFACE -----------------------------------------------------------
public:

  // -- Constructors --
  cString(const char * in = "") : value(new cStringData(StrLength(in), in)) {
    assert_error( in!=NULL, "NULL input string" );
    assert_error(value!=NULL,"Memory Allocation Error: Out of Memory"); }
  explicit cString(const int size) : value(new cStringData(size)) {
    assert_error(value!=NULL,"Memory Allocation Error: Out of Memory"); }
  cString(const cString & in) : value(in.value->NewRef()) {;}


  // -- Destructor --
  virtual ~cString() { if( value->RemoveRef() == 0 )  delete value; }

  
  // Cast to const char *
  operator const char * () const { return value->GetData(); }
  const char * operator() () const { return value->GetData(); } // Depricated
  const char * GetData() const { return value->GetData(); } // Depricated


  // Assignment Operators
  cString & operator= (const cString & in){
    ChangeValue(in.value); 
    return *this; }
  cString & operator= (const char * in){ 
    assert_error( in!=NULL, "NULL input string" );
    if( value->RemoveRef() == 0 ) delete value;
    value = new cStringData(StrLength(in),in);
    assert_error(value!=NULL,"Memory Allocation Error: Out of Memory");
    return *this; }


  // Get Size of String (Not Includeing Terminating '\0')
  int GetSize() const { return value->GetSize(); }


  // Comparisons
  int Compare(const char * in) const;  // strcmp like function
  Boolean operator== (const char * in)    const { return (Compare(in)==0); }
  Boolean operator== (const cString & in) const;  // A bit optimized
  Boolean operator!= (const char * in)    const { return !(*this==in); }
  Boolean operator<  (const char * in)    const { return (Compare(in)<0); }
  Boolean operator>  (const char * in)    const { return (Compare(in)>0); }
  Boolean operator<= (const char * in)    const { return (Compare(in)<=0); }
  Boolean operator>= (const char * in)    const { return (Compare(in)>=0); }

  // Concatination
  cString & operator+= (const char in)  { return AppendStr(1,&in); }
  cString & operator+= (const char * in){ return AppendStr(StrLength(in),in); }
  cString & operator+= (const cString & in){return AppendStr(in.GetSize(),in);}
  cString operator+ (const char in_char){ return (cString(*this) += in_char); }
  cString operator+ (const char * in)   { return (cString(*this) += in); }
  cString operator+ (const cString & in){ return (cString(*this) += in); }


  // Additional modifiers
  cString & Set(const char * fmt, ...);

  cString & Insert(const char in, int pos=0, int excise=0){
    return InsertStr(1, &in, pos, excise); }
  cString & Insert(const char * in, int pos=0, int excise=0){
    return InsertStr(StrLength(in), in, pos, excise); }
  cString & Insert(const cString & in, int pos=0, int excise=0){ 
    return InsertStr(in.GetSize(), in, pos, excise); }

  // Miscelaneous modifiers ... Do we really need these?
  void SetRoman(const int in_value){ (*this)=ToRomanNumeral(in_value); }

  // Removes 'size' characters from 'pos' (default size = to end of string)
  cString & Clip(int pos, int size = -1 /*end of string*/ ){
    if( size < 0 ) size = GetSize()-pos;
    return InsertStr(0, NULL, pos, size); } 
  cString & ClipFront(int size){  // Clip off first 'clip_size' chars
    return InsertStr(0, NULL, 0, size); } 
  cString & ClipEnd(int size){    // Clip off last 'clip_size' chars
    return InsertStr(0, NULL, GetSize()-size, size); } 

  // Replace first old_string with new_string. Return position it was found.
  int Replace(const cString & old_st, const cString & new_st, int pos=0); 

  cString Pop(const char delim);  // Remove and return up to delim char
  cString PopWord();              // Remove the first word and return it.
  cString PopLine(){ return Pop('\n'); }  // Remove and return first line

  int LeftJustify();   // Remove begining whitespace, returns num removed
  int RightJustify();  // Remove ending whitespace, returns num removed
  void Reverse(); // Reverse the character order in string.

  void ToLower(); // convert the string to lowercase.
  void ToUpper(); // convert the string to uppercase.

  void CompressWhitespace();         // Change whitespace blocks to 1 space.
  void RemoveWhitespace();           // Get rid of ALL whitespace.

  // A few functions to determine how similar strings are.
  int Distance(const cString & in_string, int offset=0) const; // Hamming Dist.
  //int SDistance(const cString & in_string) const; // Sliding Hamming Distance
  int LDistance(const cString & in_string) const; // Levenschtein Distance



  // Individal Char Access
  inline char operator[] (int index) const { 
    //return (char)((*value)[index]);
    // Below should work, but fails in HP CC -- TCC
    return const_cast(const cStringData&, *value)[index];
  }
  cCharProxy operator[] (int index) { return cCharProxy(*this,index); }




  // Information...

  // Conversion 
  int AsInt() const { return atoi(*this); }
  double AsDouble() const { return atof(*this); } 
  
  // Accessors & Information 
  Boolean IsEmpty() const { return GetSize()==0; } // Can just call GetSize
  int CountWhitespace(int start=0) const; // Counts whitespace from 'start'
  int CountWordsize(int start=0) const; // Counts non-whitespace from 'start'
  int CountLinesize(int start=0) const; // Counts until first \n from 'start'
  int CountNumLines() const;            // Count number of lines in string
  int CountNumWords() const;            // Count numbers of words in string
  cString GetWord(int word_id=0) const; // Get a specific word from the string
  cString GetWordAt(int start=0) const; // Get the first word after start

  Boolean IsWhitespace(int pos) const {
    return ( (*this)[pos] == ' '  || (*this)[pos] == '\t' ||
	     (*this)[pos] == '\r' || (*this)[pos] == '\n' ); }
  Boolean IsUpperLetter(int pos) const {
    return ((*this)[pos] >= 'A' && (*this)[pos] <= 'Z');
  }
  Boolean IsLowerLetter(int pos) const {
    return ((*this)[pos] >= 'a' && (*this)[pos] <= 'z');
  }
  Boolean IsLetter(int pos) const {
    return IsUpperLetter(pos) || IsLowerLetter(pos);
  }
  Boolean IsNumber(int pos) const {
    return ((*this)[pos] >= '0' && (*this)[pos] <= '9');
  }
  Boolean IsAlphaNumeric(int pos) const {
    return IsLetter(pos) || IsNumber(pos);
  }

  // Searching (Find first occurence after 'pos', -1 = not found)
  int Find(char in_char, int pos=0) const; 
  int Find(const char * in, int pos=0) const{ 
    return FindStr(in, StrLength(in), pos); }
  int Find(const cString & in, int pos=0) const{ 
    return FindStr(in, in.GetSize(), pos); }

  int FindWord(const cString & in, int pos=0) const; 

  cString Substring(int start, int size) const ; // return substring.
  Boolean IsSubstring(const cString & in_string, int start) const;


// {{{ -- INTERNALS -------------------------------------------------------
protected:
  // -- Internal Functions --
  void ChangeValue(cStringData * new_ref){   // If you didn't make new_value
    if( value->RemoveRef() == 0 ) delete value;
    value = new_ref->NewRef();
  }

  void TakeValue(cStringData * new_ref){     // If you made new_value!
    if( value->RemoveRef() == 0 ) delete value;
    value = new_ref;
  }
      
  // Methods that take input string size (unsafe to call from outside)
  cString & AppendStr(const int in_size, const char * in);  // Optimized
  cString & InsertStr(const int in_size, const char * in, 
		      int pos, int excise=0);
  int FindStr(const char * in_string, const int in_size, int pos) const;

  // -- Internal Data --
protected:
  cStringData * value;

// }}} End Internals

};



/*
  // Depricated
  inline void Clear() { delete [] data; data = new char[1]; data[0]='\0'; }
  inline cString & Wrap(char * in_data) // Too Dangerous
    { if (data) delete [] data; data = in_data; return *this; }
  Boolean HasWord(const cString & in_word) const;  // FindWord replaces
  */


// {{{ ** External cString Functions **

// iostream input
istream & operator >> (istream & in, cString & string);

// }}}





// NOT UPDATED...  --@TCC

class cStringBlock {
private:
  int num_lines;
  cString * block;
public:
  cStringBlock(int in_lines=1);
  cStringBlock(const cStringBlock & in_block);
  ~cStringBlock();

  int WordGrep(cString in_string) const;  // Find first line num with word.

  const cStringBlock & operator=(const cStringBlock & in_block);
  inline cString & operator[](int line_num) { return block[line_num]; }
  inline const cString & GetLine(int line_num) const
    { return block[line_num]; }
  inline const cString & GetLastLine() const { return block[num_lines - 1]; }
  inline cString & EditLastLine() { return block[num_lines - 1]; }

  inline int GetNumLines() const { return num_lines; }
};

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

  inline void RemoveFront()
  { cStringEntry * out_entry = root.GetNext(); Remove(out_entry); }
  inline void RemoveRear()
  { cStringEntry * out_entry = root.GetPrev(); Remove(out_entry); }
  inline void RemoveCurrent() { Remove(current); }

  void Clear();
  
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


// -- INLINE INCLUDES --

#include "string.ii"


#endif
