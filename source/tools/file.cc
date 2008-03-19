//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
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

cInitFile::cInitFile() : filetype("unknown")
{
}

cInitFile::cInitFile(cString in_filename)
  : cFile(in_filename)
  , filetype("unknown")
{
}

cInitFile::~cInitFile()
{
}

void cInitFile::Load()
{
  if (!IsOpen()) return;   // The file must be opened!
  line_list.Clear();       // Empty the line list before we start.

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
}

void cInitFile::LoadStream(istream & in_stream)
{
  if (in_stream.good() == false) {
    cerr << "Bad stream sent to cInitFile::LoadStream()" << endl;
    return;
  }

  line_list.Clear();       // Empty the line list before we start.

  char cur_line[MAX_STRING_LENGTH];
  in_stream.getline(cur_line, MAX_STRING_LENGTH);

  // If this file doesn't work properly, return.
  if( !in_stream && !strlen(cur_line) )  return;

  in_stream.getline(cur_line, MAX_STRING_LENGTH);
  while ( in_stream ) {
    line_list.PushRear(cur_line);
    in_stream.getline(cur_line, MAX_STRING_LENGTH);
  }
}

/*
void cInitFile::Save(const cString & in_filename)
{
  cString save_filename(GetFilename());
  if (in_filename != "") save_filename = in_filename;

  FILE * fp_save = fopen(save_filename(), "w");

  // Go through the lines saving them...
  for (cStringSLLE * cur_line = line_list;
       cur_line != NULL;
       cur_line = cur_line->GetNext()) {
    fprintf(fp_save, "%s\n", cur_line->String()());
  }

  fclose(fp_save);
}
*/


void cInitFile::ReadHeader()
{
  // If we don't have at least two lines loaded in, then we definately don't
  // have the proper header information.
  if (line_list.GetSize() < 2) return;

  cString type_line = GetLine(0);
  cString format_line = GetLine(1);

  if (type_line.PopWord() == "#filetype") filetype = type_line.PopWord();
  if (format_line.PopWord() == "#format") file_format.Load(format_line);
}


void cInitFile::Compress()
{
  cStringList comp_list; // Put all of the compressed strings into a new list.
  cString long_line = " ";
  
  bool long_line_flag = false;

  while (line_list.GetSize() > 0) {
    cString cur_line = line_list.Pop();
    
    // Remove all characters past a comment mark and reduce whitespace.
    int comment_pos = cur_line.Find('#');
    if (comment_pos >= 0) cur_line.Clip(comment_pos);
    cur_line.CompressWhitespace();

    // If the line has a continuation mark at the end put into long_line string

    if (cur_line.IsContinueLine()) {
      long_line_flag = true;
      long_line += cur_line;

    // Else if the line is one that immediately follows a line with a 
    // continuation mark 

    } else if (long_line_flag){
      long_line_flag = false;
      long_line += cur_line;
      long_line.CompressWhitespace();
      cur_line = long_line;
      long_line = " ";
    }

    // If this line is not empty. and not in the middle of a continuation 
    // line, keep it.

    if ( (cur_line.IsEmpty() == false) && !long_line_flag ) {
      comp_list.PushRear(cur_line);
    }
  }

  if (long_line_flag) {
    cerr << "Last line of \"" << GetFilename() << "\" ends with a \"" <<
      CONTINUE_LINE_CHAR << "\" and entire line was ignored." << endl;
  }
 
  // Move the compressed lines back over to our main line list.
  while (comp_list.GetSize() > 0) {
    line_list.Push( comp_list.PopRear() );
  }
}

void cInitFile::AddLine(cString & in_string)
{
  line_list.Push(in_string);
}

cString cInitFile::GetLine(int line_num)
{
  if (line_num < 0 || line_num >= line_list.GetSize()) return "";
  return line_list.GetLine(line_num);
}

cString cInitFile::RemoveLine()
{
  assert(line_list.GetSize() > 0);

  return line_list.Pop();
}


bool cInitFile::Find(cString & in_string, const cString & keyword,
		     int col) const
{
  // Loop through all of the lines looking for this keyword.
  cStringIterator list_it(line_list);
  while ( list_it.AtEnd() == false ) {
    list_it.Next();
    cString cur_string = list_it.Get();
    if (cur_string.GetWord(col) == keyword) {
      in_string = cur_string;
      return true;
    }
  }

  return false;    // Not Found...
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



