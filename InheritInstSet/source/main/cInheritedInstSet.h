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
#include "defs.h"

#include "cInstSet.h"


class cInstSet;
class cWorld;

class cInheritedInstSet : public cInstSet{

  private:
    tArray<int>    m_redundancies;  
    tArray< tArray<int> >  m_allowed_redundancies;
  
    void InitRedRandomly();
    void InitRedByBaseInstSet();
    void InitRedByValue(int val);
    
  
    bool MutateAllInsts(double p);
    bool MutateSingleInst();
  
    inline int GetRandomRedundancy(int id);
    inline bool RandProceed(double p);
  
    cInheritedInstSet();  // @not_implemented
    cInheritedInstSet& operator=(const cInheritedInstSet&); // @not_implemented
  
  
  public:
    cInheritedInstSet(const cInheritedInstSet* in);
    cInheritedInstSet(const cInstSet* in, int init_val, 
                      const tArray< tArray<int> >& allowed_redundancies);
    cInstSet* MakeInstSet();
    
    void DoMutation(eIIS_MUT_TYPE type, double prob);
  
    void Sync();
  

};

#endif
