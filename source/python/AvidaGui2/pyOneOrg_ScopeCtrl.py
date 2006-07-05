# -*- coding: utf-8 -*-

from pyOneOrg_ScopeView import pyOneOrg_ScopeView
from pyTimeline import pyTimeline
import qt
import os

class pyOneOrg_ScopeCtrl(pyOneOrg_ScopeView):

  def __init__(self,parent = None,name = None,fl = 0):
    pyOneOrg_ScopeView.__init__(self,parent,name,fl)

  def construct(self, session_mdl):
    self.m_session_mdl = session_mdl
    self.m_organism_scope_ctrl.construct(self.m_session_mdl)
    self.m_execution_step_slider.setMinValue(0)
    self.m_execution_step_slider.setMaxValue(0)
    self.m_timeline.setMinValue(0)
    self.m_timeline.setMaxValue(0)
    self.m_timer = qt.QTimer()
    self.m_next = qt.QTimer()
    self.m_timer_interval = 100

    self.m_organism_scope_ctrl.m_timeline = self.m_timeline

    self.connect(
      self.m_execution_step_slider, qt.SIGNAL("valueChanged(int)"),
      self.sliderValueChangedSlot)
    self.connect(
      self.m_execution_step_slider, qt.SIGNAL("sliderMoved(int)"),
      self.sliderMovedSlot)

    self.connect(
      self.m_organism_scope_ctrl, qt.PYSIGNAL("gestationTimeChangedSig"),
      self.gestationTimeChangedSlot)
    self.connect(
      self.m_organism_scope_ctrl, qt.PYSIGNAL("executionStepResetSig"),
      self.executionStepResetSlot)
    self.connect(
      self.m_organism_scope_ctrl, qt.PYSIGNAL("frameShownSig"),
      self.m_organism_data_ctrl.frameShownSlot)


    self.connect(
      self.m_analyze_controls_ctrl.m_rewind_btn, qt.SIGNAL("clicked()"),
      self.rewindSlot)
    self.connect(
      self.m_analyze_controls_ctrl.m_cue_btn, qt.SIGNAL("clicked()"),
      self.cueSlot)
    self.connect(
      self.m_analyze_controls_ctrl.m_play_btn, qt.SIGNAL("clicked()"),
      self.playSlot)
    self.connect(
      self.m_analyze_controls_ctrl.m_pause_btn, qt.SIGNAL("clicked()"),
      self.pauseSlot)
    self.connect(
      self.m_analyze_controls_ctrl.m_prev_btn, qt.SIGNAL("clicked()"),
      self.backSlot)
    self.connect(
      self.m_analyze_controls_ctrl.m_next_btn, qt.SIGNAL("clicked()"),
      self.advanceSlot)

    self.connect(self.m_timer, qt.SIGNAL("timeout()"), self.advanceSlot)


  def sliderValueChangedSlot(self, frame_number):
#    print "pyOneOrg_ScopeCtrl.sliderValueChangedSlot(", frame_number, ")."
    self.m_organism_scope_ctrl.showFrame(frame_number)
    self.m_timeline.setValue(frame_number)


  def gestationTimeChangedSlot(self, gestation_time):
    print "pyOneOrg_ScopeCtrl.gestationTimeChangedSlot called, gestation_time ", gestation_time
    self.m_execution_step_slider.setMaxValue(gestation_time - 1)
    self.m_timeline.setMaxValue(gestation_time - 1)
    self.rewindSlot()

  def executionStepResetSlot(self, execution_step):
    print "pyOneOrg_ScopeCtrl.executionStepResetSlot called, execution_step ", execution_step
    self.m_execution_step_slider.setValue(execution_step)
    self.m_timeline.setValue(execution_step)
    # This may be redundant (I'm not sure). @kgn
    self.m_execution_step_slider.emit(qt.SIGNAL("valueChanged(int)"),(execution_step,))

  def sliderMovedSlot(self, frame_number):
    print "pyOneOrg_ScopeCtrl.sliderMovedSlot()."
    self.pauseSlot()

  def rewindSlot(self):
    print "pyOneOrg_ScopeCtrl.rewindSlot()."
    self.m_execution_step_slider.setValue(0)
    self.m_timeline.setValue(0)
    self.pauseSlot()

  def cueSlot(self):
    print "pyOneOrg_ScopeCtrl.cueSlot()."
    self.m_execution_step_slider.setValue(self.m_execution_step_slider.maxValue())
    self.m_timeline.setValue(self.m_timeline.maxValue())
    self.pauseSlot()

  def pauseSlot(self):
    print "pyOneOrg_ScopeCtrl.pauseSlot()."
    self.m_analyze_controls_ctrl.m_widget_stack.raiseWidget(self.m_analyze_controls_ctrl.m_play_page)
    self.m_timer.stop()

  def playSlot(self):
    print "pyOneOrg_ScopeCtrl.playSlot()."
    self.m_analyze_controls_ctrl.m_widget_stack.raiseWidget(self.m_analyze_controls_ctrl.m_pause_page)
    self.m_timer.start(self.m_timer_interval, False)

  def backSlot(self):
    #print "pyOneOrg_ScopeCtrl.backSlot()."
    slider_value = self.m_execution_step_slider.value()
    if slider_value <= self.m_execution_step_slider.minValue():
      self.pauseSlot()
    else:
      self.m_execution_step_slider.setValue(slider_value - 1)
      self.m_timeline.setValue(slider_value - 1)

  def advanceSlot(self):
    #print "pyOneOrg_ScopeCtrl.advanceSlot()."
    slider_value = self.m_execution_step_slider.value()
    if self.m_execution_step_slider.maxValue() <= slider_value:
      self.pauseSlot()
    else:
      self.m_execution_step_slider.setValue(slider_value + 1)
      self.m_timeline.setValue(slider_value + 1)
