//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "stat.hh"
#include "functions.hh"


using namespace std;


// cCountTracker /////////////////////////////////////////////////////////////

void cCountTracker::Inc()
{
  cur_count++;
  total_count++;
}

void cCountTracker::Dec() 
{
  cur_count--;
}

void cCountTracker::Next()
{
  last_count = cur_count;
  cur_count = 0;
}

void cCountTracker::Clear()
{
  cur_count = 0;
  last_count = 0;
  total_count = 0;
}



// cDoubleSum ////////////////////////////////////////////////////////////////

const double cDoubleSum::INF_ERR = 0;


// cIntSum ///////////////////////////////////////////////////////////////////

const double cIntSum::INF_ERR = 0;


// cRunningAverage ////////////////////////////////////////////////////////////


cRunningAverage::cRunningAverage( int window_size ) : 
  m_values(0), m_s1(0), m_s2(0), m_window_size( window_size ),
  m_pointer(0), m_n(0) 
{
  assert( m_window_size > 1 );
  m_values = new double[ m_window_size ];
}


cRunningAverage::~cRunningAverage() {
  delete [] m_values;
}


void
cRunningAverage::Add( double value ) {
  m_s1 += value;
  m_s2 += value*value;
  if ( m_n < m_window_size ) {
    m_values[ m_n ] = value;
    m_n += 1;
  }
  else{
    double out_v = m_values[ m_pointer ];
    m_s1 -= out_v;
    m_s2 -= out_v * out_v;
    m_values[ m_pointer++ ] = value;
    if ( m_pointer == m_window_size ) m_pointer = 0;
  }
}


void
cRunningAverage::Clear() {
  m_s1 = 0;
  m_s2 = 0;
  m_pointer = 0;
  m_n = 0;
}

// cHistogram /////////////////////////////////////////////////////////////////

cHistogram::cHistogram(int in_max, int in_min) {
  max_bin = in_max;
  min_bin = in_min;
  entry_count = 0;
  entry_total = 0;

  int num_bins = max_bin - min_bin + 1;
  bins = new int[num_bins];
  for (int i = 0; i < num_bins; i++)   bins[i] = 0;
}

void cHistogram::Resize(int new_max, int new_min)
{
#ifdef DEBUG
  if (new_max < new_min) {
    fprintf(stderr, "ERROR: Trying to resize histogram to [%d,%d]\n",
	    new_min, new_max);
    return;
  }
#endif

  // Calculate new num bins.
  int new_num_bins = new_max - new_min + 1;

  // Setup new bins, copying over information...
  int cur_bin = 0;
  int overlap_min = Max(min_bin, new_min);
  int overlap_max = Min(max_bin, new_max);

  int * new_bins = new int[new_num_bins];
  for (cur_bin = new_min; cur_bin < min_bin; cur_bin++)
    new_bins[cur_bin - new_min] = 0;
  for (cur_bin = max_bin; cur_bin <= new_max; cur_bin++)
    new_bins[cur_bin - new_min] = 0;
  for (cur_bin = overlap_min; cur_bin <= overlap_max; cur_bin++)
    new_bins[cur_bin - new_min] = bins[cur_bin - min_bin];

  // Re-count bins...
  int new_count = 0;
  int new_total = 0;
  for (int i = 0; i < new_num_bins; i++) {
    new_count += new_bins[i];
    new_total += new_bins[i] * (i + new_min);
  }
  entry_count = new_count;
  entry_total = new_total;

  delete [] bins;
  bins = new_bins;
  max_bin = new_max;
  min_bin = new_min;
}

void cHistogram::Print()
{
  FILE * fp = fopen("test.dat", "w");
  fprintf(fp, "Min = %d, Max = %d, Count = %d, Total = %d, Ave = %f\n",
	  min_bin, max_bin, entry_count, entry_total, GetAverage());
  for (int i = min_bin; i <= max_bin; i++) {
    fprintf(fp, "%d : %d\n", i, bins[i - min_bin]);
  }
  fflush(fp);
  fclose(fp);
}
