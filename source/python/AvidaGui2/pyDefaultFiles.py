class pyDefaultFiles:

  def __init__(self, out_file_name = None, full_out_file_name = None):
    lines = []
    lines = lines + ["environment.default", "##############################################################################"]
    lines = lines + ["environment.default", "#"]
    lines = lines + ["environment.default", "# This is the setup file for the task/resource system.  From here, you can"]
    lines = lines + ["environment.default", "# setup the available resources (including their inflow and outflow rates) as"]
    lines = lines + ["environment.default", "# well as the reactions that the organisms can trigger by performing tasks."]
    lines = lines + ["environment.default", "#"]
    lines = lines + ["environment.default", "# This file is currently setup to reward 9 tasks, all of which use the"]
    lines = lines + ["environment.default", "# \"infinite\" resource, which is undepletable."]
    lines = lines + ["environment.default", "#"]
    lines = lines + ["environment.default", "# For information on how to use this file, see:  doc/environment.html"]
    lines = lines + ["environment.default", "# For other sample environments, see:  source/support/config/ "]
    lines = lines + ["environment.default", "#"]
    lines = lines + ["environment.default", "##############################################################################"]
    lines = lines + ["environment.default", ""]
    lines = lines + ["environment.default", "REACTION  NOT  not   process:value=1.0:type=pow  requisite:max_count=1"]
    lines = lines + ["environment.default", "REACTION  NAND nand  process:value=1.0:type=pow  requisite:max_count=1"]
    lines = lines + ["environment.default", "REACTION  AND  and   process:value=2.0:type=pow  requisite:max_count=1"]
    lines = lines + ["environment.default", "REACTION  ORN  orn   process:value=2.0:type=pow  requisite:max_count=1"]
    lines = lines + ["environment.default", "REACTION  OR   or    process:value=3.0:type=pow  requisite:max_count=1"]
    lines = lines + ["environment.default", "REACTION  ANDN andn  process:value=3.0:type=pow  requisite:max_count=1"]
    lines = lines + ["environment.default", "REACTION  NOR  nor   process:value=4.0:type=pow  requisite:max_count=1"]
    lines = lines + ["environment.default", "REACTION  XOR  xor   process:value=4.0:type=pow  requisite:max_count=1"]
    lines = lines + ["environment.default", "REACTION  EQU  equ   process:value=5.0:type=pow  requisite:max_count=1"]
    lines = lines + ["events.default", "##############################################################################"]
    lines = lines + ["events.default", "#"]
    lines = lines + ["events.default", "# This is the setup file for the events system.  From here, you can"]
    lines = lines + ["events.default", "# configure any actions that you want to have happen during the course of"]
    lines = lines + ["events.default", "# an experiment, including setting the times for data collection."]
    lines = lines + ["events.default", "#"]
    lines = lines + ["events.default", "# basic syntax:  [trigger]  [start:interval:stop]  [event] [arguments...]"]
    lines = lines + ["events.default", "#"]
    lines = lines + ["events.default", "# This file is currently setup to start off a population full of the "]
    lines = lines + ["events.default", "# starting organism specified in genesis, and then record key information"]
    lines = lines + ["events.default", "# every 100 updates."]
    lines = lines + ["events.default", "#"]
    lines = lines + ["events.default", "# For information on how to use this file, see:  doc/events.html"]
    lines = lines + ["events.default", "# For other sample event configurations, see:  source/support/config/"]
    lines = lines + ["events.default", "#"]
    lines = lines + ["events.default", "##############################################################################"]
    lines = lines + ["events.default", ""]
    lines = lines + ["events.default", "# Print all of the standard data files..."]
    lines = lines + ["events.default", "# u 0:100 print_dom                 # Save the most abundant genotypes"]
    lines = lines + ["events.default", "# u 0:1:end print_average_data    # Save info about they average genotypes"]
    lines = lines + ["events.default", "# u 0:100:end print_dominant_data   # Save info about most abundant genotypes"]
    lines = lines + ["events.default", "# u 0:100:end print_stats_data      # Collect satistics about entire pop."]
    lines = lines + ["events.default", "# u 0:1:end print_count_data      # Count organisms, genotypes, species, etc."]
    lines = lines + ["events.default", "# u 0:100:end print_tasks_data      # Save organisms counts for each task."]
    lines = lines + ["events.default", "# u 0:100:end print_time_data       # Track time conversion (generations, etc.)"]
    lines = lines + ["events.default", "# u 0:100:end print_resource_data   # Track resource abundance."]
    lines = lines + ["events.default", ""]
    lines = lines + ["events.default", "# A few data files not printed by default"]
    lines = lines + ["events.default", "# u 100:100:end print_error_data      # Std. Error on averages."]
    lines = lines + ["events.default", "# u 100:100:end print_variance_data   # Variance on averages."]
    lines = lines + ["events.default", "# u 100:100:end print_totals_data     # Total counts over entire run."]
    lines = lines + ["events.default", "# u 100:100:end print_tasks_exe_data  # Num. times tasks have been executed."]
    lines = lines + ["events.default", ""]
    lines = lines + ["events.default", "# Setup the exit time and full population data collection."]
    lines = lines + ["events.default", "# u 50000:50000 detail_pop           # Save current state of population."]
    lines = lines + ["events.default", "# u 50000:50000 dump_historic_pop    # Save ancestors of current population."]
    lines = lines + ["events.default", ""]
    lines = lines + ["genesis.default", "#############################################################################"]
    lines = lines + ["genesis.default", "# This file includes all the basic run-time defines for avida."]
    lines = lines + ["genesis.default", "# For more information, see doc/genesis.html"]
    lines = lines + ["genesis.default", "#############################################################################"]
    lines = lines + ["genesis.default", ""]
    lines = lines + ["genesis.default", "VERSION_ID 2.2.1		# Do not change this value!"]
    lines = lines + ["genesis.default", ""]
    lines = lines + ["genesis.default", "### Architecture Variables ###"]
    lines = lines + ["genesis.default", "MAX_UPDATES  -1         # Maximum updates to run simulation (-1 = no limit)"]
    lines = lines + ["genesis.default", "MAX_GENERATIONS -1      # Maximum generations to run simulation (-1 = no limit)"]
    lines = lines + ["genesis.default", "END_CONDITION_MODE 0	# End run when ..."]
    lines = lines + ["genesis.default", "			# 0 = MAX_UPDATES _OR_ MAX_GENERATIONS is reached"]
    lines = lines + ["genesis.default", "			# 1 = MAX_UPDATES _AND_ MAX_GENERATIONS is reached"]
    lines = lines + ["genesis.default", "WORLD-X 30		# Width of the world in Avida mode."]
    lines = lines + ["genesis.default", "WORLD-Y 30		# Height of the world in Avida mode."]
    lines = lines + ["genesis.default", "WORLD_GEOMETRY 1        # 1 = Bounded Grid"]
    lines = lines + ["genesis.default", "                        # 2 = Torus (Default)"]
    lines = lines + ["genesis.default", "RANDOM_SEED 0		# Random number seed. (0 for based on time)"]
    lines = lines + ["genesis.default", "HARDWARE_TYPE 0		# 0 = Original CPUs"]
    lines = lines + ["genesis.default", "			# 1 = New, Stack-based CPUs"]
    lines = lines + ["genesis.default", "MAX_CPU_THREADS 1	# Number of Threads CPUs can spawn"]
    lines = lines + ["genesis.default", ""]
    lines = lines + ["genesis.default", "### Configuration Files ###"]
    lines = lines + ["genesis.default", "DEFAULT_DIR ../work/              # Directory in which config files are found"]
    lines = lines + ["genesis.default", "INST_SET inst_set.default         # File containing instruction set"]
    lines = lines + ["genesis.default", "EVENT_FILE events.cfg             # File containing list of events during run"]
    lines = lines + ["genesis.default", "ANALYZE_FILE analyze.cfg          # File used for analysis mode"]
    lines = lines + ["genesis.default", "ENVIRONMENT_FILE environment.cfg  # File that describes the environment"]
    lines = lines + ["genesis.default", "# START_CREATURE @ancestor.organism   # Organism to seed the soup"]
    lines = lines + ["genesis.default", ""]
    lines = lines + ["genesis.default", "### Reproduction ###"]
    lines = lines + ["genesis.default", "BIRTH_METHOD 4    # 0 = Replace random organism in neighborhood"]
    lines = lines + ["genesis.default", "		  # 1 = Replace oldest organism in neighborhood"]
    lines = lines + ["genesis.default", "		  # 2 = Replace largest Age/Merit in neighborhood"]
    lines = lines + ["genesis.default", "		  # 3 = Place only in empty cells in neighborhood"]
    lines = lines + ["genesis.default", "		  # 4 = Replace random from entire population (Mass Action)"]
    lines = lines + ["genesis.default", "		  # 5 = Replace oldest in entire population (like Tierra)"]
    lines = lines + ["genesis.default", "PREFER_EMPTY 1    # Are empty cells given preference in offspring placement?"]
    lines = lines + ["genesis.default", "DEATH_METHOD 2    # 0 = Never die of old age."]
    lines = lines + ["genesis.default", "		  # 1 = Die when inst executed = AGE_LIMIT (with deviation)"]
    lines = lines + ["genesis.default", "		  # 2 = Die when inst executed = length * AGE_LIMIT (+ dev.)"]
    lines = lines + ["genesis.default", "AGE_LIMIT 20      # Modifies DEATH_METHOD"]
    lines = lines + ["genesis.default", "AGE_DEVIATION 0   # Modified DEATH_METHOD"]
    lines = lines + ["genesis.default", "ALLOC_METHOD 0    # 0 = Allocated space is set to default instruction."]
    lines = lines + ["genesis.default", "                  # 1 = Set to section of dead genome (Necrophilia)"]
    lines = lines + ["genesis.default", "                  # 2 = Allocated space is set to random instruction."]
    lines = lines + ["genesis.default", "DIVIDE_METHOD 1   # 0 = Divide leaves state of mother untouched."]
    lines = lines + ["genesis.default", "                  # 1 = Divide resets state of mother"]
    lines = lines + ["genesis.default", "                  #     (after the divide, we have 2 children)"]
    lines = lines + ["genesis.default", ""]
    lines = lines + ["genesis.default", "GENERATION_INC_METHOD 1 # 0 = Only the generation of the child is"]
    lines = lines + ["genesis.default", "                        #     increased on divide."]
    lines = lines + ["genesis.default", "			# 1 = Both the generation of the mother and child are"]
    lines = lines + ["genesis.default", "			#     increased on divide (good with DIVIDE_METHOD 1)."]
    lines = lines + ["genesis.default", ""]
    lines = lines + ["genesis.default", "### Divide Restrictions ####"]
    lines = lines + ["genesis.default", "CHILD_SIZE_RANGE 1.0	# Maximal differential between child and parent sizes."]
    lines = lines + ["genesis.default", "MIN_COPIED_LINES 0.5    # Code fraction which must be copied before divide."]
    lines = lines + ["genesis.default", "MIN_EXE_LINES    0.5    # Code fraction which must be executed before divide."]
    lines = lines + ["genesis.default", "REQUIRE_ALLOCATE   1    # Is a an allocate required before a divide? (0/1)"]
    lines = lines + ["genesis.default", "REQUIRED_TASK -1  # Number of task required for successful divide."]
    lines = lines + ["genesis.default", ""]
    lines = lines + ["genesis.default", "### Mutations ###"]
    lines = lines + ["genesis.default", ""]
    lines = lines + ["genesis.default", "# mutations that occur during execution.."]
    lines = lines + ["genesis.default", "POINT_MUT_PROB  0.0     # Mutation rate (per-location per update)"]
    lines = lines + ["genesis.default", "COPY_MUT_PROB   0.0075  # Mutation rate (per copy)."]
    lines = lines + ["genesis.default", ""]
    lines = lines + ["genesis.default", "# mutations that occur on divide..."]
    lines = lines + ["genesis.default", "INS_MUT_PROB    0.0     # Insertion rate (per site, applied on divide)."]
    lines = lines + ["genesis.default", "DEL_MUT_PROB    0.0     # Deletion rate (per site, applied on divide)."]
    lines = lines + ["genesis.default", "DIV_MUT_PROB    0.0     # Mutation rate (per site, applied on divide)."]
    lines = lines + ["genesis.default", "DIVIDE_MUT_PROB 0.0     # Mutation rate (per divide)."]
    lines = lines + ["genesis.default", "DIVIDE_INS_PROB 0.0     # Insertion rate (per divide)."]
    lines = lines + ["genesis.default", "DIVIDE_DEL_PROB 0.0     # Deletion rate (per divide)."]
    lines = lines + ["genesis.default", "PARENT_MUT_PROB 0.0     # Per-site, in parent, on divide"]
    lines = lines + ["genesis.default", ""]
    lines = lines + ["genesis.default", "# heads based mutations"]
    lines = lines + ["genesis.default", "# READ_SHIFT_PROB   0.0"]
    lines = lines + ["genesis.default", "# READ INS_PROB     0.0"]
    lines = lines + ["genesis.default", "# READ_DEL_PROB     0.0"]
    lines = lines + ["genesis.default", "# WRITE_SHIFT_PROB  0.0"]
    lines = lines + ["genesis.default", "# WRITE_INS_PROB    0.0"]
    lines = lines + ["genesis.default", "# WRITE_DEL_PROB    0.0"]
    lines = lines + ["genesis.default", ""]
    lines = lines + ["genesis.default", ""]
    lines = lines + ["genesis.default", "### Mutation reversions ###"]
    lines = lines + ["genesis.default", "# these slow down avida a lot, and should be set to 0 normally."]
    lines = lines + ["genesis.default", "REVERT_FATAL       0.0  # Should any mutations be reverted on birth?"]
    lines = lines + ["genesis.default", "REVERT_DETRIMENTAL 0.0  #   0.0 to 1.0; Probability of reversion."]
    lines = lines + ["genesis.default", "REVERT_NEUTRAL     0.0"]
    lines = lines + ["genesis.default", "REVERT_BENEFICIAL  0.0"]
    lines = lines + ["genesis.default", ""]
    lines = lines + ["genesis.default", "STERILIZE_FATAL       0.0  # Should any mutations clear (kill) the organism?"]
    lines = lines + ["genesis.default", "STERILIZE_DETRIMENTAL 0.0  #   0.0 to 1.0; Probability of reset."]
    lines = lines + ["genesis.default", "STERILIZE_NEUTRAL     0.0"]
    lines = lines + ["genesis.default", "STERILIZE_BENEFICIAL  0.0"]
    lines = lines + ["genesis.default", ""]
    lines = lines + ["genesis.default", "FAIL_IMPLICIT     0	# Should copies that failed *not* due to mutations"]
    lines = lines + ["genesis.default", "			# be eliminated?"]
    lines = lines + ["genesis.default", ""]
    lines = lines + ["genesis.default", "### Time Slicing ###"]
    lines = lines + ["genesis.default", "AVE_TIME_SLICE 30"]
    lines = lines + ["genesis.default", "SLICING_METHOD 2	# 0 = CONSTANT: all organisms get default..."]
    lines = lines + ["genesis.default", "			# 1 = PROBABILISTIC: Run _prob_ proportional to merit."]
    lines = lines + ["genesis.default", "			# 2 = INTEGRATED: Perfectly integrated deterministic."]
    lines = lines + ["genesis.default", "SIZE_MERIT_METHOD 4	# 0 = off (merit is independent of size)"]
    lines = lines + ["genesis.default", "			# 1 = Merit proportional to copied size"]
    lines = lines + ["genesis.default", "			# 2 = Merit prop. to executed size"]
    lines = lines + ["genesis.default", "			# 3 = Merit prop. to full size"]
    lines = lines + ["genesis.default", "			# 4 = Merit prop. to min of executed or copied size"]
    lines = lines + ["genesis.default", "			# 5 = Merit prop. to sqrt of the minimum size"]
    lines = lines + ["genesis.default", "TASK_MERIT_METHOD 1	# 0 = No task bonuses"]
    lines = lines + ["genesis.default", "			# 1 = Bonus just equals the task bonus"]
    lines = lines + ["genesis.default", "THREAD_SLICING_METHOD 0 # 0 = One thread executed per time slice."]
    lines = lines + ["genesis.default", "			# 1 = All threads executed each time slice."]
    lines = lines + ["genesis.default", ""]
    lines = lines + ["genesis.default", "MAX_LABEL_EXE_SIZE 1	# Max nops marked as executed when labels are used"]
    lines = lines + ["genesis.default", "MERIT_TIME 1            # 0 = Merit Calculated when task completed"]
    lines = lines + ["genesis.default", "		        # 1 = Merit Calculated on Divide"]
    lines = lines + ["genesis.default", "MAX_NUM_TASKS_REWARDED -1  # -1 = Unlimited"]
    lines = lines + ["genesis.default", ""]
    lines = lines + ["genesis.default", "### Genotype Info ###"]
    lines = lines + ["genesis.default", "THRESHOLD 1		# Number of organisms in a genotype needed for it"]
    lines = lines + ["genesis.default", "			#   to be considered viable."]
    lines = lines + ["genesis.default", "GENOTYPE_PRINT 0	# 0/1 (off/on) Print out all threshold genotypes?"]
    lines = lines + ["genesis.default", "GENOTYPE_PRINT_DOM 0	# Print out a genotype if it stays dominant for"]
    lines = lines + ["genesis.default", "                        #   this many updates. (0 = off)"]
    lines = lines + ["genesis.default", "SPECIES_THRESHOLD 2     # max failure count for organisms to be same species"]
    lines = lines + ["genesis.default", "SPECIES_RECORDING 0	# 1 = full, 2 = limited search (parent only)"]
    lines = lines + ["genesis.default", "SPECIES_PRINT 0		# 0/1 (off/on) Print out all species?"]
    lines = lines + ["genesis.default", "TEST_CPU_TIME_MOD 20    # Time allocated in test CPUs (multiple of length)"]
    lines = lines + ["genesis.default", "TRACK_MAIN_LINEAGE 1    # Track primary lineage leading to final population?"]
    lines = lines + ["genesis.default", ""]
    lines = lines + ["genesis.default", "### Log Files ###"]
    lines = lines + ["genesis.default", "LOG_CREATURES 0		# 0/1 (off/on) toggle to print file."]
    lines = lines + ["genesis.default", "LOG_GENOTYPES 0		# 0 = off, 1 = print ALL, 2 = print threshold ONLY."]
    lines = lines + ["genesis.default", "LOG_THRESHOLD 0		# 0/1 (off/on) toggle to print file."]
    lines = lines + ["genesis.default", "LOG_SPECIES 0		# 0/1 (off/on) toggle to print file."]
    lines = lines + ["genesis.default", "LOG_LANDSCAPE 0		# 0/1 (off/on) toggle to print file."]
    lines = lines + ["genesis.default", ""]
    lines = lines + ["genesis.default", "LOG_LINEAGES 0          # 0/1 (off/on) to log advantageous mutations"]
    lines = lines + ["genesis.default", "# This one can slow down avida a lot. It is used to get an idea of how"]
    lines = lines + ["genesis.default", "# often an advantageous mutation arises, and where it goes afterwards."]
    lines = lines + ["genesis.default", "# See also LINEAGE_CREATION_METHOD."]
    lines = lines + ["genesis.default", ""]
    lines = lines + ["genesis.default", "LINEAGE_CREATION_METHOD 0"]
    lines = lines + ["genesis.default", "# Lineage creation options are.  Works only when LOG_LINEAGES is set to 1."]
    lines = lines + ["genesis.default", "#   0 = manual creation (on inject, use successive integers as lineage labels)."]
    lines = lines + ["genesis.default", "#   1 = when a child's (potential) fitness is higher than that of its parent."]
    lines = lines + ["genesis.default", "#   2 = when a child's (potential) fitness is higher than max in population."]
    lines = lines + ["genesis.default", "#   3 = when a child's (potential) fitness is higher than max in dom. lineage"]
    lines = lines + ["genesis.default", "#	*and* the child is in the dominant lineage, or (2)"]
    lines = lines + ["genesis.default", "#   4 = when a child's (potential) fitness is higher than max in dom. lineage"]
    lines = lines + ["genesis.default", "#	(and that of its own lineage)"]
    lines = lines + ["genesis.default", "#   5 = same as child's (potential) fitness is higher than that of the"]
    lines = lines + ["genesis.default", "#       currently dominant organism, and also than that of any organism"]
    lines = lines + ["genesis.default", "#       currently in the same lineage."]
    lines = lines + ["genesis.default", "#   6 = when a child's (potential) fitness is higher than any organism"]
    lines = lines + ["genesis.default", "#       currently in the same lineage."]
    lines = lines + ["genesis.default", "#   7 = when a child's (potential) fitness is higher than that of any"]
    lines = lines + ["genesis.default", "#       organism in its line of descent"]
    lines = lines + ["genesis.default", ""]
    lines = lines + ["genesis.default", "### END ###"]
    lines = lines + ["genesis.default", ""]
    lines = lines + ["genesis.default", ""]
    lines = lines + ["inst_set.default", "nop-A      1   # a"]
    lines = lines + ["inst_set.default", "nop-B      1   # b"]
    lines = lines + ["inst_set.default", "nop-C      1   # c"]
    lines = lines + ["inst_set.default", "if-n-equ   1   # d"]
    lines = lines + ["inst_set.default", "if-less    1   # e"]
    lines = lines + ["inst_set.default", "pop        1   # f"]
    lines = lines + ["inst_set.default", "push       1   # g"]
    lines = lines + ["inst_set.default", "swap-stk   1   # h"]
    lines = lines + ["inst_set.default", "swap       1   # i "]
    lines = lines + ["inst_set.default", "shift-r    1   # j"]
    lines = lines + ["inst_set.default", "shift-l    1   # k"]
    lines = lines + ["inst_set.default", "inc        1   # l"]
    lines = lines + ["inst_set.default", "dec        1   # m"]
    lines = lines + ["inst_set.default", "add        1   # n"]
    lines = lines + ["inst_set.default", "sub        1   # o"]
    lines = lines + ["inst_set.default", "nand       1   # p"]
    lines = lines + ["inst_set.default", "IO         1   # q   Puts current contents of register and gets new."]
    lines = lines + ["inst_set.default", "h-alloc    1   # r   Allocate as much memory as organism can use."]
    lines = lines + ["inst_set.default", "h-divide   1   # s   Cuts off everything between the read and write heads"]
    lines = lines + ["inst_set.default", "h-copy     1   # t   Combine h-read and h-write"]
    lines = lines + ["inst_set.default", "h-search   1   # u   Search for matching template, set flow head & return info"]
    lines = lines + ["inst_set.default", "               #   #   if no template, move flow-head here, set size&offset=0."]
    lines = lines + ["inst_set.default", "mov-head   1   # v   Move ?IP? head to flow control."]
    lines = lines + ["inst_set.default", "jmp-head   1   # w   Move ?IP? head by fixed amount in CX.  Set old pos in CX."]
    lines = lines + ["inst_set.default", "get-head   1   # x   Get position of specified head in CX."]
    lines = lines + ["inst_set.default", "if-label   1   # y"]
    lines = lines + ["inst_set.default", "set-flow   1   # z   Move flow-head to address in ?CX? "]
    lines = lines + ["inst_set.default", ""]
    lines = lines + ["inst_set.default", "#adv-head   1"]
    lines = lines + ["inst_set.default", "#jump-f     1"]
    lines = lines + ["inst_set.default", "#jump-b     1"]
    lines = lines + ["inst_set.default", "#call       1"]
    lines = lines + ["inst_set.default", "#return     1"]
    lines = lines + ["inst_set.default", "#if-bit-1   1"]
    lines = lines + ["inst_set.default", "#get        1"]
    lines = lines + ["inst_set.default", "#put        1"]
    lines = lines + ["inst_set.default", "#h-read     1"]
    lines = lines + ["inst_set.default", "#h-write    1"]
    lines = lines + ["inst_set.default", "#set-head   1"]
    lines = lines + ["inst_set.default", "#search-f   1"]
    lines = lines + ["inst_set.default", "#search-b   1"]
    lines = lines + ["inst_set.default", ""]
    lines = lines + ["inst_set.default", ""]
    lines = lines + ["inst_set.default", "# Works on multiple nops:  pop  push  inc  dec  IO  adv-head "]
    lines = lines + ["inst_set.default", ""]
    lines = lines + ["inst_set.default", "# What if we add a new head.  Search will return the location of something,"]
    lines = lines + ["inst_set.default", "# and put the new head there.  Then set-head will move another head to that"]
    lines = lines + ["inst_set.default", "# point.  In the case of the copy loop, it only needs to be set once and"]
    lines = lines + ["inst_set.default", "# this will speed up the code quite a bit!"]
    lines = lines + ["inst_set.default", ""]
    lines = lines + ["inst_set.default", "# Search with no template returns current position (abs line number) in"]
    lines = lines + ["inst_set.default", "# genome."]
    lines = lines + ["freezer/@default.empty", "*SETTINGS"]
    lines = lines + ["freezer/@default.empty", "RANDOM_SEED 0"]
    lines = lines + ["freezer/@default.empty", "DEATH_METHOD 2"]
    lines = lines + ["freezer/@default.empty", "COPY_MUT_PROB 0.03"]
    lines = lines + ["freezer/@default.empty", "WORLD-X 30"]
    lines = lines + ["freezer/@default.empty", "WORLD-Y 30"]
    lines = lines + ["freezer/@default.empty", "BIRTH_METHOD 0"]
    lines = lines + ["freezer/@default.empty", "START_CREATURE0 default"]
    lines = lines + ["freezer/@default.empty", "AGE_LIMIT 100"]
    lines = lines + ["freezer/@default.empty", "MAX_UPDATES -1"]
    lines = lines + ["freezer/@ancestor.organism", "rucavcqgfcqapqcccccccccccccccccccccccccccutycasvab"]
    out_lines = []
    num_lines = len(lines)/2
    for line_num in range(num_lines):
      if lines[line_num * 2] == out_file_name:
        out_lines = out_lines + [lines[(line_num * 2) + 1]]
    full_out_file = open(full_out_file_name, 'w')
    for line in out_lines:
      full_out_file.write(line + "\n")
    full_out_file.close()
