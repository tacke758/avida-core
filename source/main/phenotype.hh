//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef PHENOTYPE_HH
#define PHENOTYPE_HH

// For the moment, bonuses are static.  This should be fixed!!

// Bonus base is the initial bonus value
const double BONUS_BASE = 1;

#include "../tools/tArray.hh"
#include "stats.hh"
#include "tasks.hh"
#include "merit.hh"

class sReproData;
class sCreatureInfo;

class cPhenotype {
private:
  const cTaskLib * task_lib;
  cTaskCount task_count;

  double merit_base;
  double bonus;      // the task bonuses

  cMerit merit;

  // Info the merit is based on (for save and restore)
  tArray<int> last_tasks;
  double last_merit_base;
  double last_gestation_time;

public:
  cPhenotype(const cTaskLib * _task_lib);
  ~cPhenotype();

    /**
     * This function is run whenever a new creature takes over the
     * CPU. It erases all of the data from the previous creatures.
     **/
    void Clear(double base_merit, double initial_bonus = BONUS_BASE);

    /**
     * The following function is run whenever a new creature takes over the
     * CPU. It erases all of the data from the previous creatures.
     * Propagates information to "last" variables.
     **/
    void Clear(const sReproData & repro_data);

    void Clear(const sReproData & repro_data, const cMerit & parent_merit);
  // void Clear(int in_size, const cMerit & parent_merit); // Deprecated

    /**
     * This function is run whenever a creature executes a successful divide.
     * It sets the creature up for the next gestation cycle without any change
     * in what creature controls this CPU.
     **/
    void DivideReset(const sReproData & repro_data);

    /**
     * This function runs whenever a *test* CPU divides. It processes much of
     * the information for that CPU in order to actively reflect its executed
     * and copied size in its merit.
     **/
    void TestDivideReset(int _size, int _copied_size, int _exe_size);

    /**
     * Input Task Test (to be called from hardware!)
     **/
    void TestInput(cIOBuf & inputs, cIOBuf & outputs){
	bonus = task_lib->TestInput(inputs, outputs, task_count, bonus); }

    /**
     * Output Task Test (to be called from hardware!)
     **/
    void TestOutput(cIOBuf & inputs, cIOBuf & outputs){
	bonus = task_lib->TestOutput(inputs, outputs, task_count, bonus); }

  // Accessors
  inline const cTaskLib * GetTaskLib(){ return task_lib; };
  inline int GetNumTasks() const { return task_count.GetNumTasks(); }
  inline const cTaskCount & GetTaskCount() const { return task_count; }
  inline int GetTaskCount(int task_num) const { return task_count[task_num]; }
  inline const cString & GetTaskKeyword(int task_num) const
    { return task_lib->GetTaskKeyword(task_num); }
  inline int GetParentTaskCount(int task_num) const
    { return last_tasks[task_num]; }
  inline double GetMeritBase() const { return merit_base; }

  // This is a crappy function... come up with a better way @TCC
  inline void SetMerit(double in_merit) { merit = in_merit; }
  inline void SetMerit(cMerit in_merit) { merit = in_merit; }

  inline const cMerit & GetMerit() const { return merit; }
  inline const cMerit GetCurMerit() const {
    return cMerit( merit_base * bonus ); }
  inline const double GetBonus() const { return bonus; }

  inline double GetFitness(){ return merit.GetDouble() / last_gestation_time; }
  inline double GetGestationTime(){ return last_gestation_time; }

  static inline int CalcSizeMerit(int full_size, int copied_size,
				  int exe_size);

  // State saving, loading, and initializing
  cMerit & ReCalcMerit();
  int SaveState(ofstream & fp);
  int LoadState(ifstream & fp);
};




inline int cPhenotype::CalcSizeMerit(int full_size, int copied_size,
				     int exe_size)
{
  int out_size = cConfig::GetBaseSizeMerit();

  switch (cConfig::GetSizeMeritMethod()) {
  case SIZE_MERIT_COPIED:
    out_size = copied_size;
    break;
  case SIZE_MERIT_EXECUTED:
    out_size = exe_size;
    break;
  case SIZE_MERIT_FULL:
    out_size = full_size;
    break;
  case SIZE_MERIT_LEAST:
    out_size = full_size;
    if (out_size > copied_size) out_size = copied_size;
    if (out_size > exe_size)    out_size = exe_size;
    break;
  case SIZE_MERIT_SQRT_LEAST:
    out_size = full_size;
    if (out_size > copied_size) out_size = copied_size;
    if (out_size > exe_size)    out_size = exe_size;
    out_size = (int) sqrt((double) out_size);
    break;
  case SIZE_MERIT_OFF:
  default:
    out_size = 100;
    break;
  }

  return out_size;
}

#endif



