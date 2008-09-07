//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "stats.hh"
#include "genotype.hh"

cLocalStats::cLocalStats(const cGenesis & genesis)
{
  int i;
  int temp_flag;

  g_memory.Add(C_STATISTICS);

  current_update = 0;
  
  // Get information from genesis file...

  copy_mut_rate   = genesis.ReadInt("COPY_MUT_RATE");
  point_mut_rate  = genesis.ReadInt("POINT_MUT_RATE");
  divide_mut_rate  = genesis.ReadInt("DIVIDE_MUT_RATE");
  divide_ins_rate  = genesis.ReadInt("DIVIDE_INS_RATE");
  divide_del_rate  = genesis.ReadInt("DIVIDE_DEL_RATE");

  population_mode = genesis.ReadInt("MODE");

  // Open files to place the statistics in.

  average_data_interval = genesis.ReadInt("SAVE_AVERAGE_DATA");
  if (average_data_interval && population_mode != MODE_PLAYBACK)
    fp_average_data.open("average.dat");
  else fp_average_data.close();

  dominant_data_interval = genesis.ReadInt("SAVE_DOMINANT_DATA");
  if (dominant_data_interval && population_mode != MODE_PLAYBACK)
    fp_dominant_data.open("dominant.dat");
  else fp_dominant_data.close();

  count_data_interval = genesis.ReadInt("SAVE_COUNT_DATA");
  if (count_data_interval && population_mode != MODE_PLAYBACK)
    fp_count_data.open("count.dat");
  else fp_count_data.close();

  totals_data_interval = genesis.ReadInt("SAVE_TOTALS_DATA");
  if (totals_data_interval && population_mode != MODE_PLAYBACK)
    fp_totals_data.open("totals.dat");
  else fp_totals_data.close();

  tasks_data_interval = genesis.ReadInt("SAVE_TASKS_DATA");
  if (tasks_data_interval && population_mode != MODE_PLAYBACK)
    fp_tasks_data.open("tasks.dat");
  else fp_tasks_data.close();

  stats_data_interval = genesis.ReadInt("SAVE_STATS_DATA");
  if (stats_data_interval && population_mode != MODE_PLAYBACK)
    fp_stats_data.open("stats.dat");
  else fp_stats_data.close();

  // Open Status files.

  genotype_status_interval = genesis.ReadInt("SAVE_GENOTYPE_STATUS");
  if (genotype_status_interval && population_mode != MODE_PLAYBACK)
    fp_genotype_status.open("genotype.status");
  else fp_genotype_status.close();

  diversity_status_interval = genesis.ReadInt("SAVE_DIVERSITY_STATUS");
  if (diversity_status_interval && population_mode != MODE_PLAYBACK)
    fp_diversity_status.open("diversity.status");
  else fp_diversity_status.close();


  // Open log files.

  temp_flag = genesis.ReadInt("LOG_BREED_COUNT");
  if (temp_flag && population_mode != MODE_PLAYBACK)
    fp_breed_log.open("breed_count.log");
  else fp_breed_log.close();

  temp_flag = genesis.ReadInt("LOG_CREATURES");  
  if (temp_flag && population_mode != MODE_PLAYBACK) {
    fp_creature_log.open("creature.log");
  } else {
    fp_creature_log.close();
  }

  temp_flag = genesis.ReadInt("LOG_PHYLOGENY");  
  if (temp_flag && population_mode != MODE_PLAYBACK) {
    fp_phylogeny_log.open("phylogeny.log");
  } else {
    fp_phylogeny_log.close();
  }

  temp_flag = genesis.ReadInt("LOG_GENOTYPES");
  log_threshold_only = FALSE;
  if (temp_flag && population_mode != MODE_PLAYBACK){
    fp_genotype_log.open("genotype.log");
    if (temp_flag > 1) log_threshold_only = TRUE;
  }else{
    fp_genotype_log.close();
  }

  temp_flag = genesis.ReadInt("LOG_THRESHOLD");
  if (temp_flag && population_mode != MODE_PLAYBACK)
    fp_threshold_log.open("threshold.log");
  else fp_threshold_log.close();

  temp_flag = genesis.ReadInt("LOG_SPECIES");
  if (temp_flag && population_mode != MODE_PLAYBACK)
    fp_species_log.open("species.log");
  else fp_species_log.close();

  // Update information

  ave_merit      = 0;
  best_merit     = 0;
  best_fitness   = 0;
  best_gestation = 0.0;
  max_fitness    = 0.0;

  ave_gestation  = 0.0;
  ave_repro_rate = 0.0;
  entropy        = 0.0;
  energy         = 0.0;
  ave_fitness    = 0.0;
  ave_size       = 0.0;
  ave_copy_size  = 0.0;
  ave_exe_size   = 0.0;
  ave_mem_size   = 0.0;

  ave_creature_age  = 0.0;
  ave_genotype_age  = 0.0;
  ave_threshold_age = 0.0;
  ave_species_age   = 0.0;

  num_deaths     = 0;
  num_executed   = 0;
  num_creatures  = 0;
  num_genotypes  = 0;
  num_threshold  = 0;
  num_species    = 0;
  num_thresh_species = 0;
  num_breed_true = 0;

  num_genotypes_last = 1;

  tot_creatures = 0;
  tot_genotypes = 0;
  tot_threshold = 0;
  tot_species   = 0;
  tot_thresh_species = 0;
  tot_executed  = 0;

  num_tasks = NUM_TASKS;
  task_count = g_memory.GetInts(num_tasks);
  for (i = 0; i < num_tasks; i++) {
    task_count[i] = 0;
  }
  get_count = 0;
  put_count = 0;
  ggp_count = 0;
}

cLocalStats::~cLocalStats()
{
  g_memory.Remove(C_STATISTICS);
  g_memory.Free(task_count, num_tasks);

  cout<<"Closing stats object..."<<endl;

  // Close all the statistics files.

  fp_average_data.close();
  fp_dominant_data.close();
  fp_count_data.close();
  fp_totals_data.close();
  fp_tasks_data.close();
  fp_stats_data.close();

  fp_genotype_status.close();
  fp_diversity_status.close();

  fp_breed_log.close();
  fp_creature_log.close();
  fp_genotype_log.close();
  fp_threshold_log.close();
  fp_species_log.close();
}

void cLocalStats::SetBestGenotype(cGenotype * best_genotype)
{
  best_merit = best_genotype->GetApproxMerit();
  best_gestation = best_genotype->GetApproxGestationTime();
  best_fitness = best_genotype->GetApproxFitness();
  best_copied_size = best_genotype->GetApproxCopiedSize();
  best_exe_size = best_genotype->GetApproxExecutedSize();

  best_size = best_genotype->GetLength();
  best_genotype_id = best_genotype->GetID();
  best_name = best_genotype->GetName();
  best_births = best_genotype->GetCreaturesBorn();
  best_abundance = best_genotype->GetNumCPUs();
}

void cLocalStats::ProcessUpdate(int new_update)
{
  current_update = new_update;
  int i;

  // Calculate any final stats which need to be printed.
  double best_repro_rate = 0;
  double max_repro_rate = 0;
  if (ave_merit.GetBase()) {
    best_repro_rate = best_fitness / ave_merit.GetDouble();
    max_repro_rate = max_fitness / ave_merit.GetDouble();
  }
  int genotype_change = num_genotypes - num_genotypes_last;
  double ave_births = ((double) num_deaths) / ((double) num_genotypes);
  double ave_abundance = ((double) num_creatures) / ((double) num_genotypes);

  // Print average.dat
  if( fp_average_data.good()
      && (current_update % average_data_interval == 0) ){
    //"%d %5.3f %5.3f %5.3f %.2e %6.1f %6.1f %6.1f %6.3f %6.3f\n",
    fp_average_data<<current_update<<" "
		   <<ave_merit.GetDouble()<<" "
		   <<ave_gestation<<" "
		   <<ave_fitness<<" "
		   <<ave_repro_rate<<" "
		   <<ave_size<<" "
		   <<ave_copy_size<<" "
		   <<ave_exe_size<<" "
		   <<ave_births<<" "
		   <<ave_abundance<<endl;
  }

  // Print dominant.dat
  if( fp_dominant_data.good()
      && (current_update % dominant_data_interval == 0) ){
    //"%d %5.0f %5.1f %5.3f %.2e %5d %5.1f %5.1f %3d %d %5.3f %5.3f %s\n",
    fp_dominant_data<<current_update<<" "
		    <<best_merit.GetDouble()<<" "
		    <<best_gestation<<" "
		    <<best_fitness<<" "
		    <<best_repro_rate<<" "
		    <<best_size<<" "
		    <<best_copied_size<<" "
		    <<best_exe_size<<" "
		    <<best_births<<" "
		    <<best_abundance<<" "
		    <<max_fitness<<" "
		    <<max_repro_rate<<" "
		    <<best_name<<endl;
  }

  // Print stats.dat
  if( fp_stats_data.good()
      && (current_update % stats_data_interval == 0)) {
    //"%5d %8.5f %8.5f %8.5f %8.5f %8.5f %4d %8.5f %8.5f\n",
    fp_stats_data<<current_update<<" "
		 <<energy<<" "
		 <<(1.0-ave_fidelity)<<" "
		 <<1.0-dom_fidelity<<" "
		 <<-log(ave_fidelity)<<" "
		 <<-log(dom_fidelity)<<" "
		 <<genotype_change<<" "
		 <<entropy<<" "
		 <<species_entropy<<endl;
  }

  // Print count.dat
  if( fp_count_data.good()
      && (current_update % count_data_interval == 0)) {
    //"%5d %8d %5d %5d %5d %5d %5d %5d %5d %5d\n",
    fp_count_data<<current_update<<" "
		 <<num_executed<<" "
		 <<num_creatures<<" "
		 <<num_genotypes<<" "
		 <<num_threshold<<" "
		 <<num_species<<" "
		 <<num_thresh_species<<" "
		 <<num_deaths<<" "
		 <<num_breed_true<<" "
		 <<num_parasites<<endl;
  }

  // Print totals.dat
  tot_executed += num_executed;
  if( fp_totals_data.good()
      && (current_update % totals_data_interval == 0)) {
    fp_totals_data<<current_update<<" "
		  <<tot_executed<<" "
		  <<tot_creatures<<" "
		  <<tot_genotypes<<" "
		  <<tot_threshold<<" "
		  <<tot_species<<endl;
  }
  
  // Print tasks.dat
  if (fp_tasks_data.good() && (current_update % tasks_data_interval == 0)) {
    fp_tasks_data<<current_update<<" "
		 <<get_count<<" "
		 <<put_count<<" "
		 <<ggp_count;
    for (i = 0; i < num_tasks; i++) {
      fp_tasks_data<<" "<<task_count[i];
    }
    fp_tasks_data<<endl;
  }

  // Zero-out any variables which need to be cleared at end of update.

  num_deaths = 0;
  num_executed = 0;
  for (i = 0; i < num_tasks; i++) {
    task_count[i] = 0;
  }
  get_count = 0;
  put_count = 0;
  ggp_count = 0;

  best_merit = 0;
  best_gestation = 0.0;
  best_fitness = 0.0;
  max_fitness = 0.0;
}


void cLocalStats::FlushFP(){
  fp_average_data.flush();
  fp_dominant_data.flush();
  fp_count_data.flush();
  fp_totals_data.flush();
  fp_tasks_data.flush();
  fp_stats_data.flush();

  fp_genotype_status.flush();
  fp_diversity_status.flush();

  fp_creature_log.flush();
  fp_breed_log.flush();
  fp_genotype_log.flush();
  fp_threshold_log.flush();
  fp_species_log.flush();
}
  

cGlobalStats::cGlobalStats()
{
#ifdef TEST
  fp_test.open("test.dat");
#else
  fp_test.close();
#endif

  current_update = 0;
}

cGlobalStats::~cGlobalStats()
{
#ifdef TEST
  fp_test.close();
#endif
}

cGlobalStats stats;
