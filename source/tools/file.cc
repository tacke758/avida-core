//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

extern "C" {
#include <stdlib.h>
#include <errno.h>   // needed for FOPEN error constants (MSVC)
}

#include "tools.hh"  // for g_debug global
#include "file.hh"


using namespace std;


////////////
//  cFile
////////////



//bool cFile::Open(cString _fname, int flags)
// porting to gcc 3.1 -- k
bool cFile::Open(cString _fname, ios::openmode flags)
{
  if( IsOpen() ) Close();    // If a file is already open, clost it first.
  fp.open(_fname(), flags);  // Open the new file.

  // Test if there was an error, and if so, try again!
  int err_id = fp.fail();
  if( err_id ){
    fp.clear();
    fp.open(_fname(), flags);
  }

  // If there is still an error, determine its type and report it.
  err_id = fp.fail();
  if (err_id){
    cString error_desc = "?? Unknown Error??";

    // See if we can determine a more exact error type.
    if (err_id == EACCES) error_desc = "Access denied";
    else if (err_id == EINVAL) error_desc = "Invalid open flag or access mode";
    else if (err_id == ENOENT) error_desc = "File or path not found";

    // Print the error.
    cerr << "Unable to open file '" << _fname
	 << "' : " << error_desc << endl;
    return false;
  }

  filename = _fname;
  is_open = true;

  // Return true only if there were no problems...
  return( fp.good() && !fp.fail() );
}

bool cFile::Close()
{
  if (is_open == true) {
    fp.close();
    return true;
  }
  return false;
}

bool cFile::ReadLine(cString & in_string)
{
  char cur_line[MAX_STRING_LENGTH];
  cur_line[0]='\0';
  fp.getline(cur_line, MAX_STRING_LENGTH);
  if( fp.bad() ){
    return false;
  }
  in_string = cur_line;
  return true;
}

/*
int cFile::CountLines()
{
  char cur_line[MAX_STRING_LENGTH];

  Rewind();
  int n_lines = -1;

  do{
    fp.getline(cur_line, MAX_STRING_LENGTH);
    n_lines++;
  } while( cur_line[0]!='\0' && !fp.eof() );

  return n_lines;
}
*/



////////////////
//  cInitFile
////////////////

cInitFile::cInitFile() : filetype("unknown"), active_line(0)
{
}

cInitFile::cInitFile(cString in_filename)
  : cFile(in_filename)
  , filetype("unknown")
  , active_line(0)
{
}

cInitFile::~cInitFile()
{
}

void cInitFile::Load()
{
  if (!IsOpen()) return;   // The file must be opened!
  cStringList line_list;   // Create a list to load all of the lines into.

  cString buf;
  ReadLine(buf);

  // If this file doesn't work properly, return!
  if ( Eof() && !buf.GetSize() ) return;

  line_list.PushRear(buf);

  ReadLine(buf);
  while( !Eof() || buf.GetSize() ){
    line_list.PushRear(buf);
    ReadLine(buf);
  }

  // Copy all of the lines into the line array.
  const int file_size = line_list.GetSize();
  line_array.Resize(file_size);

  for (int i = 0; i < file_size; i++) {
    line_array[i].line = line_list.Pop();
    line_array[i].line_num = i;
    line_array[i].used = false;
  }
}

void cInitFile::LoadStream(istream & in_stream)
{
  if (in_stream.good() == false) {
    cerr << "Bad stream sent to cInitFile::LoadStream()" << endl;
    return;
  }

  cStringList line_list;   // Create a list to load all of the lines into.

  char cur_line[MAX_STRING_LENGTH];
  in_stream.getline(cur_line, MAX_STRING_LENGTH);

  // If this file doesn't work properly, return.
  if( !in_stream && !strlen(cur_line) )  return;

  in_stream.getline(cur_line, MAX_STRING_LENGTH);
  while ( in_stream ) {
    line_list.PushRear(cur_line);
    in_stream.getline(cur_line, MAX_STRING_LENGTH);
  }

  // Copy all of the lines into the line array.
  const int file_size = line_list.GetSize();
  line_array.Resize(file_size);

  for (int i = 0; i < file_size; i++) {
    line_array[i].line = line_list.Pop();
    line_array[i].line_num = i;
    line_array[i].used = false;
  }
}


void cInitFile::Save(const cString & in_filename)
{
  cString save_filename(GetFilename());
  if (in_filename != "") save_filename = in_filename;
  
  ofstream fp_save(save_filename());

  // Go through the lines saving them...
  for (int i = 0; i < line_array.GetSize(); i++) {
    fp_save << line_array[i].line << endl;
  }

  fp_save.close();
}


void cInitFile::ReadHeader()
{
  cString type_line = GetLine(0);
  cString format_line = GetLine(1);

  if (type_line.PopWord() == "#filetype") filetype = type_line.PopWord();
  if (format_line.PopWord() == "#format") file_format.Load(format_line);
}


void cInitFile::Compress()
{
  // We're going to handle this compression in multiple passes to make it
  // clean and easy.

  const int num_lines = line_array.GetSize();

  // PASS 1: Remove all comments -- everything after a '#' sign -- and
  // compress all whitespace into a single space.
  for (int i = 0; i < num_lines; i++) {
    cString & cur_line = line_array[i].line;

    // Remove all characters past a comment mark and reduce whitespace.
    int comment_pos = cur_line.Find('#');
    if (comment_pos >= 0) cur_line.Clip(comment_pos);
    cur_line.CompressWhitespace();
  }

  // PASS 2: Merge each line ending with a continue marker '\' with the
  // next line.

  int prev_line_id = -1;
  bool continued = false;
  for (int i = 0; i < num_lines; i++) {
    // If the current line is a continuation, append it to the previous line.
    if (continued == true) {
      line_array[prev_line_id].line += line_array[i].line;
      line_array[i].line = "";
    }
    else prev_line_id = i;

    // See if the prev_line is continued, and if it is, take care of it.
    cString & prev_line = line_array[prev_line_id].line;
    if (prev_line.GetSize() > 0 &&
	prev_line[prev_line.GetSize() - 1] == '\\') {
      prev_line.ClipEnd(1);  // Remove continuation mark.
      continued = true;
    }
    else continued = false;
  }

  // PASS 3: Remove now-empty lines.

  int next_id = 0;
  for (int i = 0; i < num_lines; i++) {
    // If we should keep this line, compact it.
    if (line_array[i].line.GetSize() > 0) {
      if (next_id != i) line_array[next_id] = line_array[i];
      next_id++;
    }
  }

  // Clip any extra lines at the end of the array.

  line_array.Resize(next_id);

  // Move the active line back to the beginning to avoid confusion.
  active_line = 0;
}


void cInitFile::AddLine(cString & in_string)
{
  extra_lines.Push(in_string);
}

cString cInitFile::GetLine(int line_num)
{
  if (line_num < 0 || line_num >= line_array.GetSize()) return "";
  return line_array[line_num].line;
}


bool cInitFile::Find(cString & in_string, const cString & keyword,
		     int col) const
{
  bool found = false;

  // Loop through all of the lines looking for this keyword.  Start with
  // the actual file...
  for (int line_id = 0; line_id < line_array.GetSize(); line_id++) {
    cString cur_string = line_array[line_id].line;

    // If we found the keyword, return it and stop.    
    if (cur_string.GetWord(col) == keyword) {
      line_array[line_id].used = true;
      in_string = cur_string;
      found = true;
    }
  }

  // Next, look through any extra lines appended to the file.
  cStringIterator list_it(extra_lines);
  while ( list_it.AtEnd() == false ) {
    list_it.Next();
    cString cur_string = list_it.Get();

    // If we found the keyword, return it and stop.
    if (cur_string.GetWord(col) == keyword) {
      in_string = cur_string;
      found = true;
    }
  }

  return found;    // Not Found...
}


cString cInitFile::ReadString(const cString & name, cString def) const
{
  // See if we definately can't find the keyword.
  if (name == "" || IsOpen() == false) return def;

  // Search for the keyword.
  cString cur_line;
  if (Find(cur_line, name, 0) == false) {
    if (verbose == true) {
      cerr << "Warning: " << name << " not in \"" << GetFilename()
	   << "\", defaulting to: " << def <<endl;
    }
    return def;
  }

  // Pop off the keyword, and return the remainder of the line.
  cur_line.PopWord();
  return cur_line;
}


bool cInitFile::WarnUnused() const
{
  bool found = false;

  for (int i = 0; i < line_array.GetSize(); i++) {
    if (line_array[i].used == false) {
      if (found == false) {
	found = true;
	cerr << "Warning unknown lines in input file '" << filename
	     << "':" << endl;
      }
      cerr << " " << line_array[i].line_num + 1
	   << ": " << line_array[i].line
	   << endl;
    }
  }

  return found;
}


/////////////////
//  cGenesis
/////////////////

cGenesis::cGenesis()
{
}

cGenesis::cGenesis(const cString & in_filename) : cInitFile(in_filename)
{
  if( !IsOpen() ){
    cerr<<"Failed to open '" << in_filename << "' file!!!\n" << endl;
  }

  Load();
  Compress();
  Close();
}

//int cGenesis::Open(cString _filename, int mode)
// porting to gcc 3.1 -- k
int cGenesis::Open(cString _filename, ios::openmode mode)
{
  if( IsOpen() ){
    cerr << "Trying to reopen '" << _filename << "' file!!!\n" << endl;
    return 0;
  }

  int base_ret = cFile::Open(_filename, mode);
  Load();
  Compress();
  Close();

  return base_ret;
}

void cGenesis::AddInput(const cString & in_name, int in_value)
{
  cString in_string;
  in_string.Set("%s %d", in_name(), in_value);
  AddLine(in_string);
}

void cGenesis::AddInput(const cString & in_name, const cString & in_value)
{
  cString in_string;
  in_string.Set("%s %s", in_name(), in_value());
  AddLine(in_string);
}

int cGenesis::ReadInt(const cString & name, int base, bool warn) const
{
  if (name == "" || IsOpen() == false) return base;
  cString cur_line;

  if (!Find(cur_line, name, 0)) { 
    if (verbose == true && warn == true) {
      cout << "Warning: " << name << " not in \"" << GetFilename()
	   << "\", defaulting to: " << base <<endl;
    }
    return base;
  }

  return cur_line.GetWord(1).AsInt();
}

double cGenesis::ReadFloat(const cString & name, float base, bool warn) const
{
  if (name == "" || IsOpen() == false) return base;
  cString cur_line;

  if (!Find(cur_line, name, 0)) {
    if (verbose == true && warn == true) {
      cout << "Warning: " << name << " not in \"" << GetFilename()
	   << "\", defaulting to: " << base <<endl;
    }
    return base;
  }

  return cur_line.GetWord(1).AsDouble();
}



