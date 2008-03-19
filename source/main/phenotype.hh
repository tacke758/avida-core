//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef PHENOTYPE_HH
#define PHENOTYPE_HH

// For the moment, bonuses are static.  This should be fixed!!

// Bonus base is the initial bonus value
const double BONUS_BASE = 1;

#include "stats.hh"
#include "tasks.hh"

class sReproData;
class sCreatureInfo;

class cPhenotype {
private:
  const cTaskLib * task_lib;
  cTaskCount task_count;

  double merit_base;
  double bonus;      // the task bonuses

  cMerit merit;
public:
  cPhenotype(const cTaskLib * _task_lib);
  ~cPhenotype();

  void Clear(int in_size);
  void Clear(const sReproData & repro_data);
  void Clear(const sReproData & repro_data, const cMerit & parent_merit);
  void Clear(int in_size, const cMerit & parent_merit);
  void DivideReset(const sReproData & repro_data);
  void TestDivideReset(int _size, int _copied_size, int _exe_size);

  // Input & Output Task Tests (to be called from hardware!)
  void TestInput(cIOBuf & inputs, cIOBuf & outputs){
    bonus = task_lib->TestInput(inputs, outputs, task_count, bonus); } 
  void TestOutput(cIOBuf & inputs, cIOBuf & outputs){
    bonus = task_lib->TestOutput(inputs, outputs, task_count, bonus); }
    
  // Accessors
  inline const cTaskLib * GetTaskLib(){ return task_lib; };
  inline int GetNumTasks() const { return task_count.GetNumTasks(); }
  inline int GetTaskCount(int task_num) const { return task_count[task_num]; }

  inline void SetMerit(cMerit in_merit) { merit = in_merit; } // This is a crappy function... come up with a better way @TCC
  inline const cMerit & GetMerit() const { return merit; }
  inline const cMerit GetCurMerit() const {
    return cMerit( (UINT) (merit_base * bonus + .5) ); }
  inline const double GetBonus() const { return bonus; }

  static inline int CalcSizeMerit(int full_size, int copied_size,
				  int exe_size);
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
    out_size = 1;
    break;
  }

  return out_size;
}

#endif



