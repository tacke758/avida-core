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


class cDemeManager{
  friend class cDemeCompeitions;
  friend class cDemeTriggers;
  
  
  private:
    cWorld*        m_world;
    cPopulation&   m_population;
    tArray<cDeme*> m_demes;
    tArray<double> m_deme_fitness;
    double         m_total_deme_fitness;
  
  
    static cGenome DoGermlineMutation(const cGenome& source_germ);
    
    cDemeManager();
    cDemeManager(const cDemeManager&);
    cDemeManager operator=(const cDemeManager&);
  
  public:
    cDemeManager(cPopulation& p);
    ~cDemeManager();
  
    int GetNumDemes() { return deme_array.GetSize(); }
    cDeme* GetDeme(int i) { return deme_array[i]; }
    
    // Deme-related methods
    void CompeteDemes(void (*fitfunc)(cDemeManager&), tArray<int> (*selfunc)(cDemeManager&));
    void ReplicateDemes(bool (*trigfunc)(cDemeManager&, int));
    void DivideDemes();
    void ResetDemes();
    void CopyDeme(int source_id, int target_id);
    void CopyDemeGermline(int source_id, int target_id);
    void SterileRandomInjection(int source_id, int target_id);
    void SterileGermlineInjection(int source_id, int target_id);
    void SpawnDeme(int source_id, int target_id=-1);
    void SterilizeDeme(int deme_id);
  
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
    const tArray<double>& GetDemeResources(int deme_id) { return GetDeme(deme_id).GetDemeResourceCount().GetResources(); }
    void UpdateDemeCellResources(const tArray<double>& res_change, const int cell_id);
  
    static tArray<int> CompeteFitnessProportional(cDemeManager& mgr);
    static tArray<int> CompeteTournament(cDemeManager& mgr);
};


#endif
