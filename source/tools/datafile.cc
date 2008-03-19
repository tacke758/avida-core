//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
/* datafile.cc ****************************************************************
 cDataFile
 cDataFileManager
******************************************************************************/

#include "datafile.hh"
#include "string_util.hh"

#include <stdio.h>
#include <time.h> // for writing time stamps


using namespace std;


void cDataFile::Init()
{
  m_descr_written = false;
  num_cols = 0;
}

cDataFile::cDataFile()
{
  Init();
}

cDataFile::cDataFile(cString name) : m_name(name)
{
  Init();
   
  if( fopen(name,"r") ){
    // cout << "File " << name() << " exists and is being overwritten" << endl;
  }
  m_fp.open(name);
  assert(m_fp.good());
  m_descr_written = false;
}


void cDataFile::Write( double x, const char * descr )
{
  if ( !m_descr_written ){
    m_data += cStringUtil::Stringf( "%f ", x );
    WriteColumnDesc(descr);
  }
  else m_fp << x << " ";
}


void cDataFile::Write( int i, const char * descr )
{
  if ( !m_descr_written ){
    m_data += cStringUtil::Stringf( "%i ", i );
    WriteColumnDesc(descr);
  }
  else m_fp << i << " ";
}


void cDataFile::Write( const char * data_str, const char * descr )
{
  if ( !m_descr_written ) {
    m_data += cStringUtil::Stringf( "%s ", data_str );
    WriteColumnDesc(descr);
  }
  else m_fp << data_str << " ";
}


void cDataFile::WriteBlockElement(double x, int element, int x_size)
{
  m_fp << x << " ";
  if (((element + 1) % x_size) == 0) {
    m_fp << "\n";
  }
}

void cDataFile::WriteColumnDesc( const char * descr )
{
  if ( !m_descr_written ){
    num_cols++;
    m_descr += cStringUtil::Stringf( "# %2d: %s\n", num_cols, descr );
  }
}

void cDataFile::WriteComment( const char * descr )
{
  if ( !m_descr_written ) m_descr += cStringUtil::Stringf( "# %s\n", descr );
}


void cDataFile::WriteRawComment( const char * descr )
{
  if ( !m_descr_written ) m_descr += cStringUtil::Stringf( "%s\n", descr );
}

void cDataFile::WriteRaw( const char * descr )
{
  m_fp << cStringUtil::Stringf( "%s\n", descr );
}




void cDataFile::WriteTimeStamp()
{
  if ( !m_descr_written ){
    time_t time_p = time( 0 );
    m_descr += cStringUtil::Stringf( "# %s", ctime( &time_p ) );
  }
}

void cDataFile::FlushComments()
{
  if ( !m_descr_written ){
    m_fp << m_descr;
    m_descr_written = true;
  }
}


void cDataFile::Endl()
{
  if ( !m_descr_written ){
    m_fp << m_descr << endl;
    m_fp << m_data << endl;
    m_descr_written = true;
  }
  else m_fp << endl;
}



//////////////////////
//  cDataFileManager
//////////////////////


cDataFileManager::~cDataFileManager()
{
  while (data_file_list.GetSize()) {
    delete data_file_list.Pop();
  }
}


cDataFile * cDataFileManager::InternalFind(const cString & name)
{
  tListIterator<cDataFile> list_it(data_file_list);
  while (list_it.Next() != NULL) {
    if (list_it.Get()->GetName() == name) return list_it.Get();
  }
  return NULL;
}


cDataFile & cDataFileManager::Get(const cString & name)
{
  // Find the file by this name...
  cDataFile * found_file = InternalFind(name);

  // If it hasn't been found, create it...
  if (found_file == NULL) {
    found_file = new cDataFile(name);
    data_file_list.Push(found_file);
  }

  // Make sure we got the name right...
  assert( found_file->GetName() == name );

  // and return it.
  return *found_file;
}


ofstream & cDataFileManager::GetOFStream(const cString & name)
{
  // Find the file by this name...
  cDataFile * found_file = InternalFind(name);

  // If it hasn't been found, create it...
  if (found_file == NULL) {
    found_file = new cDataFile(name);
    data_file_list.Push(found_file);
  }

  // Make sure we got the name right...
  assert( found_file->GetName() == name );

  // And return the releven stream...
  return found_file->GetOFStream();
}


void cDataFileManager::FlushAll()
{
  tListIterator<cDataFile> list_it(data_file_list);
  while (list_it.Next() != NULL) list_it.Get()->Flush();
}


bool cDataFileManager::Remove(const cString & name)
{
  cDataFile * found_file = InternalFind(name);
  if (found_file == NULL) return false;

  delete found_file;
  return true;
}

