//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include <fstream>

#include "genebank.hh"

#include "species.hh"
#include "genotype.hh"
#include "config.hh"
#include "stats.hh"

#include "../tools/tArray.hh"
#include "../cpu/test_util.hh"


using namespace std;


///////////////////////
//  cGenotypeControl
///////////////////////

cGenotypeControl::cGenotypeControl(cGenebank & in_gb) : genebank(in_gb)
{
  size = 0;
  best = NULL;
  coalescent = NULL;
  for (int i = 0; i < GENOTYPE_THREADS; i++) threads[i] = NULL;

  historic_list = NULL;
  historic_count = 0;
}

cGenotypeControl::~cGenotypeControl()
{
}

bool cGenotypeControl::OK()
{
  int ret_value = true;

  // Cycle through the list, making sure all connections are proper, size
  // is correct, and all genotypes are OK().

  cGenotype * cur_pos = best;
  for (int i = 0; i < size; i++) {
    if (!cur_pos->OK()) ret_value = false;
    assert (cur_pos->GetNext()->GetPrev() == cur_pos);
    cur_pos = cur_pos->GetNext();
  }

  assert (cur_pos == best);

  return ret_value;
}

void cGenotypeControl::Insert(cGenotype & in_genotype, cGenotype * prev_genotype)
{
  if (prev_genotype == NULL) {
    assert(size == 0); // Destroying a full genotype queue...

    best = &in_genotype;
    best->SetNext(best);
    best->SetPrev(best);
  }
  else {
    in_genotype.SetPrev(prev_genotype);
    in_genotype.SetNext(prev_genotype->GetNext());
    prev_genotype->SetNext(&in_genotype);
    in_genotype.GetNext()->SetPrev(&in_genotype);
  }

  size++;
}

void cGenotypeControl::Remove(cGenotype & in_genotype)
{
  if (size == 1) {
    best = NULL;
  }
  if (&in_genotype == best) {
    best = best->GetNext();
  }

  in_genotype.GetNext()->SetPrev(in_genotype.GetPrev());
  in_genotype.GetPrev()->SetNext(in_genotype.GetNext());
  in_genotype.SetNext(NULL);
  in_genotype.SetPrev(NULL);

  size--;
}

void cGenotypeControl::RemoveHistoric(cGenotype & in_genotype)
{
  if (historic_count == 1) {
    historic_list = NULL;
  }
  if (&in_genotype == historic_list) {
    historic_list = historic_list->GetNext();
  }

  in_genotype.GetNext()->SetPrev(in_genotype.GetPrev());
  in_genotype.GetPrev()->SetNext(in_genotype.GetNext());
  in_genotype.SetNext(NULL);
  in_genotype.SetPrev(NULL);

  historic_count--;
}

void cGenotypeControl::InsertHistoric(cGenotype & in_genotype)
{
  if (historic_count == 0) {
    in_genotype.SetNext(&in_genotype);
    in_genotype.SetPrev(&in_genotype);
  }
  else {
    in_genotype.SetPrev(historic_list->GetPrev());
    in_genotype.SetNext(historic_list);
    historic_list->GetPrev()->SetNext(&in_genotype);
    historic_list->SetPrev(&in_genotype);
  }

  historic_list = &in_genotype;
  historic_count++;
}


int cGenotypeControl::UpdateCoalescent()
{
  // Test to see if any updating needs to be done...
  // Don't update active coalescent genotype, or if there is more than
  // one offspring.
  if (coalescent != NULL &&
      (coalescent->GetNumOrganisms() > 0 ||
       coalescent->GetNumOffspringGenotypes() > 1)) {
    return coalescent->GetDepth();
  }

  // If there is no best, there is nothing to search through...
  if (best == NULL) return -1;

  // Find the new point...
  cGenotype * test_gen = best;
  cGenotype * found_gen = best;
  cGenotype * parent_gen = best->GetParentGenotype();

  while (parent_gen != NULL) {
    // See if this genotype should be the new found genotype...
    if (test_gen->GetNumOrganisms() > 0 ||
	test_gen->GetNumOffspringGenotypes() > 1) {
      found_gen = test_gen;
    }

    // Move to the next genotype...
    test_gen = parent_gen;
    parent_gen = test_gen->GetParentGenotype();
  }

  coalescent = found_gen;

  return coalescent->GetDepth();
}


bool cGenotypeControl::CheckPos(cGenotype & in_genotype)
{
  int next_OK = false;
  int prev_OK = false;

  if (in_genotype.GetNumOrganisms() >= in_genotype.GetNext()->GetNumOrganisms()) {
    next_OK =true;
  }
  if (in_genotype.GetNumOrganisms() <= in_genotype.GetPrev()->GetNumOrganisms()) {
    prev_OK =true;
  }

  if ((&in_genotype == best && next_OK) ||
      (next_OK && prev_OK) ||
      (&in_genotype == best->GetPrev() && prev_OK)) {
    return true;
  }

  return false;
}

void cGenotypeControl::Insert(cGenotype & new_genotype)
{
  // If there is nothing in the list, add this.

  if (size == 0) {
    Insert(new_genotype, NULL);
  }

  // Otherwise tack it on the end.

  else {
    Insert(new_genotype, best->GetPrev());
  }
}

bool cGenotypeControl::Adjust(cGenotype & in_genotype)
{
  if (in_genotype.GetDeferAdjust() == true) return true;

  cGenotype * cur_genotype = in_genotype.GetPrev();

  // Check to see if this genotype should be removed completely.

  if (in_genotype.GetNumOrganisms() == 0) {
    genebank.RemoveGenotype(in_genotype);
    return false;
  }

  // Do not adjust if this was and still is the best genotype, or is
  // otherwise in the proper spot...

  if (CheckPos(in_genotype)) {
    return true;
  }

  // Otherwise, remove it from the queue for just the moment.

  Remove(in_genotype);

  // Also, if this genotype is the best, put it there.

  if (in_genotype.GetNumOrganisms() > best->GetNumOrganisms()) {
    Insert(in_genotype, best->GetPrev());
    best = &in_genotype;
    return true;
  }

  // Finally, find out where this genotype *does* go.

  while (cur_genotype->GetNumOrganisms() >= in_genotype.GetNumOrganisms() &&
	 cur_genotype != best->GetPrev()) {
    cur_genotype = cur_genotype->GetNext();
  }
  while (cur_genotype->GetNumOrganisms() < in_genotype.GetNumOrganisms() &&
	 cur_genotype != best) {
    cur_genotype = cur_genotype->GetPrev();
  }

  Insert(in_genotype, cur_genotype);

  return true;
}


cGenotype * cGenotypeControl::Find(const cGenome & in_genome) const
{
  int i;
  cGenotype * cur_genotype = best;

  for (i = 0; i < size; i++) {
    if (in_genome == cur_genotype->GetGenome()) {
      return cur_genotype;
    }
    cur_genotype = cur_genotype->GetNext();
  }

  return NULL;
}

int cGenotypeControl::FindPos(cGenotype & in_genotype, int max_depth)
{
  cGenotype * temp_genotype = best;
  if (max_depth < 0 || max_depth > size) max_depth = size;

  for (int i = 0; i < max_depth; i++) {
    if (temp_genotype == &in_genotype) return i;
    temp_genotype = temp_genotype->GetNext();
  }

  return -1;
}

cGenotype * cGenotypeControl::Next(int thread)
{
  return threads[thread] = threads[thread]->GetNext();
}

cGenotype * cGenotypeControl::Prev(int thread)
{
  return threads[thread] = threads[thread]->GetPrev();
}


///////////////////
//  cSpeciesQueue
///////////////////

cSpeciesQueue::cSpeciesQueue()
{
  size = 0;
  first = NULL;
}

cSpeciesQueue::~cSpeciesQueue()
{
}

void cSpeciesQueue::InsertRear(cSpecies & new_species)
{
  // If the queue doesn't exist, create it with this species as the only
  // element.

  if (!first) {
    first = &new_species;
    first->SetNext(first);
    first->SetPrev(first);
  }

  // Otherwise, put this species at the end of the queue.

  else {
    new_species.SetNext(first);
    new_species.SetPrev(first->GetPrev());
    first->GetPrev()->SetNext(&new_species);
    first->SetPrev(&new_species);
  }

  size++;
}

void cSpeciesQueue::Remove(cSpecies & in_species)
{
  size--;

  // If the queue is now empty, delete it properly.
  if (size == 0) {
    first = NULL;
    return;
  }

  // If we are removing the first element of the queue, slide the queue to
  // the new first before removing it.

  if (first == &in_species) {
    first = in_species.GetNext();
  }

  // Remove the in_species

  in_species.GetPrev()->SetNext(in_species.GetNext());
  in_species.GetNext()->SetPrev(in_species.GetPrev());
  in_species.SetNext(NULL);
  in_species.SetPrev(NULL);
}

void cSpeciesQueue::Adjust(cSpecies & in_species)
{
  // First move it up the list if need be...

  cSpecies * prev_species = in_species.GetPrev();
  while (&in_species != first &&
	 in_species.GetNumThreshold() > prev_species->GetNumThreshold()) {
    // Swap the position of this species with the previous one.
    if (prev_species == first) first = &in_species;

    // Outer connections...
    prev_species->SetNext(in_species.GetNext());
    in_species.GetNext()->SetPrev(prev_species);
    in_species.SetPrev(prev_species->GetPrev());
    prev_species->GetPrev()->SetNext(&in_species);

    // Inner connections...
    prev_species->SetPrev(&in_species);
    in_species.SetNext(prev_species);

    prev_species = in_species.GetPrev();
  }
	
  // Then see if it needs to be moved down.
  cSpecies * next_species = in_species.GetNext();
  while (next_species != first &&
	 in_species.GetNumThreshold() < next_species->GetNumThreshold()) {
    // Swap the position of this species with the next one.
    if (&in_species == first) first = next_species;

    // Outer Connections...
    in_species.SetNext(next_species->GetNext());
    next_species->GetNext()->SetPrev(&in_species);
    next_species->SetPrev(in_species.GetPrev());
    in_species.GetPrev()->SetNext(next_species);

    // Inner Connections...
    in_species.SetPrev(next_species);
    next_species->SetNext(&in_species);

    next_species = in_species.GetNext();
  }
}

void cSpeciesQueue::Purge()
{
  cSpecies * cur_species = first;
  cSpecies * next_species = NULL;

  // Loop through the species deleting them until there is only one left.
  for (int i = 1; i < size; i++) {
    next_species = cur_species->GetNext();
    delete cur_species;
    cur_species = next_species;
  }

  // And delete that last one.
  delete cur_species;
  first = NULL;
  size = 0;
}

bool cSpeciesQueue::OK(int queue_type)
{
  cSpecies * cur_species = NULL;
  int count = 0;
  bool ret_value = true;

  while (first && cur_species != first) {
    // If we are just starting, set cur_species to the first element.
    if (!cur_species) cur_species = first;

    // Check that the list is correct in both directions...

    assert (cur_species->GetNext()->GetPrev() == cur_species);

    // Check to make sure the current species is OK() and that it should
    // indeed be in this list.

    if (!cur_species->OK()) ret_value = false;

    assert (queue_type == cur_species->GetQueueType());
    count++;

    assert (count <= size);

    cur_species = cur_species->GetNext();
  }

  assert (count == size);

  return ret_value;
}


/////////////////////
//  cSpeciesControl
/////////////////////

cSpeciesControl::cSpeciesControl(cGenebank & in_gb) : genebank(in_gb)
{
}

cSpeciesControl::~cSpeciesControl()
{
}

void cSpeciesControl::Remove(cSpecies & in_species)
{
  switch (in_species.GetQueueType()) {
  case SPECIES_QUEUE_ACTIVE:
    active_queue.Remove(in_species);
    break;
  case SPECIES_QUEUE_INACTIVE:
    inactive_queue.Remove(in_species);
    break;
  case SPECIES_QUEUE_GARBAGE:
    garbage_queue.Remove(in_species);
    break;
  default:
    break;
  }

  in_species.SetQueueType(SPECIES_QUEUE_NONE);
}

void cSpeciesControl::Adjust(cSpecies & in_species)
{
  // Only adjust if this species is in the active queue.

  if (in_species.GetQueueType() == SPECIES_QUEUE_ACTIVE) {
    active_queue.Adjust(in_species);
  }
}

void cSpeciesControl::SetActive(cSpecies & in_species)
{
  Remove(in_species);
  active_queue.InsertRear(in_species);
  in_species.SetQueueType(SPECIES_QUEUE_ACTIVE);
}

void cSpeciesControl::SetInactive(cSpecies & in_species)
{
  Remove(in_species);
  inactive_queue.InsertRear(in_species);
  in_species.SetQueueType(SPECIES_QUEUE_INACTIVE);
}

void cSpeciesControl::SetGarbage(cSpecies & in_species)
{
  Remove(in_species);
  garbage_queue.InsertRear(in_species);
  in_species.SetQueueType(SPECIES_QUEUE_GARBAGE);
}

bool cSpeciesControl::OK()
{
  int ret_value = true;

  // Check the queues.

  assert (active_queue.OK(SPECIES_QUEUE_ACTIVE));
  assert (inactive_queue.OK(SPECIES_QUEUE_INACTIVE));
  assert (garbage_queue.OK(SPECIES_QUEUE_GARBAGE));

  return ret_value;
}

int cSpeciesControl::FindPos(cSpecies & in_species, int max_depth)
{
  cSpecies * temp_species = active_queue.GetFirst();
  if (max_depth < 0 || max_depth > active_queue.GetSize()) {
    max_depth = active_queue.GetSize();
  }

  for (int i = 0; i < max_depth; i++) {
    if (temp_species == &in_species) return i;
    temp_species = temp_species->GetNext();
  }

  return -1;
}

cSpecies * cSpeciesControl::Find(cGenotype & in_genotype, int record_level)
{
  cSpecies * found_species = NULL;
  int cur_count, best_count = MAX_CREATURE_SIZE;
  cSpecies * cur_species;

  if (record_level == SPECIES_RECORD_FULL) {
    cur_species = active_queue.GetFirst();
    int size = active_queue.GetSize();
    for (int i = 0; i < size; i++) {
      cur_count = cur_species->Compare(in_genotype.GetGenome(),
				       cConfig::GetSpeciesThreshold());
      if (cur_count != -1 && cur_count <= cConfig::GetSpeciesThreshold() &&
	  cur_count < best_count) {
	found_species = cur_species;
	best_count = cur_count;
      }
      cur_species = cur_species->GetNext();
    }
  }

  if (record_level == SPECIES_RECORD_LIMITED) {
    cur_species = in_genotype.GetSpecies();

    if (cur_species) {
      int num_diff = cur_species->Compare(in_genotype.GetGenome(),
					  cConfig::GetSpeciesThreshold());
      if (num_diff != -1 && num_diff <= cConfig::GetSpeciesThreshold()) {
	found_species = cur_species;
      }
    }
  }

  return found_species;
}

void cSpeciesControl::Purge(cStats & stats)
{
  cSpecies * cur_species = garbage_queue.GetFirst();
  for (int i = 0; i < garbage_queue.GetSize(); i++) {
    stats.RemoveSpecies(cur_species->GetID(),
			cur_species->GetParentID(),
			cur_species->GetTotalGenotypes(),
			cur_species->GetTotalOrganisms(),
			stats.GetUpdate() - cur_species->GetUpdateBorn());
    cur_species = cur_species->GetNext();
  }
  garbage_queue.Purge();
}



///////////////
//  cGenebank
///////////////

cGenebank::cGenebank(cStats & _stats)
  : stats(_stats)
{
  for (int i = 0; i < MAX_CREATURE_SIZE; i++) {
    genotype_count[i] = 0;
  }

  genotype_control = new cGenotypeControl(*this);
  species_control  = new cSpeciesControl(*this);

}

cGenebank::~cGenebank()
{
  delete genotype_control;
  delete species_control;
}

void cGenebank::UpdateReset()
{
  static int genotype_dom_time = 0;
  static int prev_dom = -1;

  cGenotype * best_genotype = GetBestGenotype();

  species_control->Purge(stats);
  if (best_genotype && best_genotype->GetID() != prev_dom) {
    genotype_dom_time = 0;
    prev_dom = best_genotype->GetID();
  }
  else {
    genotype_dom_time++;
    if (genotype_dom_time == cConfig::GetGenotypePrintDom()) {
      cString filename;
      filename.Set("genebank/%s", best_genotype->GetName()());
      cTestUtil::PrintGenome(best_genotype->GetGenome(), 
			     filename, best_genotype, stats.GetUpdate());
    }
  }

//   tArray<int> hist_array(15);
//   hist_array.SetAll(0);
//   int total_gens = 0;
  
//   for (int i = 0; i < GENOTYPE_HASH_SIZE; i++) {
//     int cur_val = active_genotypes[i].GetSize();
//     total_gens += cur_val;
//     if (cur_val < 15) hist_array[cur_val]++;
//     else cout << cur_val << " ";
//   }
//   cout << endl;
//   for (int i = 0; i < 15; i++) {
//     cout << i << " : " << hist_array[i] << endl;
//   }
//   cout << "Total genotypes: " << total_gens << endl;
}

cString cGenebank::GetLabel(int in_size, int in_num)
{
  char alpha[6];
  char full_name[12];
  int i;

  for (i = 4; i >= 0; i--) {
    alpha[i] = (in_num % 26) + 'a';
    in_num /= 26;
  }
  alpha[5] = '\0';

  sprintf(full_name, "%03d-%s", in_size, alpha);

  return full_name;
}


void cGenebank::AddGenotype(cGenotype * in_genotype, int list_num)
{
  assert( in_genotype != 0 );
  
  if (list_num < 0) list_num = FindCRC(in_genotype->GetGenome());
  
  active_genotypes[list_num].Push(in_genotype);
  genotype_control->Insert(*in_genotype);
  stats.AddGenotype(in_genotype->GetID());

  // Speciation... If we are creating a new genotype here, we must
  // initilize it to the species of its parent genotype.

  cSpecies * parent_species = NULL;
  if ( in_genotype->GetParentGenotype() != NULL) {
    parent_species = in_genotype->GetParentGenotype()->GetSpecies();
  }

  in_genotype->SetSpecies(parent_species);
  if (parent_species != NULL) parent_species->AddGenotype();
}


cGenotype * cGenebank::AddGenotype(const cGenome & in_genome,
				   cGenotype * parent_genotype)
{
  int list_num = FindCRC(in_genome);
  cGenotype * found_genotype = FindGenotype(in_genome, list_num);

  if (!found_genotype) {
    found_genotype = new cGenotype(stats.GetUpdate());
    found_genotype->SetGenome(in_genome);
    found_genotype->SetParent(parent_genotype);
    
    AddGenotype(found_genotype, list_num);
  }

  return found_genotype;
}

const cGenotype * cGenebank::FindGenotype(const cGenome & in_genome,
				    int list_num) const
{
  if (list_num < 0) list_num = FindCRC(in_genome);

  tConstListIterator<cGenotype> list_it(active_genotypes[list_num]);
  while (list_it.Next() != NULL) {
    if (list_it.GetConst()->GetGenome() == in_genome) break;
  }
  return list_it.GetConst();
}

cGenotype * cGenebank::FindGenotype(const cGenome & in_genome,
				    int list_num)
{
  if (list_num < 0) list_num = FindCRC(in_genome);

  tListIterator<cGenotype> list_it(active_genotypes[list_num]);
  while (list_it.Next() != NULL) {
    if (list_it.Get()->GetGenome() == in_genome) break;
  }
  return list_it.Get();
}

void cGenebank::RemoveGenotype(cGenotype & in_genotype)
{
  // If this genotype is still active, mark it no longer active and
  // take it out of the hash table so it doesn't have any new organisms
  // assigned to it.

  if (in_genotype.GetActive() == true) {
    int list_num = FindCRC(in_genotype.GetGenome());
    active_genotypes[list_num].Remove(&in_genotype);
    genotype_control->Remove(in_genotype);
    in_genotype.Deactivate(stats.GetUpdate());
    if (cConfig::GetTrackMainLineage()) {
      genotype_control->InsertHistoric(in_genotype);
    }
  }

  // If we are tracking the main lineage, we only want to delete a
  // genotype when all of its decendents have also died out.

  if (cConfig::GetTrackMainLineage()) {
    // If  there are more offspring genotypes, hold off on deletion...
    if (in_genotype.GetNumOffspringGenotypes() != 0) return;

    // If this is a dead end, delete it and recurse up...
    cGenotype * parent = in_genotype.GetParentGenotype();
    if (parent != NULL) {
      parent->RemoveOffspringGenotype();

      // Test to see if we need to update the coalescent genotype.
      const int new_coal = genotype_control->UpdateCoalescent();
      stats.SetCoalescentGenotypeDepth(new_coal);
      // cout << "Set coalescent to " << found_gen->GetDepth() << endl;

      if (parent->GetNumOrganisms() == 0) {
	// Regardless, run RemoveGenotype on the parent.
	RemoveGenotype(*parent);
      }
    }

    genotype_control->RemoveHistoric(in_genotype);
  }

  // Handle the relevent statistics...
  stats.RemoveGenotype(in_genotype.GetID(),
	      in_genotype.GetParentID(), in_genotype.GetParentDistance(),
	      in_genotype.GetDepth(), in_genotype.GetTotalOrganisms(),
              in_genotype.GetTotalParasites(),
	      stats.GetUpdate() - in_genotype.GetUpdateBorn(),
              in_genotype.GetLength());
  if (in_genotype.GetThreshold()) {
    stats.RemoveThreshold(in_genotype.GetID());
  }


  // Speciation...  If a Threshold genotype was removed, the position of this
  // species in the active list will at least shift, and it is possible that
  // the species is made inactive, or removed all-togeather.  If it is a non-
  // threshold genotype, then the species will only be effected if this was
  // the last genotype of that species.

  cSpecies * cur_species = in_genotype.GetSpecies();
  if (cur_species) {

    // First, re-adjust the species.

    cur_species->RemoveGenotype();

    // Then, check to see how this species changes if it is a threshold.

    if (in_genotype.GetThreshold()) {
      cur_species->RemoveThreshold(in_genotype);

      // If we are out of thresholds, move this species to the inactive
      // list for now.  Otherwise, just adjust it.

      if (cur_species->GetNumThreshold() == 0) {
	species_control->SetInactive(*cur_species);
      }
      else {
	species_control->Adjust(*cur_species);
      }
    }

    // Finally, remove the species completely if it has no genotypes left.

    if (!cur_species->GetNumGenotypes()) {
      species_control->SetGarbage(*cur_species);
    }
  }


  delete &in_genotype;
}

void cGenebank::ThresholdGenotype(cGenotype & in_genotype)
{
  cSpecies * found_species = NULL;

  in_genotype.SetName( GetLabel(in_genotype.GetLength(),
				genotype_count[in_genotype.GetLength()]++) );
  in_genotype.SetThreshold();

  // If speciation is on, assign a species to the genotype now that it is
  // threshold.

  if (cConfig::GetSpeciesRecording()) {
    // Record the old species to know if it changes.

    cSpecies * old_species = in_genotype.GetSpecies();

    // Determine the "proper" species.

    found_species = species_control->Find(in_genotype,
					  cConfig::GetSpeciesRecording());

    // If no species was found, create a new one.

    if (!found_species) {
      found_species = new cSpecies(in_genotype.GetGenome(), stats.GetUpdate());
      if (in_genotype.GetSpecies())
	found_species->SetParentID(in_genotype.GetSpecies()->GetID());
      species_control->SetActive(*found_species);
      stats.AddSpecies(found_species->GetID());

      // Since this is a new species, see if we should be printing it.

      if (cConfig::GetSpeciesPrint()) {
	cString filename;
	filename.Set("genebank/spec-%04d", found_species->GetID());
	cTestUtil::PrintGenome(in_genotype.GetGenome(), filename,
			       &in_genotype, stats.GetUpdate());
      }
    }
    else {
      // If we are not creating a new species, but are adding a threshold
      // to one which is currently in-active, make sure to move it back to
      // the active list.

      if (found_species->GetNumThreshold() == 0) {
	species_control->SetActive(*found_species);
      }
    }

    // Now that we know for sure what the species is, and that it is in
    // the proper list, setup both the species and the genotype.

    in_genotype.SetSpecies(found_species);
    found_species->AddThreshold(in_genotype);

    // Finally test to see if the species has been changed, and adjust
    // accordingly.

    if (found_species != old_species) {
      found_species->AddGenotype();
      if (old_species) {
	old_species->RemoveGenotype();
	if (old_species->GetNumGenotypes() == 0)
	  species_control->SetGarbage(*old_species);
      }
    }
    else {
      if (found_species->GetNumThreshold() > 1) {
	species_control->Adjust(*found_species);
      }
    }
  }

  // Do the relevent statistics...

  if (cConfig::GetSpeciesRecording()) {
    stats.AddThreshold(in_genotype.GetID(), in_genotype.GetName()(),
			 found_species->GetID());
  } else {
    stats.AddThreshold(in_genotype.GetID(), in_genotype.GetName()());
  }

  // Print the genotype?

  if (cConfig::GetGenotypePrint()) {
    cString filename;
    filename.Set("genebank/%s", in_genotype.GetName()());
    cTestUtil::PrintGenome(in_genotype.GetGenome(), filename,
			   &in_genotype, stats.GetUpdate());
  }
}

bool cGenebank::AdjustGenotype(cGenotype & in_genotype)
{
  if (!genotype_control->Adjust(in_genotype)) return false;

  if ((in_genotype.GetNumOrganisms() >= cConfig::GetThreshold() ||
       &in_genotype == genotype_control->GetBest()) &&
      !(in_genotype.GetThreshold())) {
    ThresholdGenotype(in_genotype);
  }

  return true;
}

bool cGenebank::SaveClone(ofstream & fp)
{
  // This method just save the counts at each size-class of genotypes.
  // The rest is reconstructable.

  // Save the numbers of organisms we're up to at each size.
  fp << MAX_CREATURE_SIZE << " ";
  for (int i = 0; i < MAX_CREATURE_SIZE; i++) {
    fp << genotype_count[i] << " ";
  }

  return true;
}

bool cGenebank::LoadClone(ifstream & fp)
{
  // This method just restores the counts at each size-class of genotypes.
  // The rest of the loading process should be handled elsewhere.

  // Load the numbers of organisms we're up to at each size.
  int max_size;
  fp >> max_size;
  assert (max_size <= MAX_CREATURE_SIZE); // MAX_CREATURE_SIZE too small
  for (int i = 0; i < max_size && i < MAX_CREATURE_SIZE; i++) {
    fp >> genotype_count[i];
  }

  return true;
}

bool cGenebank::DumpTextSummary(ofstream & fp)
{
  genotype_control->Reset(0);
  for (int i = 0; i < genotype_control->GetSize(); i++) {
    cGenotype * genotype = genotype_control->Get(0);
    fp << genotype->GetGenome().AsString() << " "
       << genotype->GetNumOrganisms() << " "
       << genotype->GetID() << endl;
    genotype_control->Next(0);
  }

  return true;
}

bool cGenebank::DumpDetailedSummary(ofstream & fp)
{
  genotype_control->Reset(0);
  for (int i = 0; i < genotype_control->GetSize(); i++) {
    DumpDetailedEntry(genotype_control->Get(0), fp);
    genotype_control->Next(0);
  }

  return true;
}

bool cGenebank::DumpHistoricSummary(ofstream & fp)
{
  genotype_control->ResetHistoric(0);
  for (int i = 0; i < genotype_control->GetHistoricCount(); i++) {
    DumpDetailedEntry(genotype_control->Get(0), fp);
    genotype_control->Next(0);
  }

  return true;
}

void cGenebank::DumpDetailedEntry(cGenotype * genotype, ofstream & fp)
{
  fp << genotype->GetID() << " "                //  1
     << genotype->GetParentID() << " "          //  2
     << genotype->GetParentDistance() << " "    //  3
     << genotype->GetNumOrganisms() << " "      //  4
     << genotype->GetTotalOrganisms() << " "    //  5
     << genotype->GetLength() << " "            //  6
     << genotype->GetMerit() << " "             //  7
     << genotype->GetGestationTime() << " "     //  8
     << genotype->GetFitness() << " "           //  9
     << genotype->GetUpdateBorn() << " "        // 10
     << genotype->GetUpdateDeactivated() << " " // 11
     << genotype->GetDepth() << " "             // 12
     << genotype->GetGenome().AsString() << " " // 13
     << endl;
}

bool cGenebank::OK()
{
  bool ret_value = true;
  int i;

  // Check components...

  if (!genotype_control->OK() || !species_control->OK()) {
    ret_value = false;
  }

  // Now to double check the numbers of genotypes (and threshold) for each
  // species.  This will only work if debug mode is on.

#ifdef DEBUG
  // Initialize debug routines in the species.

  cSpecies * cur_species = NULL;
  cSpecies * first_active = species_control->GetFirst();
  cSpecies * first_inactive = species_control->GetFirstInactive();
  cSpecies * first_garbage = species_control->GetFirstGarbage();

  while (cur_species != first_active) {
    if (!cur_species) cur_species = first_active;
    cur_species->debug_num_genotypes = 0;
    cur_species->debug_num_threshold = 0;
    cur_species = cur_species->GetNext();
  }

  cur_species = NULL;
  while (cur_species != first_inactive) {
    if (!cur_species) cur_species = first_inactive;
    cur_species->debug_num_genotypes = 0;
    cur_species->debug_num_threshold = 0;
    cur_species = cur_species->GetNext();
  }

  cur_species = NULL;
  while (cur_species != first_garbage) {
    if (!cur_species) cur_species = first_garbage;
    cur_species->debug_num_genotypes = 0;
    cur_species->debug_num_threshold = 0;
    cur_species = cur_species->GetNext();
  }


  // Check the species for each genotype and place results in the species.

  cGenotype * cur_gen = genotype_control->GetBest();
  for (i = 0; i < genotype_control->GetSize(); i++) {
    if (cur_gen->GetSpecies()) {
      cur_gen->GetSpecies()->debug_num_genotypes++;
      if (cur_gen->GetThreshold()) {
	cur_gen->GetSpecies()->debug_num_threshold++;
      }
    }
    cur_gen = cur_gen->GetNext();
  }

  // Finally, make sure all the numbers match up.

  cur_species = NULL;
  while (cur_species != first_active) {
    if (!cur_species) cur_species = first_active;
    assert(cur_species->debug_num_genotypes == cur_species->GetNumGenotypes());
    assert(cur_species->debug_num_threshold == cur_species->GetNumThreshold());
    assert(cur_species->debug_num_genotypes >= cur_species->debug_num_threshold);

    assert (cur_species->debug_num_threshold);
    cur_species = cur_species->GetNext();
  }

  cur_species = NULL;
  while (cur_species != first_inactive) {
    if (!cur_species) cur_species = first_inactive;
    assert(cur_species->debug_num_genotypes == cur_species->GetNumGenotypes());
    assert(cur_species->debug_num_threshold == cur_species->GetNumThreshold());
    assert(cur_species->debug_num_threshold);
    assert(cur_species->debug_num_genotypes);
  }

  cur_species = NULL;
  while (cur_species != first_garbage) {
    if (!cur_species) cur_species = first_garbage;
    assert(cur_species->debug_num_genotypes == 0 &&
	   cur_species->debug_num_threshold == 0);
  }

#endif

  assert (ret_value == true);

  return ret_value;
}

int cGenebank::CountNumCreatures()
{
  int i;
  int total = 0;

  genotype_control->Reset(0);
  for (i = 0; i < genotype_control->GetSize(); i++) {
    total += genotype_control->Get(0)->GetNumOrganisms();
    genotype_control->Next(0);
  }

  return total;
}


unsigned int cGenebank::FindCRC(const cGenome & in_genome) const
{
  unsigned int total = 0;

  for (int i = 0; i < in_genome.GetSize(); i++) {
    total += (in_genome[i].GetOp() + 3) * i;
  }

  return total % GENOTYPE_HASH_SIZE;
}

void cGenebank::SpeciesTest(char * message, cGenotype & genotype)
{
  cSpecies * cur_species = genotype.GetSpecies();

//    if (cur_species) {
//      g_debug.Comment("UD %d: %s on genotype [%d] (size %d) of species [%d]",
//  		    stats.GetUpdate(), message, genotype.GetID(),
//  		    genotype.GetNumOrganisms(), cur_species->GetID());
//    } else {
//      g_debug.Comment("UD %d: %s on genotype [%d] (size %d) (no species)",
//  	    stats.GetUpdate(), message, genotype.GetID(),
//  	    genotype.GetNumOrganisms());
//    }

//    if (cur_species) {
//      g_debug.Comment("   Species [%d] has %d gen and %d thresh. (list %d)",
//  	    cur_species->GetID(), cur_species->GetNumGenotypes(),
//  	    cur_species->GetNumThreshold(), cur_species->GetQueueType());
//    }
}
