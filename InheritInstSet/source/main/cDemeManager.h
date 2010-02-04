/*
 *  cDemeManager.h
 *  Avida
 *
 *  Created by Matthew Rupp on 1/18/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#ifndef cDemeManager_h
#define cDemeManager_h

# ifndef cPopulation_h
#include "cPopulation.h"
# endif

#ifndef cDeme_h
#include "cDeme.h"
#endif

#ifndef tArray_h
#include "tArray.h"
#endif

#ifndef tDictionary_h
#include "tDictionary_h"
#endif

#ifndef cDemeMethods_h
#include "cDemeMethodsLib.h"
#endif

class cResourceCount; 



class cDemeManager{
  friend class cDemeCompetitions;
  friend class cDemeTriggers;
  friend class cDemeSelections;
  friend class cDemeReplications;
  
  
  private:
    cWorld*          m_world;
    cPopulation&     m_population;
    int              m_deme_size_x;
    int              m_deme_size_y;
    int              m_deme_size;
    tArray<cDeme*>   m_demes;
    tArray<double>   m_deme_fitness;
    double           m_total_deme_fitness;
    cDemeMethodsLib  m_methods;
  
    cGenome DoGermlineMutation(const cGenome& source_germ);
    
    cDemeManager();
    cDemeManager(const cDemeManager&);
    cDemeManager operator=(const cDemeManager&);
  
  public:
  
    cDemeManager(cPopulation& p);
    ~cDemeManager();
  
    int GetNumDemes() const { return m_demes.GetSize(); }
    cDeme* GetDeme(int i) { return m_demes[i]; }
    int GetDemeSize() const {return m_deme_size;}
    int GetDemeSizeX() const {return m_deme_size_x;}
    int GetDemeSizeY() const {return m_deme_size_y;}
    cPopulation& GetPopulation() { return m_population;}
    
    // Deme-related methods
    void CompeteDemes(const cString& fit_fun, const cString& sel_fun, const cString& repl_fun);
    void ReplicateDemes(const cString& trigger, const cString& repl_func);
    void CopyDeme(int source_id, int target_id);
    void CopyDemeGermline(int source_id, int target_id);
    void ResetDemes();
    void SterilizeDeme(int deme_id);
    void ResetDemeOrganisms();
    void SpawnDeme(int source_id, int target_id=-1);
  
    void SterileInjectCenter(const cGenome& genome, int target_id);
    void SterileInjectRandom(const cGenome& genome, int target_id);
    void SterileInjectFull(const cGenome& genome, int target_id);
  
    
    cOrganism* SampleRandomDemeOrganism(int deme_id);
    const cGenome*   SampleRandomDemeGenome(int deme_id);
  
    // Deme-related stats methods
    void PrintDemeAllStats();
    void PrintDemeDonor();
    void PrintDemeFitness();
    void PrintDemeGestationTime();
    void PrintDemeInstructions();
    void PrintDemeLifeFitness();
    void PrintDemeMerit();
    void PrintDemeMutationRate();
    void PrintDemeReceiver();
    void PrintDemeResource();
    void PrintDemeSpatialResData(cResourceCount res, const int i, const int deme_id) const;
    void PrintDemeSpatialEnergyData();
    void PrintDemeSpatialSleepData();
    void PrintDemeTasks();
    const tArray<double>& GetDemeResources(int deme_id) { return GetDeme(deme_id)->GetDemeResourceCount().GetResources(); }
    void UpdateDemeCellResources(const tArray<double>& res_change, const int cell_id);
};

#endif
