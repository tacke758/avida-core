/*
 *  cInheritedInstSet.h
 *  Avida
 *
 *  Created by Matthew Rupp on 1/4/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#ifndef cInheritedInstSet_h
#define cInheritedInstSet_h

#include "tArray.h"
#include "tList.h"

#include "cInstSet.h"


class cInstSet;
class cWorld;

typedef tArray< tArray<double> > tInstRed;

class cInheritedInstSet : public cInstSet{

  private:
    tInstRed  m_allowed_redundancies;
  
    void InitRedRandomly();
    void InitRedByBaseInstSet();
    void InitRedByValue(int val);
    
  
   
    inline double GetRandomRedundancy(int id);
    inline bool RandProceed(double p);
  
    cInheritedInstSet();  // @not_implemented
    cInheritedInstSet& operator=(const cInheritedInstSet&); // @not_implemented
  
  
  public:
    virtual ~cInheritedInstSet() {;}
    cInheritedInstSet(const cInheritedInstSet* in);
    cInheritedInstSet(const cInstSet* in, tInstRed& allowed_redundancies);
    cInheritedInstSet(const cInstSet* in, int init_val, 
                      const tInstRed& allowed_redundancies);
    cInstSet* MakeInstSet();
    
    bool MutateAllInsts(double p);
    bool MutateSingleInst(double p);
  
    void Sync();
  

};

#endif
