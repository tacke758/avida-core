//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
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

#include <iostream.h>
#include <fstream.h>
#include "../defs.hh"
#include "assert.hh"
#include "string.hh"

class cStats;
class cDataFileManager;

/**
 * This class encapsulates file handling for the class @ref cDataFileManager.
 * The files it creates are write only. Reading of files is handled by 
 * @ref cFile.
 */

class cDataFile {

private:
  cString name;
  ofstream fp;

public:
    /**
     * The empty constructor does nothing.
     **/
    cDataFile() {;}

    /**
     * This constructor opens a file of the given name, and makes sure
     * the file is usable.
     *
     * @param _name The name of the file to open.
     **/
    cDataFile(cString _name);

    /**
     * The desctructor automatically closes the file.
     **/
    ~cDataFile(){ fp.close(); }

    /**
     * @return A bool that indicates whether the file is actually usable.
     **/
    bool Good() const { return fp.good(); }

    /**
     * @return The output stream corresponding to the file.
     **/
    ofstream & GetOFStream(){ return fp; }

    /**
     * This function makes sure that all cached data is written to the disk.
     **/
    void Flush(){ fp.flush(); }
};


/**
 * This class helps to manage a collection of data files. It is possible
 * to add files, to remove files, and to access existing files by name.
 **/

class cDataFileManager {
private:

  class cEntry {
  public:
    cString name;
    cDataFile file;
    cEntry * next;
    cEntry(const cString & _name, cEntry * _next) :
     name(_name), file(_name), next(_next) {;}
  };

  cEntry * head;
  cEntry * prev;
  cEntry * next;

  cEntry * InternalFind(const cString & name);

public:
  cDataFileManager() : head(NULL), prev(NULL), next(NULL) {;}
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
     * @return The ofstream.
     * @param name The name of the file to look up/create.
     **/
  ofstream & GetOFStream(const cString & name);

  void FlushAll();

    /** Removes the given file, thereby closing it.
     *
     * @return TRUE if file existed, otherwise false.
     **/
  bool Remove(const cString & name);
};



#endif // DATAFILE_HH






