/* string.cc ******************************************************************
 String classes 
  // Conversion operators
  inline int AsInt() const;          // Return 'int' value of string.
  inline float AsFloat() const;      // Return 'float' vault of string.

 charles@krl.caltech.edu & travc@ugcs.caltech.edu
 Time-stamp: <97/07/08 16:52:54 travc>

 cString: basic string class
******************************************************************************/

#include "string.hh"

extern "C"{
#include <stdio.h>
#include <stdarg.h>
//#include <string.h>
}



/* Uncomment if bool is supported //- TCC
// {{{ IOStream Overloads

// Fix ostream output of bool
ostream & ostream::operator << (bool _in){
  if( _in==true ) 
    return (*this<<"true"); 
  else 
    return (*this<<"false");
}

// }}}
*/


// {{{ ** class cString **


// {{{ -- Constants --
const int cString::MAX_LENGTH = 1024;

// }}}


// {{{ -- Class Functions (static) --

cString cString::Stringf(const char * fmt, ...) {
  va_list argp;
  char buf[MAX_LENGTH];

  va_start(argp, fmt);
  vsprintf(buf, fmt, argp);
  va_end(argp);

  return cString(buf);
}


cString cString::ToRomanNumeral(const int in_value) {
  cString ret_string;
  if (in_value < 0) {
    ret_string = "-";
    ret_string += ToRomanNumeral(-in_value);
  } else if (in_value > 3999) {
    // Out of bounds; return a blank;
  } else if (in_value >= 1000) {
    ret_string = "M";
    ret_string += ToRomanNumeral(in_value - 1000);
  } else if (in_value >= 900) {
    ret_string = "CM";
    ret_string += ToRomanNumeral(in_value - 900);
  } else if (in_value >= 500) {
    ret_string = "D";
    ret_string += ToRomanNumeral(in_value - 500);
  } else if (in_value >= 400) {
    ret_string = "CD";
    ret_string += ToRomanNumeral(in_value - 400);
  } else if (in_value >= 100) {
    ret_string = "C";
    ret_string += ToRomanNumeral(in_value - 100);
  } else if (in_value >= 90) {
    ret_string = "XC";
    ret_string += ToRomanNumeral(in_value - 90);
  } else if (in_value >= 50) {
    ret_string = "L";
    ret_string += ToRomanNumeral(in_value - 50);
  } else if (in_value >= 40) {
    ret_string = "XL";
    ret_string += ToRomanNumeral(in_value - 40);
  } else if (in_value >= 10) {
    ret_string = "X";
    ret_string += ToRomanNumeral(in_value - 10);
  } else if (in_value == 9) {
    ret_string = "IX";
  } else if (in_value >= 5) {
    ret_string = "V";
    ret_string += ToRomanNumeral(in_value - 5);
  } else if (in_value == 4) {
    ret_string = "IV";
  } else if (in_value > 0) {
    ret_string = "I";
    ret_string += ToRomanNumeral(in_value - 1);
  }
  // else we already have it exactly and don't need to return anything.

  return ret_string;
}


int cString::StrLength(const char * _in){
  int size;
  for( size=0; _in[size]!='\0'; ++size ); // Count chars in _in
  return size;
}

// }}}


// {{{ -- Constructors & Destructor --

cString::cString() {
  CharPtrConstruct("");
}

cString::cString(const char * _in) {
  CharPtrConstruct(_in);
}

cString::cString(const cString & rhs) {
  CharPtrConstruct(rhs.ToCharPtr());
}
  
/*
cString::cString(const int size) {
  // NOTE: DOES NOT INITIALIZE CHARS!!!
  data = new char[size+1];                    // Allocate new data
  GetChar(size)='\0';
}
*/

cString::cString(char * _in, boolean allocated) {
  // only valid for _in already allocated
  // take ownership of _in
  (void)allocated;
  data = _in;
}

void cString::CharPtrConstruct(const char * _in){
  int size;
  for( size=0; _in[size]!='\0'; ++size ); // Count chars in _in
  data = new char[size+1];                    // Allocate new data
  for( int i=0; i<=size; ++i ){ GetChar(i)=_in[i]; } // Copy actual chars
}

// }}}


// {{{ -- Primary Interface -- 

cString & cString::Set(const char * fmt, ...) {
  va_list argp;
  char buf[MAX_LENGTH];

  va_start(argp, fmt);
  vsprintf(buf, fmt, argp);
  va_end(argp);

  Equals(buf);
  return *this;
}


cString & cString::Equals(const char * _in){
  boolean longer=FALSE;                   //  assume _in is not longer than
  int size = StrLength(_in);
  
  delete [] data;                           // Remove old data 
  data = new char[size+1];                  // Allocate new data
  data[size] = 0;

  // Copy actual chars
  for( int i=0; i<size; ++i ){              
    GetChar(i)=_in[i];
  }
  return *this;
}


cString cString::operator+ (char in_char) {
  int i = 0;
  if( in_char == '\0' ) return cString(*this);
  char * new_data = new char[GetSize()+1];
  while( GetChar(i) != '\0' ){
    new_data[i] = GetChar(i);
    i++;
  }
  new_data[i++] = in_char;
  new_data[i++] = '\0';
  return cString(new_data,TRUE);
}


cString cString::operator+ (const char * in_string) {
  // Make sure we are actually appending something...
  if (in_string[0] == '\0') return cString(*this); 
  int i = 0, j = 0;
  char * new_data = new char[GetSize()+StrLength(in_string)+1];
  while (data[i] != '\0') {
    new_data[i] = data[i];
    i++;
  }
  while (in_string[j] != '\0') {
    new_data[i+j] = in_string[j];
    j++;
  }
  new_data[i+j]='\0';
  return cString(new_data,TRUE);
}

// }}}


// {{{ -- Additonal Interface --


cString & cString::Insert(const char * in_string, const int pos) {
  int size = GetSize();
  if( size == 0 ){
    Equals(in_string);
    return *this;
  }

  int in_size = StrLength(in_string);
  char * new_data = new char[size + in_size + 1];
  
  int cur_pos = 0;
  for (cur_pos = 0; cur_pos < pos; cur_pos++) {
    new_data[cur_pos] = GetChar(cur_pos);    
  }
  for (cur_pos = 0; cur_pos < in_size; cur_pos++) {
    new_data[pos + cur_pos] = in_string[cur_pos];
  }
  for (cur_pos = pos; cur_pos <= size; cur_pos++) {
    new_data[cur_pos + in_size] = GetChar(cur_pos);
  }

  delete [] data;
  data = new_data;
  return *this;
}


// }}}


// {{{ -- Old Interface --


int cString::CountWhitespace(int start) const {
  if (start >= GetSize()) return 0;
  int ws_count = 0;
  // Count the ammount of whitespace.
  while (IsWhitespace(start + ws_count)) ws_count++;
  return ws_count;
}


int cString::CountWordsize(int start) const {
  if (start >= GetSize()) return 0;
  int let_count = 0;
  // Count the ammount of whitespace.
  while (!IsWhitespace(start + let_count) && data[start + let_count] != '\0')
    let_count++;
  return let_count;
}


int cString::CountLinesize(int start) const {
  if (start >= GetSize()) return 0;
  int count = 0;
  // Count the ammount of whitespace.
  while (data[start + count] != '\n' && data[start + count] != '\0') count++;
  return count;
}

int cString::CountNumLines() const {
  int num_lines = 1;
  for (int i = 0; data[i] != '\0'; i++) if (data[i] == '\n') num_lines++;
  return num_lines;
}


int cString::Clip(int clip_pos) {
  if (clip_pos >= GetSize() || clip_pos < 0) return GetSize();

  char * new_data = new char[clip_pos + 1];
  for (int i = 0; i < clip_pos; i++) {
    new_data[i] = data[i];
  }
  new_data[clip_pos] = '\0';

  delete [] data;
  data = new_data;

  return clip_pos;
}

int cString::ClipFront(int clip_size)
{
  if (clip_size > GetSize()) clip_size = GetSize();

  // Cut off the first clip_size characters.
  char * new_data = new char[sizeof(data) - clip_size + 1];


  for (int i = 0; i<clip_size; i++) {
    new_data[i] = GetChar(i + clip_size);
    if( new_data[i] == '\0' ) break;
  }
  
  delete [] data;
  data = new_data;

  return clip_size;
}

int cString::ClipEnd(int clip_size)
{
  int data_size = GetSize();
  if (clip_size > data_size) clip_size = data_size;

  // Cut off the last clip_size characters.
  char * new_data = new char[data_size - clip_size + 1];

  int i;
  for (i = 0; i < data_size - clip_size; i++) {
    new_data[i] = data[i];
  }
  new_data[i] = '\0';
  
  delete [] data;
  data = new_data;

  return clip_size;
}

cString cString::Substring(int start, int size)
{
  char * new_data = new char[size];
  for (int i = 0; i < size; i++) {
    new_data[i] = data[i + start];
  }
  return cString(new_data,TRUE);
}


int cString::LeftJustify()
{
  int ws_count = CountWhitespace();

  // If there was no whitespace, stop here.
  if (!ws_count) return 0;

  return ClipFront(ws_count);
}

int cString::RightJustify()
{
  int size = GetSize();
  int ws_count = 0;

  while (IsWhitespace(size - ws_count - 1)) ws_count++;

  // If there was no whitespace, stop here.
  if (!ws_count) return 0;

  return ClipEnd(ws_count);
}


int cString::GetNumWords() const {
  int num_words = 0;
  int pos = CountWhitespace();     // Skip initial whitespace.
  int size = GetSize();

  while (pos < size) {
    pos += CountWordsize(pos);
    pos += CountWhitespace(pos);
    num_words++;
  }

  return num_words;
}


cString cString::GetWord(int word_id) const {
  // Skip past initial whitespace.
  int pos = CountWhitespace();
  int cur_word = 0;
  
  while (cur_word < word_id) {
    pos += CountWordsize(pos);
    pos += CountWhitespace(pos);
    cur_word++;
  }

  int word_size = CountWordsize(pos);
  char * new_data = new char[word_size + 1];
  
  for (int i = 0; i < word_size; i++) {
    new_data[i] = GetChar(pos + i);
  }
  new_data[word_size] = '\0';

  cString out_string(new_data);
  delete [] new_data;

  return out_string;
}


cString cString::GetWordAt(int start) const {
  // Skip past initial whitespace.
  int pos = start + CountWhitespace(start);
  int word_size = CountWordsize(pos);

  char * new_data = new char[word_size + 1];
  
  for (int i = 0; i < word_size; i++) {
    new_data[i] = GetChar(pos + i);
  }
  new_data[word_size] = '\0';

  cString out_string(new_data);
  delete [] new_data;

  return out_string;
}

cString cString::PopWord()
{
  int i;
  int size = GetSize();

  // Advance past first word, and spaces on both sides.
  int start1 = CountWhitespace();
  int start2 = start1 + CountWordsize(start1);
  start2 += CountWhitespace(start2);

  // Calculate new sizes.
  int new_size = size - start2;
  int word_size = CountWordsize(start1);

  // Build the outgoing string.
  char * out_data = new char[word_size + 1];
  for (i = 0; i < word_size; i++) out_data[i] = data[start1 + i];
  out_data[word_size] = '\0';
  cString out_string(out_data);
  delete [] out_data;

  // Rebuild the main string.
  char * new_data = new char[new_size + 1];
  for (i = 0; i < new_size; i++) new_data[i] = data[start2 + i];
  new_data[new_size] = '\0';
  delete [] data;
  data = new_data;

  return out_string;
}


cString cString::PopLine()
{
  int i;
  int size = GetSize();

  // Advance past first word, and spaces on both sides.
  int line_size = CountLinesize();
  int new_size = size - line_size - 1;

  // Build the outgoing string.
  char * out_data = new char[line_size + 1];
  for (i = 0; i < line_size; i++) out_data[i] = data[i];
  out_data[line_size] = '\0';
  cString out_string(out_data);
  delete [] out_data;

  // Rebuild the main string.
  char * new_data = NULL;
  if (new_size == -1) {
    new_data = new char[1];
    new_data[0] = '\0';
  } else {
    new_data = new char[new_size + 1];
    for (i = 0; i < new_size; i++) new_data[i] = data[line_size + i + 1];
    new_data[new_size] = '\0';
  }
  delete [] data;
  data = new_data;

  return out_string;
}


int cString::Find(char in_char, int start_pos) const {
  int pos = start_pos;
  while (GetChar(pos) != '\0') {
    if (GetChar(pos) == in_char) return pos; // Found!
    pos++;
  }
  return -1; // Not Found
}


int cString::Find(const char * in_string, int start_pos) const {
  int pos = start_pos;
  int size1 = GetSize();
  int size2 = StrLength(in_string);
 
  while( GetChar(pos) != '\0' ){
    if( GetChar(pos) == in_string[0] ){
      // see if we have found the string...
      int i;
      for( i = 1; i < size2; i++ ){
	if( pos + i >= size1 ) return -1; // Too near this string's end.
	if( GetChar(pos + i) != in_string[i] ) break; // Match failure!
      }

      // If we have made it fully through the loop, we have found a match!
      if( i == size2 ) return pos;
    }
    pos++;
  }

  return -1;
}


int cString::Replace(const cString & old_string, const cString & new_string,
		     int start_pos)
{
  int position = Find(old_string, start_pos);
  if (position == -1) return -1;

  // Find the sizes of the sub-strings.
  int old_sub_size = old_string.GetSize();
  int new_sub_size = new_string.GetSize();
  int i;

  // If the string size has changed, adjust for it.
  if (old_sub_size != new_sub_size) {
    int string_size = GetSize();
    char * new_data = new char[string_size - old_sub_size + new_sub_size + 1];

    // Copy over the un-changed data.
    for (i = 0; i < position; i++) {
      new_data[i] = data[i];
    }
    for (i = position; i <= string_size - old_sub_size; i++) {
      new_data[i + new_sub_size] = data[i + old_sub_size];
    }
    delete [] data;
    data = new_data;
  }
  
  for (i = 0; i < new_sub_size; i++) {
    data[position + i] = new_string.GetChar(i);
  }

  return position;
}

void cString::CompressWhitespace()
{
  // This is _very_ inefficent, but works fine, and certainly "fast enough"
  // for reasonable length strings.

  cString new_string(GetWord(0));
  int num_words = GetNumWords();

  for (int word_num = 1; word_num < num_words; word_num++) {
    new_string += " ";
    new_string += GetWord(word_num);
  }

  operator=(new_string);
}

void cString::RemoveWhitespace()
{
  // This is _very_ inefficent, but works fine, and certainly "fast enough"
  // for reasonable length strings.

  cString new_string(GetWord(0));
  int num_words = GetNumWords();

  for (int word_num = 1; word_num < num_words; word_num++) {
    new_string += GetWord(word_num);
  }

  operator=(new_string);
}

void cString::Reverse()
{
  int size = GetSize();
  char * new_data = new char[size + 1];
  for (int i = 0; i < size; i++) {
    new_data[i] = data[size - i - 1];
  }
  new_data[size] = '\0';
  delete [] data;
  new_data = data;
}

// Hamming distance.
int cString::Distance(const cString & in_string, int offset) const
{
  if (offset < 0) return in_string.Distance(*this, -offset);

  const int size1 = GetSize();
  const int size2 = in_string.GetSize();

  // Calculate by how much the strings overlap.
  int overlap = size1 - offset;
  if (overlap > size2) overlap = size2;

  // Initialize the distance to that part of the strings which do not
  // overlap.
  int num_diffs = size1 + size2 - 2 * overlap;

  // Step through the overlapped section and add on any additional differences.
  for (int i = 0; i < overlap; i++) {
    if (data[i + offset] != in_string.GetChar(i)) num_diffs++;
  }

  return num_diffs;
}

// Sliding hamming distance.
int cString::SDistance(const cString & in_string) const
{
  in_string.GetSize();
  // @CAO fill in...

  return 0;
}

// Levenschtein Distance
int cString::LDistance(const cString & in_string) const
{
  const int size1 = GetSize();
  const int size2 = in_string.GetSize();

  if (!size1) return size2;
  if (!size2) return size1;

  int * cur_row  = new int[size1];  // The row we are calculating
  int * prev_row = new int[size1];  // The last row we calculater
  int * temp_row = NULL;            // Used to swap cur_row and prev_row.

  // Initialize rows to diff from nothing.
  int i, j;
  for (i = 0; i < size1; i++) {
    cur_row[i] = i + 1;
    prev_row[i] = i + 1;
  }

  for (i = 0; i < size2; i++) {
    // Initialize the first entry in the cur_row.
    cur_row[0] = (i < prev_row[0]) ? i : prev_row[0];
    if (data[0] != in_string.GetChar(i)) cur_row[0]++;

    // Move down the cur_row and fill it out.
    for (j = 1; j < size1; j++) {
      // Set the current position the the minimal of the three numbers to
      // the upper right in the chart.
      cur_row[j] = (prev_row[j] < prev_row[j-1]) ? prev_row[j] : prev_row[j-1];
      if (cur_row[j-1] < cur_row[j]) cur_row[j] = cur_row[j-1];

      // If the letters for this postion differ, increment the chart position.
      if (data[j] != in_string.GetChar(i)) cur_row[j]++;
    }

    // Swap cur_row and prev_row.
    temp_row = cur_row;
    cur_row = prev_row;
    prev_row = temp_row;
  }

  // Now that we are done, return the bottom-right corner of the chart.

  int value = prev_row[size1 - 1];
  
  delete [] cur_row;
  delete [] prev_row;

  return value;
}

boolean cString::HasWord(const cString & in_word) const
{
  int num_words = GetNumWords();
  for (int i = 0; i < num_words; i++) {
    if (GetWord(i) == in_word) return TRUE;
  }
  return FALSE;
}


void cString::ToLower()
{
  for (int pos = 0; data[pos] != '\0'; pos++) {
    if (data[pos] >= 'A' && data[pos] <= 'Z')
      data[pos] += 'a' - 'A';
  }
}

void cString::ToUpper()
{
  for (int pos = 0; data[pos] != '\0'; pos++) {
    if (data[pos] >= 'a' && data[pos] <= 'z')
      data[pos] += 'A' - 'a';
  }
}


boolean cString::operator==(const char * in_string) const
{
  int i;
  for (i = 0; GetChar(i) == in_string[i] && GetChar(i) != '\0'; i++);
  return (in_string[i] == '\0' && GetChar(i) == '\0');
}

// }}}


// }}} ** end class cString **



// {{{ ** External cString Functions **

// iostream output functions
ostream & operator << (ostream & out, const cString & string){
  out<<string.ToCharPtr();
  return out;
}
istream & operator >> (istream & in, cString & string){
  char buf[cString::MAX_LENGTH]; 
  in>>buf;
  string=buf;
  return in;
}

// }}}





/*



///////////////////
//  cStringBlock
///////////////////

cStringBlock::cStringBlock(int in_lines)
{
  num_lines = in_lines;
  if (num_lines) block = new cString[num_lines];
  else block = NULL;
}

cStringBlock::cStringBlock(const cStringBlock & in_block)
{
  num_lines = in_block.num_lines;
  if (num_lines) block = new cString[num_lines];
  else block = NULL;

  for (int i = 0; i < num_lines; i++) {
    block[i] = in_block.GetLine(i);
  }
}

cStringBlock::~cStringBlock()
{
  if (block) delete [] block;
}

const cStringBlock & cStringBlock::operator=(const cStringBlock & in_block)
{
  num_lines = in_block.num_lines;

  if (block) delete [] block;
  if (num_lines) block = new cString[num_lines];
  else block = NULL;

  for (int i = 0; i < num_lines; i++) {
    block[i] = in_block.GetLine(i);
  }

  return *this;
}
  
void cStringBlock::Print()
{
  for (int i = 0; i < num_lines; i++) {
    printf("%s\n", block[i]());
  }
}

void cStringBlock::FPrint(FILE * fp)
{
  for (int i = 0; i < num_lines; i++) {
    fprintf(fp, "%s\n", block[i]());
  }
}

int cStringBlock::WordGrep(cString in_string) const
{
  for (int i = 0; i < num_lines; i++) {
    if (block[i].HasWord(in_string)) return i;
  }

  return -1;
}

/////////////////
//  cStringList 
/////////////////

cStringList::~cStringList()
{
  while (size) RemoveFront();
}

cString & cStringList::InsertFront(const cString & new_entry)
{
  cStringEntry * new_string_entry
    = new cStringEntry(new_entry, root.GetNext(), &root);
  
  size++;
  root.GetNext()->SetPrev(new_string_entry);
  root.SetNext(new_string_entry);
  current = new_string_entry;

  return new_string_entry->GetString();
}

cString & cStringList::InsertRear(const cString & new_entry)
{
  cStringEntry * new_string_entry
    = new cStringEntry(new_entry, &root, root.GetPrev());
  
  size++;
  root.GetPrev()->SetNext(new_string_entry);
  root.SetPrev(new_string_entry);
  current = new_string_entry;

  return new_string_entry->GetString();
}

cString & cStringList::InsertCurrent(const cString & new_entry)
{
  cStringEntry * new_string_entry
    = new cStringEntry(new_entry, current->GetNext(), current);
  
  size++;
  current->GetNext()->SetPrev(new_string_entry);
  current->SetNext(new_string_entry);

  return new_string_entry->GetString();
}

void cStringList::Clear()
{
  while (size) RemoveFront();
}


void cStringList::InsertCurrent(cStringList & in_list)
{
  for (in_list.ResetTail(); !in_list.AtRoot(); in_list.Prev()) {
    InsertCurrent(in_list.GetCurrent());
  }
}

int cStringList::OK()
{
  int result = TRUE;
  int current_found = FALSE; // Make sure current is in the list.

  if (current == &root) current_found = TRUE;

  if (!size) {
    if (root.GetNext() != &root || root.GetPrev() != &root) {
      // printf("ERROR: empty cStringList improperly threaded!\n");
      result = FALSE;
    }
  }
  else {
    int count = 0;

    for (cStringEntry * test_entry = root.GetNext();
	 test_entry != &root;
	 test_entry = test_entry->GetNext()) {
      if (current == test_entry) current_found = TRUE;
      if (test_entry->GetNext()->GetPrev() != test_entry) {
	// printf("ERROR: Incorrectly threaded cStringList\n");
	result = FALSE;
      }
      count++;
      if (count > size) {
	// g_debug.Error("cStringList size is incorrect!");
	result = FALSE;
	break;
      }
    }
  }

  if (!current_found) result = FALSE;

  return result;
}

void cStringList::Remove(cStringEntry * in_entry)
{
  if (current == in_entry) Next();
  in_entry->GetNext()->SetPrev(in_entry->GetPrev());
  in_entry->GetPrev()->SetNext(in_entry->GetNext());
  delete in_entry;
  size--;
}

void cStringList::Print()
{
  int cur_line = 0;
  for (cStringEntry * cur_entry = root.GetNext();
       cur_entry != &root;
       cur_entry = cur_entry->GetNext()) {
    printf("%d: %s\n", cur_line, cur_entry->GetString()());
    cur_line++;
  }
}

void cStringList::CompressWhitespace()
{
  for (cStringEntry * cur_entry = root.GetNext();
       cur_entry != &root;
       cur_entry = cur_entry->GetNext()) {
    cur_entry->GetString().CompressWhitespace();
  }
}

void cStringList::RemoveWhitespace()
{
  for (cStringEntry * cur_entry = root.GetNext();
       cur_entry != &root;
       cur_entry = cur_entry->GetNext()) {
    cur_entry->GetString().RemoveWhitespace();
  }
}

void cStringList::CompressLinefeeds()
{
  int last_line_empty = FALSE;

  current = root.GetNext();
  while(current != &root) {
    if (current->GetString().IsEmpty()) {
      if (last_line_empty) RemoveCurrent();
      else current = current->GetNext();

      last_line_empty = TRUE;
    } else {
      last_line_empty = FALSE;
      current = current->GetNext();
    }
  }
}

void cStringList::ToUpper()
{
  for (cStringEntry * cur_entry = root.GetNext();
       cur_entry != &root;
       cur_entry = cur_entry->GetNext()) {
    cur_entry->GetString().ToUpper();
  }
}

void cStringList::ToLower()
{
  for (cStringEntry * cur_entry = root.GetNext();
       cur_entry != &root;
       cur_entry = cur_entry->GetNext()) {
    cur_entry->GetString().ToLower();
  }
}

void cStringList::ReverseLines()
{
  for (cStringEntry * cur_entry = root.GetNext();
       cur_entry != &root;
       cur_entry = cur_entry->GetNext()) {
    cur_entry->GetString().Reverse();
  }
}

int cStringList::FindKeyword(const cString & keyword, int col)
{
  cStringEntry * current_save = current;
  while (current != &root) {
    if (current->GetString().GetWord(col) == keyword) {
      return TRUE;
    }
    current = current->GetNext();
  }

  current = current_save;
  return FALSE;
}

int cStringList::Grep(const cString & grepword)
{
  cStringEntry * current_save = current;
  while (current != &root) {
    if (current->GetString().Find(grepword) >= 0) {
      return TRUE;
    }
    current = current->GetNext();
  }

  current = current_save;
  return FALSE;
}

cString & cStringList::SetCurrent(int pos)
{
  Reset(); 
  for (int i = 0; i < pos; i++) Next();  

  return GetCurrent();
}
*/
