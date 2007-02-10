"""
Class that uses Avida and an Avida configuration to produce a set of
last lines of Avida output files, and then compares them to a set of
corresponding expected last lines that were generated by the companion
class TailGenerator.
"""

import sys
import os               
import shutil

class TailChecker:

  def __init__(self,
    seed_set,
    run_subdir_path_base,
    genesis_file_path,
    output_file_name,
    expectation_file_path_base,
    expectation_file_name,
    avida_exe_path,
    avida_args
  ):
    self.seed_set = seed_set
    self.run_subdir_path_base = run_subdir_path_base
    self.genesis_file_path = genesis_file_path
    self.output_file_name = output_file_name
    self.expectation_file_path_base = expectation_file_path_base
    self.expectation_file_name = expectation_file_name
    self.avida_exe_path = avida_exe_path
    self.avida_args = avida_args

    self.failures_dict = {}

  def _check_tail(self, seed):
    run_subdir = self.run_subdir_path_base + str(seed)
    expectation_file_path = self.expectation_file_path_base + str(seed) + '/' + self.expectation_file_name
    avida_command = self.avida_exe_path + ' ' + self.avida_args % seed

    old_cwd = os.getcwd()

    print "from run_subdir " + run_subdir + " : using command : " + avida_command + " :"

    try:
      shutil.rmtree(run_subdir, True)
    except OSError:
      print "couldn't remove run-subdirectory " + run_subdir + "; continuing."

    try:
      os.makedirs(run_subdir)
    except OSError:
      print "failed to make run-subdirectory " + run_subdir + "; continuing."

    shutil.copy(self.genesis_file_path, run_subdir)
    os.chdir(run_subdir)
    avida_output_fd = os.popen(avida_command)
    line = avida_output_fd.readline()
    while line != "":
      print line,
      line = avida_output_fd.readline()
    avida_output_fd.close()

    detail_fd = file(self.output_file_name,"rU") 
    next_detail_line = detail_fd.readline()
    while next_detail_line != "":
      last_detail_line = next_detail_line
      next_detail_line = detail_fd.readline()
    detail_fd.close()

    print "last line of output file \"" + self.output_file_name + "\" : "
    print last_detail_line
    
    expected_last_line_fd = file(expectation_file_path, "rU")
    expected_last_detail_line = expected_last_line_fd.readline()
    expected_last_line_fd.close()

    if last_detail_line != expected_last_detail_line:
      print "last line of output file doesn't match expected last line."
      self.failures_dict[seed] = (last_detail_line, expected_last_detail_line)
    else:
      print "last line of output file matches expected last line."

    os.chdir(old_cwd)

  def check_tails(self):
    for seed in self.seed_set:
      self._check_tail(seed)

