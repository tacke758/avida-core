//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
/* datafile.hh ****************************************************************
 cDataFile
 cDataFileManager

 charles@krl.caltech.edu & travc@ugcs.caltech.edu
 Time-stamp: <1999-01-07 10:02:09 travc>

 cString: basic string class
******************************************************************************/

#ifndef DATAFILE_HH
#define DATAFILE_HH

#include <assert.h>
//#include <iostream.h>
//#include <fstream.h>
// porting to gcc 3.1 -- k
#include <iostream>
#include <fstream>

#include "string.hh"
#include "tList.hh"

class cDataFileManager;

/**
 * This class encapsulates file handling for the class @ref cDataFileManager.
 * The files it creates are write only. Reading of files is handled by
 * @ref cFile.
 */

class cDataFile {

private:
  cString m_name;
  cString m_data;
  cString m_descr;
  int num_cols;
  
  bool m_descr_written;
  
  std::ofstream m_fp;

  void Init();
public:
  /**
   * The empty constructor does nothing.
   **/
  cDataFile();

  /**
   * This constructor opens a file of the given name, and makes sure
   * the file is usable.
   *
   * @param _name The name of the file to open.
   **/
  cDataFile(cString name);

  /**
   * The desctructor automatically closes the file.
   **/
  ~cDataFile(){ m_fp.close(); }

  /**
   * @return The file name used
   **/
  const cString & GetName() const { return m_name; }

  /**
   * @return A bool that indicates whether the file is actually usable.
   **/
  bool Good() const { return m_fp.good(); }

  /**
   * This function allows low-level write access to the file. Although
   * sometimes usefull, it provides the possibility to circumvent the
   * automatic documentation. Use with care.
   *
   * @return The output stream corresponding to the file.
   **/
  std::ofstream & GetOFStream() { return m_fp; }

  /**
   * Outputs a value into the data file.
   *
   * @param x The value to write (as double, int, or char *)
   *
   * @param descr A string that describes the meaning of the value. The string
   * will be written only once, before the first data line has been finished.
   **/

  void Write( double x,              const char * descr );
  void Write( int i,                 const char * descr );
  void Write( const char * data_str, const char * descr );
  void WriteBlockElement (double x, int element, int x_size );

  /**
   * Writes a descriptive string into a data file. The string is only
   * written if the first data line hasn't been completed (Endl() hasn't
   * been called. This allows to output initial comments into a file.
   **/
  void WriteComment( const char * descr );

  /**
   * Same as WriteComment, but doesn't automatically include the # in the
   * front of the line.  This should only be used in special circumstances
   * where something outside of a typical comment needs to be at the top.
   **/
  void WriteRawComment( const char * descr );

  /**
   * Writes text string any where in the data file. This should only be used 
   * in special circumstances where something outside of a typical comment 
   * needs to be placed in the file.
   **/
  void WriteRaw( const char * descr );

  /**
   * Writes the description for a single column; keeps track of column numbers.
   **/
  void WriteColumnDesc(const char * descr );

  /**
   * Writes the current time into the data file. The time string is only
   * written if the first data line hasn't been completed (Endl() hasn't
   * been called.
   **/
  void WriteTimeStamp();

  /**
   * This function writes the comments that have accumulated. There should
   * normally be no reason to call this function. Endl() does the same thing
   * in a safer way.
   **/
  void FlushComments();
  
  /**
   * Write all data to disk and start a new line.
   **/
  void Endl();

  /**
   * Has the header been written to the file yet?
   **/
  bool HeaderDone() { return m_descr_written; }
  
  /**
   * This function makes sure that all cached data is written to the disk.
   **/
  void Flush(){ m_fp.flush(); }
};


/**
 * This class helps to manage a collection of data files. It is possible
 * to add files, to remove files, and to access existing files by name.
 **/

class cDataFileManager {
private:
  tList<cDataFile> data_file_list;

  cDataFile * InternalFind(const cString & name);

public:
  cDataFileManager() { ; }
  ~cDataFileManager();

  /**
   * Looks up the @ref cDataFile corresponding to the given name. If that
   * file hasn't been created previously, it is created now.
   *
   * @return The @ref cDataFile.
   * @param name The name of the file to look up/create.
   **/
  cDataFile & Get(const cString & name);

  /**
   * Looks up the ofstream corresponding to the file of the given name.
   * If that file hasn't been created previously, it is created now.
   *
   * Read the cautionary remarks about the function with the same name in
   * @ref cDataFile.
   *
   * @return The ofstream.
   * @param name The name of the file to look up/create.
   **/
  std::ofstream & GetOFStream(const cString & name);

  void FlushAll();

  /** Removes the given file, thereby closing it.
   *
   * @return true if file existed, otherwise false.
   **/
  bool Remove(const cString & name);
};



#endif // DATAFILE_HH






