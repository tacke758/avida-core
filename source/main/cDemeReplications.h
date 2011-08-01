/*
 *  cDemeReplication.h
 *  Avida
 *
 *  Created by Matthew Rupp on 1/25/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#ifndef cDemeReplications_h
#define cDemeReplications_h

class cDemeManager;

typedef void (*tDemeReplication)(cDemeManager& mgr, int source, int target);

class cDemeReplications{
  
  private:
    cDemeReplications();
    cDemeReplications(const cDemeReplications&);
    cDemeReplications& operator=(const cDemeReplications&);
  
    static bool PrepareReplication(cDemeManager& mgr, int source, int target, const cGenome*& injected);
    static bool PrepareReplication(cDemeManager& mgr, int source, int target);
                          
  public:
    static void SterileInjectionAtCenter(cDemeManager& mgr, int source, int target);
    static void SterileInjectionAtRandom(cDemeManager& mgr, int source, int target);
    static void SterileFullInjection(cDemeManager& mgr, int source, int target);
    static void InjectCopy(cDemeManager& mgr, int source, int target);
};

#endif
