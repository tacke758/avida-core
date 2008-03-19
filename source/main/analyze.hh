//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef ANALYZE_HH
#define ANALYZE_HH

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>

#include "../tools/tArray.hh"
#include "../tools/tList.hh"

#include "../tools/datafile.hh"
#include "../tools/data_entry.hh"    // Used for lists of commands
#include "../tools/functions.hh"
#include "../tools/help.hh"
#include "../tools/random.hh"
#include "../tools/string.hh"
#include "../tools/string_list.hh"
#include "../tools/string_util.hh"

#include "genome.hh"    // Used in cAnalyzeGenotype

class cInstSet;

#define MAX_BATCHES 200

// cAnalyzeGenotype    : Collection of information about loaded genotypes
// cAnalyzeCommand     : A command in a loaded program
// cAnalyzeFlowCommand : A cAnalyzeCommand containing other commands
// cAnalyzeFunction    : User-defined function
// cGenotypeBatch      : Collection of cAnalyzeGenotypes
// cAnalyze            : The master analyze object.

class cAnalyzeGenotype {
private:
  cGenome genome;            // Full Genome
  cInstSet & inst_set;       // Instruction set used in this genome
  cString name;              // Name, if one was provided in loading
  cString aligned_sequence;  // Sequence (in ASCII) after alignment
  cString tag;               // All genotypes in a batch can be tagged

  bool viable;

  // Group 1 : Load-in Stats (Only obtained if available for input file)
  int id_num;
  int parent_id;
  int num_cpus;
  int total_cpus;
  int update_born;
  int update_dead;
  int depth;

  // Group 2 : Basic Execution Stats (Obtained from test CPUs)
  int length;
  int copy_length;
  int exe_length;
  double merit;
  int gest_time;
  double fitness;
  int errors;
  double div_type;

  tArray<int> task_counts;

  // Group 3 : Stats requiring parental genotype (Also from test CPUs)
  double fitness_ratio;
  double efficiency_ratio;
  double comp_merit_ratio;
  int parent_dist;
  int ancestor_dist;
  cString parent_muts;

  // Group 4 : Landscape stats (obtained from testing all possible mutations)
  class cAnalyzeLandscape {
  public:
    double frac_dead;
    double frac_neg;
    double frac_neut;
    double frac_pos;
    cAnalyzeLandscape() : frac_dead(0.0), frac_neg(0.0),
			  frac_neut(0.0), frac_pos(0.0) { ; }
  };
  mutable cAnalyzeLandscape * landscape_stats;

  // Group 5 : More complex stats (obtained indvidually, through tests)
  cString task_order;

  cStringList special_args; // These are args placed after a ':' in details...

  int NumCompare(double new_val, double old_val) const {
    if (new_val == old_val) return  0;
    else if (new_val == 0)       return -2;
    else if (old_val == 0)       return +2;
    else if (new_val < old_val)  return -1;
    // else if (new_val > old_val)
    return +1;
  }

  int CalcMaxGestation() const;
  void CalcLandscape() const;
public:
  cAnalyzeGenotype(cString symbol_string, cInstSet & in_inst_set);
  cAnalyzeGenotype(const cGenome & _genome, cInstSet & in_inst_set);
  cAnalyzeGenotype(const cAnalyzeGenotype & _gen);
  ~cAnalyzeGenotype();

  const cStringList & GetSpecialArgs() { return special_args; }
  void SetSpecialArgs(const cStringList & _args) { special_args = _args; }

  void Recalculate(cAnalyzeGenotype * parent_genotype=NULL);
  void PrintTasks(std::ofstream & fp, int min_task=0, int max_task=-1);

  // Set...
  void SetSequence(cString _sequence);
  
  void SetName(const cString & _name) { name = _name; }
  void SetAlignedSequence(const cString & _seq) { aligned_sequence = _seq; }
  void SetTag(const cString & _tag) { tag = _tag; }

  void SetViable(bool _viable) { viable = _viable; }

  void SetID(int _id) { id_num = _id; }
  void SetParentID(int _id) { parent_id = _id; }
  void SetNumCPUs(int _cpus) { num_cpus = _cpus; }
  void SetTotalCPUs(int _cpus) { total_cpus = _cpus; }
  void SetUpdateBorn(int _born) { update_born = _born; }
  void SetUpdateDead(int _dead) { update_dead = _dead; }
  void SetDepth(int _depth) { depth = _depth; }

  void SetLength(int _length) { length = _length; }
  void SetCopyLength(int _length) { copy_length = _length; }
  void SetExeLength(int _length) { exe_length = _length; }
  void SetMerit(double _merit) { merit = _merit; }
  void SetGestTime(int _gest) { gest_time = _gest; }
  void SetFitness(double _fitness) { fitness = _fitness; }
  void SetDivType(double _div_type) { div_type = _div_type; }
  void SetParentDist(int _dist) { parent_dist = _dist; }
  void SetAncestorDist(int _dist) { ancestor_dist = _dist; }

  void SetParentMuts(const cString & in_muts) { parent_muts = in_muts; }
  void SetTaskOrder(const cString & in_order) { task_order = in_order; }

//    void SetFracDead(double in_frac);
//    void SetFracNeg(double in_frac);
//    void SetFracNeut(double in_frac);
//    void SetFracPos(double in_frac);

  // Accessors...
  const cGenome & GetGenome() const { return genome; }
  const cString & GetName() const { return name; }
  const cString & GetAlignedSequence() const { return aligned_sequence; }
  const cString & GetTag() const { return tag; }

  bool GetViable() const { return viable; }

  int GetID() const { return id_num; }
  int GetParentID() const { return parent_id; }
  int GetParentDist() const { return parent_dist; }
  int GetAncestorDist() const { return ancestor_dist; }
  int GetNumCPUs() const { return num_cpus; }
  int GetTotalCPUs() const { return total_cpus; }
  int GetLength() const { return length; }
  int GetCopyLength() const { return copy_length; }
  int GetExeLength() const { return exe_length; }
  int GetMinLength() const { return Min(exe_length, copy_length); }
  double GetMerit() const { return merit; }
  double GetCompMerit() const { return merit / (double) GetMinLength(); }
  int GetGestTime() const { return gest_time; }
  double GetEfficiency() const
    { return ((double) GetMinLength()) / (double) gest_time; }
  double GetFitness() const { return fitness; }
  double GetDivType() const { return div_type; }
  int GetUpdateBorn() const { return update_born; }
  int GetUpdateDead() const { return update_dead; }
  int GetDepth() const { return depth; }

  const cString & GetParentMuts() const { return parent_muts; }

  double GetFracDead() const;
  double GetFracNeg() const;
  double GetFracNeut() const;
  double GetFracPos() const;

  double GetFitnessRatio() const { return fitness_ratio; }
  double GetEfficiencyRatio() const { return efficiency_ratio; }
  double GetCompMeritRatio() const { return comp_merit_ratio; }

  const cString & GetTaskOrder() const { return task_order; }
  cString GetTaskList() const;

  cString GetSequence() const { return genome.AsString(); }
  cString GetHTMLSequence() const;

  cString GetMapLink() const {
    return cStringUtil::Stringf("<a href=\"tasksites.%s.html\">Phenotype Map</a>", GetName()());
  }

  int GetNumTasks() const { return task_counts.GetSize(); }
  int GetTaskCount(int task_id) const {
    if (task_id >= task_counts.GetSize()) return 0;
    if (special_args.HasString("binary")) return (task_counts[task_id] > 0);
    return task_counts[task_id];
  }

  // Comparisons...  Compares a genotype to the "previous" one, which is
  // passed in, in one specified phenotype.
  // Return values are:
  //   -2 : Toggle; no longer has phenotype it used to...
  //   -1 : Reduction in strength of phenotype
  //    0 : Identical in phenotype
  //   +1 : Improvement in phenotype
  //   +2 : Toggle; phenotype now present that wasn't.
  int CompareNULL(cAnalyzeGenotype * prev) const { (void) prev; return 0; }
  int CompareArgNULL(cAnalyzeGenotype * prev, int i) const
    { (void) prev; (void) i;  return 0; }
  int CompareLength(cAnalyzeGenotype * prev) const {
    if (GetLength() < MIN_CREATURE_SIZE &&
	prev->GetLength() > MIN_CREATURE_SIZE) return -2;
    if (GetLength() > MIN_CREATURE_SIZE &&
	prev->GetLength() < MIN_CREATURE_SIZE) return 2;
    return 0;
  }
  int CompareMerit(cAnalyzeGenotype * prev) const
    { return NumCompare(GetMerit(), prev->GetMerit()); }
  int CompareCompMerit(cAnalyzeGenotype * prev) const
    { return NumCompare(GetCompMerit(), prev->GetCompMerit()); }
  int CompareGestTime(cAnalyzeGenotype * prev) const {
    const int max_time = CalcMaxGestation();
    const int cur_time = max_time - GetGestTime();
    const int prev_time = max_time - prev->GetGestTime();
    return NumCompare(cur_time, prev_time);
  }
  int CompareEfficiency(cAnalyzeGenotype * prev) const
    { return NumCompare(GetEfficiency(), prev->GetEfficiency()); }
  int CompareFitness(cAnalyzeGenotype * prev) const
    { return NumCompare(GetFitness(), prev->GetFitness()); }
  int CompareTaskCount(cAnalyzeGenotype * prev, int task_id) const
    { return NumCompare(GetTaskCount(task_id), prev->GetTaskCount(task_id)); }

};

////////////////////////

class cAnalyzeCommand {
protected:
  cString command;
  cString args;
public:
  cAnalyzeCommand(const cString & _command, const cString & _args)
    : command(_command), args(_args) { command.ToUpper(); }
  virtual ~cAnalyzeCommand() { ; }

  const cString & GetCommand() { return command; }
  const cString & GetArgs() const { return args; }
  cString GetArgs() { return args; }
  virtual tList<cAnalyzeCommand> * GetCommandList() { return NULL; }
};

class cAnalyzeFlowCommand : public cAnalyzeCommand {
protected:
  tList<cAnalyzeCommand> command_list;
public:
  cAnalyzeFlowCommand(const cString & _command, const cString & _args)
    : cAnalyzeCommand(_command, _args) { ; }
  virtual ~cAnalyzeFlowCommand() {
    while ( command_list.GetSize() > 0 ) delete command_list.Pop();
  }

  tList<cAnalyzeCommand> * GetCommandList() { return &command_list; }
};

///////////////////////////////

class cAnalyze;

class cAnalyzeCommandDefBase {
protected:
  cString name;
public:
  cAnalyzeCommandDefBase(const cString & _name) : name(_name) { ; }
  virtual ~cAnalyzeCommandDefBase() { ; }

  virtual void Run(cAnalyze * analyze, const cString & args,
		   cAnalyzeCommand & command) const = 0;
  virtual bool IsFlowCommand() { return false; }

  const cString & GetName() const { return name; }
};

class cAnalyzeCommandDef : public cAnalyzeCommandDefBase {
private:
  void (cAnalyze::*CommandFunction)(cString);
public:
  cAnalyzeCommandDef(const cString & _name, void (cAnalyze::*_cf)(cString))
    : cAnalyzeCommandDefBase(_name), CommandFunction(_cf) { ; }
  virtual ~cAnalyzeCommandDef() { ; }

  virtual void Run(cAnalyze * analyze, const cString & args,
		   cAnalyzeCommand & command) const {
    (void) command; // used in other types of command defininitions.
    (analyze->*CommandFunction)(args);
  }
};

class cAnalyzeFlowCommandDef : public cAnalyzeCommandDefBase {
private:
  void (cAnalyze::*CommandFunction)(cString, tList<cAnalyzeCommand> &);
public:
  cAnalyzeFlowCommandDef(const cString &_name,
	 void (cAnalyze::*_cf)(cString, tList<cAnalyzeCommand> &))
    : cAnalyzeCommandDefBase(_name), CommandFunction(_cf) { ; }
  virtual ~cAnalyzeFlowCommandDef() { ; }
			 
  virtual void Run(cAnalyze * analyze, const cString & args,
		   cAnalyzeCommand & command) const {
    (analyze->*CommandFunction)(args, *(command.GetCommandList()) );
  }

  virtual bool IsFlowCommand() { return true; }
};

////////////////////////

class cAnalyzeFunction {
private:
  cString name;
  tList<cAnalyzeCommand> command_list;
public:
  cAnalyzeFunction(const cString & _name) : name(_name) { ; }
  ~cAnalyzeFunction() { 
    while ( command_list.GetSize() > 0 ) delete command_list.Pop();
  }

  const cString & GetName() { return name; }
  tList<cAnalyzeCommand> * GetCommandList() { return &command_list; }
};

class cGenotypeBatch {
private:
  tList<cAnalyzeGenotype> genotype_list;
  cString name;
  bool is_lineage;
  bool is_aligned;
public:
  cGenotypeBatch() : name(""), is_lineage(false), is_aligned(false) { ; }
  ~cGenotypeBatch() { ; }

  tList<cAnalyzeGenotype> & List() { return genotype_list; }
  cString & Name() { return name; }
  bool IsLineage() { return is_lineage; }
  bool IsAligned() { return is_aligned; }

  void SetLineage(bool _val=true) { is_lineage = _val; }
  void SetAligned(bool _val=true) { is_aligned = _val; }

};

class cAnalyze {
private:
  int cur_batch;
  cGenotypeBatch batch[MAX_BATCHES];
  tList<cAnalyzeCommand> command_list;
  tList<cAnalyzeFunction> function_list;
  tList<cAnalyzeCommandDefBase> command_lib;
  cString variables[26];
  cString local_variables[26];
  cString arg_variables[10];

  bool verbose;            // Should details be output to command line?
  int interactive_depth;   // How nested are we if in interactive mode?

  cDataFileManager data_file_manager;
  tList< tDataEntryBase<cAnalyzeGenotype> > genotype_data_list;

  cInstSet & inst_set;
  cRandom random;

private:
  // Pop specific types of arguments from an arg list.
  cString PopDirectory(cString & in_string, const cString & default_dir);
  int PopBatch(const cString & in_string);
  cAnalyzeGenotype * PopGenotype(cString gen_desc, int batch_id=-1);
  cString & GetVariable(const cString & varname);

  // Other arg-list methods
  void LoadCommandList(cInitFile & init_file, tList<cAnalyzeCommand> & clist);
  void InteractiveLoadCommandList(tList<cAnalyzeCommand> & clist);
  void PreProcessArgs(cString & args);
  void ProcessCommands(tList<cAnalyzeCommand> & clist);

  void SetupGenotypeDataList();	
  void LoadGenotypeDataList(cStringList arg_list,
	    tList< tDataEntryCommand<cAnalyzeGenotype> > & output_list);
		      
  void AddLibraryDef(const cString & name, void (cAnalyze::*_fun)(cString));
  void AddLibraryDef(const cString & name,
	     void (cAnalyze::*_fun)(cString, tList<cAnalyzeCommand> &));
  cAnalyzeCommandDefBase * FindAnalyzeCommandDef(const cString & name);
  void SetupCommandDefLibrary();
  bool FunctionRun(const cString & fun_name, cString args);

  // Batch management...
  int BatchUtil_GetMaxLength(int batch_id=-1);

  // Comamnd helpers...
  void CommandDetail_Header(std::ostream & fp, int format_type,
	    tListIterator< tDataEntryCommand<cAnalyzeGenotype> > & output_it,
	    int time_step=-1);
  void CommandDetail_Body(std::ostream & fp, int format_type,
	    tListIterator< tDataEntryCommand<cAnalyzeGenotype> > & output_it,
	    int time_step=-1, int max_time=1);
private:
  // Loading methods...
  void LoadOrganism(cString cur_string);
  void LoadBasicDump(cString cur_string);
  void LoadDetailDump(cString cur_string);
  void LoadSequence(cString cur_string);
  void LoadDominant(cString cur_string);
  void LoadFile(cString cur_string);

  // Reduction
  void FindGenotype(cString cur_string);
  void FindLineage(cString cur_string);
  void FindClade(cString cur_string);
  void SampleOrganisms(cString cur_string);
  void SampleGenotypes(cString cur_string);
  void KeepTopGenotypes(cString cur_string);

  // Direct Output Commands...
  void CommandPrint(cString cur_string);
  void CommandTrace(cString cur_string);
  void CommandPrintTasks(cString cur_string);
  void CommandDetail(cString cur_string);
  void CommandDetailTimeline(cString cur_string);
  void CommandDetailBatches(cString cur_string);
  void CommandDetailIndex(cString cur_string);

  // Population Analysis Commands...
  void CommandPrintPhenotypes(cString cur_string);
  void CommandPrintDiversity(cString cur_string);

  // Individual Organism Analysis...
  void CommandLandscape(cString cur_string);
  void CommandFitnessMatrix(cString cur_string);
  void CommandMapTasks(cString cur_string);
  void CommandAverageModularity(cString cur_string);
  void CommandMapMutations(cString cur_string);

  // Population Comparison Commands...
  void CommandHamming(cString cur_string);
  void CommandLevenstein(cString cur_string);
  void CommandSpecies(cString cur_string);

  // Lineage Analysis Commands...
  void CommandAlign(cString cur_string);

  // Build Input Files for Avida
  void WriteClone(cString cur_string);
  void WriteInjectEvents(cString cur_string);

  // Automated analysis...
  void AnalyzeMuts(cString cur_string);
  void AnalyzeInstructions(cString cur_string);
  void AnalyzeBranching(cString cur_string);
  void AnalyzeMutationTraceback(cString cur_string);
  void AnalyzeComplexity(cString cur_string);
  void AnalyzePopComplexity(cString cur_string);

  // Environment Manipulation
  void EnvironmentSetup(cString cur_string);

  // Documentation...
  void CommandHelpfile(cString cur_string);
  void CommandDocfile(cString cur_string);

  // Control...
  void VarSet(cString cur_string);
  void BatchSet(cString cur_string);
  void BatchName(cString cur_string);
  void BatchTag(cString cur_string);
  void BatchPurge(cString cur_string);
  void BatchDuplicate(cString cur_string);
  void BatchRecalculate(cString cur_string);
  void BatchRename(cString cur_string);
  void PrintStatus(cString cur_string);
  void PrintDebug(cString cur_string);
  void ToggleVerbose(cString cur_string);
  void IncludeFile(cString cur_string);
  void CommandSystem(cString cur_string);
  void CommandInteractive(cString cur_string);

  // Functions...
  void FunctionCreate(cString cur_string, tList<cAnalyzeCommand> & clist);

  // Flow Control...
  void CommandForeach(cString cur_string, tList<cAnalyzeCommand> & clist);
  void CommandForRange(cString cur_string, tList<cAnalyzeCommand> & clist);

public:
  cAnalyze(cString filename);
  ~cAnalyze();
};

#endif
