//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include <iomanip>

#include <fstream>

#include <math.h>

#include "analyze.hh"

#include "config.hh"
#include "species.hh"
#include "../cpu/hardware_method.hh"
#include "environment.hh"
#include "fitness_matrix.hh"
#include "inst_lib.hh"
#include "inst_util.hh"
#include "landscape.hh"
#include "phenotype.hh"
#include "genome_util.hh"

#include "../cpu/hardware_base.hh"
#include "../cpu/hardware_util.hh"
#include "../cpu/test_cpu.hh"
#include "../cpu/test_util.hh"

#ifdef WIN32
# ifndef WIN32_MKDIR_HACK_HH
#  include "win32_mkdir_hack.hh"
# endif
#endif


using namespace std;


//////////////////////
//  cAnalyzeGenotype
//////////////////////

cAnalyzeGenotype::cAnalyzeGenotype(cString symbol_string,
				   cInstLib & in_inst_lib)
  : genome(symbol_string)
  , inst_lib(in_inst_lib)
  , name("")
  , aligned_sequence("")
  , tag("")
  , viable(false)
  , id_num(-1)
  , parent_id(-1)
  , num_cpus(0)
  , total_cpus(0)
  , update_born(0)
  , update_dead(0)
  , depth(0)
  , length(0)
  , copy_length(0)
  , exe_length(0)
  , merit(0.0)
  , gest_time(INT_MAX)
  , fitness(0.0)
  , errors(0)
  , task_counts(0)
  , fitness_ratio(0.0)
  , efficiency_ratio(0.0)
  , comp_merit_ratio(0.0)
  , parent_dist(0)
  , ancestor_dist(0)
  , parent_muts("")
  , landscape_stats(NULL)
{
  // Make sure that the sequences jive with the inst_lib
  for (int i = 0; i < genome.GetSize(); i++) {
    if (genome[i].GetOp() >= inst_lib.GetSize()) {
      cerr << "Error: Trying to load instruction " << genome[i].GetOp()
	   << ".  Max in set is" << inst_lib.GetSize() - 1
	   << endl;
    }
  }
}

cAnalyzeGenotype::cAnalyzeGenotype(const cGenome & _genome,
				   cInstLib & in_inst_lib)
  : genome(_genome)
  , inst_lib(in_inst_lib)
  , name("")
  , aligned_sequence("")
  , tag("")
  , viable(false)
  , id_num(-1)
  , parent_id(-1)
  , num_cpus(0)
  , total_cpus(0)
  , update_born(0)
  , update_dead(0)
  , depth(0)
  , length(0)
  , copy_length(0)
  , exe_length(0)
  , merit(0.0)
  , gest_time(INT_MAX)
  , fitness(0.0)
  , errors(0)
  , task_counts(0)
  , fitness_ratio(0.0)
  , efficiency_ratio(0.0)
  , comp_merit_ratio(0.0)
  , parent_dist(0)
  , ancestor_dist(0)
  , parent_muts("")
  , landscape_stats(NULL)
{
}

cAnalyzeGenotype::cAnalyzeGenotype(const cAnalyzeGenotype & _gen)
  : genome(_gen.genome)
  , inst_lib(_gen.inst_lib)
  , name(_gen.name)
  , aligned_sequence(_gen.aligned_sequence)
  , tag(_gen.tag)
  , viable(_gen.viable)
  , id_num(_gen.id_num)
  , parent_id(_gen.parent_id)
  , num_cpus(_gen.num_cpus)
  , total_cpus(_gen.total_cpus)
  , update_born(_gen.update_born)
  , update_dead(_gen.update_dead)
  , depth(_gen.depth)
  , length(_gen.length)
  , copy_length(_gen.copy_length)
  , exe_length(_gen.exe_length)
  , merit(_gen.merit)
  , gest_time(_gen.gest_time)
  , fitness(_gen.fitness)
  , errors(_gen.errors)
  , task_counts(_gen.task_counts)
  , fitness_ratio(_gen.fitness_ratio)
  , efficiency_ratio(_gen.efficiency_ratio)
  , comp_merit_ratio(_gen.comp_merit_ratio)
  , parent_dist(_gen.parent_dist)
  , ancestor_dist(_gen.ancestor_dist)
  , parent_muts(_gen.parent_muts)
  , landscape_stats(NULL)
{
  if (_gen.landscape_stats != NULL) {
    landscape_stats = new cAnalyzeLandscape;
    *landscape_stats = *(_gen.landscape_stats);
  }
}

cAnalyzeGenotype::~cAnalyzeGenotype()
{
  if (landscape_stats != NULL) delete landscape_stats;
}


int cAnalyzeGenotype::CalcMaxGestation() const
{
  return cConfig::GetTestCPUTimeMod() * genome.GetSize();
}

void cAnalyzeGenotype::CalcLandscape() const
{
  if (landscape_stats != NULL) return;

  landscape_stats = new cAnalyzeLandscape;
  cLandscape landscape(genome, inst_lib);
  landscape.Process(1);
  landscape_stats->frac_dead = landscape.GetProbDead();
  landscape_stats->frac_neg  = landscape.GetProbNeg();
  landscape_stats->frac_neut = landscape.GetProbNeut();
  landscape_stats->frac_pos  = landscape.GetProbPos();
}

void cAnalyzeGenotype::Recalculate(cAnalyzeGenotype * parent_genotype)
{
    // Build the test info for printing.
  cCPUTestInfo test_info;
  test_info.TestThreads();
  // test_info.TraceTaskOrder();

  // Use the inst lib for this genotype...
  cInstLib * inst_lib_backup = cTestCPU::GetInstLib();
  cTestCPU::SetInstLib(&inst_lib);
  cTestCPU::TestGenome(test_info, genome);
  cTestCPU::SetInstLib(inst_lib_backup);

  viable = test_info.IsViable();

  cOrganism * test_organism = test_info.GetTestOrganism();
  cPhenotype & test_phenotype = test_organism->GetPhenotype();

  length = test_organism->GetGenome().GetSize();
  copy_length = test_phenotype.GetCopiedSize();
  exe_length = test_phenotype.GetExecutedSize();
  merit = test_phenotype.GetMerit().GetDouble();
  gest_time = test_phenotype.GetGestationTime();
  fitness = test_phenotype.GetFitness();
  errors = test_phenotype.GetLastNumErrors();
  div_type = test_phenotype.GetDivType();
  task_counts = test_phenotype.GetLastTaskCount();

  // Setup a new parent stats if we have a parent to work with.
  if (parent_genotype != NULL) {
    fitness_ratio = GetFitness() / parent_genotype->GetFitness();
    efficiency_ratio = GetEfficiency() / parent_genotype->GetEfficiency();
    comp_merit_ratio = GetCompMerit() / parent_genotype->GetCompMerit();
    parent_dist = cStringUtil::EditDistance(genome.AsString(),
		    parent_genotype->GetGenome().AsString(), parent_muts);
    ancestor_dist = parent_genotype->GetAncestorDist() + parent_dist;
  }
}


void cAnalyzeGenotype::PrintTasks(ofstream & fp, int min_task, int max_task)
{
  if (max_task == -1) max_task = task_counts.GetSize();

  for (int i = min_task; i < max_task; i++) {
    fp << task_counts[i] << " ";
  }
}

void cAnalyzeGenotype::SetSequence(cString _sequence)
{
  cGenome new_genome(_sequence);
  genome = new_genome;
}


double cAnalyzeGenotype::GetFracDead() const
{
  CalcLandscape();  // Make sure the landscape is calculated...
  return landscape_stats->frac_dead;
}

double cAnalyzeGenotype::GetFracNeg() const
{
  CalcLandscape();  // Make sure the landscape is calculated...
  return landscape_stats->frac_neg;
}

double cAnalyzeGenotype::GetFracNeut() const
{
  CalcLandscape();  // Make sure the landscape is calculated...
  return landscape_stats->frac_neut;
}

double cAnalyzeGenotype::GetFracPos() const
{
  CalcLandscape();  // Make sure the landscape is calculated...
  return landscape_stats->frac_pos;
}


cString cAnalyzeGenotype::GetTaskList() const
{
  const int num_tasks = task_counts.GetSize();
  cString out_string(num_tasks);
  
  for (int i = 0; i < num_tasks; i++) {
    const int cur_count = task_counts[i];
    if (cur_count < 10) {
      out_string[i] = '0' + cur_count;
    }
    else if (cur_count < 30) {
      out_string[i] = 'X';
    }
    else if (cur_count < 80) {
      out_string[i] = 'L';
    }
    else if (cur_count < 300) {
      out_string[i] = 'C';
    }
    else if (cur_count < 800) {
      out_string[i] = 'D';
    }
    else if (cur_count < 3000) {
      out_string[i] = 'M';
    }
    else {
      out_string[i] = '+';
    }
  }

  return out_string;
}


cString cAnalyzeGenotype::GetHTMLSequence() const
{
  cString text_genome = genome.AsString();
  cString html_code("<tt>");

  cString diff_info = parent_muts;
  char mut_type = 'N';
  int mut_pos = -1;

  cString cur_mut = diff_info.Pop(',');
  if (cur_mut != "") {
    mut_type = cur_mut[0];
    cur_mut.ClipFront(1); cur_mut.ClipEnd(1);
    mut_pos = cur_mut.AsInt();
  }

  int ins_count = 0;
  for (int i = 0; i < genome.GetSize(); i++) {
    char symbol = text_genome[i];
    if (i != mut_pos) html_code += symbol;
    else {
      // Figure out the information for the type of mutation we had...
      cString color;
      if (mut_type == 'M') {
	color = "#FF0000";	
      } else if (mut_type == 'I') {
	color = "#00FF00";
	ins_count++;
      } else { // if (mut_type == 'D') {
	color = "#0000FF";
	symbol = '*';
	i--;  // Rewind - we didn't read the handle character yet!
      }

      // Move on to the next mutation...
      cur_mut = diff_info.Pop(',');
      if (cur_mut != "") {
	mut_type = cur_mut[0];
	cur_mut.ClipFront(1); cur_mut.ClipEnd(1);
	mut_pos = cur_mut.AsInt();
	if (mut_type == 'D') mut_pos += ins_count;
      } else mut_pos = -1;

      // Tack on the current symbol...
      cString symbol_string;
      symbol_string.Set("<b><font color=\"%s\">%c</font></b>",
			color(), symbol);
      html_code += symbol_string;
    }
  }

  html_code += "</tt>";

  return html_code;
}



//////////////
//  cAnalyze
//////////////

cAnalyze::cAnalyze(cString filename)
  : cur_batch(0)
  , verbose(false)
  , interactive_depth(0)
  , inst_lib(cHardwareUtil::DefaultInstLib(cConfig::GetInstFilename()))
{
  random.ResetSeed(cConfig::GetRandSeed());

  for (int i = 0; i < MAX_BATCHES; i++) {
    batch[i].Name().Set("Batch%d", i);
  }

  cInitFile analyze_file(filename);
  analyze_file.Load();
  analyze_file.Compress();
  analyze_file.Close();

  LoadCommandList(analyze_file, command_list);
  ProcessCommands(command_list);
}

cAnalyze::~cAnalyze()
{
  data_file_manager.FlushAll();
  while (genotype_data_list.GetSize()) delete genotype_data_list.Pop();
  while (command_list.GetSize()) delete command_list.Pop();
  while (function_list.GetSize()) delete function_list.Pop();
}


//////////////// Loading methods...

void cAnalyze::LoadOrganism(cString cur_string)
{
  // LOAD_ORGANISM command...

  cString filename = cur_string.PopWord();

  cout << "Loading: " << filename << endl;

  // Setup the genome...
  cGenome genome( cInstUtil::LoadGenome(filename, inst_lib) );

  // Construct the new genotype..
  cAnalyzeGenotype * genotype = new cAnalyzeGenotype(genome, inst_lib);

  // Determine the organism's original name -- strip off directory...
  while (filename.Find('/') != -1) filename.Pop('/');
  while (filename.Find('\\') != -1) filename.Pop('\\');
  filename.Replace(".gen", "");  // Remove the .gen from the filename.
  genotype->SetName(filename);

  // And save it in the current batch.
  batch[cur_batch].List().PushRear(genotype);

  // Adjust the flags on this batch
  batch[cur_batch].SetLineage(false);
  batch[cur_batch].SetAligned(false);
}

void cAnalyze::LoadBasicDump(cString cur_string)
{
  // LOAD_BASE_DUMP

  cString filename = cur_string.PopWord();

  cout << "Loading: " << filename << endl;

  cInitFile input_file(filename);
  if (!input_file.IsOpen()) {
    cerr << "Error: Cannot load file: \"" << filename << "\"." << endl;
    exit(1);
  }
  input_file.Load();
  input_file.Compress();
  input_file.Close();

  // Setup the genome...

  while (input_file.GetNumLines() > 0) {
    cString cur_line = input_file.RemoveLine();

    // Setup the genotype for this line...
    cAnalyzeGenotype * genotype =
      new cAnalyzeGenotype(cur_line.PopWord(), inst_lib);
    int num_cpus = cur_line.PopWord().AsInt();
    int id_num = cur_line.PopWord().AsInt();
    cString name = cStringUtil::Stringf("org-%d", id_num);

    genotype->SetNumCPUs(num_cpus);
    genotype->SetID(id_num);
    genotype->SetName(name);

    // Add this genotype to the proper batch.
    batch[cur_batch].List().PushRear(genotype);
  }

  // Adjust the flags on this batch
  batch[cur_batch].SetLineage(false);
  batch[cur_batch].SetAligned(false);
}

void cAnalyze::LoadDetailDump(cString cur_string)
{
  // LOAD_DETAIL_DUMP

  cString filename = cur_string.PopWord();

  cout << "Loading: " << filename << endl;

  cInitFile input_file(filename);
  if (!input_file.IsOpen()) {
    cerr << "Error: Cannot load file: \"" << filename << "\"." << endl;
    exit(1);
  }
  input_file.Load();
  input_file.Compress();
  input_file.Close();

  // Setup the genome...

  while (input_file.GetNumLines() > 0) {
    cString cur_line = input_file.RemoveLine();

    // Setup the genotype for this line...

    int id_num      = cur_line.PopWord().AsInt();
    int parent_id   = cur_line.PopWord().AsInt();
    int parent_dist = cur_line.PopWord().AsInt();
    int num_cpus    = cur_line.PopWord().AsInt();
    int total_cpus  = cur_line.PopWord().AsInt();
    int length      = cur_line.PopWord().AsInt();
    double merit    = cur_line.PopWord().AsDouble();
    int gest_time   = cur_line.PopWord().AsInt();
    double fitness  = cur_line.PopWord().AsDouble();
    int update_born = cur_line.PopWord().AsInt();
    int update_dead = cur_line.PopWord().AsInt();
    int depth       = cur_line.PopWord().AsInt();
    cString name = cStringUtil::Stringf("org-%d", id_num);

    cAnalyzeGenotype * genotype =
      new cAnalyzeGenotype(cur_line.PopWord(), inst_lib);

    genotype->SetID(id_num);
    genotype->SetParentID(parent_id);
    genotype->SetParentDist(parent_dist);
    genotype->SetNumCPUs(num_cpus);
    genotype->SetTotalCPUs(total_cpus);
    genotype->SetLength(length);
    genotype->SetMerit(merit);
    genotype->SetGestTime(gest_time);
    genotype->SetFitness(fitness);
    genotype->SetUpdateBorn(update_born);
    genotype->SetUpdateDead(update_dead);
    genotype->SetDepth(depth);
    genotype->SetName(name);

    // Add this genotype to the proper batch.
    batch[cur_batch].List().PushRear(genotype);
  }

  // Adjust the flags on this batch
  batch[cur_batch].SetLineage(false);
  batch[cur_batch].SetAligned(false);
}

void cAnalyze::LoadSequence(cString cur_string)
{
  // LOAD_SEQUENCE

  static int sequence_count = 1;
  cString sequence = cur_string.PopWord();
  cString seq_name = cur_string.PopWord();

  cout << "Loading: " << sequence << endl;

  // Setup the genotype...
  cAnalyzeGenotype * genotype = new cAnalyzeGenotype(sequence, inst_lib);

  genotype->SetNumCPUs(1);      // Initialize to a single organism.
  if (seq_name == "") {
    seq_name = cStringUtil::Stringf("org-S%d", sequence_count);
  }
  genotype->SetName(seq_name);
  sequence_count++;

  // Add this genotype to the proper batch.
  batch[cur_batch].List().PushRear(genotype);

  // Adjust the flags on this batch
  batch[cur_batch].SetLineage(false);
  batch[cur_batch].SetAligned(false);
}

void cAnalyze::LoadDominant(cString cur_string)
{
  cerr << "Warning: \"LOAD_DOMINANT\" not implmented yet!"<<endl;
}

void cAnalyze::LoadFile(cString cur_string)
{
  // LOAD

  cString filename = cur_string.PopWord();

  cout << "Loading: " << filename << endl;

  cInitFile input_file(filename);
  if (!input_file.IsOpen()) {
    cerr << "Error: Cannot load file: \"" << filename << "\"." << endl;
    exit(1);
  }
  input_file.Load();
  input_file.ReadHeader();
  input_file.Compress();
  input_file.Close();

  const cString filetype = input_file.GetFiletype();
  if (filetype != "population_data") {
    cerr << "Error: Cannot load files of type \"" << filetype << "\"." << endl;
    exit(1);
  }

  if (verbose == true) {
    cout << "Loading file of type: " << filetype << endl;
  }

  
  // Construct a linked list of data types that can be loaded...
  tList< tDataEntryCommand<cAnalyzeGenotype> > output_list;
  tListIterator< tDataEntryCommand<cAnalyzeGenotype> > output_it(output_list);
  LoadGenotypeDataList(input_file.GetFormat(), output_list);


  // Setup the genome...
  cGenome default_genome(1);
  int load_count = 0;
  while (input_file.GetNumLines() > 0) {
    cString cur_line = input_file.RemoveLine();

    cAnalyzeGenotype * genotype =
      new cAnalyzeGenotype(default_genome, inst_lib);

    output_it.Reset();
    tDataEntryCommand<cAnalyzeGenotype> * data_command = NULL;
    while ((data_command = output_it.Next()) != NULL) {
      data_command->SetTarget(genotype);
//        genotype->SetSpecialArgs(data_command->GetArgs());
      data_command->SetValue(cur_line.PopWord());
    }

    // Make sure this genotype has a name...
    if (genotype->GetName() == "") {
      cString name = cStringUtil::Stringf("org-D%d", load_count++);
      genotype->SetName(name);
    }

    // Add this genotype to the proper batch.
    batch[cur_batch].List().PushRear(genotype);
  }

  // Adjust the flags on this batch
  batch[cur_batch].SetLineage(false);
  batch[cur_batch].SetAligned(false);
}


//////////////// Reduction....

void cAnalyze::FindGenotype(cString cur_string)
{
  // If no arguments are passed in, just find max num_cpus.
  if (cur_string.GetSize() == 0) cur_string = "num_cpus";

  if (verbose == true) {
    cout << "Reducing batch " << cur_batch << " to genotypes: ";
  }

  tList<cAnalyzeGenotype> & gen_list = batch[cur_batch].List();
  tList<cAnalyzeGenotype> found_list;
  while (cur_string.CountNumWords() > 0) {
    cString gen_desc(cur_string.PopWord());
    if (verbose) cout << gen_desc << " ";

    // Determine by lin_type which genotype were are tracking...
    cAnalyzeGenotype * found_gen = PopGenotype(gen_desc, cur_batch);

    if (found_gen == NULL) {
      cerr << "  Warning: genotype not found!" << endl;
      continue;
    }

    // Save this genotype...
    found_list.Push(found_gen);
  }
  cout << endl;

  // Delete all genotypes other than the ones found!
  while (gen_list.GetSize() > 0) delete gen_list.Pop();

  // And fill it back in with the good stuff.
  while (found_list.GetSize() > 0) gen_list.Push(found_list.Pop());

  // Adjust the flags on this batch
  batch[cur_batch].SetLineage(false);
  batch[cur_batch].SetAligned(false);
}

void cAnalyze::FindLineage(cString cur_string)
{
  cString lin_type = "num_cpus";
  if (cur_string.CountNumWords() > 0) lin_type = cur_string.PopWord();

  if (verbose == true) {
    cout << "Reducing batch " << cur_batch
	 << " to " << lin_type << " lineage " << endl;
  } else cout << "Performing lineage scan..." << endl;


  // Determine by lin_type which genotype we are tracking...
  cAnalyzeGenotype * found_gen = PopGenotype(lin_type, cur_batch);

  if (found_gen == NULL) {
    cerr << "  Warning: Genotype " << lin_type
	 << " not found.  Lineage scan aborted." << endl;
    return;
  }

  // Otherwise, trace back through the id numbers to mark all of those
  // in the ancestral lineage...

  // Construct a list of genotypes found...

  tList<cAnalyzeGenotype> found_list;
  found_list.Push(found_gen);
  int next_id = found_gen->GetParentID();
  bool found = true;
  while (found == true) {
    found = false;

    tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
    while ((found_gen = batch_it.Next()) != NULL) {
      if (found_gen->GetID() == next_id) {
	batch_it.Remove();
	found_list.Push(found_gen);
	next_id = found_gen->GetParentID();
	found = true;
	break;
      }
    }
  }

  // We now have all of the genotypes in this lineage, delete everything
  // else.

  const int total_removed = batch[cur_batch].List().GetSize();
  while (batch[cur_batch].List().GetSize() > 0) {
    delete batch[cur_batch].List().Pop();
  }

  // And fill it back in with the good stuff.
  int total_kept = found_list.GetSize();
  while (found_list.GetSize() > 0) {
    batch[cur_batch].List().PushRear(found_list.Pop());
  }

  if (verbose == true) {
    cout << "  Lineage has " << total_kept << " genotypes; "
	 << total_removed << " were removed." << endl;
  }

  // Adjust the flags on this batch
  batch[cur_batch].SetLineage(true);
  batch[cur_batch].SetAligned(false);
}

void cAnalyze::FindClade(cString cur_string)
{
  if (cur_string.GetSize() == 0) {
    cerr << "  Warning: No clade specified for FIND_CLADE.  Aborting." << endl;
    return;
  }

  cString clade_type( cur_string.PopWord() );

  if (verbose == true) {
    cout << "Reducing batch " << cur_batch
	 << " to clade " << clade_type << "." << endl;
  } else cout << "Performing clade scan..." << endl;


  // Determine by clade_type which genotype we are tracking...
  cAnalyzeGenotype * found_gen = PopGenotype(clade_type, cur_batch);

  if (found_gen == NULL) {
    cerr << "  Warning: Ancestral genotype " << clade_type
	 << " not found.  Clade scan aborted." << endl;
    return;
  }

  // Do this the brute force way... scan for one step at a time.

  // Construct a list of genotypes found...

  tList<cAnalyzeGenotype> found_list; // Found and finished.
  tList<cAnalyzeGenotype> scan_list;  // Found, but need to scan for children.
  scan_list.Push(found_gen);

  // Keep going as long as there is something in the scan list...
  while (scan_list.GetSize() > 0) {
    // Move the next genotype from the scan list to the found_list.
    found_gen = scan_list.Pop();
    int parent_id = found_gen->GetID();
    found_list.Push(found_gen);

    // Seach for all of the children of this genotype...
    tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
    while ((found_gen = batch_it.Next()) != NULL) {
      // If we found a child, place it into the scan list.
      if (found_gen->GetParentID() == parent_id) {
	batch_it.Remove();
	scan_list.Push(found_gen);
      }
    }
  }

  // We now have all of the genotypes in this clade, delete everything else.

  const int total_removed = batch[cur_batch].List().GetSize();
  while (batch[cur_batch].List().GetSize() > 0) {
    delete batch[cur_batch].List().Pop();
  }

  // And fill it back in with the good stuff.
  int total_kept = found_list.GetSize();
  while (found_list.GetSize() > 0) {
    batch[cur_batch].List().PushRear(found_list.Pop());
  }

  if (verbose == true) {
    cout << "  Clade has " << total_kept << " genotypes; "
	 << total_removed << " were removed." << endl;
  }

  // Adjust the flags on this batch
  batch[cur_batch].SetLineage(false);
  batch[cur_batch].SetAligned(false);
}

void cAnalyze::SampleOrganisms(cString cur_string)
{
  double fraction = cur_string.PopWord().AsDouble();
  int init_genotypes = batch[cur_batch].List().GetSize();

  if (verbose == true) {
    cout << "Sampling " << fraction << " organisms from batch "
	 << cur_batch << "." << endl;
  }
  else cout << "Sampling Organisms..." << endl;

  int old_total = 0;
  int new_total = 0;

  cAnalyzeGenotype * genotype = NULL;
  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
  while ((genotype = batch_it.Next()) != NULL) {
    const int old_count = genotype->GetNumCPUs();
    const int new_count = random.GetRandBinomial(old_count, fraction);
    old_total += old_count;
    new_total += new_count;

    if (new_count == 0) {
      batch_it.Remove();
      delete genotype;
    } else {
      genotype->SetNumCPUs(new_count);
    }
  }

  int num_genotypes = batch[cur_batch].List().GetSize();
  if (verbose) {
    cout << "  Removed " << old_total - new_total
	 << " organisms (" << init_genotypes - num_genotypes
	 << " genotypes); " << new_total
	 << " orgs (" << num_genotypes << " gens) remaining."
	 << endl;
  }

  // Adjust the flags on this batch
  batch[cur_batch].SetLineage(false);
  batch[cur_batch].SetAligned(false);
}

void cAnalyze::SampleGenotypes(cString cur_string)
{
  double fraction = cur_string.PopWord().AsDouble();
  int init_genotypes = batch[cur_batch].List().GetSize();

  if (verbose == true) {
    cout << "Sampling " << fraction << " genotypes from batch "
	 << cur_batch << "." << endl;
  }
  else cout << "Sampling Genotypes..." << endl;

  double frac_remove = 1.0 - fraction;

  cAnalyzeGenotype * genotype = NULL;

  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
  while ((genotype = batch_it.Next()) != NULL) {
    if (random.P(frac_remove)) {
      batch_it.Remove();
      delete genotype;
    }
  }

  int num_genotypes = batch[cur_batch].List().GetSize();
  if (verbose == true) {
    cout << "  Removed " << init_genotypes - num_genotypes
	 << " genotypes; " << num_genotypes << " remaining."
	 << endl;
  }

  // Adjust the flags on this batch
  batch[cur_batch].SetLineage(false);
  batch[cur_batch].SetAligned(false);
}

void cAnalyze::KeepTopGenotypes(cString cur_string)
{
  const int num_kept = cur_string.PopWord().AsInt();
  const int num_genotypes = batch[cur_batch].List().GetSize();
  const int num_removed = num_genotypes - num_kept;

  for (int i = 0; i < num_removed; i++) {
    delete batch[cur_batch].List().PopRear();
  }

  // Adjust the flags on this batch
  // batch[cur_batch].SetLineage(false); // Should not destroy a lineage...
  batch[cur_batch].SetAligned(false);
}



//////////////// Output Commands...

void cAnalyze::CommandPrint(cString cur_string)
{
  if (verbose == true) cout << "Printing batch " << cur_batch << endl;
  else cout << "Printing organisms..." << endl;

  cString directory = PopDirectory(cur_string, "genebank/");

  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
  cAnalyzeGenotype * genotype = NULL;
  while ((genotype = batch_it.Next()) != NULL) {
    cString filename(directory);
    filename += genotype->GetName();
    filename += ".gen";
    cTestUtil::PrintGenome(genotype->GetGenome(), filename);
    if (verbose) cout << "Printing: " << filename << endl;
  }
}

void cAnalyze::CommandTrace(cString cur_string)
{
  if (verbose == true) cout << "Tracing batch " << cur_batch << endl;
  else cout << "Tracing organisms..." << endl;

  cString directory = PopDirectory(cur_string, "genebank/");

  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
  cAnalyzeGenotype * genotype = NULL;
  while ((genotype = batch_it.Next()) != NULL) {
    cString filename(directory);
    filename += genotype->GetName();
    filename += ".trace";
    cTestCPU::TraceGenome(genotype->GetGenome(), filename);
    if (verbose) cout << "  Tracing: " << filename << endl;
  }
}

void cAnalyze::CommandPrintTasks(cString cur_string)
{
  if (verbose == true) cout << "Printing tasks in batch " << cur_batch << endl;
  else cout << "Printing tasks..." << endl;

  // Load in the variables...
  cString filename("tasks.dat");
  if (cur_string.GetSize() != 0) filename = cur_string.PopWord();

  ofstream & fp = data_file_manager.GetOFStream(filename);

  // Loop through all of the genotypes in this batch...
  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
  cAnalyzeGenotype * genotype = NULL;
  while ((genotype = batch_it.Next()) != NULL) {
    fp << genotype->GetID() << " ";
    genotype->PrintTasks(fp);
    fp << endl;
  }
}

void cAnalyze::CommandDetail(cString cur_string)
{
  if (verbose == true) cout << "Detailing batch " << cur_batch << endl;
  else cout << "Detailing..." << endl;

  // Load in the variables...
  cString filename("detail.dat");
  if (cur_string.GetSize() != 0) filename = cur_string.PopWord();

  // Construct a linked list of details needed...
  tList< tDataEntryCommand<cAnalyzeGenotype> > output_list;
  tListIterator< tDataEntryCommand<cAnalyzeGenotype> > output_it(output_list);
  LoadGenotypeDataList(cur_string, output_list);

  // Determine the file type...
  int file_type = FILE_TYPE_TEXT;
  cString file_extension(filename);
  while (file_extension.Find('.') != -1) file_extension.Pop('.');
  if (file_extension == "html") file_type = FILE_TYPE_HTML;

  // Setup the file...
  if (filename == "cout") {
    CommandDetail_Header(cout, file_type, output_it);
    CommandDetail_Body(cout, file_type, output_it);
  } else {
    ofstream & fp = data_file_manager.GetOFStream(filename);
    CommandDetail_Header(fp, file_type, output_it);
    CommandDetail_Body(fp, file_type, output_it);
  }

  // And clean up...
  while (output_list.GetSize() != 0) delete output_list.Pop();
}


void cAnalyze::CommandDetail_Header(ostream & fp, int format_type,
	    tListIterator< tDataEntryCommand<cAnalyzeGenotype> > & output_it)
{
  // Write out the header on the file
  if (format_type == FILE_TYPE_TEXT) {
    fp << "#filetype genotype_data" << endl;
    fp << "#format ";
    while (output_it.Next() != NULL) {
      const cString & entry_name = output_it.Get()->GetName();
      fp << entry_name << " ";
    }
    fp << endl << endl;

    // Give the more human-readable legend.
    fp << "# Legend:" << endl;
    int count = 0;
    while (output_it.Next() != NULL) {
      const cString & entry_desc = output_it.Get()->GetDesc();
      fp << "# " << ++count << ": " << entry_desc << endl;
    }
    fp << endl;
  } else { // if (format_type == FILE_TYPE_HTML) {
    fp << "<html>" << endl
       << "<body bgcolor=\"#FFFFFF\"" << endl
       << " text=\"#000000\"" << endl
       << " link=\"#0000AA\"" << endl
       << " alink=\"#0000FF\"" << endl
       << " vlink=\"#000044\">" << endl
       << endl
       << "<h1 align=center>Run " << batch[cur_batch].Name() << endl
       << endl
       << "<center>" << endl
       << "<table border=1 cellpadding=2><tr>" << endl;

    while (output_it.Next() != NULL) {
      const cString & entry_desc = output_it.Get()->GetDesc();
      fp << "<th bgcolor=\"#AAAAFF\">" << entry_desc << " ";
    }
    fp << "</tr>" << endl;

  }

}


void cAnalyze::CommandDetail_Body(ostream & fp, int format_type,
	    tListIterator< tDataEntryCommand<cAnalyzeGenotype> > & output_it)
{
  // Loop through all of the genotypes in this batch...
  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
  cAnalyzeGenotype * genotype = NULL;
  cAnalyzeGenotype * last_genotype = NULL;
  while ((genotype = batch_it.Next()) != NULL) {
    output_it.Reset();
    tDataEntryCommand<cAnalyzeGenotype> * data_command = NULL;
    if (format_type == FILE_TYPE_HTML) fp << "<tr>";
    while ((data_command = output_it.Next()) != NULL) {
      data_command->SetTarget(genotype);
      genotype->SetSpecialArgs(data_command->GetArgs());
      if (format_type == FILE_TYPE_HTML) {
	int compare = 0;
	if (last_genotype) {
	  last_genotype->SetSpecialArgs(data_command->GetArgs());
	  compare = data_command->Compare(last_genotype);
	}
	data_command->HTMLPrint(fp, compare);
      }
      else {  // if (format_type == FILE_TYPE_TEXT) {
	fp << data_command->GetEntry() << " ";
      }
    }
    if (format_type == FILE_TYPE_HTML) fp << "</tr>";
    fp << endl;
    last_genotype = genotype;
  }

  // If in HTML mode, we need to end the file...
  if (format_type == FILE_TYPE_HTML) {
    fp << "</table>" << endl
       << "</center>" << endl;
  }
}

void cAnalyze::CommandDetailBatches(cString cur_string)
{
  // Load in the variables...
  cString keyword("num_cpus");
  cString filename("detail_batch.dat");
  if (cur_string.GetSize() != 0) keyword = cur_string.PopWord();
  if (cur_string.GetSize() != 0) filename = cur_string.PopWord();

  if (verbose == true) cout << "Detailing batches for " << keyword << endl;
  else cout << "Detailing Batches..." << endl;

  // Scan the functions list for the keyword we need...
  SetupGenotypeDataList();
  tListIterator< tDataEntryBase<cAnalyzeGenotype> >
    output_it(genotype_data_list);

  // Divide up the keyword into its acrual entry and its arguments...
  cString cur_args = keyword;
  cString cur_entry = cur_args.Pop(':');

  // Find its associated command...
  tDataEntryCommand<cAnalyzeGenotype> * cur_command;
  bool found = false;
  while (output_it.Next() != NULL) {
    if (output_it.Get()->GetName() == cur_entry) {
      cur_command = new tDataEntryCommand<cAnalyzeGenotype>
	(output_it.Get(), cur_args);
      found = true;
      break;
    }
  }
  if (found == false) {
    cout << "Error: Unknown data type: " << cur_entry << endl;
    return;
  }


  // Determine the file type...
  int file_type = FILE_TYPE_TEXT;
  cString file_extension(filename);
  while (file_extension.Find('.') != -1) file_extension.Pop('.');
  if (file_extension == "html") file_type = FILE_TYPE_HTML;

  ofstream & fp = data_file_manager.GetOFStream(filename);

  // Write out the header on the file
  if (file_type == FILE_TYPE_TEXT) {
    fp << "#filetype batch_data" << endl
       << "#format batch_id " << keyword << endl
       << endl;

    // Give the more human-readable legend.
    fp << "# Legend:" << endl
       << "#  Column 1 = Batch ID" << endl
       << "#  Remaining entries: " << cur_command->GetDesc() << endl
       << endl;

  } else { // if (file_type == FILE_TYPE_HTML) {
    fp << "<html>" << endl
       << "<body bgcolor=\"#FFFFFF\"" << endl
       << " text=\"#000000\"" << endl
       << " link=\"#0000AA\"" << endl
       << " alink=\"#0000FF\"" << endl
       << " vlink=\"#000044\">" << endl
       << endl
       << "<h1 align=center> Distribution of " << cur_command->GetDesc()
       << endl << endl
       << "<center>" << endl
       << "<table border=1 cellpadding=2>" << endl
       << "<tr><th bgcolor=\"#AAAAFF\">" << cur_command->GetDesc() << "</tr>"
       << endl;
  }


  // Loop through all of the batches...
  for (int i = 0; i < MAX_BATCHES; i++) {
    if (batch[i].List().GetSize() == 0) continue;

    if (file_type == FILE_TYPE_HTML) fp << "<tr><td>";
    fp << i << " ";
  
    tListIterator<cAnalyzeGenotype> batch_it(batch[i].List());
    cAnalyzeGenotype * genotype = NULL;
    while ((genotype = batch_it.Next()) != NULL) {
      output_it.Reset();
      if (file_type == FILE_TYPE_HTML) fp << "<td>";

      cur_command->SetTarget(genotype);
      genotype->SetSpecialArgs(cur_command->GetArgs());
      if (file_type == FILE_TYPE_HTML) {
	cur_command->HTMLPrint(fp, 0);
      }
      else {  // if (file_type == FILE_TYPE_TEXT) {
	fp << cur_command->GetEntry() << " ";
      }
    }
    if (file_type == FILE_TYPE_HTML) fp << "</tr>";
    fp << endl;
  }

  // If in HTML mode, we need to end the file...
  if (file_type == FILE_TYPE_HTML) {
    fp << "</table>" << endl
       << "</center>" << endl;
  }
  
  delete cur_command;
}



void cAnalyze::CommandDetailIndex(cString cur_string)
{
  cout << "Creating a Detail Index..." << endl;

  // A filename and min and max batches must be included.
  if (cur_string.CountNumWords() < 3) {
    cerr << "Error: must include filename, and min and max batch numbers."
	 << endl;
    exit(1);
  }

  // Load in the variables...
  cString filename(cur_string.PopWord());
  int min_batch = cur_string.PopWord().AsInt();
  int max_batch = cur_string.PopWord().AsInt();

  if (max_batch < min_batch) {
    cerr << "Error: min_batch=" << min_batch
	 << ", max_batch=" << max_batch << "  (incorrect order?)" << endl;
    exit(1);
  }

  // Construct a linked list of details needed...
  tList< tDataEntryBase<cAnalyzeGenotype> > output_list;
  tListIterator< tDataEntryBase<cAnalyzeGenotype> > output_it(output_list);

  // For the moment, just put everything into the output list.
  SetupGenotypeDataList();

  // If no args were given, load all of the stats.
  if (cur_string.CountNumWords() == 0) {
    tListIterator< tDataEntryBase<cAnalyzeGenotype> >
      genotype_data_it(genotype_data_list);
    while (genotype_data_it.Next() != NULL) {
      output_list.PushRear(genotype_data_it.Get());
    }
  }
  // Otherwise, load only those listed.
  else {
    while (cur_string.GetSize() != 0) {
      // Setup the next entry
      cString cur_entry = cur_string.PopWord();
      bool found_entry = false;

      // Scan the genotype data list for the current entry
      tListIterator< tDataEntryBase<cAnalyzeGenotype> >
	genotype_data_it(genotype_data_list);

      while (genotype_data_it.Next() != NULL) {
	if (genotype_data_it.Get()->GetName() == cur_entry) {
	  output_list.PushRear(genotype_data_it.Get());
	  found_entry = true;
	  break;
	}
      }

      // If the entry was not found, give a warning.
      if (found_entry == false) {
	int best_match = 1000;
	cString best_entry;

	genotype_data_it.Reset();
	while (genotype_data_it.Next() != NULL) {
	  const cString & test_str = genotype_data_it.Get()->GetName();
	  const int test_dist = cStringUtil::EditDistance(test_str, cur_entry);
	  if (test_dist < best_match) {
	    best_match = test_dist;
	    best_entry = test_str;
	  }
	}	

	cerr << "Warning: Format entry \"" << cur_entry
	     << "\" not found.  Best match is \""
	     << best_entry << "\"." << endl;
      }

    }
  }

  // Setup the file...
  ofstream fp;
  fp.open(filename);

  // Determine the file type...
  int file_type = FILE_TYPE_TEXT;
  while (filename.Find('.') != -1) filename.Pop('.'); // Grab only extension
  if (filename == "html") file_type = FILE_TYPE_HTML;

  // Write out the header on the file
  if (file_type == FILE_TYPE_TEXT) {
    fp << "#filetype genotype_data" << endl;
    fp << "#format ";
    while (output_it.Next() != NULL) {
      const cString & entry_name = output_it.Get()->GetName();
      fp << entry_name << " ";
    }
    fp << endl << endl;

    // Give the more human-readable legend.
    fp << "# Legend:" << endl;
    fp << "# 1: Batch Name" << endl;
    int count = 1;
    while (output_it.Next() != NULL) {
      const cString & entry_desc = output_it.Get()->GetDesc();
      fp << "# " << ++count << ": " << entry_desc << endl;
    }
    fp << endl;
  } else { // if (file_type == FILE_TYPE_HTML) {
    fp << "<html>" << endl
       << "<body bgcolor=\"#FFFFFF\"" << endl
       << " text=\"#000000\"" << endl
       << " link=\"#0000AA\"" << endl
       << " alink=\"#0000FF\"" << endl
       << " vlink=\"#000044\">" << endl
       << endl
       << "<h1 align=center>Batch Index" << endl
       << endl
       << "<center>" << endl
       << "<table border=1 cellpadding=2><tr>" << endl;

    fp << "<th bgcolor=\"#AAAAFF\">Batch ";
    while (output_it.Next() != NULL) {
      const cString & entry_desc = output_it.Get()->GetDesc();
      fp << "<th bgcolor=\"#AAAAFF\">" << entry_desc << " ";
    }
    fp << "</tr>" << endl;

  }

  // Loop through all of the batchs...
  for (int batch_id = min_batch; batch_id <= max_batch; batch_id++) {
    cAnalyzeGenotype * genotype = batch[batch_id].List().GetFirst();
    if (genotype == NULL) continue;
    output_it.Reset();
    tDataEntryBase<cAnalyzeGenotype> * data_entry = NULL;
    const cString & batch_name = batch[batch_id].Name();
    if (file_type == FILE_TYPE_HTML) {
      fp << "<tr><th><a href=lineage." << batch_name << ".html>"
	 << batch_name << "</a> ";
    }
    else {
      fp << batch_name << " ";
    }

    while ((data_entry = output_it.Next()) != NULL) {
      data_entry->SetTarget(genotype);
      if (file_type == FILE_TYPE_HTML) {
	fp << "<td align=center><a href=\""
	   << data_entry->GetName() << "." << batch_name << ".png\">"
	   << *data_entry << "</a> ";
      }
      else {  // if (file_type == FILE_TYPE_TEXT) {
	fp << *data_entry << " ";
      }
    }
    if (file_type == FILE_TYPE_HTML) fp << "</tr>";
    fp << endl;
  }

  // If in HTML mode, we need to end the file...
  if (file_type == FILE_TYPE_HTML) {
    fp << "</table>" << endl
       << "</center>" << endl;
  }
}


///// Population Analysis Commands ////

void cAnalyze::CommandPrintPhenotypes(cString cur_string)
{
  if (verbose == true) cout << "Printing phenotypes in batch "
			    << cur_batch << endl;
  else cout << "Printing phenotypes..." << endl;

  // Load in the variables...
  cString filename("phenotype.dat");
  if (cur_string.GetSize() != 0) filename = cur_string.PopWord();

  // Make sure we have at least one genotype...
  if (batch[cur_batch].List().GetSize() == 0) return;

  // Setup the phenotype categories...
  const int num_tasks = batch[cur_batch].List().GetFirst()->GetNumTasks();
  const int num_phenotypes = 1 << (num_tasks + 1);
  tArray<int> phenotype_counts(num_phenotypes);
  tArray<int> genotype_counts(num_phenotypes);
  tArray<double> total_length(num_phenotypes);
  tArray<double> total_gest(num_phenotypes);

  phenotype_counts.SetAll(0);
  genotype_counts.SetAll(0);
  total_length.SetAll(0.0);
  total_gest.SetAll(0.0);

  // Loop through all of the genotypes in this batch...
  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
  cAnalyzeGenotype * genotype = NULL;
  while ((genotype = batch_it.Next()) != NULL) {
    int phen_id = 0;
    if (genotype->GetViable() == true) phen_id++;
    for (int i = 0; i < num_tasks; i++) {
      if (genotype->GetTaskCount(i) > 0)  phen_id += 1 << (i+1);
    }
    phenotype_counts[phen_id] += genotype->GetNumCPUs();
    genotype_counts[phen_id]++;
    total_length[phen_id] += genotype->GetNumCPUs() * genotype->GetLength();
    total_gest[phen_id] += genotype->GetNumCPUs() * genotype->GetGestTime();
  }

  // Print out the results...
  
  ofstream & fp = data_file_manager.GetOFStream(filename);

  fp << "# 1: Number of organisms of this phenotype" << endl
     << "# 2: Number of genotypes of this phenotye" << endl
     << "# 3: Average Genome Length" << endl
     << "# 4: Average Gestation Time" << endl
     << "# 5: Viability of Phenotype" << endl
     << "# 6+: Tasks performed in this phenotype" << endl
     << endl;

  // @CAO Lets do this inefficiently for the moment, but print out the
  // phenotypes in order.

  while (true) {
    // Find the next phenotype to print...
    int max_count = phenotype_counts[0];
    int max_position = 0;
    for (int i = 0; i < num_phenotypes; i++) {
      if (phenotype_counts[i] > max_count) {
	max_count = phenotype_counts[i];
	max_position = i;
      }
    }

    if (max_count == 0) break; // we're done!

    fp << phenotype_counts[max_position] << " "
       << genotype_counts[max_position] << " "
       << total_length[max_position] / phenotype_counts[max_position] << " "
       << total_gest[max_position] / phenotype_counts[max_position] << " "
       << (max_position & 1) << "  ";
    for (int i = 1; i <= num_tasks; i++) {
      if ((max_position >> i) & 1 > 0) fp << "1 ";
      else fp << "0 ";
    }
    fp << endl;
    phenotype_counts[max_position] = 0;
  }

  fp.close();
}


// Print various diversity metrics from the current batch of genotypes...
void cAnalyze::CommandPrintDiversity(cString cur_string)
{
  if (verbose == true) cout << "Printing diversity data for batch "
			    << cur_batch << endl;
  else cout << "Printing diversity data..." << endl;

  // Load in the variables...
  cString filename("diversity.dat");
  if (cur_string.GetSize() != 0) filename = cur_string.PopWord();

  // Make sure we have at least one genotype...
  if (batch[cur_batch].List().GetSize() == 0) return;

  // Setup the task categories...
  const int num_tasks = batch[cur_batch].List().GetFirst()->GetNumTasks();
  tArray<int> task_count(num_tasks);
  tArray<int> task_gen_count(num_tasks);
  tArray<double> task_gen_dist(num_tasks);
  tArray<double> task_site_entropy(num_tasks);
  task_count.SetAll(0);
  task_gen_count.SetAll(0);

  // We must determine the average hamming distance between genotypes in
  // this batch that perform each task.  Levenstein distance would be ideal,
  // but takes a while, so we'll do it this way first.  For the calculations,
  // we need to know home many times each instruction appears at each
  // position for each genotype collection that performs a particular task.
  const int num_insts = inst_lib.GetSize();
  const int max_length = BatchUtil_GetMaxLength();
  tMatrix<int> inst_freq(max_length, num_insts+1);

  for (int task_id = 0; task_id < num_tasks; task_id++) {
    inst_freq.SetAll(0);

    // Loop through all genotypes, singling out those that do current task...
    tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
    cAnalyzeGenotype * genotype = NULL;
    while ((genotype = batch_it.Next()) != NULL) {
      if (genotype->GetTaskCount(task_id) == 0) continue;

      const cGenome & genome = genotype->GetGenome();
      const int num_cpus = genotype->GetNumCPUs();
      task_count[task_id] += num_cpus;
      task_gen_count[task_id]++;
      for (int i = 0; i < genotype->GetLength(); i++) {
	inst_freq( i, genome[i].GetOp() ) += num_cpus;
      }
      for (int i = genotype->GetLength(); i < max_length; i++) {
	inst_freq(i, num_insts) += num_cpus; // Entry for "past genome end"
      }
    }

    // Analyze the data for this entry...
    const int cur_count = task_count[task_id];
    const int total_pairs = cur_count * (cur_count - 1) / 2;
    int total_dist = 0;
    double total_ent = 0;
    for (int pos = 0; pos < max_length; pos++) {
      // Calculate distance component...
      for (int inst1 = 0; inst1 < num_insts; inst1++) {
	if (inst_freq(pos, inst1) == 0) continue;
	for (int inst2 = inst1+1; inst2 <= num_insts; inst2++) {
	  total_dist += inst_freq(pos, inst1) * inst_freq(pos, inst2);
	}
      }

      // Calculate entropy component...
      for (int i = 0; i <= num_insts; i++) {
	const int cur_freq = inst_freq(pos, i);
	if (cur_freq == 0) continue;
	const double p = ((double) cur_freq) / (double) cur_count;
	total_ent -= p * log(p);
      }
    }

    task_gen_dist[task_id] = ((double) total_dist) / (double) total_pairs;
    task_site_entropy[task_id] = total_ent;
  }

  // Print out the results...
  cDataFile & df = data_file_manager.Get(filename);

  for (int i = 0; i < num_tasks; i++) {
    df.Write(i,                    "# 1: Task ID");
    df.Write(task_count[i],        "# 2: Number of organisms performing task");
    df.Write(task_gen_count[i],    "# 3: Number of genotypes performing task");
    df.Write(task_gen_dist[i],     "# 4: Average distance between genotypes performing task");
    df.Write(task_site_entropy[i], "# 5: Total per-site entropy of genotypes performing task");
    df.Endl();
  }
}


void cAnalyze::CommandLandscape(cString cur_string)
{
  if (verbose == true) cout << "Landscaping batch " << cur_batch << endl;
  else cout << "Landscapping..." << endl;

  // Load in the variables...
  cString filename;
  if (cur_string.GetSize() != 0) filename = cur_string.PopWord();
  int dist = 1;
  if (cur_string.GetSize() != 0) dist = cur_string.PopWord().AsInt();

  // If we're given a file, write to it.
  ofstream & fp = data_file_manager.GetOFStream(filename);
  
  // Loop through all of the genotypes in this batch...
  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
  cAnalyzeGenotype * genotype = NULL;
  while ((genotype = batch_it.Next()) != NULL) {
    cLandscape landscape(genotype->GetGenome(), inst_lib);
    landscape.Process(dist);
    fp << genotype->GetName() << " ";
    landscape.PrintStats(fp);
  }
}

void cAnalyze::CommandFitnessMatrix(cString cur_string)
{
  if (verbose == true) cout << "Calculating fitness matrix for batch " << cur_batch << endl;
  else cout << "Calculating fitness matrix..." << endl;

  cout << "Warning: considering only first genotype of the batch!" << endl;

  // Load in the variables...
  int depth_limit = 4;
  if (cur_string.GetSize() != 0) depth_limit = cur_string.PopWord().AsInt();

  double fitness_threshold_ratio = .9;
  if (cur_string.GetSize() != 0) fitness_threshold_ratio = cur_string.PopWord().AsDouble();

  int ham_thresh  = 1;
  if (cur_string.GetSize() != 0) ham_thresh = cur_string.PopWord().AsInt();

  double error_rate_min = 0.005;
  if (cur_string.GetSize() != 0) error_rate_min = cur_string.PopWord().AsDouble();

  double error_rate_max = 0.05;
  if (cur_string.GetSize() != 0) error_rate_max = cur_string.PopWord().AsDouble();

  double error_rate_step = 0.005;
  if (cur_string.GetSize() != 0) error_rate_step = cur_string.PopWord().AsDouble();

  double vect_fmax = 1.1;
  if (cur_string.GetSize() != 0) vect_fmax = cur_string.PopWord().AsDouble();

  double vect_fstep = .1;
  if (cur_string.GetSize() != 0) vect_fstep = cur_string.PopWord().AsDouble();

  int diag_iters = 200;
  if (cur_string.GetSize() != 0) diag_iters = cur_string.PopWord().AsInt();

  int write_ham_vector = 0;
  if (cur_string.GetSize() != 0) write_ham_vector = cur_string.PopWord().AsInt();

  int write_full_vector = 0;
  if (cur_string.GetSize() != 0) write_full_vector = cur_string.PopWord().AsInt();

  // Consider only the first genotypes in this batch...
  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
  cAnalyzeGenotype * genotype = batch_it.Next();

  cFitnessMatrix matrix( genotype->GetGenome(), &inst_lib);

  matrix.CalcFitnessMatrix( depth_limit, fitness_threshold_ratio, ham_thresh, error_rate_min, error_rate_max, error_rate_step, vect_fmax, vect_fstep, diag_iters, write_ham_vector, write_full_vector );
}


void cAnalyze::CommandMapTasks(cString cur_string)
{
  cout << "Constructing genotype-phenotype maps..." << endl;

  // Load in the variables...
  cString directory = PopDirectory(cur_string, "phenotype/");
  int print_mode = 0;   // 0=Normal, 1=Boolean results
  int file_type = FILE_TYPE_TEXT;

  // HTML special flags...
  bool link_maps = false;  // Should muliple maps be linked together?
  bool link_insts = false; // Should links be made to instruction descs?

  // Collect any other format information needed...
  tList< tDataEntryCommand<cAnalyzeGenotype> > output_list;
  tListIterator< tDataEntryCommand<cAnalyzeGenotype> > output_it(output_list);

  cStringList arg_list(cur_string);

  cout << "Found " << arg_list.GetSize() << " args." << endl;

  // Check for some command specific variables.
  if (arg_list.PopString("0") != "") print_mode = 0;
  if (arg_list.PopString("1") != "") print_mode = 1;
  if (arg_list.PopString("text") != "") file_type = FILE_TYPE_TEXT;
  if (arg_list.PopString("html") != "") file_type = FILE_TYPE_HTML;
  if (arg_list.PopString("link_maps") != "") link_maps = true;
  if (arg_list.PopString("link_insts") != "") link_insts = true;

  cout << "There are " << arg_list.GetSize() << " column args." << endl;

  LoadGenotypeDataList(arg_list, output_list);

  cout << "Args are loaded." << endl;

  const int num_cols = output_list.GetSize();

  // Give some information in verbose mode.
  if (verbose == true) {
    cout << "  outputing as ";
    if (print_mode == 1) cout << "boolean ";
    if (file_type == FILE_TYPE_TEXT) {
      cout << "text files." << endl;
    } else { // if (file_type == FILE_TYPE_HTML) {
      cout << "HTML files";
      if (link_maps == true) cout << "; linking files together";
      if (link_maps == true) cout << "; linking inst names to descs";
      cout << "." << endl;
    }
    cout << "  Format: ";

    output_it.Reset();
    while (output_it.Next() != NULL) {
      cout << output_it.Get()->GetName() << " ";
    }
    cout << endl;
  }


  ///////////////////////////////////////////////////////
  // Loop through all of the genotypes in this batch...

  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
  cAnalyzeGenotype * genotype = NULL;
  while ((genotype = batch_it.Next()) != NULL) {
    if (verbose == true) cout << "  Mapping " << genotype->GetName() << endl;

    // Construct this filename...
    cString filename;
    if (file_type == FILE_TYPE_TEXT) {
      filename.Set("%stasksites.%s.dat", directory(), genotype->GetName()());
    } else {   //  if (file_type == FILE_TYPE_HTML) {
      filename.Set("%stasksites.%s.html", directory(), genotype->GetName()());
    }
    ofstream fp(filename());

    // Construct linked filenames...
    cString next_file("");
    cString prev_file("");
    if (link_maps == true) {
      // Check the next genotype on the list...
      if (batch_it.Next() != NULL) {
	next_file.Set("tasksites.%s.html", batch_it.Get()->GetName()());
      }
      batch_it.Prev();  // Put the list back where it was...

      // Check the previous genotype on the list...
      if (batch_it.Prev() != NULL) {
	prev_file.Set("tasksites.%s.html", batch_it.Get()->GetName()());
      }
      batch_it.Next();  // Put the list back where it was...
    }

    // Calculate the stats for the genotype we're working with...
    genotype->Recalculate();

    // Headers...
    if (file_type == FILE_TYPE_TEXT) {
      fp << "-1 "  << batch[cur_batch].Name() << " "
	 << genotype->GetID() << " ";

      tDataEntryCommand<cAnalyzeGenotype> * data_command = NULL;
      while ((data_command = output_it.Next()) != NULL) {
	data_command->SetTarget(genotype);
	fp << data_command->GetEntry() << " ";
      }
      fp << endl;

    } else { // if (file_type == FILE_TYPE_HTML) {
      // Mark file as html
      fp << "<html>" << endl;

      // Setup any javascript macros needed...
      fp << "<head>" << endl;
      if (link_insts == true) {
	fp << "<script language=\"javascript\">" << endl
	   << "function Inst(inst_name)" << endl
	   << "{" << endl
	   << "var filename = \"help.\" + inst_name + \".html\";" << endl
	   << "newwin = window.open(filename, 'Instruction', "
	   << "'toolbar=0,status=0,location=0,directories=0,menubar=0,"
	   << "scrollbars=1,height=150,width=300');" << endl
	   << "newwin.focus();" << endl
	   << "}" << endl
	   << "</script>" << endl;
      }
      fp << "</head>" << endl;

      // Setup the body...
      fp << "<body bgcolor=\"#FFFFFF\"" << endl
	 << " text=\"#000000\"" << endl
	 << " link=\"#0000AA\"" << endl
	 << " alink=\"#0000FF\"" << endl
	 << " vlink=\"#000044\">" << endl
	 << endl
	 << "<h1 align=center>Run " << batch[cur_batch].Name()
	 << ", ID " << genotype->GetID() << "</h1>" << endl
	 << "<center>" << endl
	 << endl;

      // Links?
      fp << "<table width=90%><tr><td align=left>";
      if (prev_file != "") fp << "<a href=\"" << prev_file << "\">Prev</a>";
      else fp << "&nbsp;";
      fp << "<td align=right>";
      if (next_file != "") fp << "<a href=\"" << next_file << "\">Next</a>";
      else fp << "&nbsp;";
      fp << "</tr></table>" << endl;

      // The table
      fp << "<table border=1 cellpadding=2>" << endl;

      // The headings...///
      fp << "<tr><td colspan=3> ";
      output_it.Reset();
      while (output_it.Next() != NULL) {
	fp << "<th>" << output_it.Get()->GetDesc() << " ";
      }
      fp << "</tr>" << endl;

      // The base creature...
      fp << "<tr><th colspan=3>Base Creature";
      tDataEntryCommand<cAnalyzeGenotype> * data_command = NULL;
      cAnalyzeGenotype null_genotype("a", inst_lib);
      while ((data_command = output_it.Next()) != NULL) {
	data_command->SetTarget(genotype);
	genotype->SetSpecialArgs(data_command->GetArgs());
	if (data_command->Compare(&null_genotype) > 0) {
	  fp << "<th bgcolor=\"#00FF00\">";
	}
	else  fp << "<th bgcolor=\"#FF0000\">";

	if (data_command->HasArg("blank") == true) fp << "&nbsp;" << " ";
	else fp << data_command->GetEntry() << " ";
      }
      fp << "</tr>" << endl;
    }


    const int max_line = genotype->GetLength();
    const cGenome & base_genome = genotype->GetGenome();
    cGenome mod_genome(base_genome);

    // Keep track of the number of failues/successes for attributes...
    int * col_pass_count = new int[num_cols];
    int * col_fail_count = new int[num_cols];
    for (int i = 0; i < num_cols; i++) {
      col_pass_count[i] = 0;
      col_fail_count[i] = 0;
    }

    // Build an empty instruction into the an instruction library.
    cInstLib map_inst_lib(inst_lib);
    map_inst_lib.Add("map-null", &cHardwareBase::Inst_Nop);
    const cInstruction null_inst = map_inst_lib.GetInst("map-null");

    // Loop through all the lines of code, testing the removal of each.
    for (int line_num = 0; line_num < max_line; line_num++) {
      int cur_inst = base_genome[line_num].GetOp();
      char cur_symbol = base_genome[line_num].GetSymbol();

      mod_genome[line_num] = null_inst;
      cAnalyzeGenotype test_genotype(mod_genome, map_inst_lib);
      test_genotype.Recalculate();

      if (file_type == FILE_TYPE_HTML) fp << "<tr><td align=right>";
      fp << (line_num + 1) << " ";
      if (file_type == FILE_TYPE_HTML) fp << "<td align=center>";
      fp << cur_symbol << " ";
      if (file_type == FILE_TYPE_HTML) fp << "<td align=center>";
      if (link_insts == true) {
	fp << "<a href=\"javascript:Inst('"
	   << map_inst_lib.GetName(cur_inst)
	   << "')\">";
      }
      fp << map_inst_lib.GetName(cur_inst) << " ";
      if (link_insts == true) fp << "</a>";


      // Print the individual columns...
      output_it.Reset();
      tDataEntryCommand<cAnalyzeGenotype> * data_command = NULL;
      int cur_col = 0;
      while ((data_command = output_it.Next()) != NULL) {
	data_command->SetTarget(&test_genotype);
	test_genotype.SetSpecialArgs(data_command->GetArgs());
	int compare = data_command->Compare(genotype);
	if (file_type == FILE_TYPE_HTML) {
	  data_command->HTMLPrint(fp, compare,
				  !(data_command->HasArg("blank")));
	} 
	else fp << data_command->GetEntry() << " ";

	if (compare == -2) col_fail_count[cur_col]++;
	else if (compare == 2) col_pass_count[cur_col]++;
	cur_col++;
      }
      if (file_type == FILE_TYPE_HTML) fp << "</tr>";
      fp << endl;

      // Reset the mod_genome back to the original sequence.
      mod_genome[line_num].SetOp(cur_inst);
    }


    // Construct the final line of the table with all totals...
    if (file_type == FILE_TYPE_HTML) {
      fp << "<tr><th colspan=3>Totals";

      for (int i = 0; i < num_cols; i++) {
	if (col_pass_count[i] > 0) {
	  fp << "<th bgcolor=\"#00FF00\">" << col_pass_count[i];
	}
	else if (col_fail_count[i] > 0) {
	  fp << "<th bgcolor=\"#FF0000\">" << col_fail_count[i];
	}
	else fp << "<th>0";
      }
      fp << "</tr>" << endl;

      // And close everything up...
      fp << "</table>" << endl
	 << "</center>" << endl;
    }

    delete [] col_pass_count;
    delete [] col_fail_count;

  }
}


void cAnalyze::CommandMapMutations(cString cur_string)
{
  cout << "Constructing genome mutations maps..." << endl;

  // Load in the variables...
  cString directory = PopDirectory(cur_string, "mutations/");
  int file_type = FILE_TYPE_TEXT;

  cStringList arg_list(cur_string);

  // Check for some command specific variables.
  if (arg_list.PopString("text") != "") file_type = FILE_TYPE_TEXT;
  if (arg_list.PopString("html") != "") file_type = FILE_TYPE_HTML;

  // Give some information in verbose mode.
  if (verbose == true) {
    cout << "  outputing as ";
    if (file_type == FILE_TYPE_TEXT) cout << "text files." << endl;
    else cout << "HTML files." << endl;
  }


  ///////////////////////////////////////////////////////
  // Loop through all of the genotypes in this batch...

  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
  cAnalyzeGenotype * genotype = NULL;
  while ((genotype = batch_it.Next()) != NULL) {
    if (verbose == true) {
      cout << "  Creating mutation map for " << genotype->GetName() << endl;
    }

    // Construct this filename...
    cString filename;
    if (file_type == FILE_TYPE_TEXT) {
      filename.Set("%smut_map.%s.dat", directory(), genotype->GetName()());
    } else {   //  if (file_type == FILE_TYPE_HTML) {
      filename.Set("%smut_map.%s.html", directory(), genotype->GetName()());
    }
    ofstream fp(filename());

    // Calculate the stats for the genotype we're working with...
    genotype->Recalculate();
    const double base_fitness = genotype->GetFitness();
    const int num_insts = inst_lib.GetSize();

    // Headers...
    if (file_type == FILE_TYPE_TEXT) {
      fp << "# 1: Genome instruction ID (pre-mutation)" << endl;
      for (int i = 0; i < num_insts; i++) {
	fp << "# " << i+1 <<": Fit if mutated to '"
	   << inst_lib.GetName(i) << "'" << endl;
      }
      fp << "# " << num_insts + 2 << ": Knockout" << endl;
      fp << "# " << num_insts + 3 << ": Fraction Lethal" << endl;
      fp << "# " << num_insts + 4 << ": Fraction Detremental" << endl;
      fp << "# " << num_insts + 5 << ": Fraction Neutral" << endl;
      fp << "# " << num_insts + 6 << ": Fraction Beneficial" << endl;
      fp << "# " << num_insts + 7 << ": Average Fitness" << endl;
      fp << "# " << num_insts + 8 << ": Expected Entropy" << endl;
      fp << "# " << num_insts + 9 << ": Original Instruction Name" << endl;
      fp << endl;

    } else { // if (file_type == FILE_TYPE_HTML) {
      // Mark file as html
      fp << "<html>" << endl;

      // Setup the body...
      fp << "<body bgcolor=\"#FFFFFF\"" << endl
	 << " text=\"#000000\"" << endl
	 << " link=\"#0000AA\"" << endl
	 << " alink=\"#0000FF\"" << endl
	 << " vlink=\"#000044\">" << endl
	 << endl
	 << "<h1 align=center>Mutation Map for Run " << batch[cur_batch].Name()
	 << ", ID " << genotype->GetID() << "</h1>" << endl
	 << "<center>" << endl
	 << endl;

      // The main chart...
      fp << "<table border=1 cellpadding=2>" << endl;

      // The headings...///
      fp << "<tr><th>Genome ";
      for (int i = 0; i < num_insts; i++) {
	fp << "<th>" << inst_lib.GetName(i) << " ";
      }
      fp << "<th>Knockout ";
      fp << "<th>Frac. Lethal ";
      fp << "<th>Frac. Detremental ";
      fp << "<th>Frac. Neutral ";
      fp << "<th>Frac. Beneficial ";
      fp << "<th>Ave. Fitness ";
      fp << "<th>Expected Entropy ";
      fp << "</tr>" << endl << endl;
    }

    const int max_line = genotype->GetLength();
    const cGenome & base_genome = genotype->GetGenome();
    cGenome mod_genome(base_genome);

    // Keep track of the number of mutations in each category...
    int total_dead = 0, total_neg = 0, total_neut = 0, total_pos = 0;
    double total_fitness = 0.0, total_entopry = 0.0;
    tArray<double> col_fitness(num_insts + 1);
    col_fitness.SetAll(0.0);

    // Build an empty instruction into the an instruction library.
    cInstLib map_inst_lib(inst_lib);
    map_inst_lib.Add("map-null", &cHardwareBase::Inst_Nop);
    const cInstruction null_inst = map_inst_lib.GetInst("map-null");

    cString color_string;  // For coloring cells...

    // Loop through all the lines of code, testing all mutations...
    for (int line_num = 0; line_num < max_line; line_num++) {
      int cur_inst = base_genome[line_num].GetOp();
      char cur_symbol = base_genome[line_num].GetSymbol();
      int row_dead = 0, row_neg = 0, row_neut = 0, row_pos = 0;
      double row_fitness = 0.0;

      // Column 1... the original instruction in the geneome.
      if (file_type == FILE_TYPE_HTML) {
	fp << "<tr><td align=right>" << inst_lib.GetName(cur_inst)
	   << " (" << cur_symbol << ") ";
      } else {
	fp << cur_inst << " ";
      }
      
      // Columns 2 to D+1 (the possible mutations)
      for (int mod_inst = 0; mod_inst < num_insts; mod_inst++) {
	mod_genome[line_num].SetOp(mod_inst);
	cAnalyzeGenotype test_genotype(mod_genome, inst_lib);
	test_genotype.Recalculate();
	const double test_fitness = test_genotype.GetFitness() / base_fitness;
	row_fitness += test_fitness;
	total_fitness += test_fitness;
	col_fitness[mod_inst] += test_fitness;

	// Categorize this mutation...
	if (test_fitness == 1.0) {           // Neutral Mutation...
	  row_neut++;
	  total_neut++;
	  if (file_type == FILE_TYPE_HTML) color_string = "#FFFFFF";
	} else if (test_fitness == 0.0) {    // Lethal Mutation...
	  row_dead++;
	  total_dead++;
	  if (file_type == FILE_TYPE_HTML) color_string = "#FF0000";
	} else if (test_fitness < 1.0) {     // Detrimental Mutation...
	  row_neg++;
	  total_neg++;
	  if (file_type == FILE_TYPE_HTML) color_string = "#FFFF00";
	} else {                             // Beneficial Mutation...
	  row_pos++;
	  total_pos++;
	  if (file_type == FILE_TYPE_HTML) color_string = "#00FF00";
	}
	  
	// Write out this cell...
	if (file_type == FILE_TYPE_HTML) {
	  fp << "<th bgcolor=\"" << color_string << "\">";
	}
	fp << test_fitness << " ";
      }

      // Column: Knockout
      mod_genome[line_num] = null_inst;
      cAnalyzeGenotype test_genotype(mod_genome, map_inst_lib);
      test_genotype.Recalculate();
      const double test_fitness = test_genotype.GetFitness();
      col_fitness[num_insts] += test_fitness;

      // Categorize this mutation if its in HTML mode (color only)...
      if (file_type == FILE_TYPE_HTML) {
	if (test_fitness == base_fitness) color_string = "#FFFFFF";
	else if (test_fitness == 0.0) color_string = "#FF0000";
	else if (test_fitness < base_fitness) color_string = "#FFFF00";
	else color_string = "#00FF00";

	fp << "<th bgcolor=\"" << color_string << "\">";
      }
      
      fp << test_fitness << " ";

      // Fraction Columns...
      if (file_type == FILE_TYPE_HTML) fp << "<th bgcolor=\"#FF0000\">";
      fp << (double) row_dead / (double) num_insts << " ";

      if (file_type == FILE_TYPE_HTML) fp << "<th bgcolor=\"#FFFF00\">";
      fp << (double) row_neg / (double) num_insts << " ";

      if (file_type == FILE_TYPE_HTML) fp << "<th bgcolor=\"#FFFFFF\">";
      fp << (double) row_neut / (double) num_insts << " ";

      if (file_type == FILE_TYPE_HTML) fp << "<th bgcolor=\"#00FF00\">";
      fp << (double) row_pos / (double) num_insts << " ";


      // Column: Average Fitness
      if (file_type == FILE_TYPE_HTML) fp << "<th>";
      fp << row_fitness / (double) num_insts << " ";

      // Column: Expected Entropy  @CAO Implement!
      if (file_type == FILE_TYPE_HTML) fp << "<th>";
      fp << 0.0 << " ";
      
      // End this row...
      if (file_type == FILE_TYPE_HTML) fp << "</tr>";
      fp << endl;

      // Reset the mod_genome back to the original sequence.
      mod_genome[line_num].SetOp(cur_inst);
    }


    // Construct the final line of the table with all totals...
    if (file_type == FILE_TYPE_HTML) {
      fp << "<tr><th>Totals";
      
      // Instructions + Knockout
      for (int i = 0; i <= num_insts; i++) {
	fp << "<th>" << col_fitness[i] / max_line << " ";
      }

      int total_tests = max_line * num_insts;
      fp << "<th>" << (double) total_dead / (double) total_tests << " ";
      fp << "<th>" << (double) total_neg / (double) total_tests << " ";
      fp << "<th>" << (double) total_neut / (double) total_tests << " ";
      fp << "<th>" << (double) total_pos / (double) total_tests << " ";
      fp << "<th>" << total_fitness / (double) total_tests << " ";
      fp << "<th>" << 0.0 << " ";


      // And close everything up...
      fp << "</table>" << endl
	 << "</center>" << endl;
    }

  }
}


void cAnalyze::CommandHamming(cString cur_string)
{
  cString filename("hamming.dat");
  if (cur_string.GetSize() != 0) filename = cur_string.PopWord();

  int batch1 = PopBatch(cur_string.PopWord());
  int batch2 = PopBatch(cur_string.PopWord());

  // We want batch2 to be the larger one for efficiency...
  if (batch[batch1].List().GetSize() > batch[batch2].List().GetSize()) {
    int tmp = batch1;  batch1 = batch2;  batch2 = tmp;
  }

  if (verbose == false) {
    cout << "Calculating Hamming Distance... ";
    cout.flush();
  } else {
    cout << "Calculating Hamming Distance between batches "
	 << batch1 << " and " << batch2 << endl;
    cout.flush();
  }

  // Setup some variables;
  cAnalyzeGenotype * genotype1 = NULL;
  cAnalyzeGenotype * genotype2 = NULL;
  int total_dist = 0;
  int total_count = 0;

  tListIterator<cAnalyzeGenotype> list1_it(batch[batch1].List());
  tListIterator<cAnalyzeGenotype> list2_it(batch[batch2].List());

  while ((genotype1 = list1_it.Next()) != NULL) {
    list2_it.Reset();
    while ((genotype2 = list2_it.Next()) != NULL) {
      // Determine the counts...
      const int count1 = genotype1->GetNumCPUs();
      const int count2 = genotype2->GetNumCPUs();
      const int num_pairs = (genotype1 == genotype2) ?
	((count1 - 1) * (count2 - 1)) : (count1 * count2);
      if (num_pairs == 0) continue;

      // And do the tests...
      const int dist =
	cGenomeUtil::FindHammingDistance(genotype1->GetGenome(),
					 genotype2->GetGenome());
      total_dist += dist * num_pairs;
      total_count += num_pairs;
    }
  }


  // Calculate the final answer
  double ave_dist = (double) total_dist / (double) total_count;
  cout << " ave distance = " << ave_dist << endl;

  cDataFile & df = data_file_manager.Get(filename);

  df.WriteComment( "Hamming distance information" );
  df.WriteTimeStamp();  

  df.Write(batch[batch1].Name(), "Name of First Batch");
  df.Write(batch[batch2].Name(), "Name of Second Batch");
  df.Write(ave_dist,             "Average Hamming Distance");
  df.Write(total_count,          "Total Pairs Test");
  df.Endl();
}

void cAnalyze::CommandLevenstein(cString cur_string)
{
  cString filename("lev.dat");
  if (cur_string.GetSize() != 0) filename = cur_string.PopWord();

  int batch1 = PopBatch(cur_string.PopWord());
  int batch2 = PopBatch(cur_string.PopWord());

  // We want batch2 to be the larger one for efficiency...
  if (batch[batch1].List().GetSize() > batch[batch2].List().GetSize()) {
    int tmp = batch1;  batch1 = batch2;  batch2 = tmp;
  }

  if (verbose == false) {
    cout << "Calculating Levenstein Distance... ";
    cout.flush();
  } else {
    cout << "Calculating Levenstein Distance between batch "
	 << batch1 << " and " << batch2 << endl;
    cout.flush();
  }

  // Setup some variables;
  cAnalyzeGenotype * genotype1 = NULL;
  cAnalyzeGenotype * genotype2 = NULL;
  int total_dist = 0;
  int total_count = 0;

  tListIterator<cAnalyzeGenotype> list1_it(batch[batch1].List());
  tListIterator<cAnalyzeGenotype> list2_it(batch[batch2].List());

  // Loop through all of the genotypes in each batch...
  while ((genotype1 = list1_it.Next()) != NULL) {
    list2_it.Reset();
    while ((genotype2 = list2_it.Next()) != NULL) {
      // Determine the counts...
      const int count1 = genotype1->GetNumCPUs();
      const int count2 = genotype2->GetNumCPUs();
      const int num_pairs = (genotype1 == genotype2) ?
	((count1 - 1) * (count2 - 1)) : (count1 * count2);
      if (num_pairs == 0) continue;

      // And do the tests...
      const int dist = cGenomeUtil::FindEditDistance(genotype1->GetGenome(),
						     genotype2->GetGenome());
      total_dist += dist * num_pairs;
      total_count += num_pairs;
    }
  }

  // Calculate the final answer
  double ave_dist = (double) total_dist / (double) total_count;
  cout << " ave distance = " << ave_dist << endl;

  cDataFile & df = data_file_manager.Get(filename);

  df.WriteComment( "Levenstein distance information" );
  df.WriteTimeStamp();  

  df.Write(batch[batch1].Name(), "Name of First Batch");
  df.Write(batch[batch2].Name(), "Name of Second Batch");
  df.Write(ave_dist,             "Average Levenstein Distance");
  df.Write(total_count,          "Total Pairs Test");
  df.Endl();
}

void cAnalyze::CommandSpecies(cString cur_string)
{
  cString filename("species.dat");
  if (cur_string.GetSize() != 0) filename = cur_string.PopWord();

  int batch1 = PopBatch(cur_string.PopWord());
  int batch2 = PopBatch(cur_string.PopWord());

  // We want batch2 to be the larger one for efficiency...
  if (batch[batch1].List().GetSize() > batch[batch2].List().GetSize()) {
    int tmp = batch1;  batch1 = batch2;  batch2 = tmp;
  }

  if (verbose == false) cout << "Calculating Species Distance... " << endl;
  else cout << "Calculating Species Distance between batch "
	    << batch1 << " and " << batch2 << endl;

  // Setup some variables;
  cAnalyzeGenotype * genotype1 = NULL;
  cAnalyzeGenotype * genotype2 = NULL;
  int total_dist = 0;
  int total_count = 0;
  int fail_count = 0;

  tListIterator<cAnalyzeGenotype> list1_it(batch[batch1].List());
  tListIterator<cAnalyzeGenotype> list2_it(batch[batch2].List());

  // Loop through all of the genotypes in each batch...
  while ((genotype1 = list1_it.Next()) != NULL) {
    cSpecies species(genotype1->GetGenome(), -1);
    list2_it.Reset();
    while ((genotype2 = list2_it.Next()) != NULL) {
      // Determine the counts...
      const int count1 = genotype1->GetNumCPUs();
      const int count2 = genotype2->GetNumCPUs();
      const int num_pairs = (genotype1 == genotype2) ?
	((count1 - 1) * (count2 - 1)) : (count1 * count2);
      if (num_pairs == 0) continue;

      // And do the tests...
      const int dist = (genotype1 == genotype2) ? 0 :
	species.Compare(genotype2->GetGenome());
      if (dist >= 0) {
	total_dist += dist * num_pairs;
	total_count += num_pairs;
      } else {
	fail_count += num_pairs;
      }
    }
  }

  // Calculate the final answer
  double ave_dist = (double) total_dist / (double) total_count;
  cout << "  ave distance = " << ave_dist
       << " in " << total_count
       << " tests, with " << fail_count
       << " additional failures."  << endl;

  cDataFile & df = data_file_manager.Get(filename);

  df.WriteComment( "Species information" );
  df.WriteTimeStamp();  

  df.Write(batch[batch1].Name(), "Name of First Batch");
  df.Write(batch[batch2].Name(), "Name of Second Batch");
  df.Write(ave_dist,             "Average Species Distance");
  df.Write(total_count,          "Total Legal Pairs");
  df.Write(fail_count,           "Total Failed Pairs");
  df.Endl();
}

void cAnalyze::CommandAlign(cString cur_string)
{
  cout << "Aligning sequences..." << endl;

  if (batch[cur_batch].IsLineage() == false && verbose == true) {
    cerr << "  Warning: sequences may not be a consecutive lineage."
	 << endl;
  }

  // Create an array of all the sequences we need to align.
  tList<cAnalyzeGenotype> & glist = batch[cur_batch].List();
  tListIterator<cAnalyzeGenotype> batch_it(glist);
  const int num_sequences = glist.GetSize();
  cString * sequences = new cString[num_sequences];

  // Move through each sequence an update it.
  batch_it.Reset();
  cString diff_info;
  for (int i = 0; i < num_sequences; i++) {
    sequences[i] = batch_it.Next()->GetGenome().AsString();
    if (i == 0) continue;
    // Track of the number of insertions and deletions to shift properly.
    int num_ins = 0;
    int num_del = 0;

    // Compare each string to the previous.
    cStringUtil::EditDistance(sequences[i], sequences[i-1], diff_info, '_');
    while (diff_info.GetSize() != 0) {
      cString cur_mut = diff_info.Pop(',');
      const char mut_type = cur_mut[0];
      cur_mut.ClipFront(1); cur_mut.ClipEnd(1);
      int position = cur_mut.AsInt();

      // Nothing to do with Mutations
      if (mut_type == 'M') continue;

      // Handle insertions...
      if (mut_type == 'I') {
	// Loop back and insert an '_' into all previous sequences.
	for (int j = 0; j < i; j++) {
	  sequences[j].Insert('_', position + num_del);
	}
	num_ins++;
      }

      // Handle Deletions...
      else if (mut_type == 'D') {
	// Insert '_' into the current sequence at the point of deletions.
	sequences[i].Insert("_", position + num_ins);
	num_del++;
      }

    }
  }

  batch_it.Reset();
  for (int i = 0; i < num_sequences; i++) {
    batch_it.Next()->SetAlignedSequence(sequences[i]);
  }

  // Cleanup
  delete [] sequences;

  // Adjust the flags on this batch
  // batch[cur_batch].SetLineage(false);
  batch[cur_batch].SetAligned(true);
}


void cAnalyze::WriteClone(cString cur_string)
{
  // Load in the variables...
  cString filename("clone.dat");
  int num_cells = -1;
  if (cur_string.GetSize() != 0) filename = cur_string.PopWord();
  if (cur_string.GetSize() != 0) num_cells = cur_string.PopWord().AsInt();
  

  ofstream & fp = data_file_manager.GetOFStream(filename);

  // Start up again at update zero...
  fp << "0 ";

  // Setup the genebank sizes of lists to all be zero.
  fp << MAX_CREATURE_SIZE << " ";
  for (int i = 0; i < MAX_CREATURE_SIZE; i++) {
    fp << "0 ";
  }

  // Save the individual genotypes
  fp << batch[cur_batch].List().GetSize() << " ";
  
  int org_count = 0;
  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
  cAnalyzeGenotype * genotype = NULL;
  while ((genotype = batch_it.Next()) != NULL) {
    org_count += genotype->GetNumCPUs();
    const int length = genotype->GetLength();
    const cGenome & genome = genotype->GetGenome();

    fp << genotype->GetID() << " "
       << length << " ";

    for (int i = 0; i < length; i++) {
      fp << genome[i].GetOp() << " ";
    }
  }

  // Write out the current state of the grid.
  
  if (num_cells == 0) num_cells = org_count;
  fp << num_cells << " ";

  batch_it.Reset();
  while ((genotype = batch_it.Next()) != NULL) {
    for (int i = 0; i < genotype->GetNumCPUs(); i++) {
      fp << genotype->GetID() << " ";
    }
  }

  // Fill out the remainder of the grid with -1
  for (int i = org_count; i < num_cells; i++) {
    fp << "-1 ";
  }
}


void cAnalyze::WriteInjectEvents(cString cur_string)
{
  // Load in the variables...
  cString filename("clone.dat");
  int start_cell = 0;
  int lineage = 0;
  if (cur_string.GetSize() != 0) filename = cur_string.PopWord();
  if (cur_string.GetSize() != 0) start_cell = cur_string.PopWord().AsInt();
  if (cur_string.GetSize() != 0) lineage = cur_string.PopWord().AsInt();
  
  ofstream & fp = data_file_manager.GetOFStream(filename);

  int org_count = 0;
  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
  cAnalyzeGenotype * genotype = NULL;
  while ((genotype = batch_it.Next()) != NULL) {
    const int cur_count = genotype->GetNumCPUs();
    org_count += cur_count;
    const cGenome & genome = genotype->GetGenome();

    fp << "u 0 inject_sequence "
       << genome.AsString() << " "
       << start_cell << " "
       << start_cell + cur_count << " "
       << genotype->GetMerit() << " "
       << lineage << " "
       << endl;
    start_cell += cur_count;
  }
}


void cAnalyze::AnalyzeMuts(cString cur_string)
{
  cout << "Analyzing Mutations" << endl;

  // Make sure we have everything we need.
  if (batch[cur_batch].IsAligned() == false) {
    cout << "  ERROR: sequences not aligned." << endl;
    return;
  }

  // Setup variables...
  cString filename("analyze_muts.dat");
  bool all_combos = false;
  if (cur_string.GetSize() != 0) filename = cur_string.PopWord();
  if (cur_string.GetSize() != 0) all_combos = cur_string.PopWord().AsInt();

  tList<cAnalyzeGenotype> & gen_list = batch[cur_batch].List();
  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());

  const int num_sequences = gen_list.GetSize();
  const int sequence_length =
    gen_list.GetFirst()->GetAlignedSequence().GetSize();
  cString * sequences = new cString[num_sequences];
  int * mut_count = new int[sequence_length];
  for (int i = 0; i < sequence_length; i++) mut_count[i] = 0;

  // Load in the sequences
  batch_it.Reset();
  int count = 0;
  while (batch_it.Next() != NULL) {
    sequences[count] = batch_it.Get()->GetAlignedSequence();
    count++;
  }

  // Count the number of changes at each site...
  for (int i = 1; i < num_sequences; i++) {       // For each pair...
    cString & seq1 = sequences[i-1];
    cString & seq2 = sequences[i];
    for (int j = 0; j < sequence_length; j++) {   // For each site...
      if (seq1[j] != seq2[j]) mut_count[j]++;
    }
  }

  // Grab the two strings we're actively going to be working with.
  cString & first_seq = sequences[0];
  cString & last_seq = sequences[num_sequences - 1];

  // Print out the header...
  ofstream & fp = data_file_manager.GetOFStream(filename);
  fp << "# " << sequences[0] << endl;
  fp << "# " << sequences[num_sequences - 1] << endl;
  fp << "# ";
  for (int i = 0; i < sequence_length; i++) {
    if (mut_count[i] == 0) fp << " ";
    else if (mut_count[i] > 9) fp << "+";
    else fp << mut_count[i];
  }
  fp << endl;
  fp << "# ";
  for (int i = 0; i < sequence_length; i++) {
    if (first_seq[i] == last_seq[i]) fp << " ";
    else fp << "^";
  }
  fp << endl << endl;

  // Count the number of diffs between the two strings we're interested in.
  const int total_diffs = cStringUtil::Distance(first_seq, last_seq);
  if (verbose) cout << "  " << total_diffs << " mutations being tested."
		    << endl;

  // Locate each difference.
  int * mut_positions = new int[total_diffs];
  int cur_mut = 0;
  for (int i = 0; i < first_seq.GetSize(); i++) {
    if (first_seq[i] != last_seq[i]) {
      mut_positions[cur_mut] = i;
      cur_mut++;
    }
  }

  // The number of mutations we need to deal with will tell us how much
  // we can attempt to do. (@CAO should be able to overide defaults)
  bool scan_combos = true;  // Scan all possible combos of mutations?
  bool detail_muts = true;  // Collect detailed info on all mutations?
  bool print_all = true;    // Print everything we collect without digestion?
  if (total_diffs > 30) scan_combos = false;
  if (total_diffs > 20) detail_muts = false;
  if (total_diffs > 10) print_all = false;

  // Start moving through the difference combinations...
  if (scan_combos) {
    const int total_combos = 1 << total_diffs;
    cout << "  Scanning through " << total_combos << " combos." << endl;

    double * total_fitness = new double[total_diffs + 1];
    double * total_sqr_fitness = new double[total_diffs + 1];
    double * max_fitness = new double[total_diffs + 1];
    cString * max_sequence = new cString[total_diffs + 1];
    int * test_count = new int[total_diffs + 1];
    for (int i = 0; i <= total_diffs; i++) {
      total_fitness[i] = 0.0;
      total_sqr_fitness[i] = 0.0;
      max_fitness[i] = 0.0;
      test_count[i] = 0;
    }

    // Loop through all of the combos...
    const int combo_step = total_combos / 79;
    for (int combo_id = 0; combo_id < total_combos; combo_id++) {
      if (combo_id % combo_step == 0) {
	cout << '.';
	cout.flush();
      }
      // Start at the first sequence and add needed changes...
      cString test_sequence = first_seq;
      int diff_count = 0;
      for (int mut_id = 0; mut_id < total_diffs; mut_id++) {
	if ((combo_id >> mut_id) & 1) {
	  const int cur_pos = mut_positions[mut_id];
	  test_sequence[cur_pos] = last_seq.GetData()[cur_pos];
	  diff_count++;
	}
      }

      // Determine the fitness of the current sequence...
      cGenome test_genome(test_sequence);
      cCPUTestInfo test_info;
      test_info.TestThreads();
      cTestCPU::TestGenome(test_info, test_genome);
      const double fitness = test_info.GetGenotypeFitness();

      //cAnalyzeGenotype test_genotype(test_sequence);
      //test_genotype.Recalculate();
      //const double fitness = test_genotype.GetFitness();

      total_fitness[diff_count] += fitness;
      total_sqr_fitness[diff_count] += fitness * fitness;
      if (fitness > max_fitness[diff_count]) {
	max_fitness[diff_count] = fitness;
	max_sequence[diff_count] = test_sequence;
//  	cout << endl
//  	     << max_sequence[diff_count] << " "
//  	     << test_info.GetGenotypeMerit() << " "
//  	     << fitness << " "
//  	     << combo_id << endl;
      }
      test_count[diff_count]++;
    }

    // Output the results...

    for (int i = 0; i <= total_diffs; i++) {
      cAnalyzeGenotype max_genotype(max_sequence[i], inst_lib);
      max_genotype.Recalculate();
      fp << i                                         << " "  //  1
	 << test_count[i]                             << " "  //  2
	 << total_fitness[i] / (double) test_count[i] << " "  //  3
	 << max_fitness[i]                            << " "  //  4
	 << max_genotype.GetMerit()                   << " "  //  5
	 << max_genotype.GetGestTime()                << " "  //  6
	 << max_genotype.GetLength()                  << " "  //  7
	 << max_genotype.GetCopyLength()              << " "  //  8
	 << max_genotype.GetExeLength()               << " "; //  9
      max_genotype.PrintTasks(fp, 3,12);
      fp << max_sequence[i] << endl;
    }

    // Cleanup
    delete [] total_fitness;
    delete [] total_sqr_fitness;
    delete [] max_fitness;
    delete [] max_sequence;
    delete [] test_count;
  }
  // If we can't scan through all combos, give wanring.
  else {
    cerr << "  Warning: too many mutations (" << total_diffs
	 << ") to scan through combos." << endl;
  }


  // Cleanup...
  delete [] sequences;
  delete [] mut_count;
  delete [] mut_positions;
}

void cAnalyze::AnalyzeInstructions(cString cur_string)
{
  if (verbose == true) {
    cout << "Analyzing Instructions in batch " << cur_batch << endl;
  }
  else cout << "Analyzeing Instructions..." << endl;

  // Load in the variables...
  cString filename("inst_analyze.dat");
  if (cur_string.GetSize() != 0) filename = cur_string.PopWord();
  const int num_insts = inst_lib.GetSize();

  // Setup the file...
  ofstream fp(filename);

  // Determine the file type...
  int file_type = FILE_TYPE_TEXT;
  while (filename.Find('.') != -1) filename.Pop('.');
  if (filename == "html") file_type = FILE_TYPE_HTML;

  // If we're in HTML mode, setup the header...
  if (file_type == FILE_TYPE_HTML) {
    // Document header...
    fp << "<html>" << endl
       << "<body bgcolor=\"#FFFFFF\"" << endl
       << " text=\"#000000\"" << endl
       << " link=\"#0000AA\"" << endl
       << " alink=\"#0000FF\"" << endl
       << " vlink=\"#000044\">" << endl
       << endl
       << "<h1 align=center>Instruction Chart: "
       << batch[cur_batch].Name() << endl
       << "<br><br>" << endl
       << endl;

    // Instruction key...
    const int num_cols = 6;
    const int num_rows = ((num_insts - 1) / num_cols) + 1;
    fp << "<table border=2 cellpadding=3>" << endl
       << "<tr bgcolor=\"#AAAAFF\"><th colspan=6>Instruction Set Legend</tr>"
       << endl;
    for (int i = 0; i < num_rows; i++) {
      fp << "<tr>";
      for (int j = 0; j < num_cols; j++) {
	const int inst_id = i + j * num_rows;
	if (inst_id < num_insts) {
	  cInstruction cur_inst(inst_id);
	  fp << "<td><b>" << cur_inst.GetSymbol() << "</b> : "
	     << inst_lib.GetName(inst_id) << " ";
	}
	else {
	  fp << "<td>&nbsp; ";
	}
      }
      fp << "</tr>" << endl;
    }
    fp << "</table>" << endl
       << "<br><br><br>" << endl;

    // Main table header...
    fp << "<center>" << endl
       << "<table border=1 cellpadding=2>" << endl
       << "<tr><th bgcolor=\"#AAAAFF\">Run # <th bgcolor=\"#AAAAFF\">Length"
       << endl;
    for (int i = 0; i < num_insts; i++) {
      cInstruction cur_inst(i);
      fp << "<th bgcolor=\"#AAAAFF\">" << cur_inst.GetSymbol() << " ";
    }
    fp << "</tr>" << endl;
  }
  else { // if (file_type == FILE_TYPE_TEXT) {
    fp << "#RUN_NAME  LENGTH  ";
    for (int i = 0; i < num_insts; i++) {
      cInstruction cur_inst(i);
      fp << cur_inst.GetSymbol() << ":" << inst_lib.GetName(i) << " ";
    }
    fp << endl;
  }

  // Figure out how often we expect each instruction to appear...
  const double exp_freq = 1.0 / (double) num_insts;
  const double min_freq = exp_freq * 0.5;
  const double max_freq = exp_freq * 2.0;

  double total_length = 0.0;
  tArray<double> total_freq(num_insts);
  for (int i = 0; i < num_insts; i++) total_freq[i] = 0.0;

  // Loop through all of the genotypes in this batch...
  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
  cAnalyzeGenotype * genotype = NULL;
  while ((genotype = batch_it.Next()) != NULL) {
    // Setup for counting...
    tArray<int> inst_bin(num_insts);
    for (int i = 0; i < num_insts; i++) inst_bin[i] = 0;

    // Count it up!
    const int genome_size = genotype->GetLength();
    for (int i = 0; i < genome_size; i++) {
      const int inst_id = genotype->GetGenome()[i].GetOp();
      inst_bin[inst_id]++;
    }

    // Print it out...
    if (file_type == FILE_TYPE_HTML) fp << "<tr><th>";
    fp << genotype->GetName() << " ";
    if (file_type == FILE_TYPE_HTML) fp << "<td align=center>";
    total_length += genome_size;
    fp << genome_size << " ";
    for (int i = 0; i < num_insts; i++) {
      const double inst_freq = ((double) inst_bin[i]) / (double) genome_size;
      total_freq[i] += inst_freq;
      if (file_type == FILE_TYPE_HTML) {
	if (inst_freq == 0.0) fp << "<td bgcolor=\"FFAAAA\">";
	else if (inst_freq < min_freq) fp << "<td bgcolor=\"FFFFAA\">";
	else if (inst_freq < max_freq) fp << "<td bgcolor=\"AAAAFF\">";
	else fp << "<td bgcolor=\"AAFFAA\">";
      }
      fp << cStringUtil::Stringf("%04.3f", inst_freq) << " ";
    }
    if (file_type == FILE_TYPE_HTML) fp << "</tr>";
    fp << endl;
  }

  if (file_type == FILE_TYPE_HTML) {
    int num_genomes = batch[cur_batch].List().GetSize();
    fp << "<tr><th>Average <th>" << total_length / num_genomes << " ";
    for (int i = 0; i < num_insts; i++) {
      double inst_freq = total_freq[i] / num_genomes;
      if (inst_freq == 0.0) fp << "<td bgcolor=\"FF0000\">";
      else if (inst_freq < min_freq) fp << "<td bgcolor=\"FFFF00\">";
      else if (inst_freq < max_freq) fp << "<td bgcolor=\"0000FF\">";
      else fp << "<td bgcolor=\"00FF00\">";
      fp << cStringUtil::Stringf("%04.3f", inst_freq) << " ";
    }
    fp << "</tr>" << endl
       << "</table></center>" << endl;
  }
}

void cAnalyze::AnalyzeBranching(cString cur_string)
{
  if (verbose == true) {
    cout << "Analyzing branching patterns in batch " << cur_batch << endl;
  }
  else cout << "Analyzeing Branches..." << endl;

  // Load in the variables...
  cString filename("branch_analyze.dat");
  if (cur_string.GetSize() != 0) filename = cur_string.PopWord();
  const int num_insts = inst_lib.GetSize();

  // Setup the file...
  ofstream fp(filename);

}

void cAnalyze::AnalyzeMutationTraceback(cString cur_string)
{
  if (verbose == true) {
    cout << "Analyzing mutation traceback in batch " << cur_batch << endl;
  }
  else cout << "Analyzing mutation traceback..." << endl;

  // This works best on lineages, so warn if we don't have one.
  if (batch[cur_batch].IsLineage() == false && verbose == true) {
    cerr << "  Warning: trying to traceback mutations outside of lineage."
	 << endl;
  }

  if (batch[cur_batch].List().GetSize() == 0) {
    cerr << "Error: Trying to traceback mutations with no genotypes in batch."
	 << endl;
    return;
  }

  // Make sure all genotypes are the same length.
  int size = -1;
  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
  cAnalyzeGenotype * genotype = NULL;
  while ((genotype = batch_it.Next()) != NULL) {
    if (size == -1) size = genotype->GetLength();
    if (size != genotype->GetLength()) {
      cerr << "  Error: Trying to traceback mutations in genotypes of differing lengths." << endl;
      cerr << "  Aborting." << endl;
      return;
    }
  }

  // Setup variables...
  cString filename("analyze_traceback.dat");
  if (cur_string.GetSize() != 0) filename = cur_string.PopWord();

  // Setup a genome to store the previous values before mutations.
  tArray<int> prev_inst(size);
  prev_inst.SetAll(-1);  // -1 indicates never changed.

  // Open the output file...
  ofstream fp(filename);

  // Loop through all of the genotypes again, testing mutation reversions.
  cAnalyzeGenotype * prev_genotype = batch_it.Next();
  while ((genotype = batch_it.Next()) != NULL) {
    // Check to see if any sites have changed...
    for (int i = 0; i < size; i++) {
      if (genotype->GetGenome()[i] != prev_genotype->GetGenome()[i]) {
	prev_inst[i] = prev_genotype->GetGenome()[i].GetOp();
      }
    }

    // Next, determine the fraction of mutations that are currently adaptive.
    int num_beneficial = 0;
    int num_neutral = 0;
    int num_detrimental = 0;
    int num_static = 0;      // Sites that were never mutated.

    cGenome test_genome = genotype->GetGenome();
    cCPUTestInfo test_info;
    cTestCPU::TestGenome(test_info, test_genome);
    const double base_fitness = test_info.GetGenotypeFitness();
    
    for (int i = 0; i < size; i++) {
      if (prev_inst[i] == -1) num_static++;
      else {
	test_genome[i].SetOp(prev_inst[i]);
	cTestCPU::TestGenome(test_info, test_genome);
	const double cur_fitness = test_info.GetGenotypeFitness();
	if (cur_fitness > base_fitness) num_detrimental++;
	else if (cur_fitness < base_fitness) num_beneficial++;
	else num_neutral++;
	test_genome[i] = genotype->GetGenome()[i];
      }      
    }

    fp << genotype->GetDepth() << " "
       << num_beneficial << " "
       << num_neutral << " "
       << num_detrimental << " "
       << num_static << " "
       << endl;

    prev_genotype = genotype;
  }
}


void cAnalyze::CommandHelpfile(cString cur_string)
{
  cout << "Printing helpfiles in: " << cur_string << endl;

  cHelpManager help_control;
  if (verbose == true) help_control.SetVerbose();
  while (cur_string.GetSize() > 0) {
    help_control.LoadFile(cur_string.PopWord());
  }

  help_control.PrintHTML();
}

void cAnalyze::CommandDocfile(cString cur_string)
{
  cout << "Printing documentation files in: " << cur_string << endl;

  cHelpManager help_control;
  if (verbose == true) help_control.SetVerbose();
  while (cur_string.GetSize() > 0) {
    help_control.LoadFile(cur_string.PopWord());
  }

  help_control.PrintHTML();
}



//////////////// Control...

void cAnalyze::VarSet(cString cur_string)
{
  cString var = cur_string.PopWord();

  if (cur_string.GetSize() == 0) {
    cerr << "Error: No variable provided in SET command" << endl;
    return;
  }

  cString & cur_variable = GetVariable(var);
  cur_variable = cur_string.PopWord();

  if (verbose == true) {
    cout << "Setting " << var << " to " << cur_variable << endl;
  }
}

void cAnalyze::BatchSet(cString cur_string)
{
  int next_batch = 0;
  if (cur_string.CountNumWords() > 0) {
    next_batch = cur_string.PopWord().AsInt();
  }
  if (verbose) cout << "Setting current batch to " << next_batch << endl;
  if (next_batch >= MAX_BATCHES) {
    cerr << "  Error: max batches is " << MAX_BATCHES << endl;
    exit(1);
  } else {
    cur_batch = next_batch;
  }
}

void cAnalyze::BatchName(cString cur_string)
{
  if (cur_string.CountNumWords() == 0) {
    if (verbose) cout << "  Warning: No name given in NAME_BATCH!" << endl;
    return;
  }

  batch[cur_batch].Name() = cur_string.PopWord();
}

void cAnalyze::BatchTag(cString cur_string)
{
  if (cur_string.CountNumWords() == 0) {
    if (verbose) cout << "  Warning: No tag given in TAG_BATCH!" << endl;
    return;
  }

  if (verbose == true) {
    cout << "Tagging batch " << cur_batch
	 << " with tag '" << cur_string << "'" << endl;
  }

  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
  cAnalyzeGenotype * genotype = NULL;
  while ((genotype = batch_it.Next()) != NULL) {
    genotype->SetTag(cur_string);
  }

}

void cAnalyze::BatchPurge(cString cur_string)
{
  int batch_id = cur_batch;
  if (cur_string.CountNumWords() > 0) batch_id = cur_string.PopWord().AsInt();

  if (verbose) cout << "Purging batch " << batch_id << endl;

  while (batch[batch_id].List().GetSize() > 0) {
    delete batch[batch_id].List().Pop();
  }

  batch[batch_id].SetLineage(false);
  batch[batch_id].SetAligned(false);
}

void cAnalyze::BatchDuplicate(cString cur_string)
{
  if (cur_string.GetSize() == 0) {
    cerr << "Duplicate Error: Must include from ID!" << endl;
    exit(1);
  }
  int batch_from = cur_string.PopWord().AsInt();

  int batch_to = cur_batch;
  if (cur_string.GetSize() > 0) batch_to = cur_string.PopWord().AsInt();

  if (verbose == true) {
    cout << "Duplicating from batch " << batch_from
	 << " to batch " << batch_to << "." << endl;
  }

  tListIterator<cAnalyzeGenotype> batch_from_it(batch[batch_from].List());
  cAnalyzeGenotype * genotype = NULL;
  while ((genotype = batch_from_it.Next()) != NULL) {
    cAnalyzeGenotype * new_genotype = new cAnalyzeGenotype(*genotype);
    batch[batch_to].List().PushRear(new_genotype);
  }

  batch[batch_to].SetLineage(false);
  batch[batch_to].SetAligned(false);
}

void cAnalyze::BatchRecalculate(cString cur_string)
{
  if (verbose == true) {
    cout << "Running batch " << cur_batch << " through test CPUs..." << endl;
  } else cout << "Running through test CPUs..." << endl;

  if (verbose == true && batch[cur_batch].IsLineage() == false) {
    cerr << "  Warning: batch may not be a linege; "
	 << "parent and ancestor distances may not be correct" << endl;
  }

  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
  cAnalyzeGenotype * genotype = NULL;
  cAnalyzeGenotype * last_genotype = NULL;
  while ((genotype = batch_it.Next()) != NULL) {
    if (last_genotype != NULL &&
	genotype->GetParentID() == last_genotype->GetID()) {
      genotype->Recalculate(last_genotype);
    }
    else genotype->Recalculate();
    last_genotype = genotype;
  }
}

void cAnalyze::BatchRename(cString cur_string)
{
  if (verbose == false) cout << "Renaming organisms..." << endl;
  else cout << "Renaming organisms in batch " << cur_batch << endl;

  // If a number is given with rename, start at that number...

  int id_num = cur_string.PopWord().AsInt();
  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
  cAnalyzeGenotype * genotype = NULL;
  while ((genotype = batch_it.Next()) != NULL) {
    cString name = cStringUtil::Stringf("org-%d", id_num);
    genotype->SetID(id_num);
    genotype->SetName(name);
    id_num++;
  }
}

void cAnalyze::PrintStatus(cString cur_string)
{
  cout << "Status Report:" << endl;
  for (int i = 0; i < MAX_BATCHES; i++) {
    if (i == cur_batch || batch[i].List().GetSize() > 0) {
      cout << "  Batch " << i << " -- "
	   << batch[i].List().GetSize() << " genotypes.";
      if (i == cur_batch) cout << "  <current>";
      if (batch[i].IsLineage() == true) cout << "  <lineage>";
      if (batch[i].IsAligned() == true) cout << "  <aligned>";

      cout << endl;
    }
  }
}

void cAnalyze::PrintDebug(cString cur_string)
{
  cerr << "Debug Args: " << cur_string << endl;
}

void cAnalyze::ToggleVerbose(cString cur_string)
{
  if (verbose == false) {
    cout << "Using verbose log messages..." << endl;
    verbose = true;
  } else {
    cout << "Using non-verbose log messages..." << endl;
    verbose = false;
  }
}

void cAnalyze::IncludeFile(cString cur_string)
{
  while (cur_string.GetSize() > 0) {
    cString filename = cur_string.PopWord();

    cInitFile include_file(filename);
    include_file.Load();
    include_file.Compress();
    include_file.Close();

    tList<cAnalyzeCommand> include_list;
    LoadCommandList(include_file, include_list);
    ProcessCommands(include_list);
  }
}

void cAnalyze::CommandSystem(cString cur_string)
{
  cout << "Running System Command: " << cur_string << endl;

  system(cur_string());
}

void cAnalyze::CommandInteractive(cString cur_string)
{
  cout << "Entering interactive mode..." << endl;

  char text_input[2048];
  while (true) {
    cout << ">> ";
    cout.flush();
    cin.getline(text_input, 2048);
    cString cur_input(text_input);
    cString command = cur_input.PopWord();
    command.ToUpper();

    cAnalyzeCommand * cur_command;
    cAnalyzeCommandDefBase * command_def = FindAnalyzeCommandDef(command);
    if (command == "") {
      // Don't worry about blank lines...
      continue;
    }
    else if (command == "END" || command == "QUIT" || command == "EXIT") {
      // We are done with interactive mode...
      break;
    }
    else if (command_def != NULL && command_def->IsFlowCommand() == true) {
      // This code has a body to it... fill it out!
      cur_command = new cAnalyzeFlowCommand(command, cur_input);
      InteractiveLoadCommandList(*(cur_command->GetCommandList()));
    }
    else {
      // This is a normal command...
      cur_command = new cAnalyzeCommand(command, cur_input);
    }

    cString args = cur_command->GetArgs();
    PreProcessArgs(args);

    cAnalyzeCommandDefBase * command_fun = FindAnalyzeCommandDef(command);

    // First check for built-in functions...
    if (command_fun != NULL) command_fun->Run(this, args, *cur_command);

    // Then for user defined functions
    else if (FunctionRun(command, args) == true) { }

    // Otherwise, give an error.
    else cerr << "Error: Unknown command '" << command << "'." << endl;
  }
}


void cAnalyze::FunctionCreate(cString cur_string,
			      tList<cAnalyzeCommand> & clist)
{
  int num_args = cur_string.CountNumWords();
  if (num_args < 1) {
    cerr << "Error: Must provide function name when creating function.";
    exit(1);
  }

  cString fun_name = cur_string.PopWord();
  fun_name.ToUpper();

  if (FindAnalyzeCommandDef(fun_name) != NULL) {
    cerr << "Error: Cannot create function '" << fun_name
	 << "'; already exists." << endl;
    exit(1);
  }

  if (verbose) cout << "Creating function: " << fun_name << endl;

  // Create the new function...
  cAnalyzeFunction * new_function = new cAnalyzeFunction(fun_name);
  while (clist.GetSize() > 0) {
    new_function->GetCommandList()->PushRear(clist.Pop());
  }

  // Save the function on the new list...
  function_list.PushRear(new_function);
}

bool cAnalyze::FunctionRun(const cString & fun_name, cString args)
{
  if (verbose) {
    cout << "Running function: " << fun_name << endl;
    // << " with args: " << args << endl;
  }

  // Find the function we're about to run...
  cAnalyzeFunction * found_function = NULL;
  tListIterator<cAnalyzeFunction> function_it(function_list);
  while (function_it.Next() != NULL) {
    if (function_it.Get()->GetName() == fun_name) {
      found_function = function_it.Get();
      break;
    }
  }

  // If we were unable to find the command we're looking for, return false.
  if (found_function == NULL) return false;

  // Back up the local variables
  cString backup_arg_vars[10];
  cString backup_local_vars[26];
  for (int i = 0; i < 10; i++) backup_arg_vars[i] = arg_variables[i];
  for (int i = 0; i < 26; i++) backup_local_vars[i] = local_variables[i];

  // Set the arg variables to the passed-in args...
  arg_variables[0] = fun_name;
  for (int i = 1; i < 10; i++) arg_variables[i] = args.PopWord();
  for (int i = 0; i < 26; i++) local_variables[i] = "";

  ProcessCommands(*(found_function->GetCommandList()));

  // Restore the local variables
  for (int i = 0; i < 10; i++) arg_variables[i] = backup_arg_vars[i];
  for (int i = 0; i < 26; i++) local_variables[i] = backup_local_vars[i];

  return true;
}


int cAnalyze::BatchUtil_GetMaxLength(int batch_id)
{
  if (batch_id < 0) batch_id = cur_batch;

  int max_length = 0;

  tListIterator<cAnalyzeGenotype> batch_it(batch[batch_id].List());
  cAnalyzeGenotype * genotype = NULL;
  while ((genotype = batch_it.Next()) != NULL) {
    if (genotype->GetLength() > max_length) max_length = genotype->GetLength();
  }
  
  return max_length;
}


void cAnalyze::CommandForeach(cString cur_string,
			      tList<cAnalyzeCommand> & clist)
{
  if (verbose) cout << "Initiating Foreach loop..." << endl;

  cString var = cur_string.PopWord();
  int num_args = cur_string.CountNumWords();

  cString & cur_variable = GetVariable(var);

  for (int i = 0; i < num_args; i++) {
    cur_variable = cur_string.PopWord();

    if (verbose == true) {
      cout << "Foreach: setting " << var << " to " << cur_variable << endl;
    }
    ProcessCommands(clist);
  }

  if (verbose == true) {
    cout << "Ending Foreach on " << var << endl;
  }
}


void cAnalyze::CommandForRange(cString cur_string,
			      tList<cAnalyzeCommand> & clist)
{
  if (verbose) cout << "Initiating FORRANGE loop..." << endl;

  int num_args = cur_string.CountNumWords();
  if (num_args < 3) {
    cerr << "  Error: Must give variable, min and max with FORRANGE!"
	 << endl;
    exit(1);
  }

  cString var = cur_string.PopWord();
  double min_val = cur_string.PopWord().AsDouble();
  double max_val = cur_string.PopWord().AsDouble();
  double step_val = 1.0;
  if (num_args >=4 ) step_val = cur_string.PopWord().AsDouble();

  cString & cur_variable = GetVariable(var);

  // Seperate out all ints from not all ints...
  if (min_val == (double) ((int) min_val) &&
      max_val == (double) ((int) max_val) &&
      step_val == (double) ((int) step_val)) {
    for (int i = (int) min_val; i <= (int) max_val; i += (int) step_val) {
      cur_variable.Set("%d", i);

      if (verbose == true) {
	cout << "FORRANGE: setting " << var << " to " << cur_variable << endl;
      }
      ProcessCommands(clist);
    }
  } else {
    for (double i = min_val; i <= max_val; i += step_val) {
      cur_variable.Set("%f", i);

      if (verbose == true) {
	cout << "FORRANGE: setting " << var << " to " << cur_variable << endl;
      }
      ProcessCommands(clist);
    }
  }

  if (verbose == true) {
    cout << "Ending FORRANGE on " << var << endl;
  }
}


///////////////////  Private Methods ///////////////////////////

cString cAnalyze::PopDirectory(cString & in_string,
			       const cString & default_dir)
{
  // Determing the directory name
  cString directory(default_dir);
  if (in_string.GetSize() != 0) directory = in_string.PopWord();

  // Make sure the directory ends in a slash.  If not, add one.
  int last_pos = directory.GetSize() - 1;
  if (directory[last_pos] != '/' && directory[last_pos] != '\\') {
    directory += '/';
  }

  // Make sure the directory exists.
  FILE *fp = fopen ( directory(), "r" );
  if ( fp == 0 ){
    if ( errno == ENOENT ){
      cerr << "Directory '" << directory
	   << "' does not exist.  Creating..." << endl;
      if ( mkdir( directory(), (S_IRWXU|S_IRWXG|S_IRWXO) ) )
	cerr << " Error creating '" << directory << "'" << endl;
    }
    else cerr << " Error opening '" << directory << "'" << endl;
  }

  return directory;
}

int cAnalyze::PopBatch(const cString & in_string)
{
  int batch = cur_batch;
  if (in_string.GetSize() != 0 && in_string != "current") {
    batch = in_string.AsInt();
  }

  return batch;
}

cAnalyzeGenotype * cAnalyze::PopGenotype(cString gen_desc, int batch_id)
{
  if (batch_id == -1) batch_id = cur_batch;
  tList<cAnalyzeGenotype> & gen_list = batch[batch_id].List();
  gen_desc.ToLower();

  cAnalyzeGenotype * found_gen = NULL;
  if (gen_desc == "num_cpus")
    found_gen = gen_list.PopIntMax(&cAnalyzeGenotype::GetNumCPUs);
  else if (gen_desc == "total_cpus")
    found_gen = gen_list.PopIntMax(&cAnalyzeGenotype::GetTotalCPUs);
  else if (gen_desc == "merit")
    found_gen = gen_list.PopDoubleMax(&cAnalyzeGenotype::GetMerit);
  else if (gen_desc == "fitness")
    found_gen = gen_list.PopDoubleMax(&cAnalyzeGenotype::GetFitness);
  else if (gen_desc.IsNumeric(0))
    found_gen = gen_list.PopIntValue(&cAnalyzeGenotype::GetID,
				     gen_desc.AsInt());
  else {
    cout << "  Error: unknown type " << gen_desc << endl;
    exit(1);
  }

  return found_gen;
}


cString & cAnalyze::GetVariable(const cString & var)
{
  if (var.GetSize() != 1 ||
      (var.IsLetter(0) == false && var.IsNumeric(0) == false)) {
    cerr << "Error: Illegal variable " << var << " being used." << endl;
    exit(1);
  }

  if (var.IsLowerLetter(0) == true) {
    int var_id = (int) (var[0] - 'a');
    return variables[var_id];
  }
  else if (var.IsUpperLetter(0) == true) {
    int var_id = (int) (var[0] - 'A');
    return local_variables[var_id];
  }
  // Otherwise it must be a number...
  int var_id = (int) (var[0] - '0');
  return arg_variables[var_id];
}


void cAnalyze::LoadCommandList(cInitFile & init_file,
			       tList<cAnalyzeCommand> & clist)
{
  while (init_file.GetNumLines() != 0) {
    cString cur_string = init_file.RemoveLine();
    cString command = cur_string.PopWord();
    command.ToUpper();

    cAnalyzeCommand * cur_command;
    cAnalyzeCommandDefBase * command_def = FindAnalyzeCommandDef(command);
    if (command == "END") {
      // We are done with this section of code; break out...
      break;
    }
    else if (command_def != NULL && command_def->IsFlowCommand() == true) {
      // This code has a body to it... fill it out!
      cur_command = new cAnalyzeFlowCommand(command, cur_string);
      LoadCommandList( init_file, *(cur_command->GetCommandList()) );
    }
    else {
      // This is a normal command...
      cur_command = new cAnalyzeCommand(command, cur_string);
    }

    clist.PushRear(cur_command);
  }
}

void cAnalyze::InteractiveLoadCommandList(tList<cAnalyzeCommand> & clist)
{
  interactive_depth++;
  char text_input[2048];
  while (true) {
    for (int i = 0; i <= interactive_depth; i++) {
      cout << ">>";
    }
    cout << " ";
    cout.flush();
    cin.getline(text_input, 2048);
    cString cur_input(text_input);
    cString command = cur_input.PopWord();
    command.ToUpper();

    cAnalyzeCommand * cur_command;
    cAnalyzeCommandDefBase * command_def = FindAnalyzeCommandDef(command);
    if (command == "END") {
      // We are done with this section of code; break out...
      break;
    }
    else if (command_def != NULL && command_def->IsFlowCommand() == true) {
      // This code has a body to it... fill it out!
      cur_command = new cAnalyzeFlowCommand(command, cur_input);
      InteractiveLoadCommandList(*(cur_command->GetCommandList()));
    }
    else {
      // This is a normal command...
      cur_command = new cAnalyzeCommand(command, cur_input);
    }

    clist.PushRear(cur_command);
  }
  interactive_depth--;
}

void cAnalyze::PreProcessArgs(cString & args)
{
  int pos = 0;
  int search_start = 0;
  while ((pos = args.Find('$', search_start)) != -1) {
    // Setup the variable name that was found...
    char varlet = args[pos+1];
    cString varname("$");
    varname += varlet;

    // Determine the variable and act on it.
    int varsize = 0;
    if (varlet == '$') {
      args.Clip(pos+1, 1);
      varsize = 1;
    }
    else if (varlet >= 'a' && varlet <= 'z') {
      int var_id = (int) (varlet - 'a');
      args.Replace(varname, variables[var_id], pos);
      varsize = variables[var_id].GetSize();
    }
    else if (varlet >= 'A' && varlet <= 'Z') {
      int var_id = (int) (varlet - 'A');
      args.Replace(varname, local_variables[var_id], pos);
      varsize = local_variables[var_id].GetSize();
    }
    else if (varlet >= '0' && varlet <= '9') {
      int var_id = (int) (varlet - '0');
      args.Replace(varname, arg_variables[var_id], pos);
      varsize = arg_variables[var_id].GetSize();
    }
    search_start = pos + varsize;
  }
}

void cAnalyze::ProcessCommands(tList<cAnalyzeCommand> & clist)
{
  // Process the command list...
  tListIterator<cAnalyzeCommand> command_it(clist);
  command_it.Reset();
  cAnalyzeCommand * cur_command = NULL;
  while ((cur_command = command_it.Next()) != NULL) {
    cString command = cur_command->GetCommand();
    cString args = cur_command->GetArgs();
    PreProcessArgs(args);

    cAnalyzeCommandDefBase * command_fun = FindAnalyzeCommandDef(command);
    if (command_fun != NULL) command_fun->Run(this, args, *cur_command);
    else if (FunctionRun(command, args) == true) {
      // Found a defined function by this name.
    }
    else {
      cerr << "Error: Unknown analysis keyword '" << command << "'." << endl;
      exit(1);
    }

  }
}

void cAnalyze::SetupGenotypeDataList()
{
  if (genotype_data_list.GetSize() != 0) return; // List already setup.

  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, bool>
       ("viable",      "Is Viable (0/1)", &cAnalyzeGenotype::GetViable,
	&cAnalyzeGenotype::SetViable));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("id",          "Genome ID",       &cAnalyzeGenotype::GetID,
	&cAnalyzeGenotype::SetID));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, const cString &>
       ("tag",         "Genotype Tag",    &cAnalyzeGenotype::GetTag,
	&cAnalyzeGenotype::SetTag,
	&cAnalyzeGenotype::CompareNULL, "(none)", ""));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("parent_id",   "Parent ID",       &cAnalyzeGenotype::GetParentID,
	&cAnalyzeGenotype::SetParentID));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("parent_dist", "Parent Distance", &cAnalyzeGenotype::GetParentDist,
	&cAnalyzeGenotype::SetParentDist));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("ancestor_dist","Ancestor Distance",&cAnalyzeGenotype::GetAncestorDist,
	&cAnalyzeGenotype::SetAncestorDist));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("num_cpus",    "Number of CPUs",  &cAnalyzeGenotype::GetNumCPUs,
	&cAnalyzeGenotype::SetNumCPUs));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("total_cpus",  "Total CPUs Ever", &cAnalyzeGenotype::GetTotalCPUs,
	&cAnalyzeGenotype::SetTotalCPUs));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("length",      "Genome Length",   &cAnalyzeGenotype::GetLength,
	&cAnalyzeGenotype::SetLength, &cAnalyzeGenotype::CompareLength));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("copy_length", "Copied Length",   &cAnalyzeGenotype::GetCopyLength,
	&cAnalyzeGenotype::SetCopyLength));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("exe_length",  "Executed Length", &cAnalyzeGenotype::GetExeLength,
	&cAnalyzeGenotype::SetExeLength));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, double>
       ("merit",       "Merit",           &cAnalyzeGenotype::GetMerit,
	&cAnalyzeGenotype::SetMerit, &cAnalyzeGenotype::CompareMerit));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, double>
       ("comp_merit",  "Computational Merit", &cAnalyzeGenotype::GetCompMerit,
	(void (cAnalyzeGenotype::*)(double)) NULL, &cAnalyzeGenotype::CompareCompMerit));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, double>
       ("comp_merit_ratio", "Computational Merit Ratio",
	&cAnalyzeGenotype::GetCompMeritRatio,
	(void (cAnalyzeGenotype::*)(double)) NULL,
	&cAnalyzeGenotype::CompareCompMerit));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("gest_time",   "Gestation Time",  &cAnalyzeGenotype::GetGestTime,
	&cAnalyzeGenotype::SetGestTime,
	&cAnalyzeGenotype::CompareGestTime, "Inf."));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, double>
       ("efficiency",  "Rep. Efficiency", &cAnalyzeGenotype::GetEfficiency,
	(void (cAnalyzeGenotype::*)(double)) NULL,
	&cAnalyzeGenotype::CompareEfficiency));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, double>
       ("efficiency_ratio", "Rep. Efficiency Ratio",
	&cAnalyzeGenotype::GetEfficiencyRatio,
	(void (cAnalyzeGenotype::*)(double)) NULL,
	&cAnalyzeGenotype::CompareEfficiency));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, double>
       ("fitness",     "Fitness",         &cAnalyzeGenotype::GetFitness,
	&cAnalyzeGenotype::SetFitness, &cAnalyzeGenotype::CompareFitness));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, double>
       ("div_type",     "Divide Type",         &cAnalyzeGenotype::GetDivType,
	&cAnalyzeGenotype::SetDivType));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, double>
       ("fitness_ratio", "Fitness Ratio", &cAnalyzeGenotype::GetFitnessRatio,
	(void (cAnalyzeGenotype::*)(double)) NULL,
	&cAnalyzeGenotype::CompareFitness));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("update_born", "Update Born",     &cAnalyzeGenotype::GetUpdateBorn,
	&cAnalyzeGenotype::SetUpdateBorn));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("update_dead", "Update Dead",     &cAnalyzeGenotype::GetUpdateDead,
	&cAnalyzeGenotype::SetUpdateDead));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("depth",       "Tree Depth",      &cAnalyzeGenotype::GetDepth,
	&cAnalyzeGenotype::SetDepth));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, double>
       ("frac_dead",   "Fraction Mutations Lethal",
	&cAnalyzeGenotype::GetFracDead,
	(void (cAnalyzeGenotype::*)(double)) NULL));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, double>
       ("frac_neg",   "Fraction Mutations Detrimental",
	&cAnalyzeGenotype::GetFracNeg,
	(void (cAnalyzeGenotype::*)(double)) NULL));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, double>
       ("frac_neut",   "Fraction Mutations Neutral",
	&cAnalyzeGenotype::GetFracNeut,
	(void (cAnalyzeGenotype::*)(double)) NULL));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, double>
       ("frac_pos",   "Fraction Mutations Beneficial",
	&cAnalyzeGenotype::GetFracPos,
	(void (cAnalyzeGenotype::*)(double)) NULL));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, const cString &>
       ("parent_muts", "Mutations from Parent",
	&cAnalyzeGenotype::GetParentMuts, &cAnalyzeGenotype::SetParentMuts,
	&cAnalyzeGenotype::CompareNULL, "(none)", ""));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, const cString &>
       ("task_order", "Task Performance Order",
	&cAnalyzeGenotype::GetTaskOrder, &cAnalyzeGenotype::SetTaskOrder,
	&cAnalyzeGenotype::CompareNULL, "(none)", ""));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, cString>
       ("sequence",    "Genome Sequence",
	&cAnalyzeGenotype::GetSequence, &cAnalyzeGenotype::SetSequence, 
	&cAnalyzeGenotype::CompareNULL, "(N/A)", ""));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, const cString &>
       ("alignment",   "Aligned Sequence",
	&cAnalyzeGenotype::GetAlignedSequence,
	&cAnalyzeGenotype::SetAlignedSequence,
	&cAnalyzeGenotype::CompareNULL, "(N/A)", ""));

  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, cString>
       ("task_list",    "List of all tasks performed",
	&cAnalyzeGenotype::GetTaskList,
	(void (cAnalyzeGenotype::*)(cString)) NULL,
	&cAnalyzeGenotype::CompareNULL, "(N/A)", ""));

  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, cString>
       ("link.tasksites", "Phenotype Map", &cAnalyzeGenotype::GetMapLink,
	(void (cAnalyzeGenotype::*)(cString)) NULL));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, cString>
       ("html.sequence",  "Genome Sequence",
	&cAnalyzeGenotype::GetHTMLSequence,
	(void (cAnalyzeGenotype::*)(cString)) NULL,
	&cAnalyzeGenotype::CompareNULL, "(N/A)", ""));

  const cTaskLib & task_lib = cTestCPU::GetEnvironment()->GetTaskLib();
  for (int i = 0; i < task_lib.GetSize(); i++) {
    cString t_name, t_desc;
    t_name.Set("task.%d", i);
    t_desc = task_lib.GetTask(i).GetDesc();
    genotype_data_list.PushRear(new tArgDataEntry<cAnalyzeGenotype, int, int>
	(t_name, t_desc, &cAnalyzeGenotype::GetTaskCount, i,
	 &cAnalyzeGenotype::CompareTaskCount));
  }

  // The remaining values should actually go in a seperate list called
  // "population_data_list", but for the moment we're going to put them
  // here so that we only need to worry about a single system to load and
  // save genotype information.
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("update",       "Update Output",
	&cAnalyzeGenotype::GetUpdateDead, &cAnalyzeGenotype::SetUpdateDead));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("dom_num_cpus", "Number of Dominant Organisms",
	&cAnalyzeGenotype::GetNumCPUs, &cAnalyzeGenotype::SetNumCPUs));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("dom_depth",    "Tree Depth of Dominant Genotype",
	&cAnalyzeGenotype::GetDepth, &cAnalyzeGenotype::SetDepth));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("dom_id",       "Dominant Genotype ID",
	&cAnalyzeGenotype::GetID, &cAnalyzeGenotype::SetID));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, cString>
       ("dom_sequence", "Dominant Genotype Sequence",
	&cAnalyzeGenotype::GetSequence, &cAnalyzeGenotype::SetSequence, 
	&cAnalyzeGenotype::CompareNULL, "(N/A)", ""));
}


// Pass in the arguments for a command and fill out the entries in list
// format....

void cAnalyze::LoadGenotypeDataList(cStringList arg_list,
       tList< tDataEntryCommand<cAnalyzeGenotype> > & output_list)
{
  // For the moment, just put everything into the output list.
  SetupGenotypeDataList();

  // If no args were given, load all of the stats.
  if (arg_list.GetSize() == 0) {
    tListIterator< tDataEntryBase<cAnalyzeGenotype> >
      genotype_data_it(genotype_data_list);
    while (genotype_data_it.Next() != (void *) NULL) {
      tDataEntryCommand<cAnalyzeGenotype> * entry_command =
	new tDataEntryCommand<cAnalyzeGenotype>(genotype_data_it.Get());
      output_list.PushRear(entry_command);
    }
  }
  // Otherwise, load only those listed.
  else {
    while (arg_list.GetSize() != 0) {
      // Setup the next entry
      cString cur_args = arg_list.Pop();
      cString cur_entry = cur_args.Pop(':');
      bool found_entry = false;

      // Scan the genotype data list for the current entry
      tListIterator< tDataEntryBase<cAnalyzeGenotype> >
	genotype_data_it(genotype_data_list);

      while (genotype_data_it.Next() != (void *) NULL) {
	if (genotype_data_it.Get()->GetName() == cur_entry) {
	  tDataEntryCommand<cAnalyzeGenotype> * entry_command =
	    new tDataEntryCommand<cAnalyzeGenotype>
	    (genotype_data_it.Get(), cur_args);
	  output_list.PushRear(entry_command);
	  found_entry = true;
	  break;
	}
      }

      // If the entry was not found, give a warning.
      if (found_entry == false) {
	int best_match = 1000;
	cString best_entry;

	genotype_data_it.Reset();
	while (genotype_data_it.Next() != (void *) NULL) {
	  const cString & test_str = genotype_data_it.Get()->GetName();
	  const int test_dist = cStringUtil::EditDistance(test_str, cur_entry);
	  if (test_dist < best_match) {
	    best_match = test_dist;
	    best_entry = test_str;
	  }
	}	

	cerr << "Warning: Format entry \"" << cur_entry
	     << "\" not found.  Best match is \""
	     << best_entry << "\"." << endl;
      }

    }
  }
}


void cAnalyze::AddLibraryDef(const cString & name,
			     void (cAnalyze::*_fun)(cString))
{
  command_lib.PushRear(new cAnalyzeCommandDef(name, _fun));
}

void cAnalyze::AddLibraryDef(const cString & name,
	  void (cAnalyze::*_fun)(cString, tList<cAnalyzeCommand> &))
{
  command_lib.PushRear(new cAnalyzeFlowCommandDef(name, _fun));
}

void cAnalyze::SetupCommandDefLibrary()
{
  if (command_lib.GetSize() != 0) return; // Library already setup.

  AddLibraryDef("LOAD_ORGANISM", &cAnalyze::LoadOrganism);
  AddLibraryDef("LOAD_BASE_DUMP", &cAnalyze::LoadBasicDump);
  AddLibraryDef("LOAD_DETAIL_DUMP", &cAnalyze::LoadDetailDump);
  AddLibraryDef("LOAD_SEQUENCE", &cAnalyze::LoadSequence);
  AddLibraryDef("LOAD_DOMINANT", &cAnalyze::LoadDominant);
  AddLibraryDef("LOAD", &cAnalyze::LoadFile);

    // Reduction commands...
  AddLibraryDef("FIND_GENOTYPE", &cAnalyze::FindGenotype);
  AddLibraryDef("FIND_LINEAGE", &cAnalyze::FindLineage);
  AddLibraryDef("FIND_CLADE", &cAnalyze::FindClade);
  AddLibraryDef("SAMPLE_ORGANISMS", &cAnalyze::SampleOrganisms);
  AddLibraryDef("SAMPLE_GENOTYPES", &cAnalyze::SampleGenotypes);
  AddLibraryDef("KEEP_TOP", &cAnalyze::KeepTopGenotypes);

  // Direct output commands...
  AddLibraryDef("PRINT", &cAnalyze::CommandPrint);
  AddLibraryDef("TRACE", &cAnalyze::CommandTrace);
  AddLibraryDef("PRINT_TASKS", &cAnalyze::CommandPrintTasks);
  AddLibraryDef("DETAIL", &cAnalyze::CommandDetail);
  AddLibraryDef("DETAIL_BATCHES", &cAnalyze::CommandDetailBatches);
  AddLibraryDef("DETAIL_INDEX", &cAnalyze::CommandDetailIndex);

  // Population analysis commands...
  AddLibraryDef("PRINT_PHENOTYPES", &cAnalyze::CommandPrintPhenotypes);
  AddLibraryDef("PRINT_DIVERSITY", &cAnalyze::CommandPrintDiversity);

  // Individual organism analysis...
  AddLibraryDef("LANDSCAPE", &cAnalyze::CommandLandscape);
  AddLibraryDef("FITNESS_MATRIX", &cAnalyze::CommandFitnessMatrix);
  AddLibraryDef("MAP", &cAnalyze::CommandMapTasks);  // Deprecated...
  AddLibraryDef("MAP_TASKS", &cAnalyze::CommandMapTasks);
  AddLibraryDef("MAP_MUTATIONS", &cAnalyze::CommandMapMutations);

  // Population comparison commands...
  AddLibraryDef("HAMMING", &cAnalyze::CommandHamming);
  AddLibraryDef("LEVENSTEIN", &cAnalyze::CommandLevenstein);
  AddLibraryDef("SPECIES", &cAnalyze::CommandSpecies);

  // Lineage analysis commands...
  AddLibraryDef("ALIGN", &cAnalyze::CommandAlign);

  // Build input files for avida...
  AddLibraryDef("WRITE_CLONE", &cAnalyze::WriteClone);
  AddLibraryDef("WRITE_INJECT_EVENTS", &cAnalyze::WriteInjectEvents);

  // Automated analysis
  AddLibraryDef("ANALYZE_MUTS", &cAnalyze::AnalyzeMuts);
  AddLibraryDef("ANALYZE_INSTRUCTIONS", &cAnalyze::AnalyzeInstructions);
  AddLibraryDef("ANALYZE_BRANCHING", &cAnalyze::AnalyzeBranching);
  AddLibraryDef("ANALYZE_MUTATION_TRACEBACK",
		&cAnalyze::AnalyzeMutationTraceback);

  // Documantation...
  AddLibraryDef("HELPFILE", &cAnalyze::CommandHelpfile);

  // Control commands...
  AddLibraryDef("SET", &cAnalyze::VarSet);
  AddLibraryDef("SET_BATCH", &cAnalyze::BatchSet);
  AddLibraryDef("NAME_BATCH", &cAnalyze::BatchName);
  AddLibraryDef("TAG_BATCH", &cAnalyze::BatchTag);
  AddLibraryDef("PURGE_BATCH", &cAnalyze::BatchPurge);
  AddLibraryDef("DUPLICATE", &cAnalyze::BatchDuplicate);
  AddLibraryDef("RECALCULATE", &cAnalyze::BatchRecalculate);
  AddLibraryDef("RENAME", &cAnalyze::BatchRename);
  AddLibraryDef("STATUS", &cAnalyze::PrintStatus);
  AddLibraryDef("DEBUG", &cAnalyze::PrintDebug);
  AddLibraryDef("VERBOSE", &cAnalyze::ToggleVerbose);
  AddLibraryDef("INCLUDE", &cAnalyze::IncludeFile);
  AddLibraryDef("SYSTEM", &cAnalyze::CommandSystem);
  AddLibraryDef("INTERACTIVE", &cAnalyze::CommandInteractive);

    // Functions...
  AddLibraryDef("FUNCTION", &cAnalyze::FunctionCreate);

    // Flow commands...
  AddLibraryDef("FOREACH", &cAnalyze::CommandForeach);
  AddLibraryDef("FORRANGE", &cAnalyze::CommandForRange);
}

cAnalyzeCommandDefBase * cAnalyze::FindAnalyzeCommandDef(const cString & name)
{
  SetupCommandDefLibrary();

  tListIterator<cAnalyzeCommandDefBase> lib_it(command_lib);
  while (lib_it.Next() != (void *) NULL) {
    if (lib_it.Get()->GetName() == name) break;
  }

  return lib_it.Get();
}

