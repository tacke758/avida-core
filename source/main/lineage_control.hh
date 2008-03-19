//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef LINEAGE_CONTROL_HH
#define LINEAGE_CONTROL_HH

#ifndef LINEAGE_HH
#include "../defs.hh"
#endif

#include <list>

class cLineage;
class cBaseCPU;
class cGenotype;
class cGenebank;

class cLineageControl {
private:
  list<cLineage *> m_lineage_list;
  cLineage * m_best_lineage;  // the lineage with the highest average fitness
  cLineage * m_max_fitness_lineage; // lineage with the single highest fitness
  cLineage * m_dominant_lineage; // the lineage with the largest number of creatures.

  // a pointer to the genebank used by Avida
  cGenebank * m_genebank;

  /**
   * Adds a new lineage to the control.
   **/
  cLineage * AddLineage( double start_fitness, int parent_lin_id, int id = -1, double lineage_stat1 = 0.0, double lineage_stat2 = 0.0 );

  /**
   * Determines the best lineage (lineage with the highest fitness),
   * the dominant lineage, and removes empty lineages.
   **/
  void UpdateLineages();

  /**
   * Does the actual adding of a creature to the lineage, after AddCreature
   * has done all the preliminary work.
   **/
  void AddCreaturePrivate( cGenotype *genotype, cLineage * lineage );

  cLineageControl();
  cLineageControl( const cLineageControl & );
  cLineageControl & operator=( const cLineageControl & );
public:
  cLineageControl( cGenebank * genebank );
  ~cLineageControl();


  // manipulators
  /**
   * Adds a creature to the correct lineage. The parent genotype is necessary
   * to determine when a new lineage should be created.
   *
   * Attention: the creature is not updated, this has to happen somewhere else!
   **/
  cLineage* AddCreature( cGenotype * child_genotype, cGenotype *parent_genotype, cLineage * parent_lineage, int parent_lin_id );

  /**
   * Removes a creature from the corresponding lineage.
   **/
  void RemoveCreature( cBaseCPU *cpu );

  // accessors
  /**
   * Finds the lineage with the given id.
   * Returns 0 if the lineage does not exist.
   **/
  cLineage * FindLineage( int lineage_id ) const;

  /**
   * @return The lineage with the fastest replicating genotype.
   **/
  cLineage * GetMaxFitnessLineage() const { return m_max_fitness_lineage; }

  /**
   * @return The lineage with the highest average fitness.
   **/
  cLineage * GetBestLineage() const { return m_best_lineage; }

  /**
   * @return The most abundant lineage.
   **/
  cLineage * GetDominantLineage() const { return m_dominant_lineage; }

  /**
   * @return The current number of lineages.
   **/
  int GetSize() const { return m_lineage_list.size(); }

  /**
   * @return A list of the single lineages.
   **/
  const list<cLineage *> & GetLineageList() const { return m_lineage_list; }
};

#endif






