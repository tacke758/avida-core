//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "species.hh"
#include "../cpu/cpu.hh"
#include "../cpu/head.ii"

/////////////////
//  cSpecies
/////////////////

cSpecies::cSpecies(cCodeArray & in_code)
{
  g_memory.Add(C_SPECIES);

  static int species_count = 0;
  id_num = species_count++;
  parent_id = -1;
  symbol = '+';
  update_born = cStats::GetUpdate();

  code = in_code;

  num_genotypes = 0;
  num_threshold = 0;
  total_creatures = 0;
  total_genotypes = 0;

  queue_type = SPECIES_QUEUE_NONE;

  next = NULL;
  prev = NULL;

  // Study the code which was passed in...

  for (int i = 0; i < SPECIES_MAX_DISTANCE; i++) {
    genotype_distance[i] = 0;
  }

  // Determine the copy true of the code...

  cCPUTestInfo test_info;
  cTestCPU::TestCode(test_info, code);

  flags(SPECIES_FLAG_COPY_TRUE, test_info.CalcCopyTrue());
}

cSpecies::~cSpecies()
{
  g_memory.Remove(C_SPECIES);

  int i, total_count = 0;
  for (i = 0; i < SPECIES_MAX_DISTANCE; i++) {
    total_count += genotype_distance[i];
  }

#ifdef TEST
  // Only print out the non-trivial species.

  if (total_count > 1) {
    FILE * fp = stats.GetTestFP();

    fprintf(fp, "Species %3d: ", id_num);
    for (i = 0; i < SPECIES_MAX_DISTANCE; i++) {
      fprintf(fp, "%2d ", genotype_distance[i]);
    }
    fprintf(fp, "\n");
    fflush(fp);
  }
#endif
}

// Also, check phenotypes... @CAO
// Allow failure proportional to size?
int cSpecies::Compare(const cCodeArray & test_code, int max_fail_count)
{
  cCodeArray cross_code;
  cCPUTestInfo test_info;
  int i;
  int fail_count = 0;
  int copy_true;

  // First, make some phenotypic comparisons between creatures.
  // For now, just check that they both copy-true.

  cTestCPU::TestCode(test_info, test_code);
  copy_true = test_info.CalcCopyTrue();

  // If the creatures differ in copy_true, return maximal species difference.
  if (flags[SPECIES_FLAG_COPY_TRUE] != copy_true) {
    return MAX_CREATURE_SIZE;
  }

  // Find the optimal offset between creatures, and related variables.

  int offset = code.FindBestOffset(test_code); // The first line of B is at
                                               // line 'offset' of A.
  int start1 = (offset < 0) ? -offset : 0;
  int start2 = (offset > 0) ? offset : 0;
  int overlap = code.FindOverlap(test_code, offset);

  // The base fail count is the differences outside of creatures.
  fail_count = code.GetSize() + test_code.GetSize() - 2 * overlap;

  // If there is not enough overlap, return maximal difference.
  if (overlap < MIN_CREATURE_SIZE) {
    return MAX_CREATURE_SIZE;
  }

  // Do the crossovers at all posible points.

  for (i = 0; i < overlap; i++) {
    if (test_code.Get(i) != code[i]) {
      // Do the first direction crossover...

      // Resize the test code for the crossed-over creature, and copy the
      // data into it.  Plug in the result to the genotype and cpu.

      cross_code.Resize(code.GetSize() + test_code.GetSize() - overlap);
      cross_code.Copy(test_code, code, -offset, start1 + i);

      // Run the creature, and collect data about it!

      cTestCPU::TestCode(test_info, cross_code);
      if (!test_info.CalcCopyTrue()) {
	fail_count++;
	if (fail_count > max_fail_count) break;
      }

      // Next, do the crossover the other way...

      // Resize the test code for the crossed-over creature, and copy the
      // data into it.  Plug in the result to the genotype and cpu.

      cross_code.Resize(code.GetSize() + test_code.GetSize() - overlap);
      cross_code.Copy(code, test_code, offset, start2 + i);

      // Run this creature, and collect data about it!

      cTestCPU::TestCode(test_info, cross_code);
      if (!test_info.CalcCopyTrue()) {
	fail_count++;
	if (fail_count > max_fail_count) break;
      }
    }
  }

  return fail_count;
}

int cSpecies::OK()
{
  int ret_value = TRUE;

  // First check the id value...

  if (id_num < 0) {
    g_debug.Warning("Species has negative ID value!");
    ret_value = FALSE;
  }

  // Then check the code_array

  if (!code.OK()) {
    g_debug.Warning("Species code not registering as OK!");
    ret_value = FALSE;
  }

  // Next, check all of the other stats about this species...

  if (total_creatures < 0 || total_genotypes < 0 || num_threshold < 0 ||
      num_genotypes < 0) {
    g_debug.Error("ID:%d UD %d: T_cre=%d, T_gen=%d, N_thr=%d, N_gen=%d",
		  id_num, cStats::GetUpdate(), total_creatures,
		  total_genotypes, num_threshold, num_genotypes);
    ret_value = FALSE;
  }

  // Finally, make sire the species is registered as being in a queue.

  if (queue_type < 0 || queue_type > 3) {
    g_debug.Error("Species not in a proper queue");
    ret_value = FALSE;
  }

  return ret_value;
}

void cSpecies::AddThreshold(cGenotype & in_genotype)
{
  int distance = code.FindSlidingDistance(in_genotype.GetCode());

  if (distance >= 0 && distance < SPECIES_MAX_DISTANCE)
    genotype_distance[distance]++;

  num_threshold++;
}

void cSpecies::RemoveThreshold(cGenotype & in_genotype)
{
  total_genotypes++;
  total_creatures += in_genotype.GetTotalCPUs();
  num_threshold--;
}

void cSpecies::AddGenotype()
{
  num_genotypes++;
}

void cSpecies::RemoveGenotype() {
  num_genotypes--;
}

