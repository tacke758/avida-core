//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "playback.hh"
#include <stdlib.h>

#include "../cpu/head.ii"
#include "../cpu/cpu.ii"

cPlaybackPopulation::cPlaybackPopulation(const cGenesis & in_genesis)
 : cPopulation(in_genesis)
{
  world_x = Genesis().ReadInt("WORLD-X");
  world_y = Genesis().ReadInt("WORLD-Y");
  num_cells = world_x * world_y;
  local_stats.SetNumCells(num_cells);
  pb_genebank = new cPlaybackGenebank;
  num_creatures = 0;
   
  cpu_array = new cPlaybackCPU[num_cells];

  fp_creatures = fopen("creature.log", "r");
  fp_genotypes = fopen("genotype.log", "r");
  fp_threshold = fopen("threshold.log", "r");
  fp_species = fopen("species.log", "r");

  fscanf(fp_creatures, "%d %d %d", &next_UD, &next_loc, &next_gen_id);
}

cPlaybackPopulation::~cPlaybackPopulation()
{
  delete [] cpu_array;

  fclose(fp_creatures);
  fclose(fp_genotypes);
  fclose(fp_threshold);
  fclose(fp_species);
}

void cPlaybackPopulation::DoUpdate()
{
  int cur_update = stats.GetUpdate();
  cPlaybackGenotype * temp_gen;
  
  while (next_UD <= cur_update) {
    int old_id = cpu_array[next_loc].GetID();
    temp_gen = pb_genebank->Inc(next_gen_id);
    if (old_id >= 0) pb_genebank->Dec(old_id);
    else num_creatures++;
    cpu_array[next_loc].SetGenotypeID(next_gen_id);
    cpu_array[next_loc].SetGenotype(temp_gen);
      
    if (fscanf(fp_creatures, "%d %d %d", &next_UD, &next_loc, &next_gen_id)
	== EOF) {
      printf("Done!\n");
      exit(0);
    }
  }

  pb_genebank->AssignSymbols();
}

int cPlaybackPopulation::GetNumGenotypes()
{
 return pb_genebank->GetSize();
}

char * cPlaybackPopulation::GetBasicGrid()
{
  char * out_grid = g_memory.Get(num_cells);

  for (int i = 0; i < num_cells; i++) {
    if (cpu_array[i].GetID() != -1) out_grid[i] = '.';
    else out_grid[i] = ' ';

    if (cpu_array[i].GetGenotype() &&
	cpu_array[i].GetGenotype()->GetFlag(GENOTYPE_FLAG_THRESHOLD)) {
      out_grid[i] = 'o';
    }

    if (cpu_array[i].GetGenotype() &&
	cpu_array[i].GetGenotype()->GetSymbol()) {
      out_grid[i] = cpu_array[i].GetGenotype()->GetSymbol();
    }

//    if (cpu_array[i].GetID() == pb_genebank->GetBestID())
//      out_grid[i] = 'X';      
  }

  return out_grid;
}

char * cPlaybackPopulation::GetResourceGrid()
{
  char * out_grid = g_memory.Get(num_cells);

  for (int i = 0; i < num_cells; i++) {
    out_grid[i] = '-';
  }

  return out_grid;
}

///////////////////
//  cPlaybackCPU
///////////////////

cPlaybackCPU::cPlaybackCPU()
{
  genotype_id = -1;
  genotype = NULL;
}

///////////////////////
//  cPlaybackGenotype
///////////////////////

cPlaybackGenotype::cPlaybackGenotype(int in_id)
{
  genotype_id = in_id;
  num_CPUs = 0;
  symbol = 0;
  position = -1;

  next = NULL;
  prev = NULL;
}

cPlaybackGenotype::~cPlaybackGenotype()
{
}

//////////////////////
//  cPlaybackGenebank
//////////////////////

cPlaybackGenebank::cPlaybackGenebank()
{
  head = new cPlaybackGenotype(-1);
  head->SetNext(head);
  head->SetPrev(head);

  size = 0;
  threshold = 3; // genesis.ReadInt("THRESHOLD");

  symbol[0] = '|';
  symbol[1] = '@';
  symbol[2] = '*';
  symbol[3] = '=';
  symbol[4] = 'H';

  for(int i = 0; i < NUM_PB_SYMBOLS; i++) {
    symbol_loc[i] = NULL;
  }
}

cPlaybackGenotype * cPlaybackGenebank::Inc(int id_num)
{
  // Find the genotype with this ID.

  cPlaybackGenotype * temp_gen = Find(id_num);

  // If no genotype can be found, create one...
  
  if (!temp_gen) temp_gen = Insert(id_num);

  // Otherwise, make the necessary modifications to it.

  else {
    temp_gen->IncCPUs();

    // Mark it as threshold if it has grown populous enough.

    if (temp_gen->GetNumCPUs() >= threshold) {
      temp_gen->SetFlag(GENOTYPE_FLAG_THRESHOLD);
    }

    // Adjust the position of the genotype in the list.

    while (temp_gen->GetNumCPUs() > temp_gen->GetPrev()->GetNumCPUs() &&
	   temp_gen->GetPrev() != head) {
      ShiftPrev(temp_gen);
    }
  }

  return temp_gen;
}

void cPlaybackGenebank::Dec(int id_num)
{
  cPlaybackGenotype * temp_gen = Find(id_num);
  if (!temp_gen) g_debug.Error("Trying to remove non-existant pb_genotype");

  temp_gen->DecCPUs();
  if (!temp_gen->GetNumCPUs()) Remove(temp_gen);
  else {
    while((temp_gen->GetNumCPUs() < temp_gen->GetNext()->GetNumCPUs()) &&
	  (temp_gen->GetNext() != head)) {
      ShiftNext(temp_gen);
    }
  }
}

void cPlaybackGenebank::AssignSymbols()
{
  int i, j;

  // First screen through the existing symbols to make sure they are only
  // 'held' by legal genotypes...

  for (i = 0; i < NUM_PB_SYMBOLS; i++) {
    if (symbol_loc[i] && symbol_loc[i]->GetPosition() >= NUM_PB_SYMBOLS) {
      symbol_loc[i]->SetSymbol(0);
      symbol_loc[i] = NULL;
    }
  }

  // Next, go through the first PB_SYMBOL_THRESHOLD genotypes to make sure they
  // all do have a symbol...
  
  cPlaybackGenotype * temp_gen = head->GetNext();
  for (i = 0; i < PB_SYMBOL_THRESHOLD && temp_gen != head; i++) {
    if (!(temp_gen->GetSymbol())) {
      for (j = 0; j < NUM_PB_SYMBOLS; j++) {
	if (symbol_loc[j] == NULL) {
	  temp_gen->SetSymbol(symbol[j]);
	  symbol_loc[j] = temp_gen;
	  break;
	}
      }
    }
    temp_gen = temp_gen->GetNext();
  }

  OK();
}


int cPlaybackGenebank::GetBestID()
{
  if (head->GetNext() != head) {
    return head->GetNext()->GetID();
  }
  return -2;
}


cPlaybackGenotype * cPlaybackGenebank::Find(int id_num)
{
  cPlaybackGenotype * cur_gen;

  for (cur_gen = head->GetNext();
       cur_gen != head;
       cur_gen = cur_gen->GetNext()) {
    if (cur_gen->GetID() == id_num) {
      return cur_gen;
    }
  }

  return NULL;
}

cPlaybackGenotype * cPlaybackGenebank::Insert(int id_num)
{
  cPlaybackGenotype * temp_gen;
  temp_gen = new cPlaybackGenotype(id_num);
  temp_gen->IncCPUs();
  temp_gen->SetPosition(size);

  temp_gen->SetPrev(head->GetPrev());
  temp_gen->SetNext(head);
  head->GetPrev()->SetNext(temp_gen);
  head->SetPrev(temp_gen);

  size++;

  return temp_gen;
}

void cPlaybackGenebank::Remove(cPlaybackGenotype * in_pb_genotype)
{
  cPlaybackGenotype * next_gen = in_pb_genotype->GetNext();

  // Remove the symbol of the genotype if it has one...

  if (in_pb_genotype->GetSymbol()) {
    // Find which symbol the genotype had...
    for (int i = 0; i < NUM_PB_SYMBOLS; i++) {
      if (symbol[i] == in_pb_genotype->GetSymbol()) {
	symbol_loc[i] = NULL;
	in_pb_genotype->SetSymbol(0);
	break;
      }
    }
  }

  // Re-adjust the connections in the list, and remove the genotype...

  in_pb_genotype->GetPrev()->SetNext(in_pb_genotype->GetNext());
  in_pb_genotype->GetNext()->SetPrev(in_pb_genotype->GetPrev());
  in_pb_genotype->SetNext(NULL);
  in_pb_genotype->SetPrev(NULL);
  delete in_pb_genotype;

  // Re-adjust the positions of all the genotypes after the one removed...

  while (next_gen != head) {
    next_gen->SetPosition(next_gen->GetPosition() - 1);
    next_gen = next_gen->GetNext();
  }

  size--;
}

void cPlaybackGenebank::ShiftNext(cPlaybackGenotype * in_pb_genotype)
{
  cPlaybackGenotype * first = in_pb_genotype;
  cPlaybackGenotype * second = in_pb_genotype->GetNext();
  cPlaybackGenotype * full_next = second->GetNext();
  cPlaybackGenotype * full_prev = first->GetPrev();

  first->SetNext(full_next);
  first->SetPrev(second);
  second->SetNext(first);
  second->SetPrev(full_prev);
  full_next->SetPrev(first);
  full_prev->SetNext(second);

  // Re-adjust the positions of the swapped genotypes.

  first->SetPosition(first->GetPosition() + 1);
  second->SetPosition(second->GetPosition() - 1);
}

void cPlaybackGenebank::ShiftPrev(cPlaybackGenotype * in_pb_genotype)
{
  if (in_pb_genotype->GetPrev() == head) {
    g_debug.Warning("Prev in ShiftPrev() is the head of the list");
  }

  cPlaybackGenotype * first = in_pb_genotype->GetPrev();
  cPlaybackGenotype * second = in_pb_genotype;
  cPlaybackGenotype * full_next = second->GetNext();
  cPlaybackGenotype * full_prev = first->GetPrev();

  first->SetNext(full_next);
  first->SetPrev(second);
  second->SetNext(first);
  second->SetPrev(full_prev);
  full_next->SetPrev(first);
  full_prev->SetNext(second);

  // Re-adjust the positions of the swapped genotypes.

  first->SetPosition(first->GetPosition() + 1);
  second->SetPosition(second->GetPosition() - 1);
}

int cPlaybackGenebank::OK()
{
  int ret_ok = TRUE;

  if (head->GetID() != -1) {
    g_debug.Warning("Head ID() != -1");
    ret_ok = FALSE;
  }
  if (head->GetNumCPUs() != 0) {
    g_debug.Warning("Head CPUs != 0");
    ret_ok = FALSE;
  }
  
  int real_size = 0;
  cPlaybackGenotype * temp_gen = head->GetNext();

  while (real_size < 1000 && temp_gen != head) {
    if (temp_gen->GetPosition() != real_size) {
      g_debug.Comment("Mis-aligned position in genebank!");
    }
    temp_gen = temp_gen->GetNext();
    real_size++;
  }

  if (size != real_size) {
    g_debug.Error("real_size = %d; size = %d", real_size, size);
    ret_ok = FALSE;
  }

  return ret_ok;
}

