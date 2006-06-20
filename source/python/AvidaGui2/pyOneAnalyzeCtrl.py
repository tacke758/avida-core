# -*- coding: utf-8 -*-

from descr import descr

from qt import *
from pyOneAnalyzeView import pyOneAnalyzeView
from pyImageFileDialog import pyImageFileDialog
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
    """Disconnects menu items from One-Analyze Graph controller."""
    descr()
    self.disconnect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("saveImagesSig"),
      self.saveImagesSlot)
    self.disconnect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("printSig"),
      self.m_one_ana_graph_ctrl.printSlot)
    # Disconnect export signal
    self.disconnect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("exportAnalyzeSig"),
      self.m_one_ana_graph_ctrl.exportSlot)
  def aboutToBeRaised(self):
    """Connects items to One-Analyze Graph controller."""
    descr()
    self.connect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("saveImagesSig"),
      self.saveImagesSlot)
    self.connect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("printSig"),
      self.m_one_ana_graph_ctrl.printSlot)
    # Connect export
    self.connect(self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("exportAnalyzeSig"),
      self.m_one_ana_graph_ctrl.exportSlot)

  def dragEnterEvent( self, e ):
    descr(e)

    freezer_item_name = QString()
    if ( QTextDrag.decode( e, freezer_item_name ) ) : #freezer_item_name is a string...the file name 
      freezer_item_name = str(e.encodedData("text/plain"))
      if os.path.exists(freezer_item_name) == False:
        descr("that was not a valid path (1)")
      else: 
        e.acceptAction(True)
        descr("accepted.")

  def dropEvent( self, e ):
    freezer_item_name = QString()
    if ( QTextDrag.decode( e, freezer_item_name ) ) :
      freezer_item_name = str(e.encodedData("text/plain"))
      if os.path.exists(freezer_item_name) == False:
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

  def saveImagesSlot(self):
    "Save image of graph"
    dlg = pyImageFileDialog()
    filename, type = dlg.saveImageDialog()
    if filename:
      self.m_one_ana_graph_ctrl.m_graph_ctrl.saveImage(filename, type)
