# -*- coding: utf-8 -*-

from descr import descr

from qt import *
from pyOneAnalyzeView import pyOneAnalyzeView
import os.path

class pyOneAnalyzeCtrl(pyOneAnalyzeView):

  def __init__(self,parent = None,name = None,fl = 0):
    pyOneAnalyzeView.__init__(self,parent,name,fl)
    self.setAcceptDrops(1)

  def construct(self, session_mdl):
    self.m_session_mdl = session_mdl
    self.m_one_ana_graph_ctrl.construct(self.m_session_mdl)
    self.m_one_ana_petri_ctrl.construct(self.m_session_mdl) 
    # XXX temporarily disabled nonfunctioning gui element, reenable in
    # future when it works. @kgn
    self.m_one_ana_timeline_ctrl.hide()
    self.m_one_ana_stats_ctrl.hide()
    self.connect( self, PYSIGNAL("freezerItemDroppedInOneAnalyzeSig"),
      self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("freezerItemDroppedInOneAnalyzeSig"))
    self.connect( self, PYSIGNAL("freezerItemDoubleClickedOnInOneAnaSig"),
      self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("freezerItemDoubleClickedOnInOneAnaSig"))
    self.connect( self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("freezerItemDoubleClicked"),
      self.freezerItemDoubleClicked)

  def aboutToBeLowered(self):
    """Disconnects "Print Graph..." menu item from One-Analyze Graph controller."""
    descr()
    self.disconnect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("printGraphSig"),
      self.m_one_ana_graph_ctrl.printGraphSlot)
  def aboutToBeRaised(self):
    """Connects "Print Graph..." menu item to One-Analyze Graph controller."""
    descr()
    self.connect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("printGraphSig"),
      self.m_one_ana_graph_ctrl.printGraphSlot)

  def dragEnterEvent( self, e ):
    descr(e)

    freezer_item_name = QString()
    if ( QTextDrag.decode( e, freezer_item_name ) ) : #freezer_item_name is a string...the file name 
      if os.path.exists(str(freezer_item_name)) == False:
        descr("that was not a valid path (1)")
      else: 
        e.acceptAction(True)
        descr("accepted.")


  def dropEvent( self, e ):
    freezer_item_name = QString()
    if ( QTextDrag.decode( e, freezer_item_name ) ) :
      if os.path.exists( str(freezer_item_name)) == False:
        print "that was not a valid path(3)" 
      else: 
        self.emit(PYSIGNAL("freezerItemDroppedInOneAnalyzeSig"), (e,))

  def freezerItemDoubleClicked(self, freezer_item_name):
    if os.path.exists( str(freezer_item_name)) == False:
      print "that was not a valid path(3)"
    else:
      if self.isVisible():
        self.emit(PYSIGNAL("freezerItemDoubleClickedOnInOneAnaSig"), 
          (freezer_item_name,))
      


