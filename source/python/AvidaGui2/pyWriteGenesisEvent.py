# -*- coding: utf-8 -*-

import shutil, string, pyInstructionSet, os.path

from AvidaCore import *

from descr import *

# Class to write the working genesis, event and environment files based on 
# the contents of settings dictionary

class pyWriteGenesisEvent:

  def __init__(self, in_dict = None, session_mdl = None, workspace_dir = None, 
    freeze_dir = None, tmp_in_dir = None, tmp_out_dir = None):
    
    self.m_session_mdl = session_mdl

    # If the file "research.version" exists in the current workspace we
    # are going to ignore any settings in the petri dish

    if os.path.exists(os.path.join(workspace_dir,"research.version")):
      settings_dict = {}
    else:
      settings_dict = in_dict["SETTINGS"]
	
    # Copies default event file and add to the 
    # temporary dictionary where the input files will live

    shutil.copyfile(os.path.join(workspace_dir, "events.default"), os.path.join(tmp_in_dir, "events.cfg"))
    
    # If this is a full petri dish inject all the organisms, otherwise
    # inject the start creature in the center of the grid

    
    #if we have a full petri dish...
    if in_dict.has_key("CELLS") and \
       (not (os.path.exists(os.path.join(workspace_dir,"research.version")))): 
      cells_dict = in_dict["CELLS"]
      organisms_dict = in_dict["ORGANISMS"]
      self.m_session_mdl.m_founding_cells_dict = cells_dict

      if in_dict.has_key("ANCESTOR_NAMES"):
        session_mdl.m_ancestors_dict = in_dict["ANCESTOR_NAMES"]
      else:
        session_mdl.m_ancestors_dict = {};
      if in_dict.has_key("ANCESTOR_LINKS"):
        ancestor_link_dict = in_dict["ANCESTOR_LINKS"]
      else:
        ancestor_link_dict = {}
      self.m_session_mdl.m_founding_cells_dict = cells_dict
      self.m_session_mdl.m_cell_num_ancestor_name_dict = {}

      for tmp_cell, org_in_cell in cells_dict.iteritems():
        linage_lable_in_cell = ancestor_link_dict[org_in_cell]
        ann_name_in_cell = session_mdl.m_ancestors_dict[linage_lable_in_cell]
        session_mdl.m_cell_num_ancestor_name_dict[tmp_cell] = ann_name_in_cell
      self.m_session_mdl.m_global_num_of_ancestors = \
        len(session_mdl.m_cell_num_ancestor_name_dict)

    #if it is not a full petri dish
    else:
      self.m_session_mdl.m_cell_num_ancestor_name_dict = {}
      self.m_session_mdl.m_global_num_of_ancestors = 0
      self.m_session_mdl.m_founding_cells_dict = {}
      cells_dict = {}
      organisms_dict = {}
      session_mdl.m_ancestors_dict = {}
      ancestor_link_dict = {}
      if settings_dict.has_key("START_CREATURE0"):
        world_x = settings_dict["WORLD-X"]
        world_y = settings_dict["WORLD-Y"]

        # Count all ancestors with the name of the form START_CREATUREx

        num_ancestors = 0
        while(settings_dict.has_key("START_CREATURE" + str(num_ancestors))):
          num_ancestors = num_ancestors + 1
        self.m_session_mdl.m_global_num_of_ancestors = num_ancestors

        # Process all the ancestors

        for i in range(num_ancestors):
          start_creature = settings_dict["START_CREATURE" + str(i)]
          session_mdl.m_ancestors_dict[str(i)] = start_creature
          ancestor_link_dict[str(i)] = str(i)

          self.start_cell_location = self.find_location(world_x, world_y, 
             num_ancestors, i)
          cells_dict[str(self.start_cell_location)] = str(i)

          # This variable is used in pyPetriDishCtrl.py to outline the 
          # founding organisms

          self.m_session_mdl.m_founding_cells_dict = cells_dict
          self.m_session_mdl.m_cell_num_ancestor_name_dict[str(self.start_cell_location)] = \
            session_mdl.m_ancestors_dict[str(i)]

          # Read the genome from the dictionary

          organisms_dict[str(i)] = settings_dict["START_GENOME" + str(i)]

    shutil.copyfile(os.path.join(workspace_dir, "inst_set.default"), \
                    os.path.join(tmp_in_dir, "inst_set.default"))

    settings_dict["EVENT_FILE"] = os.path.join(tmp_in_dir, "events.cfg")
    settings_dict["ENVIRONMENT_FILE"] = os.path.join(tmp_in_dir, "environment.cfg")
    self.writeEnvironmentFile(workspace_dir, settings_dict)
    settings_dict["INST_SET"] = os.path.join(tmp_in_dir, "inst_set.default")
    genesis_file_name = self.writeGenesisFile(workspace_dir, tmp_in_dir, \
                                              settings_dict)
    
    # There's a loop around organisms_dict
    #   organism number is key, sequence is value.
    #   I need to make a merits_dict with organism number as key, and
    #   merit as value.
    genesis = cGenesis()

    cConfig.Setup(genesis)

    genesis.Open(cString(genesis_file_name))
    environment = cEnvironment()
    environment.Load(cString(settings_dict["ENVIRONMENT_FILE"]))
    environment.GetInstSet().SetInstLib(cHardwareCPU.GetInstLib())
    cHardwareUtil.LoadInstSet(cString(settings_dict["INST_SET"]), environment.GetInstSet())
    cConfig.SetNumInstructions(environment.GetInstSet().GetSize())
    cConfig.SetNumTasks(environment.GetTaskLib().GetSize())
    cConfig.SetNumReactions(environment.GetReactionLib().GetSize())
    cConfig.SetNumResources(environment.GetResourceLib().GetSize())
    test_interface = cPopulationInterface()
    BuildTestPopInterface(test_interface)
    cTestCPU.Setup(
      environment.GetInstSet(),
      environment,
      environment.GetResourceLib().GetSize(),
      test_interface)

    inst_set = environment.GetInstSet()
    merits_dict = {}
    for key in organisms_dict.keys():
      genome = organisms_dict[key]
      analyze_genotype = cAnalyzeGenotype(cString(genome), inst_set) 

      random_number_generator_state = cRandom(cTools.globalRandom())
      analyze_genotype.Recalculate()
      cTools.globalRandom().Clone(random_number_generator_state)

      merit = analyze_genotype.GetMerit()
      merits_dict[key] = merit

    self.modifyEventFile(cells_dict, organisms_dict, ancestor_link_dict,
      merits_dict, os.path.join(tmp_in_dir, "events.cfg"), tmp_out_dir)
    
  # Read the default genesis file, if there is a equivilent line in the 
  # dictionary replace it the new values, otherwise just copy the line

  def writeGenesisFile(self, workspace_dir, tmp_in_dir, settings_dict):
  
    orig_genesis_file = open(os.path.join(workspace_dir, "genesis.default"))
    lines = orig_genesis_file.readlines()
    orig_genesis_file.close()
    out_genesis_file = open(os.path.join(tmp_in_dir, "genesis.avida"), "w")
    for line in lines:
      comment_start = line.find("#")
      if comment_start > -1:
        if comment_start == 0:
          clean_line = ""
        else:
          clean_line = line[:comment_start]
      else:
        clean_line = line;
      clean_line = clean_line.strip()
      if len(clean_line) > 0:
        var_name, value = string.split(clean_line)
        var_name = var_name.upper()

        # BDB -- added second if statment clause to support pause_at hack

        if (settings_dict.has_key(var_name) == True) and \
           (var_name != "MAX_UPDATES"):
          out_genesis_file.write(var_name + " " + str(settings_dict[var_name]) + "\n")
        else:
          out_genesis_file.write(line)
      else:
         out_genesis_file.write(line)
    out_genesis_file.close()
    
    return out_genesis_file.name
   
  # Read the default environment file, if there is a reward in the
  # dictionary for a given resource print out that line in working env. file

  def writeEnvironmentFile(self, workspace_dir, settings_dict):
 
    orig_environment_file = open(os.path.join(workspace_dir, "environment.default"))
    lines = orig_environment_file.readlines()
    orig_environment_file.close()
    out_environment_file = open(settings_dict["ENVIRONMENT_FILE"], "w")
    for line in lines:
      comment_start = line.find("#")
      if comment_start > -1:
        if comment_start == 0:
          clean_line = ""
        else:
          clean_line = line[:comment_start]
      else:
        clean_line = line;
      clean_line = clean_line.strip()
      if len(clean_line) > 0:
        split_out = string.split(clean_line)
        command_name = split_out[0].upper()

        # if it is a reaction line check further (otherwise print the line)

        if command_name == "REACTION":
          resource_name = split_out[1].upper()
          resource_key = "REWARD_" + resource_name
          task_name = split_out[2]

          # If the there is a reward key for this resource check further
          # (otherwise print the line)
 
          if settings_dict.has_key(resource_key) == True:

            # If the value of the reward key is true print it out as is 
            # (otherwise print out as a zero bonus)

            if settings_dict[resource_key] == "YES":
              out_environment_file.write(line)
            else:
              out_environment_file.write("REACTION " + resource_name + " " +
                                task_name + " process:value=0.0:type=add\n")
          else:
            out_environment_file.write(line)
        else:
          out_environment_file.write(line)

    out_environment_file.close()

  def modifyEventFile(self, cells_dict, organisms_dict, ancestor_link_dict, merits_dict,
    event_file_name, tmp_out_dir = None):

    # Routine to add to the event.cfg file by inject creatures into the
    # population and adding print statements into the correct directory

    event_out_file = open(event_file_name, 'a')
    for cell in cells_dict.keys():
      part1 = "u begin inject_sequence " +  organisms_dict[cells_dict[cell]]
      part2 = " " + cell + " " + str(int(cell)+1)
      #part3 = " -1 "
      part3 = " " + str(merits_dict[cells_dict[cell]]) + " "
      if (ancestor_link_dict.has_key(cells_dict[cell])):
        part4 = ancestor_link_dict[cells_dict[cell]] + "\n"
      else:
        part4 ="\n"
      event_out_file.write(part1 + part2 + part3 + part4)
    
    # write the .dat files to the correct directory

    event_out_file.write("\nu 0:1:end print_average_data " + 
                         os.path.join(tmp_out_dir, "average.dat") +"\n")
    event_out_file.write("u 0:1:end print_count_data " + 
                         os.path.join(tmp_out_dir, "count.dat") +"\n")
    event_out_file.close()
    
  def find_location(self, world_x, world_y, num_ancestors=1, org_num=0):

    # Routine to evenly place a given ancestor into the petri dish

    # If there are more than 9 creatures place them evenly in the population
    # array (ignoring the edges)

    if (num_ancestors > 9):
      return int(float(world_x * world_y) * (float(org_num + 1)/float(num_ancestors + 1))) % (world_x * world_y)

    spots = {};
    if (num_ancestors == 1):
      spots = [0.5,0.5]
    elif (num_ancestors == 2):
      spots = [0.5,0.33, 0.5,0.67]
    elif (num_ancestors == 3):
      spots = [0.5,0.25, 0.25,0.75, 0.75,0.75]
    elif (num_ancestors == 4):
      spots = [0.33,0.33, 0.33,0.67, 0.67,0.33, 0.67,0.67]
    elif (num_ancestors == 5):
      spots = [0.25,0.25, 0.75,0.25, 0.50,0.50, 0.25,0.75, 0.75,0.75]
    elif (num_ancestors == 6):
      spots = [0.25,0.25, 0.75,0.25, 
               0.25,0.50, 0.75,0.50,
               0.25,0.75, 0.75,0.75]
    elif (num_ancestors == 7):
      spots = [0.25,0.25, 0.75,0.25, 
               0.25,0.50, 0.50,0.50, 0.75,0.50,
               0.25,0.75, 0.75,0.75]
    elif (num_ancestors == 8):
      spots = [0.25,0.25, 0.50,0.375, 0.75,0.25, 
               0.25,0.50, 0.75,0.50,
               0.25,0.75, 0.50,0.625, 0.75,0.75]
    elif (num_ancestors == 9):
      spots = [0.25,0.25, 0.50,0.25, 0.75,0.25, 
               0.25,0.50, 0.50,0.50, 0.75,0.50,
               0.25,0.75, 0.50,0.75, 0.75,0.75]

    x = spots[org_num * 2]
    y = spots[(org_num * 2) + 1]

    return int((round(world_y * y) * world_x) + round(world_x * x)) % (world_x * world_y)
