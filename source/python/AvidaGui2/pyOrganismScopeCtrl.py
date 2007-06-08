# -*- coding: utf-8 -*-

from pyOrganismScopeView2 import pyOrganismScopeView2
from AvidaCore import cAnalyzeGenotype, cGenome, cInstruction, cInstUtil, cRandom, cString, cTools
from pyHardwareTracer import pyHardwareTracer
from pyTimeline import pyTimeline, TimelineFlag

from descr import *


from qt import *

import os
import tempfile



class pyOrganismScopeCtrl(pyOrganismScopeView2):

  def __init__(self,parent = None,name = None,fl = 0):
    pyOrganismScopeView2.__init__(self,parent,name,fl)
    QToolTip.add(self,"Watch an organism run")

    if not name: self.setName("pyOrganismScopeCtrl")

    self.m_test_cpu_mutation_rate = 0.0
    self.m_seed_based_on_time = True
    self.m_debug_organism_file = None
    self.m_analyze_genotype = None
    self.m_organism_name = None

  def construct(self, session_mdl):
    print "pyOrganismScopeCtrl.construct()."
    self.m_session_mdl = session_mdl
    self.m_avida = None
    self.setAcceptDrops(1)

    test_item_name = 'jeff'
    self.emit(PYSIGNAL("freezerItemDoubleClickedOnInOnePopSig"), (test_item_name,))

    self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("setAvidaSig"), self.setAvidaSlot)
#    self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("jmcTestSig"), self.jmcTestSlot)
    #self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("setDebugOrganismFileSig"), self.setDebugOrganismFileSlot)

    #@JMC testing
    self.connect(
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("orgClickedOnSig"),
      self.jmcTestOrgSlot)

    self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("ScopeConfig_MutationSliderValueChangedSig"),
      self.MutationSliderValueChangedSlot)

    self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("ScopeConfig_RandomSeedSig"),
      self.RandomSeedSlot)

    #self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("ScopeConfig_HeadsTypeCBActivatedSig"),
    #  self.HeadsTypeCBActivatedSlot)

    #self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("ScopeConfig_ShowTaskTestsCBToggledSig"),
    #  self.ShowTaskTestsCBToggledSlot)
    #self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("ScopeConfig_ShowRegistersCBToggledSig"),
    #  self.ShowRegistersCBToggledSlot)
    #self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("ScopeConfig_AnimateHeadMovementCBToggledSig"),
    #  self.AnimateHeadMovementCBToggledSlot)
    #self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("ScopeConfig_ShowStacksCBToggledSig"),
    #  self.ShowStacksCBToggledSlot)
    #self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("ScopeConfig_ShowHeadsAsLettersCBToggledSig"),
    #  self.ShowHeadsAsLettersCBToggledSlot)
    #self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("ScopeConfig_ShowInstructionNamesCBToggledSig"),
    #  self.ShowInstructionNamesCBToggledSlot)
    #self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("ScopeConfig_ShowInputsAndOutputsCBToggledSig"),
    #  self.ShowInputsAndOutputsCBToggledSlot)
    #self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("ScopeConfig_ShowFullStacksCBToggledSig"),
    #  self.ShowFullStacksCBToggledSlot)
    #self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("ScopeConfig_AnimateInstructionCopyCBToggledSig"),
    #  self.AnimateInstructionCopyCBToggledSlot)
    #self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("ScopeConfig_ShowHardwareCBToggledSig"),
    #  self.ShowHardwareCBToggledSlot)
    #self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("ScopeConfig_AnimateOrganismDivideCBToggledSig"),
    #  self.AnimateOrganismDivideCBToggledSlot)

    #self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("ScopeConfig_LayoutSpacingSBValueChangedSig"),
    #  self.LayoutSpacingSBValueChangedSlot)
    #self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("ScopeConfig_HardwareIndicatorSBValueChangedSig"),
    #  self.HardwareIndicatorSBValueChangedSlot)

    self.connect(
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("parseOrganismFileSig"),
      self.parseOrganismFileSlot)
    self.connect(
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("parseOrganismGenomeSig"),
      self.parseOrganismGenomeSlot)
    self.connect(
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("parseOrganismGenotypeSig"),
      self.parseOrganismGenotypeSlot)

  def jmcTestOrgSlot(self, clicked_cell_item = None):
    descr("no way this org slot works++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")


  def dragEnterEvent( self, e ):
    e.acceptAction(True)

    freezer_item_list = QString()
    if ( QTextDrag.decode( e, freezer_item_list ) ) :
      freezer_item_list = str(e.encodedData("text/plain"))
      freezer_item_names = freezer_item_list.split("\t")[1:]
      descr("BDB -- if decode true" + freezer_item_list)
      if (len(freezer_item_names) > 1):
         pass
      else:
        freezer_item_name = freezer_item_names[0]
        if freezer_item_name.endswith('.organism'):
          e.accept()
        else:
          pass

  def dropEvent( self, e ):
    freezer_item_list = QString()
    if ( QTextDrag.decode( e, freezer_item_list ) ) :
      freezer_item_list = str(e.encodedData("text/plain"))
      freezer_item_names = freezer_item_list.split("\t")[1:]
      descr("BDB -- if decode true" + freezer_item_list)
      if (len(freezer_item_names) > 1):
         info("Only one organism can be dragged here")
      else:
        freezer_item_name = freezer_item_names[0]
        if os.path.exists(freezer_item_name) == False:
          warningNoMtehodName(freezer_item_name + " does not exist")
        else:
          if freezer_item_name.endswith('.organism'):
            e.accept()
            self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("parseOrganismFileSig"), (freezer_item_name,))
          else:
            info("Only organisms can be dragged here")


  def parseOrganismFileSlot( self, organism_filename ):
    #self.setDebugOrganismFile(freezer_item_name)

    # early exit if there's not organism to analyze
    if organism_filename is None:
      return

    self.m_analyze_genotype = None
    if self.m_avida:
      org_file = open(organism_filename)
      org_string = org_file.readline()
      org_string = org_string.rstrip()
      org_string = org_string.lstrip()
      org_file.close()
      self.parseOrganismGenomeSlot(org_string)
      genome = cGenome(cString(org_string))
      inst_set = self.m_avida.m_environment.GetInstSet()
      analyze_genotype = cAnalyzeGenotype(genome, inst_set)

      self.m_organism_name = os.path.basename(os.path.splitext(organism_filename)[0])

      self.parseOrganismGenotypeSlot(analyze_genotype)

  def parseOrganismGenomeSlot( self, org_string ):
    if org_string is not None:
      genome = cGenome(cString(org_string))
      inst_set = self.m_avida.m_environment.GetInstSet()
      analyze_genotype = cAnalyzeGenotype(genome, inst_set)

      self.m_organism_name = "genome: " + org_string

      self.parseOrganismGenotypeSlot(analyze_genotype)

  def parseOrganismGenotypeSlot( self, analyze_genotype ):

    self.m_analyze_genotype = analyze_genotype
    self.analyzeLoadedOrganism()

#  def jmcTest(self,avida):
#    descr("made it to jmcTest")

  def setAvidaSlot(self, avida):
    print "pyOrganismScopeCtrl.setAvidaSlot() ..."
    old_avida = self.m_avida
    self.m_avida = avida
    if(old_avida):
      print "pyOrganismScopeCtrl.setAvidaSlot() deleting old_avida ..."
      del old_avida

  def setDebugOrganismFile(self, organism_filename):
    self.m_debug_organism_file = organism_filename
    

  def analyzeLoadedOrganism(self):
    if self.m_avida and (self.m_analyze_genotype is not None):
      self.setFrames()


      class ProgressCallback:
        def __init__(self, qobj):
          self.qobj = qobj
          # FIXME : remove hard-coding of 2000 below. @kgn
          self.progress_bar = QProgressBar(2000)
          self.progress_bar.setMaximumHeight(13)
          QToolTip.add(self.progress_bar,"Loading movie")
          self.qobj.emit(PYSIGNAL("addStatusBarWidgetSig"), (self.progress_bar, 0, True))
        def clear(self):
          self.qobj.emit(PYSIGNAL("removeStatusBarWidgetSig"), (self.progress_bar,))
          pt = QPoint()
          self.progress_bar.reparent(None, pt)
          del self.progress_bar
        def __call__(self, step):
          self.progress_bar.setProgress(step)
          qApp.processEvents()


      # If and Avida population is running, pause it.
      is_avida_updating = self.m_avida.shouldUpdate()
      is_avida_alive = False
      if self.m_avida.m_population:
        if 0 < self.m_avida.m_population.GetNumOrganisms():
          is_avida_alive = True
      if is_avida_updating and is_avida_alive:
        self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("doPauseAvidaSig"), ())
        self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("statusBarMessageSig"), ("Pausing Avida...",))

        progress_callback = ProgressCallback(self.m_session_mdl.m_session_mdtr)
        # Give the user something to watch while we wait for Avida population
        # to pause. Calling the progress callback also causes the gui to
        # process pending events, so the gui doesn't completely lock-up.
        while self.m_avida.isUpdating():
          progress_callback(0)

        progress_callback.clear()

      # Save random number generator state.
      random_number_generator_state = cRandom(cTools.globalRandom())
      # Reset random number generator state.
      if self.m_seed_based_on_time:
        cTools.globalRandom().ResetSeed(-1)
      else:
        cTools.globalRandom().ResetSeed(1)

      # Tell user we're about to start organism analysis.
      self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("setOneOrganismViewNameLabelTextSig"),
      (self.m_organism_name,))
      self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("statusBarMessageSig"), ("Analyzing organism...",))
      progress_callback = ProgressCallback(self.m_session_mdl.m_session_mdtr)

      # Analyze organism.
      hardware_tracer = pyHardwareTracer(progress_callback)
      hardware_tracer.setTestCPUCopyMutationRate(self.m_test_cpu_mutation_rate)
      hardware_tracer.traceAnalyzeGenotype(self.m_analyze_genotype, self.m_avida.m_environment, should_use_resources = False)

      progress_callback(2000)

      self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("statusBarMessageSig"), ("Setting up organism scope...",))
      qApp.processEvents()

      # Translate from string genome representation to actual command names.
      inst_names = {}
      ops = {}
      inst_set = self.m_avida.m_environment.GetInstSet()
      instruction = cInstruction()
      for id in xrange(inst_set.GetSize()):
        instruction.SetOp(id)
        inst_names[instruction.GetSymbol()] = str(inst_set.GetName(instruction))
        ops[instruction.GetSymbol()] = id

      self.setInstNames(inst_names)
      self.setOps(ops)
      self.setFrames(hardware_tracer.m_hardware_trace)

      self.m_timeline.reset()
      self.flagEvents()
#      self.m_timeline.addFlag(TimelineFlag("8", 0, "Test beginning"))
#      self.m_timeline.addFlag(TimelineFlag("9", self.m_frames.getSnapshotCount(), "Test end"))


      progress_callback.clear()
      self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("statusBarClearSig"), ())

      # Restore random number generator state.
      cTools.globalRandom().Clone(random_number_generator_state)

      # If we paused a running population, restart it.
      if is_avida_updating and is_avida_alive:
        self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("doStartAvidaSig"), ())
        self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("statusBarMessageSig"), ("Unpaused Avida after organism analysis.",))


    else:
      # XXX Temporary. @kgn
      warning_message_box = QMessageBox.warning(
        self,
        "Avida-ED Beta Limitation",
        """In this beta version, you must start an Avida population before you can analyze an organism.
(After you start a population, if you want you can pause it.)""",
        QMessageBox.Ok,
        QMessageBox.NoButton,
        QMessageBox.NoButton
      )

  def MutationSliderValueChangedSlot(self, value):
    self.m_test_cpu_mutation_rate = value / 100.
    descr("self.m_test_cpu_mutation_rate", self.m_test_cpu_mutation_rate)

  def RandomSeedSlot(self, value):
    self.m_seed_based_on_time = value
    descr("self.m_seed_based_on_time", self.m_seed_based_on_time)

  #def HeadsTypeCBActivatedSlot(self, index):
  #  self.anim.setDisplayHeadsAs(index)

  #def ShowTaskTestsCBToggledSlot(self, bool):
  #  self.anim.setShowTaskTestsCBToggled(bool)

  #def ShowRegistersCBToggledSlot(self, bool):
  #  self.anim.setShowRegistersCBToggled(bool)

  #def AnimateHeadMovementCBToggledSlot(self, bool):
  #  pass

  #def ShowStacksCBToggledSlot(self, bool):
  #  self.anim.setShowStacksCBToggled(bool)

  #def ShowHeadsAsLettersCBToggledSlot(self, bool):
  #  pass

  #def ShowInstructionNamesCBToggledSlot(self, bool):
  #  self.anim.setShowInstructionNamesCBToggled(bool)

  #def ShowInputsAndOutputsCBToggledSlot(self, bool):
  #  self.anim.setShowInputsAndOutputsCBToggled(bool)

  #def ShowFullStacksCBToggledSlot(self, bool):
  #  self.anim.setShowFullStacksCBToggled(bool)

  #def AnimateInstructionCopyCBToggledSlot(self, bool):
  #  pass

  #def ShowHardwareCBToggledSlot(self, bool):
  #  self.anim.setShowHardwareCBToggled(bool)

  #def AnimateOrganismDivideCBToggledSlot(self, bool):
  #  pass

  #def LayoutSpacingSBValueChangedSlot(self, value):
  #  pass

  #def HardwareIndicatorSBValueChangedSlot(self, value):
  #  self.anim.setHardwareIndicatorSBValueChanged(value)

  def flagEvents(self):
    "Flag events on timeline"
    if self.m_frames is not None:
      # task_completed holds frame number for when a task was first completed
      task_completed = []
      task_lib = self.m_avida.m_environment.GetTaskLib()
      num_tasks = task_lib.GetSize()
      for task in xrange(num_tasks):
#        print "task: %d, %s" % (task, task_lib.GetTask(task).GetName())
        task_completed.append(False)
      for task in xrange(num_tasks):
        for frame in xrange(self.m_frames.getSnapshotCount()):
          if self.m_frames.m_tasks_info[frame][task] > 0:
            if task_completed[task] == False:
              task_completed[task] = True
              self.m_timeline.addFlag(
#                TimelineFlag("timeline_arrow.png", frame,
                TimelineFlag(str(task), frame,
                             "Completed first %s task" % (task_lib.GetTask(task).GetDesc())))
              # Task completed, don't need to search any more frames for this
              # task
              break
