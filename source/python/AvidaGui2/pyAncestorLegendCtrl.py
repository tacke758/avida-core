# -*- coding: utf-8 -*-

from qt import *
from pyAncestorLegendView import pyAncestorLegendView

from descr import *

class pyAncestorLegendCtrl(pyAncestorLegendView):

  def __init__(self,parent = None,name = None,fl = 0):
    pyAncestorLegendView.__init__(self,parent,name,fl)
    QToolTip.add(self, "<p>Legend for the ancestor map mode.</p>")


  def construct(self, session_mdl):
    descr("RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR")
    self.m_session_mdl = session_mdl
    self.m_avida = None
    self.m_current_map_mode_name = None
    self.connect(
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("setAvidaSig"),
      self.setAvidaSlot)
    self.connect(self.m_session_mdl.m_session_mdtr,
       PYSIGNAL("mapModeChangedSig"), self.setMapModeSlot)


  def setAvidaSlot(self, avida):
    print "pyAncestorLegendCtrl.setAvidaSlot() ..."
    old_avida = self.m_avida
    self.m_avida = avida
    if(old_avida):
      print "pyAncestorLegendCtrl.setAvidaSlot() disconnecting old_avida ..."
      self.disconnect(
        self.m_avida.m_avida_thread_mdtr, PYSIGNAL("AvidaUpdatedSig"),
        self.avidaUpdatedSlot)
      del old_avida
    if(self.m_avida):
      print "pyAncestorLegendCtrl.setAvidaSlot() connecting self.m_avida ..."
      self.connect(
        self.m_avida.m_avida_thread_mdtr, PYSIGNAL("AvidaUpdatedSig"),
        self.avidaUpdatedSlot)

  def setMapModeSlot(self,index):
    self.m_current_map_mode_name = index

  def avidaUpdatedSlot(self):
    pass

  def destruct(self):
    print "*** called pyAncestorLegendCtrl.py:destruct ***"
    self.m_avida = None
    self.disconnect(
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("setAvidaSig"),
      self.setAvidaSlot)
    self.m_session_mdl = None
  

