# -*- coding: utf-8 -*-

from qt import *
from pyMapLegendView import pyMapLegendView


class pyMapLegendCtrl(pyMapLegendView):

  def __init__(self,parent = None,name = None,fl = 0):
    pyMapLegendView.__init__(self,parent,name,fl)
    QToolTip.add(self, "<p>Dynamic scale for current variable (scale changes as maximum value in population increases or decreases)</p>")


  def construct(self, session_mdl):
    self.m_session_mdl = session_mdl
    self.m_avida = None
    self.m_current_map_mode_name = None
    self.connect(
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("setAvidaSig"),
      self.setAvidaSlot)
    self.connect(self.m_session_mdl.m_session_mdtr,
       PYSIGNAL("mapModeChangedSig"), self.setMapModeSlot)


  def setAvidaSlot(self, avida):
    print "pyMapLegendCtrl.setAvidaSlot() ..."
    old_avida = self.m_avida
    self.m_avida = avida
    if(old_avida):
      print "pyMapLegendCtrl.setAvidaSlot() disconnecting old_avida ..."
      self.disconnect(
        self.m_avida.m_avida_thread_mdtr, PYSIGNAL("AvidaUpdatedSig"),
        self.avidaUpdatedSlot)
      del old_avida
    if(self.m_avida):
      print "pyMapLegendCtrl.setAvidaSlot() connecting self.m_avida ..."
      self.connect(
        self.m_avida.m_avida_thread_mdtr, PYSIGNAL("AvidaUpdatedSig"),
        self.avidaUpdatedSlot)

  def setMapModeSlot(self,index):
    self.m_current_map_mode_name = index

  def avidaUpdatedSlot(self):
    pass

  def destruct(self):
    print "*** called pyMapLegendCtrl.py:destruct ***"
    self.m_avida = None
    self.disconnect(
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("setAvidaSig"),
      self.setAvidaSlot)
    self.m_session_mdl = None
  

