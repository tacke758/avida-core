//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
/* stat.hh ********************************************************************
 Statistical Tools

 charles@krl.caltech.edu & travc@ugcs.caltech.edu
 Time-stamp: <2000-08-31 15:25:31 claus>

 cDoubleSum - Maintain a Sum, Sum or Squares, and Count;
               Calculates Average and Variance
   cDoubleSum(): Constructor.  Clears state
   void Clear() : zeros all internal variables (done on construction)
   void Add(double value, double weight=1)
   void Subtract(double value, double weight=1)
   double Count()         |  N()
   double Sum()           |  S1()
   double SumOfSquares()  |  S2()
   double SumOfCubes()    |  S3()
   double S4()
   double Average()       |  Ave()
   double Variance()      |  Var()
   dobule StdDeviation()
   double StdError()
   double Skewness()      |  Skw()
   double Kurtosis()      |  Kur()


 cIntSum - Like RunningSum but for int values
   cIntSum(): Constructor.  Clears state
   void Clear() : zeros all internal variables (done on construction)
   void Add(LONGINT value, LONGINT weight=1)
   void Subtract(LONGINT value, LONGINT weight=1)
   LONGINT Count()         |  N()
   LONGINT Sum()           |  S1()
   LONGINT SumOfSquares()  |  S2()
   LONGINT SumOfCubes()    |  S3()
   LONGINT S4()
   double Average()       |  Ave()
   double Variance()      |  Var()
   dobule StdDeviation()
   double StdError()
   double Skewness()      |  Skw()
   double Kurtosis()      |  Kur()



*/

#ifndef STAT_HH
#define STAT_HH

#include <math.h>
#include <limits.h>
//#include <iostream.h>
// porting to gcc 3.1 -- k
#include <iostream>
#include <assert.h>

class cCountTracker {
private:
  int cur_count;
  int last_count;
  int total_count;
public:
  cCountTracker() { Clear(); }
  ~cCountTracker() { ; }

  int GetCur() const { return cur_count; }
  int GetLast() const { return last_count; }
  int GetTotal() const { return total_count; }

  void Inc();
  void Dec();
  void Next();
  void Clear();
};

class cDoubleSum {
private:
  double s1;  // Sum (x)
  double s2;  // Sum of squares (x^2)
  double s3;  // Sum of cubes (x^3)
  double s4;  // Sum of x^4
  double n;

public:
  static const double INF_ERR;  // Value Returned by StdError if Infinate

  cDoubleSum(){ Clear(); }

  void Clear(){ s1=0; s2=0; s3=0; n=0; }

  double Count()        const { return n; }
  double N()            const { return n; }
  double Sum()          const { return s1; }
  double S1()           const { return s1; }
  double SumOfSquares() const { return s2; }
  double S2()           const { return s2; }
  double SumOfCubes()   const { return s3; }
  double S3()           const { return s3; }
  double S4()           const { return s4; }

  double Average() const { return ( n >1 ) ? (s1/n) : 0; }

  double Variance() const { return ( n > 1 ) ?
      (s2 - s1*s1/n) / (n-1) : INF_ERR; }
    //n*(s2/n - s1/n*s1/n) / (n-1) : INF_ERR; }
  double StdDeviation() const { return sqrt(Variance()); }
  double StdError()  const { return (n > 1) ?
         sqrt(Variance()/n) : INF_ERR; }

//      old formula, implemented by TCC, not sure how it relates to 
//      the real Standard Error
//       sqrt(s2 / (n * (n-1))) : INF_ERR; }

  double Skewness() const { return ( n > 2 ) ?
       (n*s3 - 3*s2*s1 + 2*s1*s1*s1/n) / ((n-1)*(n-2)) : INF_ERR; }
     //n*n*(s3/n - 3*s2/n*s1/n + 2*s1/n*s1/n*s1/n)/((n-1)*(n-2)) : INF_ERR; }

  double Kurtosis() const { return ( n > 3 ) ?
     //n*n*(n+1)*(s4/n - 4*s3/n*s1/n + 6*s2/n*s1/n*s1/n -
     //3*s1/n*s1/n*s1/n*s1/n)/((n-1)*(n-2)*(n-3)) :
       (n+1)*(n*s4 - 4*s3*s1 + 6*s2*s1*s1/n - 3*s1*s1*s1/n*s1/n) /
       ((n-1)*(n-2)*(n-3)) :
       INF_ERR; }

  // Notation Shortcuts
  double Ave() const { return Average(); }
  double Var() const { return Variance(); }
  double Kur() const { return Kurtosis(); }
  double Skw() const { return Skewness(); }


  void Add(double value, double weight=1){
    double w_val = value * weight;
    n += weight;
    s1 += w_val;
    s2 += w_val * w_val;
    s3 += w_val * w_val * w_val;
    s4 += w_val * w_val * w_val * w_val;
  }

  void Subtract(double value, double weight=1){
    double w_val = value * weight;
    n -= weight;
    s1 -= w_val;
    s2 -= w_val * w_val;
    s3 -= w_val * w_val * w_val;
    s4 -= w_val * w_val * w_val * w_val;
  }
};



class cIntSum {
private:
  long s1;  // Sum (x)
  long s2;  // Sum of squares (x^2)
  long s3;  // Sum of cubes (x^3)
  long s4;  // Sum of x^4
  long n;

public:
  static const double INF_ERR;  // Value Returned by StdError if Infinate

  cIntSum(){ Clear(); }

  void Clear(){ s1=0; s2=0; s3=0; n=0; }

  long Count()        const { return n; }
  long N()            const { return n; }
  long Sum()          const { return s1; }
  long S1()           const { return s1; }
  long SumOfSquares() const { return s2; }
  long S2()           const { return s2; }
  long SumOfCubes()   const { return s3; }
  long S3()           const { return s3; }
  long S4()           const { return s4; }

  double Average() const { return ( n >1 ) ? ((double)s1/n) : 0; }

  double Variance() const { return ( n > 1 ) ?
      (s2 - s1*s1/(double)n) / (double)(n-1) : INF_ERR; }
    //n*(s2/n - s1/n*s1/n) / (n-1) : INF_ERR; }
  double StdDeviation() const { return sqrt(Variance()); }
  double StdError()  const { return (n > 1) ?
       sqrt(s2 / (double)(n * (n-1))) : INF_ERR; }

  double Skewness() const { return ( n > 2 ) ?
   (n*s3 - 3*s2*s1 + 2*s1*s1*s1/(double)n) / (double)((n-1)*(n-2)) : INF_ERR; }
     //n*n*(s3/n - 3*s2/n*s1/n + 2*s1/n*s1/n*s1/n)/((n-1)*(n-2)) : INF_ERR; }

  double Kurtosis() const { return ( n > 3 ) ?
     //n*n*(n+1)*(s4/n - 4*s3/n*s1/n + 6*s2/n*s1/n*s1/n -
     //3*s1/n*s1/n*s1/n*s1/n)/((n-1)*(n-2)*(n-3)) :
       (n+1)*(n*s4 - 4*s3*s1 + 6*s2*s1*s1/(double)n -
	      3*s1*s1*s1/(double)n*s1/(double)n) /
			      (double)((n-1)*(n-2)*(n-3)) :
    INF_ERR; }

  // Notation Shortcuts
  double Ave() const { return Average(); }
  double Var() const { return Variance(); }
  double Kur() const { return Kurtosis(); }
  double Skw() const { return Skewness(); }


  void Add(long value, long weight=1){
    long w_val = value * weight;
    n += weight;
    s1 += w_val;
    s2 += w_val * w_val;
    s3 += w_val * w_val * w_val;
    s4 += w_val * w_val * w_val * w_val;
  }

  void Subtract(long value, long weight=1){
    long w_val = value * weight;
    n -= weight;
    s1 -= w_val;
    s2 -= w_val * w_val;
    s3 -= w_val * w_val * w_val;
    s4 -= w_val * w_val * w_val * w_val;
  }
};



class cRunningAverage {
private:
  double *m_values;  // Array of actual values
  double m_s1;       // average
  double m_s2;       // sum of squares
  int m_window_size;        // Size of sliding window
  int m_pointer;
  int m_n;
  
  
  cRunningAverage();
  cRunningAverage( const cRunningAverage & );
  cRunningAverage & operator=( const cRunningAverage & );
public:
  cRunningAverage( int window_size );
  ~cRunningAverage();
  
  
  //manipulators
  void Add( double value );
  void Clear();
  
  
  //accessors
  double Sum()          const { return m_s1; }
  double S1()           const { return m_s1; }
  double SumOfSquares() const { return m_s2; }
  double S2()           const { return m_s2; }
  
  double Average() const { return ( m_n == m_window_size ) ? (m_s1/m_n) : 0; }

  double Variance() const { return ( m_n == m_window_size ) ?
      (m_s2 - m_s1*m_s1/m_n) / (m_n-1) : 0; }
    
  double StdDeviation() const { return sqrt(Variance()); }
  double StdError()  const { return ( m_n == m_window_size ) ?
       sqrt(m_s2 - m_s1*m_s1/m_n / (m_n * (m_n-1))) : 0; }

  // Notation Shortcuts
  double Ave() const { return Average(); }
  double Var() const { return Variance(); }
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
    std::cerr << "Trying to insert " << value << " into Histogram of range ["
	 << min_bin << "," << max_bin << "]" << std::endl;
  }

  bins[value - min_bin] += count;
  entry_count += count;
  entry_total += value * count;
}

inline void cHistogram::Remove(int value)
{
#ifdef DEBUG
  if (value > max_bin || value < min_bin) {
    std::cerr << "Trying to remove " << value << " into Histogram of range ["
	 << min_bin << "," << max_bin << "]" << std::endl;
    return;
  }
  if (bins[value] <= 1) {
    std::cerr << "WARNING: Trying to remove " << value << " from bin with "
	 << bins[value] << " entries" << std::endl;
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
    std::cerr << "WARNING: Trying to remove " << value
	 << " in histogram of range [" << min_bin << "," <<  max_bin
	 << "]" << std::endl;
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
