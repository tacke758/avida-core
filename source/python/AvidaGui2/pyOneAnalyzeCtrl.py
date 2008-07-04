# -*- coding: utf-8 -*-

from descr import *

from qt import *
from DefaultExportDir import DefaultExportDir
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
    self.connect( self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("itemDroppedInAnalysisViewerMenuBox"),
      self.loadDroppedThing)

  def testMe(self):
    descr("99999999999999999999999999999999999999999999999999999999999")

  def aboutToBeLowered(self, workspace_ctrl):
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

    workspace_ctrl.anaview_controlNo_controls_available_in_Analysis_ViewAction.setVisible(False)

  def aboutToBeRaised(self, workspace_ctrl):
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

    workspace_ctrl.anaview_controlNo_controls_available_in_Analysis_ViewAction.setVisible(True)

    workspace_ctrl.fileExportAction.setVisible(True)

  def dragEnterEvent( self, e ):
    freezer_item_list = QString()
    # freezer_item_list is a string...a tab delimited list of file names 
    if ( QTextDrag.decode( e, freezer_item_list ) ) : 
      freezer_item_list = str(e.encodedData("text/plain"))
      errors = False
      for freezer_item_name in freezer_item_list.split("\t")[1:]:
        if os.path.exists(freezer_item_name) == False:
          errors = True
      if not(errors): 
        e.acceptAction(True)

  def dropEvent( self, e ):
    freezer_item_list = QString()
    if ( QTextDrag.decode( e, freezer_item_list ) ) :
      freezer_item_list = str(e.encodedData("text/plain"))
      descr("freezer_item_list is", freezer_item_list)
      self.loadDroppedThing(freezer_item_list)

  def loadDroppedThing( self, freezer_item_list):
      for freezer_item_name in freezer_item_list.split("\t")[1:]:
        if os.path.exists(freezer_item_name) == False:
          warningNoMethodName(freezer_item_name + " does not exist.")
        else:
          if self.m_one_ana_graph_ctrl.check_file(freezer_item_name):
            self.m_one_ana_petri_ctrl.m_one_ana_pop_name.setText("")
            self.m_one_ana_petri_ctrl.pixmapLabel1.hide()
            self.emit(PYSIGNAL("freezerItemDroppedInOneAnalyzeSig"), 
                      (freezer_item_name,))
          else:
            warningNoMethodName(freezer_item_name + 
              " does not seem to be a populated dish.")

  def freezerItemDoubleClicked(self, freezer_item_list):
    for freezer_item_name in freezer_item_list.split("\t")[1:]:
      if os.path.exists( str(freezer_item_name)) == False:
         warningNoMethodName(freezer_item_name + " does not exist.")
      else:
        if self.isVisible():
          if self.m_one_ana_graph_ctrl.check_file(freezer_item_name):
            self.m_one_ana_petri_ctrl.m_one_ana_pop_name.setText("")
            self.m_one_ana_petri_ctrl.pixmapLabel1.hide()
            self.emit(PYSIGNAL("freezerItemDoubleClickedOnInOneAnaSig"), 
              (freezer_item_name,))
          else:
            warningNoMethodName(freezer_item_name + 
              " does not seem to be a populated dish.")

  def saveImagesSlot(self):
    "Save image of graph"
    initial_dir = DefaultExportDir(self.m_session_mdl.export_directory, "unititled")
    initial_file_name = os.path.join(initial_dir, "unititled")
    dlg = pyImageFileDialog()
    filename, type = dlg.saveImageDialog(initial_file_name)
    if filename:
      self.m_one_ana_graph_ctrl.m_graph_ctrl.saveImage(filename, type)
      self.m_session_mdl.export_directory = os.path.dirname(str(filename))
