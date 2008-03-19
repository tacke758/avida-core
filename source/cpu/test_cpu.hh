//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
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
  
  cString task_order;
public:
  cDivideRecord() { Clear(); }

  cDivideRecord(double _merit, int _gest, double _fit, int _errs, int _csize,
		int _esize, int _cdsize, int _cptrue, double _tfrac=0,
		double _tdiff=0, double _cdiff=0, const cString & _torder="") :
    merit(_merit), gestation_time(_gest), fitness(_fit), num_errors(_errs),
    code_size(_cdsize), copied_size(_csize), exe_size(_esize),
    copy_true(_cptrue), cycle_to(-1), thread_frac(_tfrac),
    thread_time_diff(_tdiff), thread_code_diff(_cdiff), task_order(_torder)
  { ; }
  cDivideRecord(const cDivideRecord & _in) :
    merit(_in.merit), gestation_time(_in.gestation_time), fitness(_in.fitness),
    num_errors(_in.num_errors), code_size(_in.code_size),
    copied_size(_in.copied_size), exe_size(_in.exe_size),
    copy_true(_in.copy_true), cycle_to(_in.cycle_to),
    thread_frac(_in.thread_frac), thread_time_diff(_in.thread_time_diff),
    thread_code_diff(_in.thread_code_diff), task_order(_in.task_order) { ; }
  ~cDivideRecord() { ; }

  void operator=(const cDivideRecord & _record) {
    merit = _record.merit;          gestation_time = _record.gestation_time;
    fitness = _record.fitness;      num_errors = _record.num_errors;
    code_size = _record.code_size;  copied_size = _record.copied_size;
    exe_size = _record.exe_size;    copy_true = _record.copy_true;
    cycle_to = _record.cycle_to;    thread_frac = _record.thread_frac;
    thread_time_diff = _record.thread_time_diff;
    thread_code_diff = _record.thread_code_diff;
    task_order = _record.task_order;
  }

  void Clear() {
    merit = 0.0;  gestation_time = 0;  fitness = 0.0;  num_errors = 0;
    code_size = 0;  copied_size = 0;  exe_size = 0;  copy_true = 0;
    cycle_to = 0;  thread_frac = 0.0;  thread_time_diff = 0.0;
    thread_code_diff = 0.0; task_order = "";
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
  const cString & GetTaskOrder() const { return task_order; }

  void SetCycle(int _cycle) { cycle_to = _cycle; }
};


class cCPUTestInfo {
  friend class cTestCPU;
private:
  // Inputs...
  int generation_tests;  // Maximum depth in generations to test
  bool test_threads;     // Should a report of threading be saved?
  bool print_threads;    // Should the report be printed?
  bool trace_execution;        // Should we trace this CPU?
  bool trace_task_order;       // Should we keep track of ordering of tasks?
  ofstream trace_fp;

  // Outputs...
  bool is_viable;              // Is this organism colony forming?
  int max_depth;
  int depth_found;
  int max_cycle;

  cDivideRecord div_records[MAX_TEST_DEPTH];
  mutable cDivideRecord colony_record;
  mutable bool colony_ok;

  // stored values
  int cur_id;

  // Private methods
  int GetGenerationTests() const { return generation_tests; }
  bool GetTestThreads() const { return test_threads; }
  bool GetPrintThreads() const { return print_threads; }
  bool GetTraceTaskOrder() const { return trace_task_order; }

  void SetTraceExecution(cString filename="trace.dat") {
    trace_execution = true;
    trace_fp.open(filename());
    if( !trace_fp.good() ) {
      g_debug.Warning("Unable to open %s\n", filename());
      return;
    }
  }
  bool GetTraceExecution() const { return trace_execution; }
  ofstream & GetTraceFP() { return trace_fp; }
public:
  cCPUTestInfo() : generation_tests(TEST_CPU_GENERATIONS),
        test_threads(false),  print_threads(false),  trace_execution(false),
        is_viable(true),  max_depth(0),  depth_found(0),  max_cycle(0),
        colony_ok(false), cur_id(0)
  { ; }

  ~cCPUTestInfo() { ; }

  void SetGenerationTests(int _tests) { generation_tests = _tests; }
  void TestThreads(bool _test=true) { test_threads = _test; }
  void PrintThreads(bool _print=true) { print_threads = _print; }
  void TraceTaskOrder(bool _trace=true) { trace_task_order = _trace; }

  void Clear() {
    for (int i = 0; i < MAX_TEST_DEPTH; i++) div_records[i].Clear();
    colony_ok = false;
    trace_execution = false;
    is_viable = false;
    max_depth = 0;
    depth_found = -1;
    max_cycle = 0;
    cur_id = 0;
  }

  void SetupColony() const {
    colony_record = div_records[0];
    
    // Attention: some logic is in GetColonyFitness (to set the
    // colony fitness to zero if the creature is not viable).
    // This had to be done because colony_record.fitness is not accessible
    // from here.  -- Claus
    
    colony_ok = true;
  }

  // Genotype Stats...
  int GetGenotypeCodeSize(int id=0) const
    { return div_records[id].GetCodeSize(); }
  int GetGenotypeCopiedSize(int id=0) const
    { return div_records[id].GetCopiedSize(); }
  int GetGenotypeExeSize(int id=0) const
    { return div_records[id].GetExeSize(); }
  double GetGenotypeMerit(int id=0) const
    { return div_records[id].GetMerit(); }
  int GetGenotypeGestation(int id=0) const
    { return div_records[id].GetGestationTime(); }
  double GetGenotypeFitness(int id=0) const
    { return div_records[id].GetFitness(); }
  int GetGenotypeNumErrors(int id=0) const
    { return div_records[id].GetNumErrors(); }
  double GetGenotypeThreadFrac(int id=0) const
    { return div_records[id].GetThreadFrac(); }
  double GetGenotypeThreadTimeDiff(int id=0) const
    { return div_records[id].GetThreadTimeDiff(); }
  double GetGenotypeThreadCodeDiff(int id=0) const
    { return div_records[id].GetThreadCodeDiff(); }
  bool GetGenotypeCopyTrue(int id=0) const
    { return div_records[id].GetCopyTrue(); }
  const cString & GetGenotypeTaskOrder(int id=0) const
    { return div_records[id].GetTaskOrder(); }
  int GetCycleTo(int id=0) const
    { return div_records[id].GetCycleTo(); }


  // Colony Stats...  (@CAO Fix these!)
  double GetColonyCodeSize() const {
    if (colony_ok == false) SetupColony();
    return colony_record.GetCodeSize();
  }
  double GetColonyCopiedSize() const {
    if (colony_ok == false) SetupColony();
    return colony_record.GetCopiedSize();
  }
  double GetColonyExeSize() const {
    if (colony_ok == false) SetupColony();
    return colony_record.GetExeSize();
  }
  double GetColonyMerit() const {
    if (colony_ok == false) SetupColony();
    return colony_record.GetMerit();
  }
  double GetColonyGestation() const {
    if (colony_ok == false) SetupColony();
    return colony_record.GetGestationTime();
  }
  double GetColonyFitness() const {
    if (colony_ok == false) SetupColony(); 
    if ( !is_viable )
      return 0;
    else
      return colony_record.GetFitness();
  }
  double GetColonyNumErrors() const {
    if (colony_ok == false) SetupColony();
    return colony_record.GetNumErrors();
  }
  double GetColonyThreadFrac() const {
    if (colony_ok == false) SetupColony();
    return colony_record.GetThreadFrac();
  }
  double GetColonyThreadTimeDiff() const {
    if (colony_ok == false) SetupColony();
    return colony_record.GetThreadTimeDiff();
  }
  double GetColonyThreadCodeDiff() const {
    if (colony_ok == false) SetupColony();
    return colony_record.GetThreadCodeDiff();
  }

  // General stats...
  bool IsViable() const { return is_viable; }
  int GetMaxDepth() const { return max_depth; }
  int GetMaxCycle() const { return max_cycle; }
  int GetDepthFound() const { return depth_found; }
};


class cTestCPU {
private:
  cBaseCPU cpu;

  static cTestCPU * test_cpu;                // Test CPUs...
  static cGenotype test_genotype[MAX_TEST_DEPTH]; // Test Genotypes

  cDivideRecord ProcessGestation(cCPUTestInfo & test_info);
  inline void ChangeGenotype(cGenotype * in_genotype)
    { cpu.ChangeGenotype(in_genotype); }

  static bool TestCode_Body(cCPUTestInfo & test_info, const cCodeArray & code,
			   int cur_depth);
public:
  inline cTestCPU() : cpu(TRUE) { ; }
  inline ~cTestCPU() { ; }

  static inline void Init() { test_cpu = new cTestCPU[MAX_TEST_DEPTH]; }

  // Static Methods
  static bool TestCode(cCPUTestInfo & test_info, const cCodeArray & code);
  static bool TestCode(cCPUTestInfo & test_info, const cCodeArray & code,
		       ofstream & out_fp);
  static bool TestCodeFragment(cCPUTestInfo & test_info,
			       const cCodeArray & code, int cpu_time);
  static void PrintCode(const cCodeArray & code);
  static void PrintCode(const cCodeArray & code, cString filename,
			cGenotype * genotype=NULL);
  static void TraceCode(const cCodeArray & code, cString filename="trace.dat");
  static int GetNumTasks();
  static int GetTaskCount(int task_id);
  static const cString & GetTaskKeyword(int task_id);
  
  /**
   * Returns the phenotype of the test-cpu at the given depths in
   * the divide record (can be used to find out what kind of tasks
   * the corresponding genotypes have done).
   **/
  static cPhenotype * GetPhenotype( int depths=0 ){
    return test_cpu[depths].cpu.phenotype; }
};

#endif

