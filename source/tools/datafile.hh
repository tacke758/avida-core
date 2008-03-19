/* datafile.hh ****************************************************************
 cDataFile 
 cDataFileManager 

 charles@krl.caltech.edu & travc@ugcs.caltech.edu
 Time-stamp: <98/10/22 01:58:56 travc>

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

typedef void (*tOutputMethod)(ofstream & fp);

class cDataFile {
  
private:
  cString name;
  ofstream fp;
  tOutputMethod outmethod;


public:
  cDataFile() {;}
  cDataFile(cString _name, tOutputMethod _outmethod);
  ~cDataFile(){ fp.close(); }
  
  bool Good() const { return fp.good(); }
  void Output();
  void Flush(){ fp.flush(); }

  bool CheckOutputMethod(tOutputMethod _outmethod){ 
    return (outmethod == _outmethod); }

  void OutputAverageData();

};  


class cDataFileManager {
private:

  class cEntry {
  public:
    cString name;
    cDataFile file;
    cEntry * next;
    cEntry(const cString & _name, const tOutputMethod _outmethod, 
	   cEntry * _next) : 
     name(_name), file(_name,_outmethod), next(_next) {;}
  };
  
  cEntry * head;
  cEntry * prev;
  cEntry * next;

  cEntry * InternalFind(const cString & name);

public:
  cDataFileManager() : head(NULL), prev(NULL), next(NULL) {;}
  ~cDataFileManager();

  // Add if not there
  cDataFile & Get(const cString & name, const tOutputMethod outmethod); 

  void FlushAll();

  bool Remove(const cString & name);
};



#endif // DATAFILE_HH
