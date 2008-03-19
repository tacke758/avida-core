/* memtrack.hh ****************************************************************
 Memory tracking functions

 travc@ugcs.caltech.edu
 Time-stamp: <98/06/02 16:51:32 travc>

******************************************************************************/
 
#ifndef	MEMORYTRACK_H
#define	MEMORYTRACK_H

#include "string.hh"
#include "struct.hh"


#ifdef DEBUG_INTEGRITY_CHECK


// The cIntegrityCheck class is setup on a single other class at a time
// to make sure that that class is 'behaving' properly....

#define ALLOC_TRUE 12345

#define DEBUG_TRACK_OBJECTS    0
#define DEBUG_REPORT_COMMENTS  1
#define DEBUG_REPORT_CONSTRUCT 2
#define DEBUG_REPORT_DESTRUCT  3

#ifdef INTEGRITY_CHECK
#define INTEGRITY_HASH_SIZE 211
#else
#define INTEGRITY_HASH_SIZE 1
#endif


template <class T> class tIntegrityCheck {
private:
  static int total_count;
  static int new_count;
  static int del_count;
  static int prime;
  static cList object_table[INTEGRITY_HASH_SIZE];
#ifdef INTEGRITY_CHECK
  int alloc_test;
#endif

  int Hash(void * _in) { return ((unsigned int) _in) % INTEGRITY_HASH_SIZE; }
  int Test(void * _in) { return object_table[Hash(_in)].Has(_in); }
  void Insert(void * _in) { object_table[Hash(_in)].InsertFront(_in); }
  void Remove(void * _in) { object_table[Hash(_in)].Remove(_in); }
public:
  tIntegrityCheck() {
    total_count++;
    new_count++;
#ifdef INTEGRITY_CHECK
    alloc_test = ALLOC_TRUE;
#endif
    if (GetPrime(DEBUG_REPORT_CONSTRUCT)) {
      // cout << "Creating object of type: " << DebugType() << "\n";
    }
    if (GetPrime(DEBUG_TRACK_OBJECTS)) Insert(this);
  }

  /* virtual */  ~tIntegrityCheck() {
    total_count--;
    del_count++;
#ifdef INTEGRITY_CHECK
    if (alloc_test != ALLOC_TRUE) {
      cerr << "tIntegrityCheck - Trying to delete invalid object.";
    }
    alloc_test = 0;
#endif
    if (GetPrime(DEBUG_REPORT_DESTRUCT)) {
      // cout << "Destroying object of type: " << DebugType() << "\n";
    }
    if (GetPrime(DEBUG_TRACK_OBJECTS)) Remove(this);
  }
  
  static void Report(ostream & out = cout) {
    out << "total: " << total_count << "  ";
    out << "new: " << new_count << "  ";
    out << "del: " << del_count << "\n";
    new_count = 0;
    del_count = 0;
  };  

  static int DebugOK() {
    int ok_status = TRUE;

    // Make sure we have the same number of object hashed as out total_count
    int test_count = 0;
    int test_mem_count = 0;
    for (int i = 0; i < INTEGRITY_HASH_SIZE; i++) {
      test_count += object_table[i].GetSize();
      object_table[i].Reset();
      for (int j = 0; j < object_table[i].GetSize(); j++) {
	tIntegrityCheck<T> * cur_object;
	cur_object = (tIntegrityCheck<T> *) object_table[i].GetCurrent();
	// test_mem_count += cur_object->DebugSize();
	object_table[i].Next();
      }
    }
    if (GetPrime(DEBUG_TRACK_OBJECTS) && test_count != total_count) {
      cerr << "Error in tIntegrityCheck::OK(); test_count = " << test_count
	   << ", total_count = " << total_count << "\n";
      ok_status = FALSE;
    }

    cout << "Total mem count = " <<  test_mem_count << "\n";

    return ok_status;
  }

  static int GetTotalCount() { return total_count; }
  static void Prime(int prime_type = DEBUG_REPORT_COMMENTS)
    { prime |= (1 << prime_type); }
  static void UnPrime(int prime_type = DEBUG_REPORT_COMMENTS)
    { prime &= ~(1 << prime_type); }
  static void PrimeAll() { prime = ~0; }
  static void UnPrimeAll() { prime = 0; }

  static int GetPrime(int prime_type = DEBUG_REPORT_COMMENTS)
    { return ((prime & (1 << prime_type)) != 0); }

  /*
  virtual cString DebugType() { return "Type Unknown"; }
  virtual cString DebugInfo() { return "Info Unknown"; }
  virtual int DebugSize() { return 0; }
  virtual void DebugReport(ostream & out = cout) {
    out << "Type: " << DebugType()
	<< "Info: " << DebugInfo()
	<< "Size: " << DebugSize()
	<< "\n";
  }
  */
};

// Currently tracked classes...
class cCodeArray;
class cBasicInstruction;
class cIntegratedScheduleNode;

class cIntegrityTrack {
private:
public:
  static void FullReport(ostream & out = cout) {
    out << "+--- Integrity Check ---\n";

    out << "| cCodeArray........: ";
    tIntegrityCheck<cCodeArray>::Report(out);
    out << "| cBasicInstruction.: ";
    tIntegrityCheck<cBasicInstruction>::Report(out);
    out << "| cIntSchedNode.....: ";
    tIntegrityCheck<cIntegratedScheduleNode>::Report(out);

    out << "+-----------------------\n";
  }
};


#endif  // DEBUG_INTEGRITY_CHECK

#endif  // MEMORYTRACK_H
