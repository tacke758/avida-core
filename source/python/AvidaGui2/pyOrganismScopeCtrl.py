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

  def construct(self, session_mdl):
    print "pyOrganismScopeCtrl.construct()."
    self.m_session_mdl = session_mdl
    self.m_avida = None
    self.setAcceptDrops(1)

    self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("setAvidaSig"), self.setAvidaSlot)
    self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("setDebugOrganismFileSig"), self.setDebugOrganismFileSlot)

    self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("ScopeConfig_MutationSliderValueChangedSig"),
      self.MutationSliderValueChangedSlot)

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
         warningNoMethodName("Only one organism can be dragged here")
      else:
        freezer_item_name = freezer_item_names[0]
        if os.path.exists(freezer_item_name) == False:
          warningNoMtehodName(freezer_item_name + " does not exist")
        else:
          if freezer_item_name.endswith('.organism'):
            e.accept()
            self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("setDebugOrganismFileSig"), (freezer_item_name,))
          else:
            warningNoMethodName("Only organisms can be dragged here")

  def setAvidaSlot(self, avida):
    print "pyOrganismScopeCtrl.setAvidaSlot() ..."
    old_avida = self.m_avida
    self.m_avida = avida
    if(old_avida):
      print "pyOrganismScopeCtrl.setAvidaSlot() deleting old_avida ..."
      del old_avida

  def setDebugOrganismFileSlot(self, organism_filename):
    print "pyOrganismScopeCtrl.setDebugOrganismFileSlot"
    if self.m_avida:
      self.setFrames()

      inst_set = self.m_avida.m_environment.GetInstSet()
      org_file = open(organism_filename)
      org_string = org_file.readline()
      org_string = org_string.rstrip()
      org_string = org_string.lstrip()
      org_file.close
      genome = cGenome(cString(org_string))
      analyze_genotype = cAnalyzeGenotype(genome, inst_set)


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
      if is_avida_updating:
        self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("statusBarMessageSig"), ("Pausing Avida...",))
        self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("doPauseAvidaSig"), ())
        progress_callback = ProgressCallback(self.m_session_mdl.m_session_mdtr)
        # Give the user something to watch while we wait for Avida population
        # to pause. Calling the progress callback also causes the gui to
        # process pending events, so the gui doesn't completely lock-up.
        while self.m_avida.isUpdating():
          progress_callback(0)
        progress_callback.clear()

      # Save random number generator state.
      random_number_generator_state = cRandom(cTools.globalRandom())

      # Tell user we're about to start organism analysis.
      self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("setOneOrganismViewNameLabelTextSig"), (organism_filename,))
      self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("statusBarMessageSig"), ("Analyzing organism...",))
      progress_callback = ProgressCallback(self.m_session_mdl.m_session_mdtr)

      # Analyze organism.
      hardware_tracer = pyHardwareTracer(progress_callback)
      hardware_tracer.setTestCPUCopyMutationRate(self.m_test_cpu_mutation_rate)
      hardware_tracer.traceAnalyzeGenotype(analyze_genotype, self.m_avida.m_environment, should_use_resources = False)

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

      progress_callback.clear()
      self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("statusBarClearSig"), ())

      # Restore random number generator state.
      cTools.globalRandom().Clone(random_number_generator_state)

      # If we paused a running population, restart it.
      if is_avida_updating:
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
