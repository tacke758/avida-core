# -*- coding: utf-8 -*-

from pyOneOrg_ScopeView import pyOneOrg_ScopeView
from pyTimeline import pyTimeline
from descr import *
from qt import *
#import qt
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
    # Use background color so timeline meter doesn't show up
    self.m_timeline.setFillColor(self.m_timeline.colorGroup().background())
#    self.m_timeline.setScalePosition(pyTimeline.None)
    self.m_timer = QTimer()
    self.m_next = QTimer()
    self.m_timer_interval = 100

    self.m_organism_scope_ctrl.m_timeline = self.m_timeline

    self.connect(
      self.m_execution_step_slider, SIGNAL("valueChanged(int)"),
      self.sliderValueChangedSlot)
    self.connect(
      self.m_execution_step_slider, SIGNAL("sliderMoved(int)"),
      self.sliderMovedSlot)

    self.connect(
      self.m_organism_scope_ctrl, PYSIGNAL("gestationTimeChangedSig"),
      self.gestationTimeChangedSlot)
    self.connect(
      self.m_organism_scope_ctrl, PYSIGNAL("executionStepResetSig"),
      self.executionStepResetSlot)
    self.connect(
      self.m_organism_scope_ctrl, PYSIGNAL("frameShownSig"),
      self.m_organism_data_ctrl.frameShownSlot)


    self.connect(
      self.m_analyze_controls_ctrl.m_rewind_btn, SIGNAL("clicked()"),
      self.rewindSlot)
    self.connect(
      self.m_analyze_controls_ctrl.m_cue_btn, SIGNAL("clicked()"),
      self.cueSlot)
    self.connect(
      self.m_analyze_controls_ctrl.m_play_btn, SIGNAL("clicked()"),
      self.playSlot)
    self.connect(
      self.m_analyze_controls_ctrl.m_pause_btn, SIGNAL("clicked()"),
      self.pauseSlot)
    self.connect(
      self.m_analyze_controls_ctrl.m_prev_btn, SIGNAL("clicked()"),
      self.backSlot)
    self.connect(
      self.m_analyze_controls_ctrl.m_next_btn, SIGNAL("clicked()"),
      self.advanceSlot)

    self.connect(self.m_timer, SIGNAL("timeout()"), self.advanceSlot)


  def sliderValueChangedSlot(self, frame_number):
#    print "pyOneOrg_ScopeCtrl.sliderValueChangedSlot(", frame_number, ")."
    self.m_organism_scope_ctrl.showFrame(frame_number)
    self.m_timeline.setValue(frame_number)


  def gestationTimeChangedSlot(self, gestation_time):
    descr("pyOneOrg_ScopeCtrl.gestationTimeChangedSlot called, gestation_time " + str(gestation_time))
    self.m_execution_step_slider.setMaxValue(gestation_time - 1)
    self.m_timeline.setMaxValue(gestation_time - 1)
    self.rewindSlot()

  def executionStepResetSlot(self, execution_step):
    descr("pyOneOrg_ScopeCtrl.executionStepResetSlot called, execution_step " + str(execution_step))
    self.m_execution_step_slider.setValue(execution_step)
    self.m_timeline.setValue(execution_step)
    # This may be redundant (I'm not sure). @kgn
    self.m_execution_step_slider.emit(SIGNAL("valueChanged(int)"),(execution_step,))

  def sliderMovedSlot(self, frame_number):
    descr("pyOneOrg_ScopeCtrl.sliderMovedSlot().")
    self.pauseSlot()

  def rewindSlot(self):
    descr("pyOneOrg_ScopeCtrl.rewindSlot().")
    self.m_execution_step_slider.setValue(0)
    self.m_timeline.setValue(0)
    self.pauseSlot()

  def cueSlot(self):
    descr("pyOneOrg_ScopeCtrl.cueSlot().")
    self.m_execution_step_slider.setValue(self.m_execution_step_slider.maxValue())
    self.m_timeline.setValue(self.m_timeline.maxValue())
    self.pauseSlot()

  def pauseSlot(self):
    descr("pyOneOrg_ScopeCtrl.pauseSlot().")
    self.m_analyze_controls_ctrl.m_widget_stack.raiseWidget(self.m_analyze_controls_ctrl.m_play_page)
    self.m_timer.stop()
    self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("orgScopePausedSig"), ())

  def playSlot(self):
    descr("pyOneOrg_ScopeCtrl.playSlot().")
    self.m_analyze_controls_ctrl.m_widget_stack.raiseWidget(self.m_analyze_controls_ctrl.m_pause_page)
    self.m_timer.start(self.m_timer_interval, False)
    self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("orgScopeStartedSig"), ())

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

  def getOrganismPixmap(self):
    "Return QPixmap of organism"
    #height = self.m_organism_scope_ctrl.height()
    # Hide the scrollbars so they aren't painted
    #self.m_petri_dish_ctrl.m_petri_dish_ctrl_h_scrollBar.hide()
    #self.m_petri_dish_ctrl.m_petri_dish_ctrl_v_scrollBar.hide()
    pix = QPixmap.grabWidget(
      self.m_organism_scope_ctrl,
      0, 0,
      self.m_organism_scope_ctrl.width(),
      self.m_organism_scope_ctrl.height()
    )
    #self.m_petri_dish_ctrl.m_petri_dish_ctrl_h_scrollBar.show()
    #self.m_petri_dish_ctrl.m_petri_dish_ctrl_v_scrollBar.show()
    #scale_pix = QPixmap.grabWidget(self.m_gradient_scale_ctrl, 0, 0,
    #                               self.m_gradient_scale_ctrl.width(),
    #                               self.m_gradient_scale_ctrl.height())
    #p = QPixmap(max(self.m_petri_dish_ctrl.m_canvas_view.width(),
    #                self.m_gradient_scale_ctrl.width()),
    #            dish_height + self.m_gradient_scale_ctrl.height())
    #painter = QPainter(p)
    #painter.drawPixmap(0, 0, dish_pix)
    #painter.drawPixmap(0, dish_height, scale_pix)
    #painter.end()
    return pix

