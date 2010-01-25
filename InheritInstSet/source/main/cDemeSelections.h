/*
 *  cDemeSelections.h
 *  Avida
 *
 *  Created by Matthew Rupp on 1/25/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#ifndef cDemeSelections_h
#define cDemeSelections_h

#ifndef tArray_h
#include "tArray.h"
#endif

class cDemeManager;

typedef tArray<int> (*tDemeSelection)(cDemeManager& mgr);

class cDemeSelections{
  private:
    cDemeSelections();
    cDemeSelections(const cDemeSelections&);
    cDemeSelections& operator=(const cDemeSelections&);
  
  public:
    static tArray<int>  FitnessProportional(cDemeManager& mgr);
    static tArray<int>  Tournament(cDemeManager& mgr);
};

#endif
