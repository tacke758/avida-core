extern "C" {
#include <stdlib.h>
#include <errno.h>   // needed for FOPEN error constants (MSVC)
}


#include "tools.hh"  // for g_debug
#include "file.hh"

////////////
//  cFile
////////////



int cFile::Open(cString _filename, int flags) {
  if( IsOpen() ) Close();
  fp.open(_filename(), flags);

  int errno;
  if( fp.fail() ){
    _filename.Insert(default_dir());
    fp.clear();
    fp.open(_filename(), flags);
  }

  // check for errors
  if ( (errno=fp.fail()) ){
    char * pszError;
    // error opening the file
    switch (errno) {
    case EACCES:  // Access Denied
      pszError = "Access denied";
      break;
    case EINVAL:  // Invalid open flag or access mode
      pszError = "Invalid open flag or access mode";
      break;
    case ENOENT:  // File or path not found
      pszError = "File or path not found";
      break;
    default:      // unknown error
      pszError = "?? Unknown Error??";
      break;
    }
    g_debug.Error("Unable to open file '%s': %s\n", _filename(), pszError);
// cerr << "Unable to open file '" << _filename << "' : " << pszError << endl;
// exit(8);
  }else{
    filename = _filename;
  }
  
  return( fp.good() && !fp.fail() );
}

int cFile::Close()
{
  if( filename == "" ){
    fp.close();
    return TRUE;
  }
  return FALSE;
}

int cFile::ReadLine(cString & in_string)
{
  char cur_line[1024];
  cur_line[0]='\0';
  fp.getline(cur_line, 1024);
  if( fp.bad() ){
    return FALSE;
  }
  in_string = cur_line;
  return TRUE;
}

/*
int cFile::CountLines()
{
  char cur_line[1024];

  Rewind();
  int n_lines = -1;

  do{ 
    fp.getline(cur_line, 1024);
    n_lines++;
  } while( cur_line[0]!='\0' && !fp.eof() );

  return n_lines;
}
*/



////////////////
//  cInitFile
////////////////

cInitFile::cInitFile(cString in_filename) : cFile(in_filename)
{
  num_lines = 0;
  line_list = NULL;
}

cInitFile::~cInitFile()
{
  cStringSLLE * next_element;

  while (line_list) {
    next_element = line_list->GetNext();
    delete line_list;
    line_list = next_element;
  }
}

void cInitFile::Load()
{
  if (!IsOpen()) return;

  cStringSLLE * cur_line = NULL;
  cString buf;

  ReadLine(buf);

  // If this file doesn't work properly, set the line list to NULL.
  if( Eof() && !buf.GetSize() ){
    line_list = NULL;
    return;
  }

  line_list = new cStringSLLE;
  cur_line = line_list;
  cur_line->String() = buf;
  num_lines = 1;

  ReadLine(buf);
  while( !Eof() || buf.GetSize() ){
    num_lines++;
    cur_line->SetNext(new cStringSLLE);
    cur_line = cur_line->GetNext();
    cur_line->String() = buf;

    // Read in the next line...
    ReadLine(buf);
  }
}

/*
void cInitFile::Save(char * in_filename)
{
  cString save_filename(GetFilename());
  if (in_filename != NULL) save_filename = in_filename;

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

void cInitFile::Compress()
{
  cStringSLLE * cur_line = line_list;
  cStringSLLE * last_line = NULL;

  int comment_pos;
  while (cur_line != NULL) {
    // Remove all characters past a comment mark and reduce whitespace.
    comment_pos = cur_line->String().Find('#');
    if (comment_pos >= 0) cur_line->String().Clip(comment_pos);
    cur_line->String().CompressWhitespace();

    // If this line is now empty, remove it.
    if (cur_line->String().IsEmpty()) {
      if (last_line) {
	last_line->SetNext(cur_line->GetNext());
	delete cur_line;
	cur_line = last_line->GetNext();
      } else {
	line_list = cur_line->GetNext();
	delete cur_line;
	cur_line = line_list;
      }
      num_lines--;
    }
    // If this line did have something on it, just move onto the next one.
    else {
      last_line = cur_line;
      cur_line = cur_line->GetNext();
    }
  }
  //PrintLines();
  //cerr<<GetNumLines()<<endl;
} 

void cInitFile::AddLine(cString & in_string)
{
  cStringSLLE * new_string = new cStringSLLE;
  new_string->String() = in_string;
  new_string->SetNext(line_list);
  line_list = new_string;
}

cString cInitFile::GetLine(int line_num)
{
  if (line_num < 0 || line_num >= num_lines) return "";

  cStringSLLE * cur_line = line_list;
  for (int i = 0; i < line_num; i++) cur_line = cur_line->GetNext();

  return cur_line->String();
}

cString cInitFile::RemoveLine()
{
  if( line_list==NULL ){
    g_debug.Comment("cInitFile::RemoveLine() when line_list==NULL\n");
    return cString("");
  }

  cStringSLLE * cur_element = line_list;
  line_list = line_list->GetNext();

  cString ret_string(cur_element->String());
  delete cur_element;
  num_lines--;

  return ret_string;
}

void cInitFile::PrintLines()
{
  cout<<"Printing Init File '"<<GetFilename()<<"'"<<endl;

  cStringSLLE * cur_element = line_list;  
  int cur_line = 0;
  
  while (cur_element) {
    cout<<cur_element<<": "<<cur_element->String()<<endl;
    cur_element = cur_element->GetNext();
    cur_line++;
  }
}

int cInitFile::Find(cString & in_string, cString keyword, int col) const
{
  cStringSLLE * cur_element = line_list;  

  while (cur_element) {
    if (cur_element->String().GetWord(col) == keyword) {
      in_string = cur_element->String();
      return TRUE;
    }
    cur_element = cur_element->GetNext();
  }

  // Not found...

  return FALSE;
}


/////////////////
//  cGenesis
/////////////////

cGenesis::cGenesis(const char * in_filename) : cInitFile(in_filename)
{
  if( !IsOpen() ){
    cerr<<"Failed to open '"<<in_filename<<"' file!!!\n"<<endl;
    exit(8);
  }

  Load();
  Compress();
  Close();
}

void cGenesis::AddInput(char * in_name, int in_value)
{
  cString in_string;
  in_string.Set("%s %d", in_name, in_value);
  AddLine(in_string);
}

void cGenesis::AddInput(char * in_name, char * in_value)
{
  cString in_string;
  in_string.Set("%s %s", in_name, in_value);
  AddLine(in_string);
}

int cGenesis::ReadInt(char * name, int base) const
{
  if( name == NULL ) return base; 
  cString cur_line;

  if (!Find(cur_line, name, 0)) {
    return base;
  }

  return cur_line.GetWord(1).AsInt();
}

double cGenesis::ReadFloat(char * name, float base) const
{
  if( name == NULL ) return base;
  cString cur_line;

  if (!Find(cur_line, name, 0)) {
    return base;
  }

  return cur_line.GetWord(1).AsDouble();
}

cString cGenesis::ReadString(char * name, cString base) const
{
  if( name == NULL ) return 0;
  cString cur_line;

  if (!Find(cur_line, name, 0)) {
    return base;
  }

  return cur_line.GetWord(1);
}

cString cGenesis::ReadString(char * name) const
{
  if( name == NULL ) return 0;
  cString cur_line;

  if (!Find(cur_line, name, 0)) {
    return "";
  }

  return cur_line.GetWord(1);
}


/////////////////
//  cEventList
/////////////////

cEventList::cEventList(char * in_filename) : cInitFile(in_filename) {
  event_list = NULL;
  num_events = 0;

  // Load in the proper event list and set it up.
  Load();
  Compress();

  // Build the event_list.
  BuildEventList();
}

void cEventList::Next()
{
  if (event_list) {
    cEvent * old_event = event_list;
    event_list = event_list->GetNext();
    delete old_event;
  }

  num_events--;
}

void cEventList::AddEvent(int update, const cString & name,
			  const cString & arg_list)
{
  // Build the new event.
  cEvent * new_event = new cEvent(update, name, arg_list);
  num_events++;

  // If the event goes at the top of the list, put it there.
  if (!event_list || update < event_list->GetUpdate()) {
    new_event->SetNext(event_list);
    event_list = new_event;
    return;
  }

  // Otherwise, find the new position for this event.
  cEvent * cur_event = event_list;
  while (cur_event->GetNext() && cur_event->GetNext()->GetUpdate() <= update)
    cur_event = cur_event->GetNext();

  // And, finally, place this event in the list.
  new_event->SetNext(cur_event->GetNext());
  cur_event->SetNext(new_event);
}

void cEventList::BuildEventList()
{
  // Loop through the line_list and change the lines to events.

  while ( GetNumLines()>0 ) {
    cString cur_line = RemoveLine();
    int cur_update = cur_line.PopWord().AsInt();
    cString cur_event = cur_line.PopWord();

    AddEvent(cur_update, cur_event, cur_line);
  }
}
