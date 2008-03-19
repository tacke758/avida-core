//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef FILE_HH
#define FILE_HH

//#include <fstream.h>
// porting to gcc 3.1 -- k
#include <fstream>

#include "tArray.hh"
#include "string.hh"
#include "string_list.hh"

/**
 * This class encapsulates file handling. In comparison to @ref cDataFile
 * it has somewhat different features. It is more intended for reading files.
 * In particular, by default it does not create a file that doesn't exist. 
 * Its main usage is for the class @ref cInitFile.
 **/


class cFile {
protected:
  std::fstream fp;
  cString filename;
  bool is_open; // Have we successfully opened this file?
  bool verbose; // Should file be verbose about warnings to users?
public:
  /**
   * The empty constructor does nothing.
   **/ 
  cFile() : filename(""), is_open(false), verbose(false) { ; }
  
  /**
   * This constructor opens a file of the given name.
   *
   * @param _filename The name of the file to open.
   **/
  cFile(cString _filename) : filename(""), is_open(false) { Open(_filename); }
  
  /**
   * The desctructor automatically closes the file.
   **/
  ~cFile() { if (is_open == true) fp.close(); filename = ""; }
  
  /**
   * @return The name of the file currently open.
   **/
  const cString & GetFilename() const { return filename; }
  
  /**
   * Open a file of the given name. If another file was open previously,
   * close that one first.
   *
   * @return 0 if something went wrong, and 1 otherwise.
   * @param _filename The name of the file to open.
   * @param mode The opening mode.
   **/
  //bool Open(cString _filename, int mode=(ios::in|ios::nocreate));
  // porting to gcc 3.1
  // nocreate is no longer in the class ios -- k
  bool Open(cString _filename, std::ios::openmode mode=(std::ios::in));
  
  /**
   * Close the currently open file.
   **/
  bool Close();
  
  /**
   * Reads the next line in the file.
   **/
  bool ReadLine(cString & in_string);
  
  // Tests
  bool IsOpen() const { return is_open; }
  bool Fail() const { return (fp.fail()); }
  bool Good() const { return (fp.good()); }
  bool Eof() const { return (fp.eof()); }
  // int AtStart();
  // int AtEnd();

  void SetVerbose(bool _v=true) { verbose = _v; }
};


/**
 * A class to handle initialization files.
 **/

class cInitFile : public cFile {
private:
  struct sFileLineInfo {
    cString line;
    int line_num;
    mutable bool used;
  };

  tArray<sFileLineInfo> line_array;
  cStringList extra_lines;
  cString filetype;
  cStringList file_format;

  int active_line;
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
  
  /**
   * Loads a stream into memory rather than a file.
   **/
  void LoadStream(std::istream & in_steam);
  
  void Save(const cString & in_filename = "");
  
  /**
   * Parse heading information about the contents of the file loaded into
   * memory, if available.
   **/
  void ReadHeader();

  /**
   * Remove all comments and whitespace from a file loaded into memory.
   * Comments are currently marked with the character '#'.
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
   * Get the active line and advance to the next line.
   **/
  cString GetNextLine() { return GetLine(active_line++); }


  /**
   * Reset the active line to the beginning (or the point specified...
   **/
  void ResetLine(int new_pos=0) { active_line = new_pos; }

  /**
   * Returns the line number that is active.
   **/
  int GetLineNum() { return active_line; }

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
  bool Find(cString & in_string, const cString & keyword, int col) const;
  
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
  cString ReadString(const cString & name, cString def = "") const;
  
  /**
   * Looks over all lines loaded into the file, and warns if any of them
   * have not been the targets of the Find() method.  All methods that
   * search the file for a keyword use find, so this can be used to locate
   * keywords that are not understood by the program.
   **/
  bool WarnUnused() const;

  /**
   * Return the number of lines in memory.
   **/
  int GetNumLines() const { return line_array.GetSize(); }

  const cString & GetFiletype() { return filetype; }
  cStringList & GetFormat() { return file_format; }
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
    cGenesis(const cString & filename);

    /**
     * Opens an initialization file, reads it in, removes all comments, and 
     * closes it again.
     **/
    //int Open(cString _filename, int mode=(ios::in|ios::nocreate));
    // porting to gcc 3.1
    // nocreate is no longer in the class ios -- k
    int Open(cString _filename, std::ios::openmode mode=(std::ios::in));

    /**
     * Convenience function. Adds an integer valued entry to the file in
     * memory. Uses @ref cInitFile::AddLine().
     **/
    void AddInput(const cString & in_name, int in_value);
  
    /**
     * Convenience function. Adds a string valued entry to the file in
     * memory. Uses @ref cInitFile::AddLine().
     **/
    void AddInput(const cString & in_name, const cString & in_value);
    
    /**
     * Reads an entry of type int. In case the entry does not exist,
     * the value of base is returned.
     *
     * @param name The name of the entry.
     * @param base The default value.
     * @param warn Warn user if not set?
     **/
    int ReadInt (const cString & name, int base=0, bool warn=true) const;
  
    /**
     * Reads an entry of type float. In case the entry does not exist,
     * the value of base is returned.
     *
     * @param name The name of the entry.
     * @param base The default value.
     * @param warn Warn user if not set?
     **/
     double ReadFloat (const cString & name, float base=0.0, bool warn=true) const;

  void Read(cString & _var, const cString & _name, const cString & _def="") {
    _var = ReadString(_name, _def);
  }

  void Read(int & _var, const cString & _name, const cString & _def="0") {
    _var = ReadInt(_name, _def.AsInt());
  }

  void Read(double & _var, const cString & _name, const cString & _def="0.0") {
    _var = ReadFloat(_name, _def.AsDouble());
  }

  void Read(bool & _var, const cString & _name, const cString & _def="0.0") {
    _var = ReadInt(_name, _def.AsInt()) != 0;
  }
};

#endif




