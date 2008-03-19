//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef FILE_HH
#define FILE_HH

#include <fstream.h>
#include "string.hh"

/**
 * This class encapsulates file handling. In comparison to @ref cDataFile
 * it has somewhat different features. It is more intended for reading files.
 * In particular, by default it does not create a file that doesn't exist. 
 * Its main usage is for the class @ref cInitFile.
 **/


class cFile {
private:
  fstream fp;
  cString filename;
public:
    /**
     * The empty constructor does nothing.
     **/ 
    inline cFile() : filename("") { ; }

    /**
     * This constructor opens a file of the given name.
     *
     * @param _filename The name of the file to open.
     **/
    inline cFile(cString _filename) : filename("") { Open(_filename); }
    
    /**
     * The desctructor automatically closes the file.
     **/
    ~cFile() { fp.close(); filename = ""; }

    /**
     * @return The name of the file currently open.
     **/
    inline const cString & GetFilename() const { return filename; }

    /**
     * Open a file of the given name. If another file was open previously,
     * close that one first.
     *
     * @return 0 if something went wrong, and 1 otherwise.
     * @param _filename The name of the file to open.
     * @param mode The opening mode.
     **/
    int Open(cString _filename, int mode=(ios::in|ios::nocreate));
  
    /**
     * Close the currently open file.
     **/
    int Close();
    
    /**
     * Reads the next line in the file.
     **/
    int ReadLine(cString & in_string);
  
    //int CountLines();
  //inline void Rewind() { fp.rdbuf()->seekpos(ios::beg,0); }

  // Tests
  inline int IsOpen() { return (filename != ""); }
  inline int Fail() { return (fp.fail()); }
  inline int Good() { return (fp.good()); }
  inline int Eof() { return (fp.eof()); }
  // int AtStart();
  // int AtEnd();
};


/**
 * A string Single Linked List element. Typical usage is done in the following
 * way:
 * <pre>
 * cStringSLLE *e1, *e2;
 * e1 = new cStringSLLE; // create first list element
 * e2 = new cStringSLLE; // create second list element
 * e1->String() = "Hello, ";
 * e2->String() = "World!\n";
 * e1->SetNext( e2 );
 * </pre>
 * The linked list now has the structure "Hello, " -> "World!\n".
 */

class cStringSLLE {
private:
  cString string;
  cStringSLLE * next;
public:
  inline cStringSLLE(cStringSLLE * in_next = NULL) { next = in_next; }
  inline cString & String() { return string; }
  inline cStringSLLE * GetNext() { return next; }

  inline void SetNext(cStringSLLE * in_next) { next = in_next; }
};


/**
 * A class to handle initialization files.
 **/

class cInitFile : public cFile {
private:
  int num_lines;
  cStringSLLE * line_list;

public:
    /**
     * The empty constructor constructs an object that is in a clean
     * state. You can set the file to open with @ref cFile::Open() later on.
     **/
    cInitFile();
    
    /**
     * Opens the file with the given name.
     * 
     * @param in_filename Name of the initialization file to open.
     **/
    cInitFile(cString in_filename);
    
    ~cInitFile();

    /**
     * Loads the file into memory.
     **/
    void Load();
    
  //void Save(char * in_filename = NULL);
  
    /**
     * Remove all comments and whitespace. Comments are currently marked
     * with the character '#'.
     **/
    void Compress();

    /** 
     * Add a line to the beginning of the file in memory.
     * This function is used by @ref cGenesis.
     *
     * @param in_string The string to be added.
     **/
    void AddLine(cString & in_string);
    
    /**
     * Get a line from the file in memory. If called without parameters,
     * the first line of the file is returned.
     *
     * @param line_num The line count of the line to be returned 
     * (starting from 0).
     **/
    cString GetLine(int line_num=0);
  
    /**
     * Removes the first line from the file in memory.
     *
     * @return The line removed.
     **/
    cString RemoveLine();
  
    /**
     * Sends the current contents of the file in memory to stdout.
     **/
    void PrintLines();
    
    /**
     * Checks whether any line contains a given keyword in the specified 
     * column. Stops when the first occurrence of the keyword is found.
     *
     * @return TRUE if keyword is found, FALSE otherwise.
     * @param in_string A string variable that will contain the found line
     * if search succeeds, and will have undefined contents otherwise.
     * @param keyword The keyword to look for.
     * @param col The column in which the keyword should be found.
     **/
    int Find(cString & in_string, cString keyword, int col) const;

    /**
     * Reads an entry in the initialization file that has a given keyword
     * in the first column. The keyword is not part of the returned string.
     *
     * @return The entry that has been found.
     * @param name The keyword to look for (the name of the entry).
     * @param def If the keyword is not found, def is returned. This allows
     * one to set standard values that are used if the user does not override
     * them.
     **/
    cString ReadString(char * name, cString def = "") const;

    /**
     * Return the number of lines in memory.
     **/
    inline int GetNumLines() { return num_lines; }
};


/**
 * A specialized initialization file class that is used for setting up
 * the way Avida should run.
 */

class cGenesis : public cInitFile {
public:
    /** 
     * The empty constructor creates a clean object.
     **/
    cGenesis();
  
    /**
     * This constructor opens the given initialization file, reads it in,
     * removes all comments, and closes it again.
     **/
    cGenesis(const char * filename);

    /**
     * Opens an initialization file, reads it in, removes all comments, and 
     * closes it again.
     **/
    int Open(cString _filename, int mode=(ios::in|ios::nocreate));

    /**
     * Convenience function. Adds an integer valued entry to the file in
     * memory. Uses @ref cInitFile::AddLine().
     **/
    void AddInput(const char * in_name, int in_value);
  
    /**
     * Convenience function. Adds a string valued entry to the file in
     * memory. Uses @ref cInitFile::AddLine().
     **/
    void AddInput(const char * in_name, const char * in_value);
    
    /**
     * Reads an entry of type int. In case the entry does not exist,
     * the value of base is returned.
     *
     * @param name The name of the entry.
     * @param base The default value.
     **/
    int     ReadInt   (char * name, int base     = 0) const;
  
    /**
     * Reads an entry of type float. In case the entry does not exist,
     * the value of base is returned.
     *
     * @param name The name of the entry.
     * @param base The default value.
     **/
    double  ReadFloat (char * name, float base   = 0.0) const;
};

#endif




