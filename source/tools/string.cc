/* string.cc ******************************************************************
 String classes 
  // Conversion operators
  inline int AsInt() const;          // Return 'int' value of string.
  inline float AsFloat() const;      // Return 'float' vault of string.

 charles@krl.caltech.edu & travc@ugcs.caltech.edu
 Time-stamp: <98/06/11 19:27:44 travc>

 cString: basic string class
******************************************************************************/

#include "string.hh"

extern "C"{
#include <stdio.h>
#include <stdarg.h>
}



// ** class cStringData **
// -- Constructors --
cString::cStringData::cStringData(unsigned short in_size) :
 refs(1), size(in_size), data(new char[size+1]) { 
   assert_error(data!=NULL,"Memory Allocation Error: Out of Memory");
   data[0] = '\0'; 
   data[size] = '\0'; 
 }

cString::cStringData::cStringData(unsigned short in_size, const char * in) :
 refs(1), size(in_size), data(new char[size+1]) {
   assert_error(data!=NULL,"Memory Allocation Error: Out of Memory");
   for( unsigned short i=0; i<size; ++i )  data[i]=in[i];
   data[size] = '\0';
 }

cString::cStringData::cStringData(const cStringData & in) :
 refs(1), size(in.GetSize()), data(new char[size+1]) {
   assert_error(data!=NULL,"Memory Allocation Error: Out of Memory");
   for( unsigned short i=0; i<size; ++i )  data[i]=in[i];
   data[size] = '\0';
 }



// ** class cString **

// -- Constants --
const int cString::MAX_LENGTH = 1024;


// -- Class Functions (static) --
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


int cString::StrLength(const char * in){
  // if 'in'==NULL return 0;
  int size = 0;
  if( in != NULL ){
    while( in[size]!='\0' )  ++size; // Count chars in _in
  }
  return size;
}





// -- Comparisons --

Boolean cString::operator== (const cString & in) const {
  // Compares sizes first since we have that info anyway
  int i=-1;
  if( GetSize() == in.GetSize() ){
    for( i=0; i<GetSize() && (*this)[i]==in[i]; ++i );
  }
  return ( i == GetSize() );  // i == GetSize if all chars matched
}


int cString::Compare(const char * in) const {
  // -1 (*this<in), 0 (*this==in), 1 (*this>in) ... just like strcmp()
  assert_error( in!=NULL, "NULL input string" );
  int i;
  for( i=0; (*this)[i]!='\0' && in[i]!='\0' && (*this)[i]==in[i]; ++i );
  if( (*this)[i] == in[i] ){
    return 0;                // *this == in
  }else{
    if( (*this)[i] < in[i] ){ 
      return -1;             // *this < in
    }
  }
  return 1;                  // *this > in
}
    


// -- Information --

int cString::CountWhitespace(int start) const {
  assert_error( start >= 0, "Negative Index into String");
  if( start>=GetSize() )  return 0;
  int count = 0;
  while( start+count<GetSize() && IsWhitespace(start+count) ) 
    ++count;
  return count;
}


int cString::CountWordsize(int start) const {
  assert_error( start >= 0, "Negative Index into String");
  if( start>=GetSize() )  return 0;
  int count = 0;
  while( start+count<GetSize() && !IsWhitespace(start+count) )
    ++count;
  return count;
}


int cString::CountLinesize(int start) const {
  assert_error( start >= 0, "Negative Index into String");
  if( start>=GetSize() )  return 0;
  int count = 0;
  while( start+count<GetSize() && (*this)[start+count]!='\n' )
    count++;
  return count;
}


int cString::CountNumLines() const {
  int num_lines = 1;
  for( int i=0; i<GetSize(); ++i ){
    if( (*this)[i] == '\n' )  num_lines++;
  }
  return num_lines;
}


int cString::CountNumWords() const {
  int num_words = 0;
  int pos = CountWhitespace();     // Skip initial whitespace.
  while( pos<GetSize() ) {
    pos += CountWordsize(pos);
    pos += CountWhitespace(pos);
    num_words++;
  }
  return num_words;
}


// -- Search --
int cString::Find(char in_char, int start) const {
  int pos = start;
  assert_warning( pos>=0, "Negative Position: setting to 0");
  assert_warning( pos<=GetSize(), "Position Past End of String: setting to end of string");
  if( pos <= 0 ){ 
    pos = 0;
  }else{
    if( pos > GetSize() ){  
      pos = GetSize();  
    }
  }

  while( (*this)[pos] != '\0') {
    if( (*this)[pos] == in_char) return pos; // Found!
    pos++;
  }
  return -1; // Not Found
}


int cString::FindWord(const cString & in, int pos) const {
  int rv = -1;
  assert_warning( pos>=0, "Negative Position: setting to 0");
  assert_warning( pos<=GetSize(), "Position Past End of String: setting to end of string");
  if( pos <= 0 ){ 
    pos = 0;
  }else{
    if( pos > GetSize() ){  
      pos = GetSize();  
    }
  }
  // While there is enough space to find
  while( pos!=-1 && pos+in.GetSize() < GetSize() ){ 
    cerr<<in<<" "<<pos<<endl;
    if( (pos=Find(in, pos)) >= 0 ){      // try to find it
      // if it's got whitespace on both sides, it's a word
      if( ( pos==0 || IsWhitespace(pos-1) ) 
	  && ( pos==GetSize()-1 || IsWhitespace(pos+in.GetSize()) ) ){
	rv = pos;  
	pos=GetSize(); // set so we don't keep looking
      }else{
	++pos; // go on and look further down
      }
    }
  }
  return rv;
}
	

cString cString::GetWord(int word_id) const {
  // Find positon of word
  int pos = 0;
  int cur_word = 0;
  while( pos<GetSize() && cur_word<word_id ) {  // If this isn't the word
    pos += CountWhitespace(pos);                 // Skip leading whitespace
    pos += CountWordsize(pos);                   // Skip this word
    cur_word++; 
  }
  // Return GetWordAt position... (it will skip any leading whitespace)
  return GetWordAt(pos);
}


cString cString::GetWordAt(int start) const {
  int pos = start + CountWhitespace(start);  // Skip past initial whitespace.
  int word_size = CountWordsize(pos);        // Get size of word
  cString new_string(word_size);             // Allocate new_string that size
  for (int i = 0; i < word_size; i++) {      // Copy the chars to new_string
    new_string[i] = (*this)[pos + i];
  }
  return new_string;
}


cString cString::Substring(int start, int size) const {
  assert_error( size>0, "Non-Positive Size" );
  assert_warning( start>=0, "Negative Position: setting to 0");
  assert_warning( start+size<=GetSize(),
    "Position+Size Past End of String: setting size get to end of string");
  if (start <= 0){ 
    start = 0;
  } else {
    if( start+size > GetSize() ) { 
      size = GetSize()-start;  
    }
  }
  cString new_string(size);
  for (int i=0; i<size; i++) {
    new_string[i] = (*this)[i+start];
  }
  return new_string;
}

Boolean cString::IsSubstring(const cString & in_string, int start) const {
  assert_warning( start >= 0, "Negative Start Position: setting to 0");
#ifdef DEBUG
  if (start <= 0) start = 0;
#endif

  // If the potential sub-string won't fit, return FALSE;
  if ( start + in_string.GetSize() > GetSize() ) return FALSE;

  // Otherwise, check character by character.
  for (int i = 0; i < in_string.GetSize(); i++) {
    if ( (*this)[i+start] != in_string[i] ) return FALSE;
  }

  return TRUE;
}


// -- Modifiers --

cString & cString::Set(const char * fmt, ...) {
  va_list argp;
  char buf[MAX_LENGTH];
  va_start(argp, fmt);
  vsprintf(buf, fmt, argp);
  va_end(argp);
  (*this) = buf;
  return *this;
}


int cString::Replace(const cString & old_st, const cString & new_st, int pos){
  int location;
  if( (location=Find(old_st, pos)) >= 0 ){ // If we find the old_st
    Insert(new_st, location, old_st.GetSize());
  }
  return location;
}


int cString::LeftJustify(){
  int ws_count = CountWhitespace();
  if( ws_count>0 ) 
    InsertStr(0, NULL, 0, ws_count);
  return ws_count;
}


int cString::RightJustify(){
  int ws_count = 0;
  while( GetSize()-ws_count-1>0 && IsWhitespace(GetSize()-ws_count-1) ) 
    ws_count++;
  if( ws_count>0 ) 
    InsertStr(0, NULL, GetSize()-ws_count, ws_count);
  return ws_count;
}


cString cString::Pop(const char delim){
  int pos;
  cString rv("");
  if( (pos=Find(delim)) >= 0 ){ // if it is found at all
    if( pos > 0 ){  // if the first char is not delim, return substring
      rv = Substring(0,pos);
    }
    // Trim off the front 
    InsertStr(0, NULL, 0, pos+1);
  }

  // If the deliminator is *not* found, return the whole string.
  else {
    rv = *this;
    *this = "";
  }
  return rv;
}


cString cString::PopWord(){
  int start_pos = CountWhitespace();
  int word_size = CountWordsize(start_pos);
  cString rv("");
  if( word_size > 0 ){ // if it is found at all
    rv = Substring(start_pos,word_size);
  }
  // Trim off the front 
  InsertStr(0,NULL,0,start_pos+word_size+CountWhitespace(start_pos+word_size));
  return rv;
}


void cString::ToLower()
{
  for (int pos = 0; (*this)[pos] != '\0'; pos++) {
    if( (*this)[pos] >= 'A' && (*this)[pos] <= 'Z' )
      (*this)[pos] += 'a' - 'A';
  }
}


void cString::ToUpper()
{
  for (int pos = 0; (*this)[pos] != '\0'; pos++) {
    if( (*this)[pos] >= 'a' && (*this)[pos] <= 'z' )
      (*this)[pos] += 'A' - 'a';
  }
}


void cString::Reverse()
{
  cString new_st(GetSize());
  for( int i=0; i<GetSize(); ++i ){
    // new_st[i] = (*this)[GetSize()-i-1];       // @CAO Problem in new gcc
    new_st[i] = GetData()[GetSize()-i-1];
  }
  (*this) = new_st;
}


void cString::CompressWhitespace(){
  // Eats initial whitespace
  // 2 pass method...
  int i;
  int pos = 0;
  int start_pos = CountWhitespace();
  int new_size = 0;
  Boolean ws = FALSE;

  // count the number of characters that we will need in the new string
  for( i=start_pos; i<GetSize(); ++i ){
    if( IsWhitespace(i) ){ // if it whitespace...
      if( ws == FALSE ){     // if we arn't already in a whitespace block
	ws = TRUE;             // we are now in a whitespace block
      }
    }else{                 // it isn't whitespace, so count
      if( ws==TRUE ){        // if there was a whitespace block
	++new_size;            // inc once for the block
	ws = FALSE; 
      }
      ++new_size;
    }
  }
  
  cString new_st(new_size);  // Allocate new string
  
  // Copy over the characters
  // pos will be the location in new_st, while i is the index into this
  ws = FALSE;
  for( i=start_pos; i<GetSize(); ++i ){
    if( IsWhitespace(i) ){ // if it whitespace...
      if( ws == FALSE ){     // if we arn't already in a whitespace block
	ws = TRUE;             // we are now in a whitespace block
      }
    }else{                 // it isn't whitespace, so count
      if( ws==TRUE ){        // if there was a whitespace block
	new_st[pos] = ' ';     // put a space in for the whitespace block
	++pos;                 // inc once for the block
	ws = FALSE; 
      }
      // new_st[pos] = (*this)[i]; // copy it & increment pos  @CAO prob in gcc
      new_st[pos] = GetData()[i]; // copy it & increment pos
      ++pos;
    }
  }

  (*this) = new_st;  // assign the new_st to this
}


void cString::RemoveWhitespace(){
  int i;
  int pos = 0;
  int new_size = 0;
  for( i=0; i<GetSize(); ++i ){  // count new size
    if( !IsWhitespace(i) )  ++new_size;
  }
  cString new_st(new_size);      // allocate new string 
  for( i=0; i<GetSize(); ++i ){  // count new size
    if( !IsWhitespace(i) ){
      new_st[pos] = GetData()[i]; // copy it & increment pos   @CAO prob in GCC
      ++pos;
    }
  }
  (*this) = new_st;  // assign the new_st to this
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
    if ((*this)[i + offset] != in_string[i]) num_diffs++;
  }

  return num_diffs;
}

#include <stdio.h>

// Levenschtein Distance
int cString::LDistance(const cString & in_string) const
{
  const int size1 = GetSize();
  const int size2 = in_string.GetSize();

  if (!size1) return size2;
  if (!size2) return size1;

  int * cur_row  = new int[size1];  // The row we are calculating
  int * prev_row = new int[size1];  // The last row we calculater
  int * temp_row = NULL;            // For swapping only...

  // Initialize rows to diff from nothing.
  int i, j;
  for (i = 0; i < size1; i++) {
    cur_row[i] = i + 1;
    prev_row[i] = i + 1;
  }

  for (i = 0; i < size2; i++) {
    // Initialize the first entry in the cur_row.
    if ((*this)[0] == in_string[i]) {
      cur_row[0] = i;
    } else {
      cur_row[0] = (i < prev_row[0]) ? i : prev_row[0] + 1;
    }

    // Move down the cur_row and fill it out.
    for (j = 1; j < size1; j++) {
      // If the values are equal, keep the value in the upper left.
      if ((*this)[j] == in_string[i]) {
	cur_row[j] = prev_row[j-1];
      }

      // Otherwise, set the current position the the minimal of the three
      // numbers to the upper right in the chart plus one.
      else {
	cur_row[j] =
	  (prev_row[j] < prev_row[j-1]) ? prev_row[j] : prev_row[j-1];
	if (cur_row[j-1] < cur_row[j]) cur_row[j] = cur_row[j-1];
	cur_row[j]++;
      }
    }

    // Swap cur_row and prev_row. (we only really need to move the cur row
    // over to prev, but this saves us from having to keep re-allocating
    // new rows.  We recycle!
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



// -- Internal Methods --

cString & cString::AppendStr(const int in_size, const char * in){ 
  // Optimized over InsertStr... though InsertStr will do same thing
  unsigned short i;
  // Allocate a new string
  assert_error( in_size==0 || in!=NULL, "NULL input string" );
  cStringData * new_value = new cStringData(GetSize()+in_size);
  assert_error(new_value!=NULL,"Memory Allocation Error: Out of Memory");
  for( i=0; i<GetSize(); ++i ){                   // Copy self up to pos
    (*new_value)[i] = this->operator[](i);
  }
  for( i=0; i<in_size; ++i ){               // Copy in 
    assert_warning( in[i]!='\0', "Input String Contains '\\0' or too Short");
    (*new_value)[i+GetSize()] = in[i];
  }
  TakeValue(new_value);                     // Reassing data to new data
  return(*this);
}


cString & cString::InsertStr(const int in_size, const char * in, 
			     int pos, int excise ){
  // Inserts 'in' (of length 'in_size') at postition 'pos'
  // Also excises 'excise' characters from 'pos'
  // If 'in_size'==0 then 'in' can == NULL and only excise happens

  unsigned short i;

  // Validate inputs:
  assert_error( in_size>=0, "Negative Input Size");
  // pos<0, pos=0;  if pos>GetSize, pos=GetSize
  assert_warning( pos>=0, "Negative Position: setting to 0");
  assert_warning( pos<=GetSize(), "Position Past End of String: setting to end of string & excise to 0");
  if( pos < 0 ){ 
    pos = 0;
  }else{
    if( pos > GetSize() ){  
      pos = GetSize();  
    }
  }
  // if excise<0, excise=0;  excise>GetSize, excise=GetSize
  assert_warning( excise>=0, "Negative Excise: setting to 0");
  assert_warning( excise<=GetSize()-pos, "Excise Number too Large: setting to full string");
  if( excise < 0 ){ 
    excise = 0;
  }else{
    if( excise > GetSize()-pos )  
      excise = GetSize()-pos;  
  }

  // Allocate a new string
  assert_error( in_size==0 || in!=NULL, "NULL input string" );
  cStringData * new_value = new cStringData(GetSize()+in_size-excise);
  assert_error(new_value!=NULL,"Memory Allocation Error: Out of Memory");

  for( i=0; i<pos; ++i ){                   // Copy self up to pos
    (*new_value)[i] = this->operator[](i);
  }
  for( i=0; i<in_size; ++i ){               // Copy in 
    assert_warning( in[i]!='\0', "Input String Contains '\\0' or too Short");
    (*new_value)[i+pos] = in[i];
  }
  for( i=pos+excise; i<GetSize(); ++i ){   // Copy rest of self 
    (*new_value)[i+in_size-excise] = this->operator[](i);
  }

  TakeValue(new_value);                     // Reassing data to new data
  return(*this);
}
  

int cString::FindStr(const char * in, const int in_size, int pos) const {
  assert_warning( pos>=0, "Negative Position: setting to 0");
  assert_warning( pos<=GetSize(), "Position Past End of String: setting to end of string");
  if( pos <= 0 ){ 
    pos = 0;
  }else{
    if( pos > GetSize() ){  
      pos = GetSize();  
    }
  }
 
  while( (*this)[pos] != '\0' ){
    if( GetSize()-pos < in_size ) return -1; // Too near this string's end.
    if( (*this)[pos] == in[0] ){
      // see if we have found the string...
      int i;
      for( i = 1; i < in_size; i++ ){
	assert_error( (*this)[pos+i]!='\0', "Reached end of (*this) in Find");
	assert_error( in[i]!='\0', "Reached end of 'in' in Find");
	if( (*this)[pos + i] != in[i] ) break; // Match failure!
      }
      // If we have made it fully through the loop, we have found a match!
      if( i == in_size ) return pos;
    }
    pos++;
  }
  return -1;
}



// {{{ ** External cString Functions **

istream & operator >> (istream & in, cString & string){
  char buf[cString::MAX_LENGTH]; 
  in>>buf;
  string=buf;
  return in;
}

// }}}



// NOT UPDATE... --@TCC


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
  
int cStringBlock::WordGrep(cString in_string) const
{
  for (int i = 0; i < num_lines; i++) {
    if (block[i].FindWord(in_string) >= 0) return i;
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
