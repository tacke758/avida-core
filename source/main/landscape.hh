//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1998 California Institute of Technology             //
//                                                                          //
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef LANDSCAPE_HH
#define LANDSCAPE_HH

#include "../cpu/cpu.hh"
#include "../cpu/test_cpu.hh"
#include "../tools/tArray.hh"

#define LANDSCAPE_NEUTRAL_RANGE (1.0/10.0) // 3600.00
#define LANDSCAPE_NEUTRAL_MIN (1.0 - LANDSCAPE_NEUTRAL_RANGE)
#define LANDSCAPE_NEUTRAL_MAX (1.0 + LANDSCAPE_NEUTRAL_RANGE)

struct cLandscape {
private:
  cInstLib * inst_lib;
  cCodeArray base_code;
  cCodeArray peak_code;
  double base_fitness;
  double base_merit;
  double base_gestation;
  double peak_fitness;
  double total_fitness;
  double total_sqr_fitness;

  int distance;
  int del_distance;
  int ins_distance;

  int trials;

  int total_count;

  int dead_count;
  int neg_count;
  int neut_count;
  int pos_count;

  int * site_count;

  double total_entropy;
  double complexity;

  cCPUTestInfo test_info;  // Info used for all cpu calculations.
  double neut_min;         // These two variables are a range around the base
  double neut_max;         //   fitness to be counted as neutral mutations.
  tMatrix<double> fitness_chart; // Chart of all one-step mutations.

private:
  void BuildFitnessChart();
  void ProcessCode(cCodeArray & in_code);
  void ProcessBase();
  void Process_Body(cCodeArray & cur_code, int cur_distance, int start_line,
          ostream * fp = NULL);

  void HillClimb_Body(ofstream & fp, cCodeArray & cur_code, int & gen);
  void HillClimb_Print(ofstream & fp, const cCodeArray & _code, const int gen) const;
  void TestMutPair(cCodeArray & mod_code, int line1, int line2,
    const cInstruction & mut1, const cInstruction & mut2, ostream & fp);
public:
  cLandscape(const cCodeArray & in_code, cInstLib * in_inst_lib);
  ~cLandscape();

  void Reset(const cCodeArray & in_code);

  void Process(int in_distance=1, ostream * fp = NULL);
  void ProcessDelete();
  void ProcessInsert();

  void SampleProcess(int in_trials);
  void RandomProcess(int in_trials, int in_distance=1, int min_found=0,
		     int max_trials=0, int print_if_found=FALSE);

  void TestPairs(int in_trials, ostream & fp);
  void TestAllPairs(ostream & fp);

  void HillClimb(ofstream & fp);
  
  void PrintStats(ofstream & fp);
  void PrintEntropy(ofstream & fp);
  void PrintSiteCount(ofstream & fp);
  void PrintBase(cString filename);
  void PrintPeak(cString filename);

  inline const cCodeArray & GetPeakCode() { return peak_code; }
  inline double GetAveFitness() { return total_fitness / total_count; }
  inline double GetAveSqrFitness() { return total_sqr_fitness / total_count; }
  inline double GetPeakFitness() { return peak_fitness; }

  inline double GetProbDead() { return ((double) dead_count) / total_count; }
  inline double GetProbNeg()  { return ((double) neg_count)  / total_count; }
  inline double GetProbNeut() { return ((double) neut_count) / total_count; }
  inline double GetProbPos()  { return ((double) pos_count)  / total_count; }
};

#endif