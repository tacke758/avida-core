#include <iomanip.h>

#include "analyze.hh"

#include "fitness_matrix.hh"


//////////////////////
//  cAnalyzeGenotype
//////////////////////

cAnalyzeGenotype::cAnalyzeGenotype(cString symbol_string)
  : code(symbol_string), name(""), aligned_sequence(""), viable(false),
    id_num(-1),
    parent_id(-1),
    num_cpus(0),
    total_cpus(0),
    update_born(0),
    update_dead(0),
    depth(0),
    length(0),
    copy_length(0),
    exe_length(0),
    merit(0.0),
    gest_time(INT_MAX),
    fitness(0.0),
    errors(0),
    num_tasks(0),
    task_counts(NULL),
    fitness_ratio(0.0),
    efficiency_ratio(0.0),
    comp_merit_ratio(0.0),
    parent_dist(0),
    parent_muts("")
{
}

cAnalyzeGenotype::cAnalyzeGenotype(const cCodeArray & _code)
  : code(_code), name(""), aligned_sequence(""), viable(false),
    id_num(-1),
    parent_id(-1),
    num_cpus(0),
    total_cpus(0),
    update_born(0),
    update_dead(0),
    depth(0),
    length(0),
    copy_length(0),
    exe_length(0),
    merit(0.0),
    gest_time(INT_MAX),
    fitness(0.0),
    errors(0),
    num_tasks(0),
    task_counts(NULL),
    fitness_ratio(0.0),
    efficiency_ratio(0.0),
    comp_merit_ratio(0.0),
    parent_dist(0),
    parent_muts("")
{
}

cAnalyzeGenotype::cAnalyzeGenotype(const cAnalyzeGenotype & _gen)
  : code(_gen.code), name(_gen.name), aligned_sequence(_gen.aligned_sequence),
    viable(_gen.viable),
    id_num(_gen.id_num),
    parent_id(_gen.parent_id),
    num_cpus(_gen.num_cpus),
    total_cpus(_gen.total_cpus),
    update_born(_gen.update_born),
    update_dead(_gen.update_dead),
    depth(_gen.depth),
    length(_gen.length),
    copy_length(_gen.copy_length),
    exe_length(_gen.exe_length),
    merit(_gen.merit),
    gest_time(_gen.gest_time),
    fitness(_gen.fitness),
    errors(_gen.errors),
    num_tasks(_gen.num_tasks),
    task_counts(NULL),
    fitness_ratio(_gen.fitness_ratio),
    efficiency_ratio(_gen.efficiency_ratio),
    comp_merit_ratio(_gen.comp_merit_ratio),
    parent_dist(_gen.parent_dist),
    parent_muts(_gen.parent_muts)
{
  if (_gen.task_counts != NULL) {
    task_counts = new int[num_tasks];
    for (int i = 0; i < num_tasks; i++) {
      task_counts[i] = _gen.task_counts[i];
    }
  }
}

cAnalyzeGenotype::~cAnalyzeGenotype()
{
  if (task_counts != NULL) delete [] task_counts;
}

void cAnalyzeGenotype::Recalculate(cAnalyzeGenotype * parent_genotype)
{
    // Build the test info for printing.
  cCPUTestInfo test_info;
  test_info.TestThreads();
  // test_info.TraceTaskOrder();
  cTestCPU::TestCode(test_info, code);

  viable = test_info.IsViable();

  length = test_info.GetGenotypeCodeSize();
  copy_length = test_info.GetGenotypeCopiedSize();
  exe_length = test_info.GetGenotypeExeSize();
  merit = test_info.GetGenotypeMerit();
  gest_time = test_info.GetGenotypeGestation();
  fitness = test_info.GetGenotypeFitness();

  errors = test_info.GetGenotypeNumErrors();

  if (task_counts != NULL) delete [] task_counts;
  cPhenotype * phenotype = cTestCPU::GetPhenotype();
  num_tasks = phenotype->GetNumTasks();
  task_counts = new int[num_tasks];

  for(int i=0; i < num_tasks; i++ ){
    task_counts[i] = phenotype->GetTaskCount(i);
  }

  task_order = test_info.GetGenotypeTaskOrder();

  // Setup a new parent stats if we have a parent to work with.
  if (parent_genotype != NULL) {
    fitness_ratio = GetFitness() / parent_genotype->GetFitness();
    efficiency_ratio = GetEfficiency() / parent_genotype->GetEfficiency();
    comp_merit_ratio = GetCompMerit() / parent_genotype->GetCompMerit();
    parent_dist =
      code.AsString().LDistance(parent_genotype->GetCode().AsString(),
				parent_muts);
  }
}


void cAnalyzeGenotype::PrintTasks(ofstream & fp, int min_task, int max_task)
{
  if (max_task == -1) max_task = num_tasks;

  for (int i = min_task; i < max_task; i++) {
    fp << task_counts[i] << " ";
  }
}


cString cAnalyzeGenotype::GetHTMLSequence() const
{
  cString text_code = code.AsString();
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
  for (int i = 0; i < code.GetSize(); i++) {
    char symbol = text_code[i];
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

cAnalyze::cAnalyze(cString filename) : cur_batch(0)
{
  inst_lib = cInstLib::InitInstructions(cConfig::GetInstFilename());
  random.ResetSeed(cConfig::GetRandSeed());
  verbose = false;

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

  cInitFile input_file(filename);
  if (!input_file.IsOpen()) {
    cerr << "Error: Cannot load file: \"" << filename << "\"." << endl;
    exit(1);
  }
  input_file.Load();
  input_file.Compress();
  input_file.Close();

  // Setup the code array...
  cCodeArray code(input_file.GetNumLines());

  for (int line_num = 0; line_num < code.GetSize(); line_num++) {
    cString cur_line = input_file.RemoveLine();
    code[line_num] = inst_lib->GetInst(cur_line);

    if (code[line_num] == cInstruction::GetInstError()) {
      // You're using the wrong instruction set!  YOU FOOL!
      cString error_str("Cannot load creature \"");
      error_str += filename;
      error_str += "\"\n       Unknown line: ";
      error_str += input_file.GetLine()();
      error_str += "       ";

      cerr << error_str << endl;
    }
  }

  // Construct the new genotype..
  cAnalyzeGenotype * genotype = new cAnalyzeGenotype(code);

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

  // Setup the code array...

  while (input_file.GetNumLines() > 0) {
    cString cur_line = input_file.RemoveLine();

    // Setup the genotype for this line...
    cAnalyzeGenotype * genotype = new cAnalyzeGenotype(cur_line.PopWord());
    int num_cpus = cur_line.PopWord().AsInt();
    int id_num = cur_line.PopWord().AsInt();
    cString name = cString::Stringf("org-%d", id_num);

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

  // Setup the code array...

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
    cString name = cString::Stringf("org-%d", id_num);

    cAnalyzeGenotype * genotype = new cAnalyzeGenotype(cur_line.PopWord());

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

  cout << "Loading: " << sequence << endl;

  // Setup the genotype...
  cAnalyzeGenotype * genotype = new cAnalyzeGenotype(sequence);
  cString name = cString::Stringf("org-S%d", sequence_count++);
  genotype->SetName(name);

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



//////////////// Commands...

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
    cTestCPU::PrintCode(genotype->GetCode(), filename);
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
    cTestCPU::TraceCode(genotype->GetCode(), filename);
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

  ofstream fp;
  fp.open(filename);

  // Loop through all of the genotypes in this batch...
  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
  cAnalyzeGenotype * genotype = NULL;
  while ((genotype = batch_it.Next()) != NULL) {
    fp << genotype->GetID() << " ";
    genotype->PrintTasks(fp);
    fp << endl;
  }
}

void cAnalyze::CommandRename(cString cur_string)
{
  if (verbose == false) cout << "Renaming organisms..." << endl;
  else cout << "Renaming organisms in batch " << cur_batch << endl;

  // If a number is given with rename, start at that number...

  int id_num = cur_string.PopWord().AsInt();
  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
  cAnalyzeGenotype * genotype = NULL;
  while ((genotype = batch_it.Next()) != NULL) {
    cString name = cString::Stringf("org-%d", id_num);
    genotype->SetID(id_num);
    genotype->SetName(name);
    id_num++;
  }
}

void cAnalyze::CommandLandscape(cString cur_string)
{
  if (verbose == true) cout << "Landscaping batch " << cur_batch << endl;
  else cout << "Landscapping..." << endl;

  // Load in the variables...
  cString filename("landscape.dat");
  if (cur_string.GetSize() != 0) filename = cur_string.PopWord();
  int dist = 1;
  if (cur_string.GetSize() != 0) dist = cur_string.PopWord().AsInt();

  ofstream fp;
  fp.open(filename);

  // Loop through all of the genotypes in this batch...
  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
  cAnalyzeGenotype * genotype = NULL;
  while ((genotype = batch_it.Next()) != NULL) {
    cLandscape landscape(genotype->GetCode(), inst_lib);
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

  cFitnessMatrix matrix( genotype->GetCode(), inst_lib);

  matrix.CalcFitnessMatrix( depth_limit, fitness_threshold_ratio, ham_thresh, error_rate_min, error_rate_max, error_rate_step, vect_fmax, vect_fstep, diag_iters, write_ham_vector, write_full_vector );
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

  // Setup the file...
  ofstream fp(filename);

  // Determine the file type...
  int file_type = FILE_TYPE_TEXT;
  while (filename.Find('.') != -1) filename.Pop('.');
  if (filename == "html") file_type = FILE_TYPE_HTML;

  // Write out the header on the file
  if (file_type == FILE_TYPE_TEXT) {
    fp << "#filetype <genotype_data>" << endl;
    fp << "#format ";
    while (output_it.Next() != NULL) {
      const cString & entry_name = output_it.Get()->GetName();
      fp << "<" << entry_name << "> ";
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
  } else { // if (file_type == FILE_TYPE_HTML) {
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

  // Loop through all of the genotypes in this batch...
  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
  cAnalyzeGenotype * genotype = NULL;
  cAnalyzeGenotype * last_genotype = NULL;
  while ((genotype = batch_it.Next()) != NULL) {
    output_it.Reset();
    tDataEntryCommand<cAnalyzeGenotype> * data_command = NULL;
    if (file_type == FILE_TYPE_HTML) fp << "<tr>";
    while ((data_command = output_it.Next()) != NULL) {
      data_command->SetTarget(genotype);
      genotype->SetSpecialArgs(data_command->GetArgs());
      if (file_type == FILE_TYPE_HTML) {
	int compare = 0;
	if (last_genotype) {
	  last_genotype->SetSpecialArgs(data_command->GetArgs());
	  compare = data_command->Compare(last_genotype);
	}
	data_command->HTMLPrint(fp, compare);
      }
      else {  // if (file_type == FILE_TYPE_TEXT) {
	fp << data_command->GetEntry() << " ";
      }
    }
    if (file_type == FILE_TYPE_HTML) fp << "</tr>";
    fp << endl;
    last_genotype = genotype;
  }

  // If in HTML mode, we need to end the file...
  if (file_type == FILE_TYPE_HTML) {
    fp << "</table>" << endl
       << "</center>" << endl;
  }

  // And clean up...
  while (output_list.GetSize() != 0) delete output_list.Pop();
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
	  const int test_dist = test_str.LDistance(cur_entry);
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
    fp << "#filetype <genotype_data>" << endl;
    fp << "#format ";
    while (output_it.Next() != NULL) {
      const cString & entry_name = output_it.Get()->GetName();
      fp << "<" << entry_name << "> ";
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


void cAnalyze::CommandMap(cString cur_string)
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
      cAnalyzeGenotype null_genotype("a");
      while ((data_command = output_it.Next()) != NULL) {
	data_command->SetTarget(genotype);
	if (data_command->Compare(&null_genotype) > 0) {
	  fp << "<th bgcolor=\"#00FF00\">";
	}
	else  fp << "<th bgcolor=\"#FF0000\">";
	fp << data_command->GetEntry() << " ";
      }
      fp << "</tr>" << endl;
    }


    const int max_line = genotype->GetLength();
    const cCodeArray & base_code = genotype->GetCode();
    cCodeArray mod_code(base_code);

    // Keep track of the number of failues/successes for attributes...
    int * col_pass_count = new int[num_cols];
    int * col_fail_count = new int[num_cols];
    for (int i = 0; i < num_cols; i++) {
      col_pass_count[i] = 0;
      col_fail_count[i] = 0;
    }

    // Loop through all the lines of code, testing the removal of each.
    for (int line_num = 0; line_num < max_line; line_num++) {
      int cur_inst = base_code.Get(line_num).GetOp();
      char cur_symbol = base_code.Get(line_num).GetSymbol();

      mod_code[line_num].SetOp(INSTRUCTION_NONE);
      cAnalyzeGenotype test_genotype(mod_code);
      test_genotype.Recalculate();

      if (file_type == FILE_TYPE_HTML) fp << "<tr><td align=right>";
      fp << (line_num + 1) << " ";
      if (file_type == FILE_TYPE_HTML) fp << "<td align=center>";
      fp << cur_symbol << " ";
      if (file_type == FILE_TYPE_HTML) fp << "<td align=center>";
      if (link_insts == true) {
	fp << "<a href=\"javascript:Inst('"
	   << inst_lib->GetName(cur_inst)
	   << "')\">";
      }
      fp << inst_lib->GetName(cur_inst) << " ";
      if (link_insts == true) fp << "</a>";


      // Print the individual columns...
      output_it.Reset();
      tDataEntryCommand<cAnalyzeGenotype> * data_command = NULL;
      int cur_col = 0;
      while ((data_command = output_it.Next()) != NULL) {
	data_command->SetTarget(&test_genotype);
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
      if (file_type == FILE_TYPE_HTML) fp << "</tr>" << endl;

      // Reset the mod_code back to the original sequence.
      mod_code[line_num].SetOp(cur_inst);
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
	genotype1->GetCode().FindHammingDistance(genotype2->GetCode());
      total_dist += dist * num_pairs;
      total_count += num_pairs;
    }
  }


  // Calculate the final answer
  double ave_dist = (double) total_dist / (double) total_count;
  cout << " ave distance = " << ave_dist << endl;
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
      // Determine the coutns...
      const int count1 = genotype1->GetNumCPUs();
      const int count2 = genotype2->GetNumCPUs();
      const int num_pairs = (genotype1 == genotype2) ?
	((count1 - 1) * (count2 - 1)) : (count1 * count2);
      if (num_pairs == 0) continue;

      // And do the tests...
      const int dist =
	genotype1->GetCode().FindLevensteinDistance(genotype2->GetCode());
      total_dist += dist * num_pairs;
      total_count += num_pairs;
    }
  }

  // Calculate the final answer
  double ave_dist = (double) total_dist / (double) total_count;
  cout << " ave distance = " << ave_dist << endl;
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
    cSpecies species(genotype1->GetCode());
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
	species.Compare(genotype2->GetCode());
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

  ofstream fp;
  fp.open(filename);
  fp << batch[batch1].Name() << " "
     << batch[batch2].Name() << " "
     << ave_dist << " "
     << total_count << " "
     << fail_count << " "
     << endl;

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
    sequences[i] = batch_it.Next()->GetCode().AsString();
    if (i == 0) continue;
    // Track of the number of insertions and deletions to shift properly.
    int num_ins = 0;
    int num_del = 0;

    // Compare each string to the previous.
    sequences[i].LDistance(sequences[i-1], diff_info, '_');
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


void cAnalyze::AnalyzeMuts(cString cur_string)
{
  cout << "Analyzing Mutations" << endl;

  // Make sure we have everything we need.
  if (batch[cur_batch].IsAligned() == false) {
    cout << "  Warning: sequences not aligned." << endl;
    return;
  }

  // Setup variables...
  cString filename("analyze_muts.dat");
  if (cur_string.GetSize() != 0) filename = cur_string.PopWord();

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
  ofstream fp(filename);
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
  const int total_diffs = first_seq.Distance(last_seq);
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
      cCodeArray test_code(test_sequence);
      cCPUTestInfo test_info;
      test_info.TestThreads();
      cTestCPU::TestCode(test_info, test_code);
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
      cAnalyzeGenotype max_genotype(max_sequence[i]);
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
  const int num_insts = inst_lib->GetSize();

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
	     << inst_lib->GetName(inst_id) << " ";
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
      fp << cur_inst.GetSymbol() << ":" << inst_lib->GetName(i) << " ";
    }
    fp << endl;
  }

  // Figure out how often we expect each instruction to appear...
  const double exp_freq = 1.0 / (double) num_insts;
  const double min_freq = exp_freq * 0.5;
  const double max_freq = exp_freq * 2.0;

  double total_length = 0.0;
  double total_freq[num_insts];
  for (int i = 0; i < num_insts; i++) total_freq[i] = 0.0;

  // Loop through all of the genotypes in this batch...
  tListIterator<cAnalyzeGenotype> batch_it(batch[cur_batch].List());
  cAnalyzeGenotype * genotype = NULL;
  while ((genotype = batch_it.Next()) != NULL) {
    // Setup for counting...
    int inst_bin[num_insts];
    for (int i = 0; i < num_insts; i++) inst_bin[i] = 0;

    // Count it up!
    const int genome_size = genotype->GetLength();
    for (int i = 0; i < genome_size; i++) {
      const int inst_id = genotype->GetCode().Get(i).GetOp();
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
      fp << cString::Stringf("%04.3f", inst_freq) << " ";
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
      fp << cString::Stringf("%04.3f", inst_freq) << " ";
    }
    fp << "</tr>" << endl
       << "</table></center>" << endl;
  }
}

void cAnalyze::CommandSystem(cString cur_string)
{
  cout << "Running System Command: " << cur_string << endl;

  system(cur_string());
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
	 << " parent distances may not be updated correctly" << endl;
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
      if ( mkdir( directory(), ACCESSPERMS ) )
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
       ("viable",      "Is Viable (0/1)", &cAnalyzeGenotype::GetViable));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("id",          "Genome ID",       &cAnalyzeGenotype::GetID));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("parent_id",   "Parent ID",       &cAnalyzeGenotype::GetParentID));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("parent_dist", "Parent Distance", &cAnalyzeGenotype::GetParentDist));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("num_cpus",    "Number of CPUs",  &cAnalyzeGenotype::GetNumCPUs));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("total_cpus",  "Total CPUs Ever", &cAnalyzeGenotype::GetTotalCPUs));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("length",      "Genome Length",   &cAnalyzeGenotype::GetLength,
        &cAnalyzeGenotype::CompareLength));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("copy_length", "Copied Length",   &cAnalyzeGenotype::GetCopyLength));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("exe_length",  "Executed Length", &cAnalyzeGenotype::GetExeLength));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, double>
       ("merit",       "Merit",           &cAnalyzeGenotype::GetMerit,
	&cAnalyzeGenotype::CompareMerit));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, double>
       ("comp_merit",  "Computational Merit", &cAnalyzeGenotype::GetCompMerit,
	&cAnalyzeGenotype::CompareCompMerit));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, double>
       ("comp_merit_ratio", "Computational Merit Ratio",
	&cAnalyzeGenotype::GetCompMeritRatio,
	&cAnalyzeGenotype::CompareCompMerit));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("gest_time",   "Gestation Time",  &cAnalyzeGenotype::GetGestTime,
	&cAnalyzeGenotype::CompareGestTime, "Inf."));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, double>
       ("efficiency",  "Rep. Efficiency", &cAnalyzeGenotype::GetEfficiency,
	&cAnalyzeGenotype::CompareEfficiency));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, double>
       ("efficiency_ratio", "Rep. Efficiency Ratio",
	&cAnalyzeGenotype::GetEfficiencyRatio,
	&cAnalyzeGenotype::CompareEfficiency));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, double>
       ("fitness",     "Fitness",         &cAnalyzeGenotype::GetFitness,
	&cAnalyzeGenotype::CompareFitness));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, double>
       ("fitness_ratio", "Fitness Ratio", &cAnalyzeGenotype::GetFitnessRatio,
	&cAnalyzeGenotype::CompareFitness));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("update_born", "Update Born",     &cAnalyzeGenotype::GetUpdateBorn));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("update_dead", "Update Dead",     &cAnalyzeGenotype::GetUpdateDead));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, int>
       ("depth",       "Tree Depth",      &cAnalyzeGenotype::GetDepth));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, const cString &>
       ("parent_muts", "Mutations from Parent",
	&cAnalyzeGenotype::GetParentMuts,
	&cAnalyzeGenotype::CompareNULL, "(none)", ""));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, const cString &>
       ("task_order", "Task Performance Order",
	&cAnalyzeGenotype::GetTaskOrder,
	&cAnalyzeGenotype::CompareNULL, "(none)", ""));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, cString>
       ("sequence",    "Genome Sequence",
	&cAnalyzeGenotype::GetSequence, 
	&cAnalyzeGenotype::CompareNULL, "(N/A)", ""));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, const cString &>
       ("alignment",   "Aligned Sequence",
	&cAnalyzeGenotype::GetAlignedSequence,
	&cAnalyzeGenotype::CompareNULL, "(N/A)", ""));

  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, cString>
       ("link.tasksites", "Phenotype Map", &cAnalyzeGenotype::GetMapLink));
  genotype_data_list.PushRear(new tDataEntry<cAnalyzeGenotype, cString>
       ("html.sequence",  "Genome Sequence",
	&cAnalyzeGenotype::GetHTMLSequence,
	&cAnalyzeGenotype::CompareNULL, "(N/A)", ""));

  // @CAO This must be fixed!  For the moment, only allow 80 tasks...
  for (int i = 0; i < cTestCPU::GetNumTasks(); i++) {
    cString t_name, t_desc;
    t_name.Set("task.%d", i);
    t_desc = cTestCPU::GetTaskKeyword(i);
    genotype_data_list.PushRear(new tArgDataEntry<cAnalyzeGenotype, int, int>
	(t_name, t_desc, &cAnalyzeGenotype::GetTaskCount, i,
	 &cAnalyzeGenotype::CompareTaskCount));
  }
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
    while (genotype_data_it.Next() != NULL) {
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

      while (genotype_data_it.Next() != NULL) {
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
	while (genotype_data_it.Next() != NULL) {
	  const cString & test_str = genotype_data_it.Get()->GetName();
	  const int test_dist = test_str.LDistance(cur_entry);
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

    // Reduction commands...
  AddLibraryDef("FIND_GENOTYPE", &cAnalyze::FindGenotype);
  AddLibraryDef("FIND_LINEAGE", &cAnalyze::FindLineage);
  AddLibraryDef("SAMPLE_ORGANISMS", &cAnalyze::SampleOrganisms);
  AddLibraryDef("SAMPLE_GENOTYPES", &cAnalyze::SampleGenotypes);
  AddLibraryDef("KEEP_TOP", &cAnalyze::KeepTopGenotypes);

  // Analysis commands...
  AddLibraryDef("PRINT", &cAnalyze::CommandPrint);
  AddLibraryDef("TRACE", &cAnalyze::CommandTrace);
  AddLibraryDef("PRINT_TASKS", &cAnalyze::CommandPrintTasks);
  AddLibraryDef("RENAME", &cAnalyze::CommandRename);
  AddLibraryDef("LANDSCAPE", &cAnalyze::CommandLandscape);
  AddLibraryDef("FITNESS_MATRIX", &cAnalyze::CommandFitnessMatrix);
  AddLibraryDef("DETAIL", &cAnalyze::CommandDetail);
  AddLibraryDef("DETAIL_INDEX", &cAnalyze::CommandDetailIndex);
  AddLibraryDef("MAP", &cAnalyze::CommandMap);
  AddLibraryDef("HAMMING", &cAnalyze::CommandHamming);
  AddLibraryDef("LEVENSTEIN", &cAnalyze::CommandLevenstein);
  AddLibraryDef("SPECIES", &cAnalyze::CommandSpecies);
  AddLibraryDef("ALIGN", &cAnalyze::CommandAlign);

  // Automated analysis
  AddLibraryDef("ANALYZE_MUTS", &cAnalyze::AnalyzeMuts);
  AddLibraryDef("ANALYZE_INSTRUCTIONS", &cAnalyze::AnalyzeInstructions);

  // Documantation...
  AddLibraryDef("HELPFILE", &cAnalyze::CommandHelpfile);

  // Control commands...
  AddLibraryDef("SET", &cAnalyze::VarSet);
  AddLibraryDef("SET_BATCH", &cAnalyze::BatchSet);
  AddLibraryDef("NAME_BATCH", &cAnalyze::BatchName);
  AddLibraryDef("PURGE_BATCH", &cAnalyze::BatchPurge);
  AddLibraryDef("DUPLICATE", &cAnalyze::BatchDuplicate);
  AddLibraryDef("RECALCULATE", &cAnalyze::BatchRecalculate);
  AddLibraryDef("STATUS", &cAnalyze::PrintStatus);
  AddLibraryDef("DEBUG", &cAnalyze::PrintDebug);
  AddLibraryDef("VERBOSE", &cAnalyze::ToggleVerbose);
  AddLibraryDef("INCLUDE", &cAnalyze::IncludeFile);
  AddLibraryDef("SYSTEM", &cAnalyze::CommandSystem);

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
  while (lib_it.Next() != NULL) {
    if (lib_it.Get()->GetName() == name) break;
  }

  return lib_it.Get();
}

