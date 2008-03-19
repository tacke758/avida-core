//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          // 
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef TEST_CPU_HH
#define TEST_CPU_HH

#include "../defs.hh"
#include "cpu.hh"

class cDivideRecord {
private:
  int record_id;
  int divide_id;
  int creature_id;

  double merit;
  int gestation_time;
  double fitness;
  int num_errors;
  int code_size;
  int copied_size;
  int exe_size;
  int copy_true;
  int cycle_to;

  double thread_frac;
  double thread_time_diff;
  double thread_code_diff;

  cDivideRecord * next;

  void operator=(const cDivideRecord & _record) { (void) _record; }
public:
  cDivideRecord(int div_id, double _merit, int _gest, double _fit, int _errs,
		int _csize, int _esize, int _cdsize, int _cptrue, double _tfrac,
		double _tdiff, double _cdiff) :
    record_id(0), divide_id(div_id), creature_id(-1),
    merit(_merit), gestation_time(_gest), fitness(_fit), num_errors(_errs),
    code_size(_cdsize), copied_size(_csize), exe_size(_esize),
    copy_true(_cptrue), cycle_to(-1), thread_frac(_tfrac),
    thread_time_diff(_tdiff), thread_code_diff(_cdiff),
    next(NULL)
  { ; }
  cDivideRecord(const cDivideRecord & _in) :
    record_id(0), divide_id(_in.divide_id), creature_id(_in.creature_id),
    merit(_in.merit), gestation_time(_in.gestation_time), fitness(_in.fitness),
    num_errors(_in.num_errors), code_size(_in.code_size),
    copied_size(_in.copied_size), exe_size(_in.exe_size),
    copy_true(_in.copy_true), cycle_to(_in.cycle_to),
    thread_frac(_in.thread_frac), thread_time_diff(_in.thread_time_diff),
    thread_code_diff(_in.thread_code_diff),
    next(NULL) { ; }
  ~cDivideRecord() { if (next != NULL) delete next; }

  int GetRecordID() { return record_id; }
  int GetDivideNum() { return divide_id; }
  int GetCreatureID() { return creature_id; }
  cDivideRecord * GetNext() { return next; }
  cDivideRecord * GetRecord(int _id) {
    if (divide_id == _id) return this;
    else if (next != NULL) return next->GetRecord(_id);
    else return NULL;
  }
  cDivideRecord * GetRecordByID(int _id) {
    if (record_id == _id) return this;
    else if (next != NULL) return next->GetRecordByID(_id);
    else return NULL;
  }

  double GetMerit() const { return merit; }
  int GetGestationTime() const { return gestation_time; }
  double GetFitness() const { return fitness; }
  int GetNumErrors() const { return num_errors; } 
  int GetCodeSize() const { return code_size; }
  int GetCopiedSize() const { return copied_size; }
  int GetExeSize() const { return exe_size; }
  int GetCopyTrue() const { return copy_true; }
  int GetCycleTo() const { return cycle_to; }
  double GetThreadFrac() const { return thread_frac; }
  double GetThreadTimeDiff() const { return thread_time_diff; }
  double GetThreadCodeDiff() const { return thread_code_diff; }

  void SetCycle(int _cycle) { cycle_to = _cycle; }
  void SetCreatureID(int _id) { creature_id = _id; }
  void SetNext(cDivideRecord * in_next) {
    next = in_next;
    if (next != NULL) record_id = next->record_id + 1;
  }
};


class cCPUTestInfo {
  friend class cTestCPU;
private:
  // Inputs...
  const int generation_tests;

  const int test_threads;
  const int print_threads;
  
  // Outputs...
  int is_viable;
  int num_divides;
  int max_depth;
  int depth_found;
  int max_cycle;

  cDivideRecord * div_records[MAX_TEST_DEPTH];

  // stored values
  int cur_id;
public:
  cCPUTestInfo(int _g_tests=TEST_CPU_GENERATIONS,
	       int _test_threads=FALSE,
	       int _print_threads=FALSE) :
    generation_tests(_g_tests),
    test_threads(_test_threads),
    print_threads(_print_threads),
    is_viable(TRUE),
    num_divides(0),
    max_depth(0),
    depth_found(0),
    max_cycle(0),
    cur_id (0) {
      for (int i = 0; i < MAX_TEST_DEPTH; i ++) div_records[i] = NULL;
  }
  
  ~cCPUTestInfo() {
    for (int i = 0; i < MAX_TEST_DEPTH; i++) {
      if (div_records[i] != NULL) delete div_records[i];
    }
  }

  void Clear() {
    for (int i = 0; i < MAX_TEST_DEPTH; i++) {
      if (div_records[i] != NULL) delete div_records[i];
      div_records[i] = NULL;
    }
    is_viable = FALSE;
    num_divides = 0;
    max_depth = 0;
    depth_found = -1;
    max_cycle = 0;
    cur_id = 0;
  }

  void AddDivideRecord(const cDivideRecord & _record, int gen_level=0) {
    if (gen_level == 0) num_divides++;
    cDivideRecord * new_record = new cDivideRecord(_record);
    new_record->SetNext(div_records[gen_level]);
    div_records[gen_level] = new_record;    
  }

  int GetGenerationTests() { return generation_tests; }

  int TestThreads() { return test_threads; }
  int PrintThreads() { return print_threads; }
  
  cDivideRecord * GetDivideRecord(int _id, int gen_level=0)
    { return div_records[gen_level]->GetRecord(_id); }
  cDivideRecord * GetDivideRecordByID(int _id, int gen_level=0)
    { return div_records[gen_level]->GetRecordByID(_id); }
  int GetNumRecords(int gen_level) 
    { return div_records[gen_level]->GetRecordID() + 1; }
  int GetNumDivides() { return num_divides; }
  int GetMaxDepth() { return max_depth; }
  int GetMaxCycle() { return max_cycle; }
  int GetDepthFound() { return depth_found; }

  inline int IsViable() { return is_viable; }

  // A few calculations for output...  THESE NEED TO BE FIXED!  @CAO
  inline double CalcMerit() {
    double out_merit = GetDivideRecord(1)->GetMerit();
    if (num_divides >= 2) {
      out_merit = (out_merit + GetDivideRecord(2)->GetMerit()) / 2.0;
    }
    return out_merit;
  }
  inline double CalcGestation() {
    double out_gest = GetDivideRecord(1)->GetGestationTime();
    if (num_divides >= 2) {
      out_gest = (double (out_gest + GetDivideRecord(2)->GetGestationTime()))
	/ 2.0;
    }
    return out_gest;
  }
  inline double CalcFitness() {
    if (is_viable == FALSE) return 0.0;
    double out_fit = GetDivideRecord(1)->GetFitness();
    if (num_divides >= 2) {
      out_fit = (out_fit + GetDivideRecord(2)->GetFitness()) / 2.0;
    }
    return out_fit;
  }
  inline double CalcCopiedSize() {
    double out_size = GetDivideRecord(1)->GetCopiedSize();
    if (num_divides >= 2) {
      out_size = (double (out_size + GetDivideRecord(2)->GetCopiedSize()))/ 2.0;
    }
    return out_size;
  }
  inline double CalcExeSize() {
    double out_size = GetDivideRecord(1)->GetExeSize();
    if (num_divides >= 2) {
      out_size = (double (out_size + GetDivideRecord(2)->GetExeSize()))/ 2.0;
    }
    return out_size;
  }

  // @CAO remove this, and replace in code with IsViable();
  inline int CalcCopyTrue() {
    int out_CT = GetDivideRecord(1)->GetCopyTrue();
    if (num_divides >= 2) out_CT = out_CT && GetDivideRecord(2)->GetCopyTrue();
    return out_CT;
  }
};


class cTestCPU {
private:
  cBaseCPU cpu;

private:
  static cTestCPU * test_cpu;                // Test CPUs...
  static cGenotype test_genotype[MAX_TEST_DEPTH]; // Test Genotypes

private:
  cDivideRecord ProcessGestation(cCPUTestInfo & test_info, int time_penalty);
  inline void ChangeGenotype(cGenotype * in_genotype)
    { cpu.ChangeGenotype(in_genotype); }

  static int TestCode_Body(cCPUTestInfo & test_info, const cCodeArray & code,
			   int cur_depth);
public:
  inline cTestCPU() : cpu(TRUE) { ; }
  inline ~cTestCPU() { ; }

  static inline void Init() { test_cpu = new cTestCPU[MAX_TEST_DEPTH]; }

  // Static Methods
  static int TestCode(cCPUTestInfo & test_info, const cCodeArray & code);
  static int TestCode(cCPUTestInfo & test_info, const cCodeArray & code,
		      ofstream & out_fp);
  static void PrintCode(const cCodeArray & code);
  static void PrintCode(const cCodeArray & code, cString filename, cGenotype * genotype=NULL);
  static void PrintTasks(ostream & fp);
};

#endif
