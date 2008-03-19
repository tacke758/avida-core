/******************************************************************************

cRandom

Random number generator
Random variables from various statistical distributions

******************************************************************************/

#ifndef cRANDOM_HH
#define cRANDOM_HH

#include <time.h>
#include <limits.h>
#include <math.h>
#include "tArray.hh"

class cRandom{
  public:

  // Constructor (negative seed => seed with time) ////////////////////////////
  cRandom(const int in_seed=-1);


  // Seed Accessors ///////////////////////////////////////////////////////////
  inline int GetSeed(){ return seed; }
  inline int GetOriginalSeed(){ return original_seed; }

  void ResetSeed(const int new_seed);


  // Random Number Generation /////////////////////////////////////////////////

  // Double Methods 
  inline double GetDouble(){ return Get()*_RAND_FAC; }
  inline double GetDouble(const double max){ return GetDouble() * max;}
  inline double GetDouble(const double min, const double max){
    return GetDouble()*(max-min)+min;}

  // Unsinged Int Methods
  inline unsigned int GetUInt(const unsigned int max){
    return (int) (GetDouble()*max);}
  inline unsigned int GetUInt(const unsigned int min, const unsigned int max){
    return GetUInt(max-min+1)+min; }

  // Int Method
  inline int GetInt(const int min, const int max){
    return ((int)GetUInt(max-min+1))+min; }


  // Random Event Generation //////////////////////////////////////////////////

  // P(p) => if p < [0,1) random variable 
  inline bool P(const double _p){ 
    return (Get()<(_p*_RAND_MBIG));}
  inline bool mP(const double _p){	// p = _p*10^-3
    return (Get()<_RAND_mP_FAC && Get()<(_p*_RAND_MBIG));}
  inline bool uP(const double _p){	// p = _p*10^-6
    return (Get()<_RAND_uP_FAC && Get()<(_p*_RAND_MBIG));}
  inline bool pP(const double _p){	// p = _p*10^-6
    return (Get()<_RAND_uP_FAC && Get()<_RAND_uP_FAC && 
	    Get()<(_p*_RAND_MBIG));}


  // Other neat stuff /////////////////////////////////////////////////////////
  inline UINT MutateByte(UINT value);
  inline UINT ClearByte(UINT value);
  inline UINT MutateBit(UINT value);
  inline UINT MutateBit(UINT value, int in_byte);

  int Choose(UINT num_in, tArray<UINT> & out_array);


  // Statistical functions ////////////////////////////////////////////////////
  
  // Distributions //

  // Random Variables Drawn from a Distribution
  double GetRandNormal(); // Unit Normal random variable
  inline double GetRandNormal(const double mean, const double variance){
    return mean+GetRandNormal()*sqrt(variance);}

  inline unsigned int GetRandPoisson(const double n, double p){
    // Optimizes for speed and calculability using symetry of the distribution
    if( p>.5 ) return (unsigned int)n-GetRandPoisson(n*(1-p));
    else return GetRandPoisson(n*p); }
  unsigned int GetRandPoisson(const double mean);

  unsigned int GetFullRandBinomial(const double n, const double p); // Exact
  unsigned int GetRandBinomial(const double n, const double p); // Approx


  // Internals ////////////////////////////////////////////////////////////////
  private:
  // Internal memebers
  int seed;
  int original_seed;
  int inext;
  int inextp;
  int ma[56];

  // Constants ////////////////////////////////////////////////////////////////
  // Statistical Approximation
  static const unsigned int _BINOMIAL_TO_NORMAL;  //if < n*p*(1-p)
  static const unsigned int _BINOMIAL_TO_POISSON; //if < n && !Normal approx
  // Engine
  static const unsigned int _RAND_MBIG;
  static const unsigned int _RAND_MSEED;
  // Number Generation
  static const double _RAND_FAC;
  static const double _RAND_mP_FAC;
  static const double _RAND_uP_FAC;

  // Members & functions for stat functions
  double expRV; // Exponential Random Variable for the randNormal function

  // Internal functions
  void init();	// Setup  (called by ResetSeed(in_seed);
  void initStatFunctions();

  // Basic Random number
  // Returns a random number [0,_RAND_MBIG)
  inline unsigned int Get(){
    if (++inext == 56) inext = 0;
    if (++inextp == 56) inextp = 0;
    int mj = ma[inext] - ma[inextp];
    if (mj < 0) mj += _RAND_MBIG;
    ma[inext] = mj;
    return mj;
  }

};


inline UINT cRandom::MutateByte(UINT value) {
  int byte_pos = 8 * GetUInt(4);
  int new_byte = GetUInt(256);
  value &= ~(255 << byte_pos);
  value |= new_byte << byte_pos;
  return value;
}

inline UINT cRandom::ClearByte(UINT value) {
  int byte_pos = 8 * GetUInt(4);
  value &= ~(255 << byte_pos);
  return value;
}

inline UINT cRandom::MutateBit(UINT value) {
  int bit_pos = GetUInt(32);
  value ^= (1 << bit_pos);
  return value;
}

inline UINT cRandom::MutateBit(UINT value, int in_byte) {
  int bit_pos = (in_byte) * 8 + GetUInt(8);
  value ^= (1 << bit_pos);
  return value;
}

#endif
