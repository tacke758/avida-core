/* stat.hh ********************************************************************
 Statistical Tools

 charles@krl.caltech.edu & travc@ugcs.caltech.edu
 Time-stamp: <98/07/29 23:55:35 travc>
 
 cRunningSum - Maintain a Sum, Sum or Squares, and Count; 
               Calculates Average and Variance
   cRunningSum(): Constructor.  Clears state
   void Clear() : zeros all internal variables (done on construction)
   double Sum()
   double SumOfSquares()
   double Count()
   double Average()
   double Variance()
   void Add(double value, double weight=1)  
   void Subtract(double value, double weight=1)  
*/

#ifndef STAT_HH
#define STAT_HH

#include <math.h>
#include <limits.h>
#include "assert.hh"
#include "debug.hh"

class cRunningSum {
private:
  double sum;
  double sum_of_squares;
  double count;

public:
  static const double INF_ERR;  // Value Returned by StdError if Infinate

  cRunningSum(){ Clear(); }
  
  void Clear(){ sum=0; sum_of_squares=0; count=0; }
  
  double Sum() const { return sum; }
  double SumOfSquares() const { return sum_of_squares; }
  double Count() const { return count; }
  
  double Average() const { return count ? (sum/count) : 0; }
  double Variance() const { 
    return (sum_of_squares/count)-(Average()*Average()); }
  double StdDeviation() const { return sqrt(Variance()); }
  double StdError()  const {
    return (count > 1) ? sqrt(sum_of_squares / (count * (count-1))) : INF_ERR;
  }

  void Add(double value, double weight=1){ 
    sum += value*weight;
    sum_of_squares += value*weight*value*weight;
    count += weight;
  }

  void Subtract(double value, double weight=1){ 
    sum -= value*weight;
    sum_of_squares -= value*weight*value*weight;
    count -= weight;
  }
};



class cRunningAverage {
private:
  double * value;  // Array of actual values
  int size;        // Size of sliding window

public:
  /// NOT DONE YET

};



class cHistogram {
private:
  int * bins;
  int min_bin;
  int max_bin;
  int entry_count;
  int entry_total;
public:
  cHistogram(int in_max=1, int in_min=0);
  inline ~cHistogram() { delete [] bins; }

  void Resize(int new_max, int new_min=0);
  void Print();
  inline void Clear();
  inline void Insert(int value, int count=1);
  inline void Remove(int value);
  inline void RemoveBin(int value);

  inline double GetAverage()
    { return ((double)entry_total) / (double)entry_count; }
  inline double GetCountAverage()
    { return (double) entry_count / (double) GetNumBins(); }
  inline int GetMode();
  inline double GetVariance();
  inline double GetCountVariance();
  inline double GetStdDev();
  inline double GetCountStdDev();
  inline double GetEntropy();
  inline double GetNormEntropy();

  inline int GetCount() { return entry_count; }
  inline int GetCount(int value) { return bins[value - min_bin]; }
  inline int GetTotal() { return entry_total; }
  inline int GetMinBin() { return min_bin; }
  inline int GetMaxBin() { return max_bin; }
  inline int GetNumBins() { return max_bin - min_bin + 1; }
};

inline void cHistogram::Clear()
{
  int num_bins = GetNumBins();
  for (int i = 0; i < num_bins; i++) {
    bins[i] = 0;
  }
  entry_count = 0;
  entry_total = 0;
}


inline void cHistogram::Insert(int value, int count)
{
  if (value > max_bin || value < min_bin) {
    fprintf(stderr, "Trying to insert %d into Histogram of range [%d,%d]\n",
	    value, min_bin, max_bin);
  }

  bins[value - min_bin] += count;
  entry_count += count;
  entry_total += value * count;
}

inline void cHistogram::Remove(int value)
{
#ifdef DEBUG
  if (value > max_bin || value < min_bin) {
    fprintf(stderr,
	    "WARNING: Trying to remove %d in histogram of range [%d, %d]\n",
	    value, min_bin, max_bin);
    return;
  }
  if (bins[value] <= 1) {
    fprintf(stderr,
	    "WARNING: Trying to remove %d from bin with %d entries\n",
	    value, bins[value]);
    return;
  }
#endif

  bins[value - min_bin]--;
  entry_count--;
  entry_total -= value;
}

inline void cHistogram::RemoveBin(int value)
{
#ifdef DEBUG
  if (value > max_bin || value < min_bin) {
    fprintf(stderr,
	    "WARNING: Trying to remove %d in histogram of range [%d, %d]\n",
	    value, min_bin, max_bin);
    return;
  }
#endif

  int old_size = bins[value - min_bin];
  bins[value - min_bin] = 0;

  entry_count -= old_size;
  entry_total -= value * old_size;
}

inline int cHistogram::GetMode()
{
  int num_bins = GetNumBins();
  int mode = 0;
  for (int i = 1; i < num_bins; i++) {
    if (bins[i] > bins[mode]) mode = i;
  }

  return mode + min_bin;
}

inline double cHistogram::GetVariance()
{
  if (entry_count < 2) return 0;

  double mean = GetAverage();
  double var = 0;
  double value = 0;

  int num_bins = GetNumBins();
  for (int i = 0; i < num_bins; i++) {
    value = (double) (i + min_bin) - mean;
    var += bins[i] * value * value;
  }
  var /= entry_count - 1;

  return var;
}

inline double cHistogram::GetCountVariance()
{
  int num_bins = GetNumBins();
  if (num_bins < 2) return 0;

  double mean = GetCountAverage();
  double var = 0;
  double value = 0;

  for (int i = 0; i < num_bins; i++) {
    value = (double) bins[i] - mean;
    var += value * value;
  }
  var /= num_bins - 1;

  return var;
}

inline double cHistogram::GetStdDev()
{
  return sqrt(GetVariance());
}

inline double cHistogram::GetCountStdDev()
{
  return sqrt(GetCountVariance());
}

inline double cHistogram::GetEntropy()
{
  int num_bins = GetNumBins();

  double entropy = 0.0;
  double prob = 0.0;

  for (int i = 0; i < num_bins; i++) {
    prob = (double) bins[i] / (double) entry_count;
    entropy -= prob * log(prob);
  }

  return entropy;
}

inline double cHistogram::GetNormEntropy()
{
  int num_bins = GetNumBins();

  double entropy = 0.0;
  double prob = 0.0;

  for (int i = 0; i < num_bins; i++) {
    prob = ((double) bins[i]) / (double) entry_count;
    if (prob != 0.0) entropy -= prob * log(prob);
  }

  return entropy / log((double) num_bins);
}

#endif
