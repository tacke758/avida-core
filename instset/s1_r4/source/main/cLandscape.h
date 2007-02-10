/*
 *  cLandscape.h
 *  Avida
 *
 *  Called "landscape.hh" prior to 12/5/05.
 *  Copyright 2005-2006 Michigan State University. All rights reserved.
 *  Copyright 1993-2003 California Institute of Technology.
 *
 */

#ifndef cLandscape_h
#define cLandscape_h

#ifndef cCPUTestInfo_h
#include "cCPUTestInfo.h"
#endif
#ifndef cGenome_h
#include "cGenome.h"
#endif
#ifndef cString_h
#include "cString.h"
#endif
#ifndef tMatrix_h
#include "tMatrix.h"
#endif

class cAvidaContext;
class cDataFile;
class cInstSet;
class cInstruction;
class cTestCPU;
class cWorld;

class cLandscape
{
private:
  cWorld* m_world;
  const cInstSet& inst_set;
  cGenome base_genome;
  cGenome peak_genome;
  double base_fitness;
  double base_merit;
  double base_gestation;
  double peak_fitness;
  double total_fitness;
  double total_sqr_fitness;

  int distance;

  int trials;
  int m_min_found;
  int m_max_trials;

  int total_count;

  int dead_count;
  int neg_count;
  int neut_count;
  int pos_count;

  double pos_size; 
  double neg_size; 

  int total_epi_count;
  int pos_epi_count;
  int neg_epi_count;
  int no_epi_count;
  int dead_epi_count;

  double pos_epi_size; 
  double neg_epi_size; 
  double no_epi_size; 

  int* site_count;

  double total_entropy;
  double complexity;

  cCPUTestInfo test_info;  // Info used for all cpu calculations.
  double neut_min;         // These two variables are a range around the base
  double neut_max;         //   fitness to be counted as neutral mutations.
  tMatrix<double> fitness_chart; // Chart of all one-step mutations.
  
  int m_num_found;


  void BuildFitnessChart(cAvidaContext& ctx, cTestCPU* testcpu);
  void ProcessGenome(cAvidaContext& ctx, cTestCPU* testcpu, cGenome& in_genome);
  void ProcessBase(cAvidaContext& ctx, cTestCPU* testcpu);
  void Process_Body(cAvidaContext& ctx, cTestCPU* testcpu, cGenome& cur_genome, int cur_distance, int start_line);

  double TestMutPair(cAvidaContext& ctx, cTestCPU* testcpu, cGenome& mod_genome, int line1, int line2,
                     const cInstruction& mut1, const cInstruction& mut2);

  cLandscape(); // @not_implemented
  cLandscape(const cLandscape&); // @not_implemented
  cLandscape& operator=(const cLandscape&); // @not_implemented

public:
  cLandscape(cWorld* world, const cGenome& in_genome, const cInstSet& in_inst_set);
  ~cLandscape();

  void Reset(const cGenome& in_genome);

  void Process(cAvidaContext& ctx);
  void ProcessDelete(cAvidaContext& ctx);
  void ProcessInsert(cAvidaContext& ctx);
  void PredictWProcess(cAvidaContext& ctx, cDataFile& df, int update = -1);
  void PredictNuProcess(cAvidaContext& ctx, cDataFile& df, int update = -1);
  
  inline void SetDistance(int in_distance) { distance = in_distance; }
  inline void SetTrials(int in_trials) { trials = in_trials; }
  inline void SetMinFound(int min_found) { m_min_found = min_found; }
  inline void SetMaxTrials(int max_trials) { m_max_trials = max_trials; }

  void SampleProcess(cAvidaContext& ctx);
  void RandomProcess(cAvidaContext& ctx);
  
  inline int GetNumFound() { return m_num_found; }

  void TestPairs(cAvidaContext& ctx);
  void TestAllPairs(cAvidaContext& ctx);

  void HillClimb(cAvidaContext& ctx, cDataFile& df);

  void PrintStats(cDataFile& df, int update = -1);
  void PrintEntropy(cDataFile& fp);
  void PrintSiteCount(cDataFile& fp);

  inline const cGenome& GetPeakGenome() { return peak_genome; }
  inline double GetAveFitness() { return total_fitness / total_count; }
  inline double GetAveSqrFitness() { return total_sqr_fitness / total_count; }
  inline double GetPeakFitness() { return peak_fitness; }

  inline double GetProbDead() const { return static_cast<double>(dead_count) / total_count; }
  inline double GetProbNeg()  const { return static_cast<double>(neg_count) / total_count; }
  inline double GetProbNeut() const { return static_cast<double>(neut_count) / total_count; }
  inline double GetProbPos()  const { return static_cast<double>(pos_count) / total_count; }
  inline double GetAvPosSize() const { if (pos_count == 0) return 0; else return pos_size / pos_count; }
  inline double GetAvNegSize() const { if (neg_count == 0) return 0; else return neg_size / neg_count; }
  inline double GetProbEpiDead() const
  {
    if (total_epi_count == 0) return 0; else return static_cast<double>(dead_epi_count) / total_epi_count;
  }
  inline double GetProbEpiPos()  const
  { 
    if (total_epi_count == 0) return 0; else return static_cast<double>(pos_epi_count) / total_epi_count;
  }
  inline double GetProbEpiNeg()  const
  { 
    if (total_epi_count == 0) return 0; else return static_cast<double>(neg_epi_count) / total_epi_count;
  }
  inline double GetProbNoEpi() const
  {
    if (total_epi_count == 0) return 0; else return static_cast<double>(no_epi_count) / total_epi_count;
  }
  inline double GetAvPosEpiSize() const { if (pos_epi_count == 0) return 0; else return pos_epi_size / pos_epi_count; }
  inline double GetAvNegEpiSize() const { if (neg_epi_count == 0) return 0; else return neg_epi_size / neg_epi_count; }
  inline double GetAvNoEpiSize() const { if (no_epi_count == 0) return 0; else return no_epi_size / no_epi_count; }

  inline int GetNumTrials() const { return trials; }
  inline double GetTotalEntropy() const { return total_entropy; }
  inline double GetComplexity() const { return complexity; }
};


#ifdef ENABLE_UNIT_TESTS
namespace nLandscape {
  /**
   * Run unit tests
   *
   * @param full Run full test suite; if false, just the fast tests.
   **/
  void UnitTests(bool full = false);
}
#endif  

#endif
