//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include <fstream>

#include "inject_genebank.hh"

#include "inject_genotype.hh"
#include "config.hh"
#include "stats.hh"

#include "../cpu/test_util.hh"

using namespace std;

/////////////////////
//  cInjectGenotypeQueue
/////////////////////

cInjectGenotypeQueue::cInjectGenotypeQueue()
{
  size = 0;
  root.SetNext(&root);
  root.SetPrev(&root);
}


cInjectGenotypeQueue::~cInjectGenotypeQueue()
{
  while (root.GetNext() != &root) {
    Remove(root.GetNext());
  }
}

bool cInjectGenotypeQueue::OK()
{
  bool result = true;
  int count = 0;

  for (cInjectGenotypeElement * temp_element = root.GetNext();
       temp_element != &root;
       temp_element = temp_element->GetNext()) {
    assert (temp_element->GetNext()->GetPrev() == temp_element);
    assert (temp_element->GetInjectGenotype()->GetID() >= 0);

    count++;
    assert (count <= size);
  }

  assert (count == size);

  return result;
}

void cInjectGenotypeQueue::Insert(cInjectGenotype & in_inject_genotype)
{
  cInjectGenotypeElement * new_element = new cInjectGenotypeElement(&in_inject_genotype);
  new_element->SetNext(root.GetNext());
  new_element->SetPrev(&root);
  root.GetNext()->SetPrev(new_element);
  root.SetNext(new_element);
  size++;
}

void cInjectGenotypeQueue::Remove(cInjectGenotype & in_inject_genotype)
{
  cInjectGenotypeElement * cur_element;

  for (cur_element = root.GetNext();
       cur_element != &root;
       cur_element = cur_element->GetNext()) {
    if (cur_element->GetInjectGenotype() == &in_inject_genotype) break;
  }

  assert (cur_element != &root);

  Remove(cur_element);
}

void cInjectGenotypeQueue::Remove(cInjectGenotypeElement * in_element)
{
  in_element->GetPrev()->SetNext(in_element->GetNext());
  in_element->GetNext()->SetPrev(in_element->GetPrev());
  in_element->SetNext(NULL);
  in_element->SetPrev(NULL);
  delete(in_element);

  size--;
}

cInjectGenotype * cInjectGenotypeQueue::Find(const cGenome & in_genome) const
{
  for (cInjectGenotypeElement * cur_element = root.GetNext();
       cur_element != &root;
       cur_element = cur_element->GetNext()) {
    if (cur_element->GetInjectGenotype()->GetGenome() == in_genome) {
      return cur_element->GetInjectGenotype();
    }
  }

  return NULL;
}


////////////////////////////
//  cInjectGenotypeControl
////////////////////////////

cInjectGenotypeControl::cInjectGenotypeControl(cInjectGenebank & in_gb) : genebank(in_gb)
{
  size = 0;
  best = NULL;
  coalescent = NULL;
  for (int i = 0; i < INJECTGENOTYPE_THREADS; i++) threads[i] = NULL;

  historic_list = NULL;
  historic_count = 0;
}

cInjectGenotypeControl::~cInjectGenotypeControl()
{
}

bool cInjectGenotypeControl::OK()
{
  int ret_value = true;

  // Cycle through the list, making sure all connections are proper, size
  // is correct, and all genotypes are OK().

  cInjectGenotype * cur_pos = best;
  for (int i = 0; i < size; i++) {
    if (!cur_pos->OK()) ret_value = false;
    assert (cur_pos->GetNext()->GetPrev() == cur_pos);
    cur_pos = cur_pos->GetNext();
  }

  assert (cur_pos == best);

  return ret_value;
}

void cInjectGenotypeControl::Insert(cInjectGenotype & in_inject_genotype, cInjectGenotype * prev_genotype)
{
  if (prev_genotype == NULL) {
    assert(size == 0); // Destroying a full genotype queue...

    best = &in_inject_genotype;
    best->SetNext(best);
    best->SetPrev(best);
  }
  else {
    in_inject_genotype.SetPrev(prev_genotype);
    in_inject_genotype.SetNext(prev_genotype->GetNext());
    prev_genotype->SetNext(&in_inject_genotype);
    in_inject_genotype.GetNext()->SetPrev(&in_inject_genotype);
  }

  size++;
}

void cInjectGenotypeControl::Remove(cInjectGenotype & in_inject_genotype)
{
  if (size == 1) {
    best = NULL;
  }
  if (&in_inject_genotype == best) {
    best = best->GetNext();
  }

  in_inject_genotype.GetNext()->SetPrev(in_inject_genotype.GetPrev());
  in_inject_genotype.GetPrev()->SetNext(in_inject_genotype.GetNext());
  in_inject_genotype.SetNext(NULL);
  in_inject_genotype.SetPrev(NULL);

  size--;
}

void cInjectGenotypeControl::RemoveHistoric(cInjectGenotype & in_inject_genotype)
{
  if (historic_count == 1) {
    historic_list = NULL;
  }
  if (&in_inject_genotype == historic_list) {
    historic_list = historic_list->GetNext();
  }

  in_inject_genotype.GetNext()->SetPrev(in_inject_genotype.GetPrev());
  in_inject_genotype.GetPrev()->SetNext(in_inject_genotype.GetNext());
  in_inject_genotype.SetNext(NULL);
  in_inject_genotype.SetPrev(NULL);

  historic_count--;
}

void cInjectGenotypeControl::InsertHistoric(cInjectGenotype & in_inject_genotype)
{
  if (historic_count == 0) {
    in_inject_genotype.SetNext(&in_inject_genotype);
    in_inject_genotype.SetPrev(&in_inject_genotype);
  }
  else {
    in_inject_genotype.SetPrev(historic_list->GetPrev());
    in_inject_genotype.SetNext(historic_list);
    historic_list->GetPrev()->SetNext(&in_inject_genotype);
    historic_list->SetPrev(&in_inject_genotype);
  }

  historic_list = &in_inject_genotype;
  historic_count++;
}

/*int cInjectGenotypeControl::UpdateCoalescent()
{
  // Test to see if any updating needs to be done...
  // Don't update active coalescent genotype, or if there is more than
  // one offspring.
  if (coalescent != NULL &&
      (coalescent->GetNumInjected() > 0) ||
      coalescent->GetNumOffspringGenotypes() > 1) ) {
    return coalescent->GetDepth();
  }

  // If there is no best, there is nothing to search through...
  if (best == NULL) return -1;

  // Find the new point...
  cInjectGenotype * test_gen = best;
  cInjectGenotype * found_gen = best;
  cInjectGenotype * parent_gen = best->GetParentGenotype();

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
}*/


bool cInjectGenotypeControl::CheckPos(cInjectGenotype & in_inject_genotype)
{
  int next_OK = false;
  int prev_OK = false;

  if (in_inject_genotype.GetNumInjected() >= in_inject_genotype.GetNext()->GetNumInjected()) {
    next_OK =true;
  }
  if (in_inject_genotype.GetNumInjected() <= in_inject_genotype.GetPrev()->GetNumInjected()) {
    prev_OK =true;
  }

  if ((&in_inject_genotype == best && next_OK) ||
      (next_OK && prev_OK) ||
      (&in_inject_genotype == best->GetPrev() && prev_OK)) {
    return true;
  }

  return false;
}

void cInjectGenotypeControl::Insert(cInjectGenotype & new_genotype)
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

bool cInjectGenotypeControl::Adjust(cInjectGenotype & in_inject_genotype)
{
  //if (in_inject_genotype.GetDeferAdjust() == true) return true;

  cInjectGenotype * cur_inject_genotype = in_inject_genotype.GetPrev();

  // Check to see if this genotype should be removed completely.

  if (in_inject_genotype.GetNumInjected() == 0) {
    genebank.RemoveInjectGenotype(in_inject_genotype);
    return false;
  }

  // Do not adjust if this was and still is the best genotype, or is
  // otherwise in the proper spot...

  if (CheckPos(in_inject_genotype)) {
    return true;
  }

  // Otherwise, remove it from the queue for just the moment.

  Remove(in_inject_genotype);

  // Also, if this genotype is the best, put it there.

  if (in_inject_genotype.GetNumInjected() > best->GetNumInjected()) {
    Insert(in_inject_genotype, best->GetPrev());
    best = &in_inject_genotype;
    return true;
  }

  // Finally, find out where this genotype *does* go.

  while (cur_inject_genotype->GetNumInjected() >= in_inject_genotype.GetNumInjected() &&
	 cur_inject_genotype != best->GetPrev()) {
    cur_inject_genotype = cur_inject_genotype->GetNext();
  }
  while (cur_inject_genotype->GetNumInjected() < in_inject_genotype.GetNumInjected() &&
	 cur_inject_genotype != best) {
    cur_inject_genotype = cur_inject_genotype->GetPrev();
  }

  Insert(in_inject_genotype, cur_inject_genotype);

  return true;
}


cInjectGenotype * cInjectGenotypeControl::Find(const cGenome & in_genome) const
{
  int i;
  cInjectGenotype * cur_inject_genotype = best;

  for (i = 0; i < size; i++) {
    if (in_genome == cur_inject_genotype->GetGenome()) {
      return cur_inject_genotype;
    }
    cur_inject_genotype = cur_inject_genotype->GetNext();
  }

  return NULL;
}

int cInjectGenotypeControl::FindPos(cInjectGenotype & in_inject_genotype, int max_depth)
{
  cInjectGenotype * temp_genotype = best;
  if (max_depth < 0 || max_depth > size) max_depth = size;

  for (int i = 0; i < max_depth; i++) {
    if (temp_genotype == &in_inject_genotype) return i;
    temp_genotype = temp_genotype->GetNext();
  }

  return -1;
}

cInjectGenotype * cInjectGenotypeControl::Next(int thread)
{
  return threads[thread] = threads[thread]->GetNext();
}

cInjectGenotype * cInjectGenotypeControl::Prev(int thread)
{
  return threads[thread] = threads[thread]->GetPrev();
}

////////////////////
//  cInjectGenebank
////////////////////

cInjectGenebank::cInjectGenebank(cStats & in_stats)
  : stats(in_stats)
{
  for (int i = 0; i < MAX_CREATURE_SIZE; i++) {
    inject_genotype_count[i] = 0;
  }

  inject_genotype_control = new cInjectGenotypeControl(*this);

}

cInjectGenebank::~cInjectGenebank()
{
  delete inject_genotype_control;
}

void cInjectGenebank::UpdateReset()
{
  static int genotype_dom_time = 0;
  static int prev_dom = -1;

  cInjectGenotype * best_inject_genotype = GetBestInjectGenotype();

  if (best_inject_genotype && best_inject_genotype->GetID() != prev_dom) {
    genotype_dom_time = 0;
    prev_dom = best_inject_genotype->GetID();
  }
  else {
    genotype_dom_time++;
    if (genotype_dom_time == cConfig::GetGenotypePrintDom()) {
      cString filename;
      filename.Set("genebank/%s", best_inject_genotype->GetName()());
      cTestUtil::PrintGenome(best_inject_genotype, best_inject_genotype->GetGenome(), 
			     filename, stats.GetUpdate());
    }
  }
}

cString cInjectGenebank::GetLabel(int in_size, int in_num)
{
  char alpha[6];
  char full_name[12];
  int i;

  for (i = 4; i >= 0; i--) {
    alpha[i] = (in_num % 26) + 'a';
    in_num /= 26;
  }
  alpha[5] = '\0';

  sprintf(full_name, "p%03d-%s", in_size, alpha);

  return full_name;
}

void cInjectGenebank::AddInjectGenotype(cInjectGenotype * in_inject_genotype, int in_list_num)
{
  assert( in_inject_genotype != 0 );
  
  if ( in_list_num < 0 )
    in_list_num = FindCRC(in_inject_genotype->GetGenome()) % INJECTGENOTYPE_HASH_SIZE;
  
  active_inject_genotypes[in_list_num].Insert(*in_inject_genotype);
  inject_genotype_control->Insert(*in_inject_genotype);
  //stats.AddGenotype(in_inject_genotype->GetID());
}


cInjectGenotype * cInjectGenebank::AddInjectGenotype(const cGenome & in_genome,
				   cInjectGenotype * parent_genotype)
{
  int list_num = FindCRC(in_genome) % INJECTGENOTYPE_HASH_SIZE;
  cInjectGenotype * found_genotype;

  found_genotype = active_inject_genotypes[list_num].Find(in_genome);

  if (!found_genotype) {
    found_genotype = new cInjectGenotype(stats.GetUpdate());
    found_genotype->SetGenome(in_genome);
    found_genotype->SetParent(parent_genotype);
    if(parent_genotype!=NULL)
      {
	parent_genotype->SetCanReproduce();
      }
    AddInjectGenotype( found_genotype, list_num );
  }
  return found_genotype;
}

cInjectGenotype * cInjectGenebank::FindInjectGenotype(const cGenome & in_genome) const
{
  int list_num = FindCRC(in_genome) % INJECTGENOTYPE_HASH_SIZE;
  return active_inject_genotypes[list_num].Find(in_genome);
}

void cInjectGenebank::RemoveInjectGenotype(cInjectGenotype & in_inject_genotype)
{
  // If this genotype is still active, mark it no longer active and
  // take it out of the hash table so it doesn't have any new organisms
  // assigned to it.

  if (in_inject_genotype.GetActive() == true) {
    int list_num = FindCRC(in_inject_genotype.GetGenome()) % INJECTGENOTYPE_HASH_SIZE;
    active_inject_genotypes[list_num].Remove(in_inject_genotype);
    inject_genotype_control->Remove(in_inject_genotype);
    //in_inject_genotype.Deactivate(stats.GetUpdate());
    if (cConfig::GetTrackMainLineage()) {
      inject_genotype_control->InsertHistoric(in_inject_genotype);
    }
  }

  // If we are tracking the main lineage, we only want to delete a
  // genotype when all of its decendents have also died out.

  /*if (cConfig::GetTrackMainLineage()) {
    // If  there are more offspring genotypes, hold off on deletion...
    if (in_inject_genotype.GetNumOffspringGenotypes() != 0) return;

    // If this is a dead end, delete it and recurse up...
    cInjectGenotype * parent = in_inject_genotype.GetParentGenotype();
    if (parent != NULL) {
      parent->RemoveOffspringGenotype();

      // Test to see if we need to update the coalescent genotype.
      const int new_coal = inject_genotype_control->UpdateCoalescent();
      stats.SetCoalescentGenotypeDepth(new_coal);
      // cout << "Set coalescent to " << found_gen->GetDepth() << endl;

      if (parent->GetNumInjected() == 0) {
	// Regardless, run RemoveGenotype on the parent.
	RemoveGenotype(*parent);
      }
    }

    inject_genotype_control->RemoveHistoric(in_inject_genotype);
  }

  // Handle the relevent statistics...
  stats.RemoveGenotype(in_inject_genotype.GetID(),
	      in_inject_genotype.GetParentID(), in_inject_genotype.GetParentDistance(),
	      in_inject_genotype.GetDepth(), in_inject_genotype.GetTotalOrganisms(),
              in_inject_genotype.GetTotalParasites(),
	      stats.GetUpdate() - in_inject_genotype.GetUpdateBorn(),
              in_inject_genotype.GetLength());
  if (in_inject_genotype.GetThreshold()) {
  stats.RemoveThreshold(in_inject_genotype.GetID());
  }*/

  delete &in_inject_genotype;
}

void cInjectGenebank::ThresholdInjectGenotype(cInjectGenotype & in_inject_genotype)
{
  in_inject_genotype.SetName( GetLabel(in_inject_genotype.GetLength(),
				inject_genotype_count[in_inject_genotype.GetLength()]++) );
  in_inject_genotype.SetThreshold();

  //stats.AddThreshold(in_inject_genotype.GetID(), in_inject_genotype.GetName()());
  
  // Print the genotype?

  if (cConfig::GetGenotypePrint()) {
    cString filename;
    filename.Set("genebank/%s", in_inject_genotype.GetName()());
    //cTestUtil::PrintGenome(in_inject_genotype.GetGenome(), filename,
    //			   &in_inject_genotype, stats.GetUpdate());
  }
}

bool cInjectGenebank::AdjustInjectGenotype(cInjectGenotype & in_inject_genotype)
{
  if (!inject_genotype_control->Adjust(in_inject_genotype)) return false;

  if ((in_inject_genotype.GetNumInjected() >= cConfig::GetThreshold() ||
       &in_inject_genotype == inject_genotype_control->GetBest()) &&
      !(in_inject_genotype.GetThreshold())) {
    ThresholdInjectGenotype(in_inject_genotype);
  }

  return true;
}

bool cInjectGenebank::SaveClone(ofstream & fp)
{
  // This method just save the counts at each size-class of genotypes.
  // The rest is reconstructable.

  // Save the numbers of organisms we're up to at each size.
  fp << MAX_CREATURE_SIZE << " ";
  for (int i = 0; i < MAX_CREATURE_SIZE; i++) {
    fp << inject_genotype_count[i] << " ";
  }

  return true;
}

bool cInjectGenebank::LoadClone(ifstream & fp)
{
  // This method just restores the counts at each size-class of genotypes.
  // The rest of the loading process should be handled elsewhere.

  // Load the numbers of organisms we're up to at each size.
  int max_size;
  fp >> max_size;
  assert (max_size <= MAX_CREATURE_SIZE); // MAX_CREATURE_SIZE too small
  for (int i = 0; i < max_size && i < MAX_CREATURE_SIZE; i++) {
    fp >> inject_genotype_count[i];
  }

  return true;
}

bool cInjectGenebank::DumpTextSummary(ofstream & fp)
{
  inject_genotype_control->Reset(0);
  for (int i = 0; i < inject_genotype_control->GetSize(); i++) {
    cInjectGenotype * genotype = inject_genotype_control->Get(0);
    fp << genotype->GetGenome().AsString() << " "
       << genotype->GetNumInjected() << " "
       << genotype->GetID() << endl;
    inject_genotype_control->Next(0);
  }

  return true;
}

bool cInjectGenebank::DumpDetailedSummary(const cString & file, int update)
{
  inject_genotype_control->Reset(0);
  for (int i = 0; i < inject_genotype_control->GetSize(); i++) {
    DumpDetailedEntry(inject_genotype_control->Get(0), file, update);
    inject_genotype_control->Next(0);
  }
  return true;
}

/*bool cInjectGenebank::DumpHistoricSummary(ofstream & fp)
{
  inject_genotype_control->ResetHistoric(0);
  for (int i = 0; i < inject_genotype_control->GetHistoricCount(); i++) {
    DumpDetailedEntry(inject_genotype_control->Get(0), fp);
    inject_genotype_control->Next(0);
  }

  return true;
}*/

void cInjectGenebank::DumpDetailedEntry(cInjectGenotype * genotype, const cString & filename, int update)
{
  //if(genotype->CanReproduce())
  //  {
      cDataFile & df = stats.GetDataFile(filename);
      
      df.WriteComment( "Avida parasite dump data" );
      df.WriteTimeStamp();
      
      df.Write( genotype->GetID(),                 "parasite genotype ID");
      df.Write( genotype->GetName(),              "parasite genotype name");
      df.Write( genotype->GetParentID(),           "parasite parent ID");
      df.Write( genotype->GetNumInjected(),        "current number of injected creatures with this genotype");
      df.Write( genotype->GetTotalInjected(),      "total number of injected creatures with this genotype");
      df.Write( genotype->GetLength(),             "genotype length");
      df.Write( genotype->GetUpdateBorn(),         "update this genotype was born");
      df.Write( genotype->CanReproduce(),          "has this genotype reproduced?");
      df.Write( genotype->GetGenome().AsString(),  "genome of this genotype");
      df.Endl();
      //}
}

bool cInjectGenebank::OK()
{
  bool ret_value = true;
  int i;

  // Check components...

  if (!inject_genotype_control->OK()) {
    ret_value = false;
  }

  // Loop through all of the reference lists for matching genotypes...

  for (i = 0; i < INJECTGENOTYPE_HASH_SIZE; i++) {
    assert (active_inject_genotypes[i].OK());
  }

  assert (ret_value == true);

  return ret_value;
}

int cInjectGenebank::CountNumCreatures()
{
  int i;
  int total = 0;

  inject_genotype_control->Reset(0);
  for (i = 0; i < inject_genotype_control->GetSize(); i++) {
    total += inject_genotype_control->Get(0)->GetNumInjected();
    inject_genotype_control->Next(0);
  }

  return total;
}


unsigned int cInjectGenebank::FindCRC(const cGenome & in_genome) const
{
  unsigned int total = 13;
  int i;

  for (i = 0; i < in_genome.GetSize(); i++) {
    total *= in_genome[i].GetOp() + 10 + i << 6;
    total += 3;
  }

  return total;
}

