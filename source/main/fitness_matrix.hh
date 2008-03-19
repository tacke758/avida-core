#ifndef FITNESS_MATRIX_HH
#define FITNESS_MATRIX_HH


#include <iostream>
#include <set>
#include <vector>
#include <map>
#include <list>
#include <sys/timeb.h>

#include "../cpu/cpu.hh"
#include "../cpu/test_cpu.hh"
#include "../defs.hh"
#include "../tools/tools.hh"
#include "stats.hh"
#include "code_array.hh"
#include "classes.hh"

using namespace std;


/**
 * This class stores the genome of an Avida organism only.
 * Cannot be run in a simulation.
 **/

class MyCodeArrayLessThan;

class cMxCodeArray {
private:
  int size;
  int max_size;
  cInstruction * data;
  double m_merit;
  int m_gestation_time;
  static const int m_max_gestation_time;
  int m_num_instructions;
  mutable map<int, double, less<int> > m_trans_probs;


public:
  cMxCodeArray();
  explicit cMxCodeArray(int ninst, int in_size=0, int in_max_size=0);
  cMxCodeArray(const cMxCodeArray &in_code);
  cMxCodeArray(const cCodeArray &in_code, int in_max_size=0);
  virtual ~cMxCodeArray();

  void operator=(const cMxCodeArray &other_code);
  int operator==(const cMxCodeArray &other_code) const;
  inline cInstruction & operator[](int index);
  bool operator<(const cMxCodeArray &other_code) const;

  void CopyDataTo(cCodeArray & other_code) const;
  void Resize(int new_size);
  void ResetSize(int new_size, int new_max=0);
  inline int GetSize() const { return size; }
  inline double GetFitness() const {
    if ( m_gestation_time == m_max_gestation_time )
      return 0;
    else return m_merit/m_gestation_time;}
  double GetMerit() const { return m_merit; }
  double GetGestationTime() const { return m_gestation_time; }
  void CalcFitness();

  inline const cInstruction & Get(int index) const {
#ifdef DEBUG
    assert(index < size);
    if (index >= size) g_debug.Error("Get(%d) in code size %d", index, size);
#endif
    return data[index];
  }

  int FindInstruction(const cInstruction & inst, int start_index=0);
  inline int HasInstruction(const cInstruction & inst){
    return ( FindInstruction(inst) >= 0 ) ? TRUE : FALSE; }

  inline void Set(int index, const cInstruction & value);

  void Reset();
  void Clear();
  void Randomize();
  int OK() const;
  cString AsString() const;

  cString DebugType() { return "cMxCodeArray"; }
  cString DebugInfo() { return cString::Stringf("size = %d", size); }
  int DebugSize() { return max_size; } // cString::Stringf("max_size = %d", max_size); }



  /* For Depth Limited Search */
  void Get1Mutants(list<cMxCodeArray>& mutants) const;
  void SetNumInstructions(int ninst) {m_num_instructions = ninst;}


  /* For Transition Matrix */
  int MakeTransitionList(const set<cMxCodeArray, MyCodeArrayLessThan >& m_data_set, int hamDistThresh, double errorRate) const;
  const map<int, double, less<int> >& GetTransitionList() const;
  int HammingDistance(const cMxCodeArray &other_gene) const;
  double TransitionProbability(const cMxCodeArray &other_gene, double errorRate) const;
  void PrintTransitionList(ostream &fp, int size) const;
};



class MyCodeArrayLessThan{
public:
  bool operator()(const cMxCodeArray &x, const cMxCodeArray &y) const
  {
    return x < y;
  }
};


class cFitnessMatrix {
private:

  /* genome data */
  cMxCodeArray m_start_genotype;
  cInstLib *m_inst_lib;
  set<cMxCodeArray, MyCodeArrayLessThan > m_data_set;
  double m_fitness_threshhold;

  /* parameters for search */
  int m_depth_limit;
  double m_fitness_threshold_ratio;

  /* parameters for diagonalization */
  int m_ham_thresh;
  double m_error_rate_min;
  double m_error_rate_max;
  double m_error_rate_step;
  int m_diag_iters;


  /* statistics of search */
  vector<int> m_DFSNumDead;
  vector<int> m_DFSNumBelowThresh;
  vector<int> m_DFSNumOK;
  vector<int> m_DFSNumNew;
  vector<int> m_DFSNumVisited;
  vector<int> m_DFSDepth;
  int m_DFS_MaxDepth;
  int m_DFS_NumRecorded;
  time_t m_search_start_time;
  time_t m_search_end_time;


  /* Methods for Depth-Limited Search of Genotype Space */

  void DepthLimitedSearch(const cMxCodeArray& startNode, ofstream& log_file, int currDepth=0);
  bool MutantOK(double fitness);
  void CollectData(ofstream& log_file);


  /* Methods for Diagonalization of Transition Matrix */

  double Diagonalize(vector<double>& randomVect, int hamDistThresh, double errorRate, ofstream& logfile);
  void MakeRandomVector(vector<double>& newVect, int size);
  void VectorDivideBy(vector<double>& vect, double div);
  double VectorNorm(const vector<double> &vect);
  void MatrixVectorMultiply(const vector<double>& vect, vector<double>& result);
  void MakeTransitionProbabilities(int hamDistThresh, double errorRate, ofstream& logfile);


  /* Data Output */

  void PrintGenotypes(ostream &fp);
  void PrintTransitionMatrix(ostream& fp, int hamDistThresh, double errorRate, double avg_fitness, bool printMatrix=false);
  void PrintHammingVector(ostream& fp,const vector<double>& dataVect, double errProb, double avgFit);
  void PrintFitnessVector(ostream& fp,const vector<double>& dataVect, double errProb, double avgFit, double maxFit, double step);
  void PrintFullVector(ostream& fp, const vector<double>& dataVect, double errProb, double avgFit);


public:
  cFitnessMatrix(const cCodeArray,  cInstLib * inst_lib);
  ~cFitnessMatrix();

  /**
   * The main entry function.
   *
   * @param depth_limit Limits the depth of the search
   * (how far should we go out in Hamming distance).
   *
   * @param fitness_threshold_ratio Creatures with fitnesses below the
   * starting fitness times this value are rejected
   *
   * @param ham_thresh The threshold for the construction of the matrix
   * (what transitions are included).
   *
   * @param error_rate_min The minimum error rate for which the matrix
   * should be diagonalized.
   *
   * @param error_rate_max The maximum error rate for which the matrix
   * should be diagonalized.
   *
   * @param error_rate_step The interval between two error rates at which
   * the matrix gets diagonalized.
   *
   * @param vect_fmax The maximum fitness to be considered in the output
   * vector. (We output concentrations of genotypes in bins of given fitness
   * width. This is the maximum fitness we consider).
   *
   * @param vect_fstep The width of the fitness bins for output.
   *
   * @param diag_iters The number of iterations for the diagonalization
   * of the matrix.
   *
   * @param write_ham_vector Should we also write a concentration vector
   * grouped according to Hamming distances?
   *
   * @param write_full_vector Should we also write the full concentration
   * vector?
   **/
  void CalcFitnessMatrix( int depth_limit, double fitness_threshold_ratio, int ham_thresh, double error_rate_min, double error_rate_max, double error_rate_step, double vect_fmax, double vect_fstep, int diag_iters, bool write_ham_vector, bool write_full_vector );

};









cInstruction & cMxCodeArray::operator[](int index)
{
#ifdef DEBUG
  assert(index < size);

  if (index >= size) {
    g_debug.Error("UD %d: Trying to index %d into code size %d (op[])",
		  cStats::GetUpdate(), index, size);
  }
  if (index < 0) {
    g_debug.Error("UD %d: operator[](%d) - Index is < 0! (size=%d)",
	    cStats::GetUpdate(), index, size);
    assert(FALSE);
  }
#endif
  return data[index];
}

inline int cMxCodeArray::operator==(const cMxCodeArray &other_code) const
{
  // Make sure the sizes are the same.
  if (size != other_code.size) return FALSE;

  // Then go through line by line.
  for (int i = 0; i < size; i++)
    if (data[i].GetOp() != other_code.data[i].GetOp()) return FALSE;

  return TRUE;
}




inline void cMxCodeArray::Set(int index, const cInstruction & value)
{
  operator[](index) = value;
}


#endif
