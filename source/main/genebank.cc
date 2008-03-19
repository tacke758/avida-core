//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "genebank.hh"
#include "geneology.hh"
#include "species.hh"
#include "population.hh"
#include "../cpu/cpu.ii"

/////////////////////
//  cGenotypeQueue
/////////////////////

cGenotypeQueue::cGenotypeQueue()
{
  size = 0;
  root.SetNext(&root);
  root.SetPrev(&root);
}


cGenotypeQueue::~cGenotypeQueue()
{
  while (root.GetNext() != &root) {
    Remove(root.GetNext());
  }
}

int cGenotypeQueue::OK()
{
  int result = TRUE;
  int count = 0;

  for (cGenotypeElement * temp_element = root.GetNext();
       temp_element != &root;
       temp_element = temp_element->GetNext()) {
    if (temp_element->GetNext()->GetPrev() != temp_element) {
      g_debug.Error("Mis-threaded genotype queue.");
      result = FALSE;
    }

    if (temp_element->GetGenotype()->GetID() < 0) {
      g_debug.Warning("Genotype with negativ ID in queue");
    }

    count++;
    if (count > size) {
      g_debug.Error("GenotypeQueue has size < real size");
      result = FALSE;
      break;
    }
  }

  if (count != size) {
    g_debug.Error("Genotype Queue has incorrect size!");
    result = FALSE;
  }

  return result;
}

void cGenotypeQueue::Insert(cGenotype & in_genotype)
{
  cGenotypeElement * new_element = new cGenotypeElement(&in_genotype);
  new_element->SetNext(root.GetNext());
  new_element->SetPrev(&root);
  root.GetNext()->SetPrev(new_element);
  root.SetNext(new_element);
  size++;
}

void cGenotypeQueue::Remove(cGenotype & in_genotype)
{
  cGenotypeElement * cur_element;

  for (cur_element = root.GetNext();
       cur_element != &root;
       cur_element = cur_element->GetNext()) {
    if (cur_element->GetGenotype() == &in_genotype) break;
  }

#ifdef DEBUG
  if (cur_element == &root) {
    g_debug.Warning("Genotype to be removed not found in genotype queue!");
  }
#endif

  Remove(cur_element);
}

void cGenotypeQueue::Remove(cGenotypeElement * in_element)
{
  in_element->GetPrev()->SetNext(in_element->GetNext());
  in_element->GetNext()->SetPrev(in_element->GetPrev());
  in_element->SetNext(NULL);
  in_element->SetPrev(NULL);
  delete(in_element);

  size--;
}

cGenotype * cGenotypeQueue::Find(const cCodeArray & in_code) const
{
  for (cGenotypeElement * cur_element = root.GetNext();
       cur_element != &root;
       cur_element = cur_element->GetNext()) {
    if (cur_element->GetGenotype()->GetCode() == in_code) {
      return cur_element->GetGenotype();
    }
  }

  return NULL;
}


///////////////////////
//  cGenotypeControl
///////////////////////

cGenotypeControl::cGenotypeControl(cGenebank & in_gb) : genebank(in_gb)
{
  size = 0;
  best = NULL;
  for (int i = 0; i < GENOTYPE_THREADS; i++) threads[i] = NULL;
}

cGenotypeControl::~cGenotypeControl()
{
}

int cGenotypeControl::OK()
{
  int ret_value = TRUE;

  // Cycle through the list, making sure all connections are proper, size
  // is correct, and all genotypes are OK().

  cGenotype * cur_pos = best;
  for (int i = 0; i < size; i++) {
    if (!cur_pos->OK()) ret_value = FALSE;
    if (cur_pos->GetNext()->GetPrev() != cur_pos) {
      g_debug.Error("Genotype Queue mis-threaded!");
      ret_value = FALSE;
    }
    cur_pos = cur_pos->GetNext();
  }

  if (cur_pos != best) {
    g_debug.Error("Genotype List is *not* cycling properly!");
  }

  return ret_value;
}

void cGenotypeControl::Insert(cGenotype & in_genotype, cGenotype * prev_genotype)
{
  if (!prev_genotype) {
#ifdef DEBUG
    if (size) {
      g_debug.Error("Destroying a full genotype queue...");
      size = 0;
    }
#endif
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

int cGenotypeControl::CheckPos(cGenotype & in_genotype)
{
  int next_OK = FALSE;
  int prev_OK = FALSE;

  if (in_genotype.GetNumCPUs() >= in_genotype.GetNext()->GetNumCPUs()) {
    next_OK =TRUE;
  }
  if (in_genotype.GetNumCPUs() <= in_genotype.GetPrev()->GetNumCPUs()) {
    prev_OK =TRUE;
  }

  if ((&in_genotype == best && next_OK) ||
      (next_OK && prev_OK) ||
      (&in_genotype == best->GetPrev() && prev_OK)) {
    return TRUE;
  }

  return FALSE;
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

int cGenotypeControl::Adjust(cGenotype & in_genotype)
{
  cGenotype * cur_genotype = in_genotype.GetPrev();

  // Check to see if this genotype should be removed completely.

  if (in_genotype.GetNumCPUs() == 0) {
    genebank.RemoveGenotype(in_genotype);
    return FALSE;
  }

  // Do not adjust if this was and still is the best genotype, or is
  // otherwise in the proper spot...

  if (CheckPos(in_genotype)) {
    return TRUE;
  }

  // Otherwise, remove it from the queue for just the moment.

  Remove(in_genotype);

  // Also, if this genotype is the best, put it there.

  if (in_genotype.GetNumCPUs() > best->GetNumCPUs()) {
    Insert(in_genotype, best->GetPrev());
    best = &in_genotype;
    return TRUE;
  }

  // Finally, find out where this genotype *does* go.

  while (cur_genotype->GetNumCPUs() >= in_genotype.GetNumCPUs() &&
	 cur_genotype != best->GetPrev()) {
    cur_genotype = cur_genotype->GetNext();
  }
  while (cur_genotype->GetNumCPUs() < in_genotype.GetNumCPUs() &&
	 cur_genotype != best) {
    cur_genotype = cur_genotype->GetPrev();
  }

  Insert(in_genotype, cur_genotype);

  return TRUE;
}


cGenotype * cGenotypeControl::Find(const cCodeArray & in_code) const
{
  int i;
  cGenotype * cur_genotype = best;

  for (i = 0; i < size; i++) {
    if (in_code == cur_genotype->GetCode()) {
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

int cSpeciesQueue::OK(int queue_type)
{
  cSpecies * cur_species = NULL;
  int count = 0;
  int ret_value = TRUE;

  while (first && cur_species != first) {
    // If we are just starting, set cur_species to the first element.
    if (!cur_species) cur_species = first;

    // Check that the list is correct in both directions...

    if (cur_species->GetNext()->GetPrev() != cur_species) {
      g_debug.Error("List discontiguous in rev. direction at %d");
      ret_value = FALSE;
    }

    // Check to make sure the current species is OK() and that it should
    // indeed be in this list.

    if (!cur_species->OK()) ret_value = FALSE;

    if (queue_type != cur_species->GetQueueType()) {
      g_debug.Warning("Species in incorrect control list!");
      ret_value = FALSE;
    }
    count++;

    if (count > size) {
      g_debug.Error("Queue has more species than size indicates.");
      ret_value = FALSE;
      break;
    }

    cur_species = cur_species->GetNext();
  }

  if (count != size) {
    g_debug.Error("Incorrect size in species queue.");
    ret_value = FALSE;
  }

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
  // Only adjust if this creature is in the active queue.

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

int cSpeciesControl::OK()
{
  int ret_value = TRUE;

  // Check the queues.

  if (!active_queue.OK(SPECIES_QUEUE_ACTIVE)) {
    g_debug.Warning("Problem in Active Species Queue");
    ret_value = FALSE;
  }

  if (!inactive_queue.OK(SPECIES_QUEUE_INACTIVE)) {
    g_debug.Warning("Problem in Inactive Species Queue");
    ret_value = FALSE;
  }

  if (!garbage_queue.OK(SPECIES_QUEUE_GARBAGE)) {
    g_debug.Warning("Problem in Garbage Species Queue");
    ret_value = FALSE;
  }

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
      cur_count = cur_species->Compare(in_genotype.GetCode(),
				       cConfig::GetThreshold());
      if (cur_count <= cConfig::GetThreshold() && cur_count < best_count) {
	found_species = cur_species;
	best_count = cur_count;
      }
      cur_species = cur_species->GetNext();
    }
  }

  if (record_level == SPECIES_RECORD_LIMITED) {
    cur_species = in_genotype.GetSpecies();

    if (cur_species) {
      int num_diff = cur_species->Compare(in_genotype.GetCode(),
					  cConfig::GetThreshold());
      if (num_diff <= cConfig::GetThreshold()) found_species = cur_species;
    }
  }

  return found_species;
}

void cSpeciesControl::Purge()
{
  cSpecies * cur_species = garbage_queue.GetFirst();
  for (int i = 0; i < garbage_queue.GetSize(); i++) {
    cStats::RemoveSpecies(cur_species->GetID(),
			  cur_species->GetParentID(),
			  cur_species->GetTotalGenotypes(),
			  cur_species->GetTotalCreatures(),
			  cur_species->GetAge());
    cur_species = cur_species->GetNext();
  }
  garbage_queue.Purge();
}


///////////////
//  cGenebank
///////////////

cGenebank::cGenebank(cPopulation * in_population)
{
  population = in_population;
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

  species_control->Purge();
  if (best_genotype->GetID() != prev_dom) {
    genotype_dom_time = 0;
    prev_dom = best_genotype->GetID();
  }
  else {
    genotype_dom_time++;
    if (genotype_dom_time == cConfig::GetGenotypePrintDom()) {
      cString filename;
      filename.Set("genebank/%s", best_genotype->GetName()());
      cTestCPU::PrintCode(best_genotype->GetCode(), filename, best_genotype);
    }
  }
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

cGenotype * cGenebank::AddGenotype(const cCodeArray & in_code,
				   cSpecies * parent_species)
{
  int list_num = FindCRC(in_code) % GENOTYPE_HASH_SIZE;
  cGenotype * found_genotype;

  found_genotype = active_genotypes[list_num].Find(in_code);

  if (!found_genotype) {
    found_genotype = new cGenotype(cStats::GetUpdate());
    found_genotype->SetCode(in_code);
    active_genotypes[list_num].Insert(*found_genotype);
    genotype_control->Insert(*found_genotype);
    cStats::AddGenotype(found_genotype->GetID());

    // Speciation... If we are creating a new genotype here, we must
    // initilize it to the species of its parent genotype.

    found_genotype->SetSpecies(parent_species);
    if (parent_species) parent_species->AddGenotype();
  }

  return found_genotype;
}

cGenotype * cGenebank::FindGenotype(const cCodeArray & in_code) const
{
  int list_num = FindCRC(in_code) % GENOTYPE_HASH_SIZE;
  return active_genotypes[list_num].Find(in_code);
}

void cGenebank::RemoveGenotype(cGenotype & in_genotype)
{
  int list_num = FindCRC(in_genotype.GetCode()) % GENOTYPE_HASH_SIZE;

#ifdef GENEOLOGY
  population->GetGeneology()->GenotypeDeath(&in_genotype);
#endif

  active_genotypes[list_num].Remove(in_genotype);

  genotype_control->Remove(in_genotype);

  // Handle the relevent statistics...
  cStats::RemoveGenotype(in_genotype.GetID(),
      in_genotype.GetParentID(), in_genotype.GetParentDistance(),
      in_genotype.GetDepth(),
      in_genotype.GetTotalCPUs(), in_genotype.GetTotalParasites(),
      in_genotype.GetAge(), in_genotype.GetLength());
  if (in_genotype.GetThreshold()) {
    cStats::RemoveThreshold(in_genotype.GetID());
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
      found_species = new cSpecies(in_genotype.GetCode());
      if (in_genotype.GetSpecies())
	found_species->SetParentID(in_genotype.GetSpecies()->GetID());
      species_control->SetActive(*found_species);
      cStats::AddSpecies(found_species->GetID());

      // Since this is a new species, see if we should be printing it.

      if (cConfig::GetSpeciesPrint()) {
	char filename[40];
	sprintf(filename, "genebank/spec-%04d", found_species->GetID());
	cTestCPU::PrintCode(in_genotype.GetCode(), filename, &in_genotype);
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
    cStats::AddThreshold(in_genotype.GetID(), in_genotype.GetName()(),
			 found_species->GetID());
  } else {
    cStats::AddThreshold(in_genotype.GetID(), in_genotype.GetName()());
  }

  // Print the genotype?

  if (cConfig::GetGenotypePrint()) {
    char filename[40];
    sprintf(filename, "genebank/%s", in_genotype.GetName()());
    cTestCPU::PrintCode(in_genotype.GetCode(), filename, &in_genotype);
  }
}

int cGenebank::AdjustGenotype(cGenotype & in_genotype)
{
  if (!genotype_control->Adjust(in_genotype)) return FALSE;

  if ((in_genotype.GetNumCPUs() >= cConfig::GetThreshold() ||
       &in_genotype == genotype_control->GetBest()) &&
      !(in_genotype.GetThreshold())) {
    ThresholdGenotype(in_genotype);
  }

  return TRUE;
}

int cGenebank::SaveClone(ofstream & fp)
{
  // This method just save the counts at each size-class of genotypes.
  // The rest is reconstructable.

  // Save the numbers of creatures we're up to at each size.
  fp << MAX_CREATURE_SIZE << " ";
  for (int i = 0; i < MAX_CREATURE_SIZE; i++) {
    fp << genotype_count[i] << " ";
  }

  return TRUE;
}

int cGenebank::LoadClone(ifstream & fp)
{
  // This method just restores the counts at each size-class of genotypes.
  // LoadCode into the CPUs should handle the rest from there.

  // Load the numbers of creatures we're up to at each size.
  int max_size;
  fp >> max_size;
  assert_warning(max_size <= MAX_CREATURE_SIZE, "Max creature size too small");
  for (int i = 0; i < max_size && i < MAX_CREATURE_SIZE; i++) {
    fp >> genotype_count[i];
  }

  return TRUE;
}

int cGenebank::OK()
{
  int ret_value = TRUE;
  int i;

  // Check components...

  if (!genotype_control->OK() || !species_control->OK()) {
    ret_value = FALSE;
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
    if (cur_species->debug_num_genotypes != cur_species->GetNumGenotypes()) {
      g_debug.Error("UD %d: Species [%d] has %d genotypes but records %d",
		    cStats::GetUpdate(), cur_species->GetID(),
		    cur_species->debug_num_genotypes,
		    cur_species->GetNumGenotypes());
      ret_value = FALSE;
    }
    if (cur_species->debug_num_threshold != cur_species->GetNumThreshold()) {
      g_debug.Error("Species num_threshold incorrect!");
      ret_value = FALSE;
    }
    if (cur_species->debug_num_genotypes < cur_species->debug_num_threshold) {
      g_debug.Error("Active species has more threshold than genotypes.");
      ret_value = FALSE;
    }
    if (!cur_species->debug_num_threshold) {
      g_debug.Error("Species in active queue with no threshold");
      ret_value = FALSE;
    }
    cur_species = cur_species->GetNext();
  }

  cur_species = NULL;
  while (cur_species != first_inactive) {
    if (!cur_species) cur_species = first_inactive;
    if (cur_species->debug_num_genotypes != cur_species->GetNumGenotypes()) {
      g_debug.Error("Species num_genotypes incorrect!");
      ret_value = FALSE;
    }
    if (cur_species->debug_num_threshold != cur_species->GetNumThreshold()) {
      g_debug.Error("Species num_threshold incorrect!");
      ret_value = FALSE;
    }
    if (cur_species->debug_num_threshold) {
      g_debug.Error("Species in inactive queue with threshold genotype.");
      ret_value = FALSE;
    }
    if (!cur_species->debug_num_genotypes) {
      g_debug.Error("Species with not genotypes markes only inactive.");
      ret_value = FALSE;
    }
  }

  cur_species = NULL;
  while (cur_species != first_garbage) {
    if (!cur_species) cur_species = first_garbage;
    if (cur_species->debug_num_genotypes || cur_species->debug_num_threshold) {
      g_debug.Error("Non-empty species in garbage queue.");
      ret_value = FALSE;
    }
  }

#endif

  // Loop through all of the reference lists for matching genotypes...

  for (i = 0; i < GENOTYPE_HASH_SIZE; i++) {
    if (!active_genotypes[i].OK()) {
      g_debug.Warning("active genotype list is failing!");
    }
  }

  if (!ret_value) g_debug.Warning("Genebank::OK() is failing!");

  return ret_value;
}

int cGenebank::CountNumCreatures()
{
  int i;
  int total = 0;

  genotype_control->Reset(0);
  for (i = 0; i < genotype_control->GetSize(); i++) {
    total += genotype_control->Get(0)->GetNumCPUs();
    genotype_control->Next(0);
  }

  return total;
}

unsigned int cGenebank::GetTotalMemory()
{
  int i;
  int total = 0;

  genotype_control->Reset(0);
  for (i = 0; i < genotype_control->GetSize(); i++) {
    total += genotype_control->Get(0)->GetLength();
    genotype_control->Next(0);
  }

  return total;
}

unsigned int cGenebank::FindCRC(const cCodeArray & in_code) const
{
  unsigned int total = 13;
  int i;

  for (i = 0; i < in_code.GetSize(); i++) {
    total *= in_code.Get(i).GetOp() + 10 + i << 6;
    total += 3;
  }

  return total;
}

void cGenebank::SpeciesTest(char * message, cGenotype & genotype)
{
  cSpecies * cur_species = genotype.GetSpecies();

  if (cur_species) {
    g_debug.Comment("UD %d: %s on genotype [%d] (size %d) of species [%d]",
		    cStats::GetUpdate(), message, genotype.GetID(),
		    genotype.GetNumCPUs(), cur_species->GetID());
  } else {
    g_debug.Comment("UD %d: %s on genotype [%d] (size %d) (no species)",
	    cStats::GetUpdate(), message, genotype.GetID(),
	    genotype.GetNumCPUs());
  }

  if (cur_species) {
    g_debug.Comment("   Species [%d] has %d gen and %d thresh. (list %d)",
	    cur_species->GetID(), cur_species->GetNumGenotypes(),
	    cur_species->GetNumThreshold(), cur_species->GetQueueType());
  }
}
