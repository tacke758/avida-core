//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef PHENOTYPE_HH
#define PHENOTYPE_HH

// For the moment, bonuses are static.  This should be fixed!!

// BONUS1 is the shift for the first time a task is compleated; BONUS 2 is
// for subsequent times.
#define BONUS1 2
#define BONUS2 1

#define TASK_MERIT_OFF         0
#define TASK_MERIT_EXPONENTIAL 1
#define TASK_MERIT_LINEAR      2

#define SIZE_MERIT_OFF         0
#define SIZE_MERIT_COPIED      1
#define SIZE_MERIT_EXECUTED    2
#define SIZE_MERIT_FULL        3
#define SIZE_MERIT_LEAST       4

#include "stats.hh"

struct sReproData;
struct sCreatureInfo;

class cPhenotype {
private:
  int get_count;
  int put_count;
  int ggp_count;
  
  int get_bonus;
  int put_bonus;
  int ggp_bonus;
  
  cFlags flags0;  // 0 = get, 1 = put, 2 = ggp.

  int task_count[NUM_TASKS];
  int bonus_chart[NUM_TASKS];
  cFlags task_flags;

  cMerit cur_merit;
  cMerit merit;

  inline void AssignTask(int task_num);
public:
  cPhenotype();
  ~cPhenotype();

  void Clear(int in_size);
  void Clear(const sCreatureInfo & cpu_info);
  void Clear(const sCreatureInfo & cpu_info, const cMerit & parent_merit);
  void Clear(const sReproData & repro_data);
  void Clear(const sReproData & repro_data, const cMerit & parent_merit);
  void DivideReset(const sReproData & repro_data);

  inline void AddGet();
  inline void AddPut();
  void SetTasks(int * input_buffer, int buf_size, int value);

  cMerit GetGetBonus() const;
  cMerit GetPutBonus() const;
  cMerit GetGGPBonus() const;
  cMerit GetTaskBonus(int task_num) const;
  
  inline int GetGetCount() const { return get_count; }
  inline int GetPutCount() const { return put_count; }
  inline int GetGGPCount() const { return ggp_count; }
  inline int GetTaskCount(int task_num) const { return task_count[task_num]; }

  inline int GetGetFlag() const { return flags0.GetFlag(TASK_GET); }
  inline int GetPutFlag() const { return flags0.GetFlag(TASK_PUT); }
  inline int GetGGPFlag() const { return flags0.GetFlag(TASK_GGP); }
  inline int GetTaskFlag(int task_num) const
    { return task_flags.GetFlag(task_num); }
  
  inline const cMerit & GetMerit() const { return merit; }
  inline const cMerit & GetCurMerit() const { return cur_merit; }
};

inline void cPhenotype::AddGet()
{
  cur_merit = GetGetBonus();
  get_count++;
  flags0.SetFlag(TASK_GET);
}

inline void cPhenotype::AddPut()
{
  cur_merit = GetPutBonus();
  put_count++;
  flags0.SetFlag(TASK_PUT);

  // Check if this Put completes a Get-Get-Put sequence...

  if (get_count >= 2 && !ggp_count) {
    cur_merit = GetGGPBonus();
    ggp_count++;
    flags0.SetFlag(TASK_GGP);
  }
}

inline void cPhenotype::AssignTask(int task_num)
{
  cur_merit = GetTaskBonus(task_num);
  task_count[task_num]++;
  task_flags.SetFlag(task_num);
}

#endif



