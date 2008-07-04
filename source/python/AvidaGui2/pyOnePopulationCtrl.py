# -*- coding: utf-8 -*-

from descr import descr, warning

from pyAvida import pyAvida
from qt import *
from pyOnePopulationView import pyOnePopulationView
from pyButtonListDialog import pyButtonListDialog
from pyGraphCtrl import PrintFilter
from pyImageFileDialog import pyImageFileDialog
import pyNewIconView
import os.path

class pyOnePopulationCtrl(pyOnePopulationView):

  def __init__(self,parent = None,name = None,fl = 0):
    pyOnePopulationView.__init__(self,parent,name,fl)
    self.setAcceptDrops(1)

  def construct(self, session_mdl):
    self.m_session_mdl = session_mdl
    self.m_one_pop_petri_dish_ctrl.construct(self.m_session_mdl)
    self.m_one_pop_graph_ctrl.construct(self.m_session_mdl)
    self.m_one_pop_stats_ctrl.construct(self.m_session_mdl)
    # XXX temporarily disabled nonfunctioning gui element, reenable in
    # future when it works. @kgn
    self.m_one_pop_timeline_ctrl.hide()
    self.connect( self, PYSIGNAL("petriDishDroppedInPopViewSig"),
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("petriDishDroppedInPopViewSig"))   
    self.connect( self, PYSIGNAL("freezerItemDoubleClickedOnInOnePopSig"), 
      self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("freezerItemDoubleClickedOnInOnePopSig"))
    self.connect( self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("freezerItemDoubleClicked"), self.freezerItemDoubleClickedSlot)
    self.connect(self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("restartPopulationSig"), self.restartPopulationSlot)

  def aboutToBeLowered(self, workspace_ctrl):
    """Disconnects menu items from One-Pop Graph controller."""
    descr()
    self.disconnect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("printSig"),
      self.printSlot)
    self.disconnect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("printPetriDishSig"),
      self.m_one_pop_petri_dish_ctrl.printPetriDishSlot)
    self.disconnect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("printGraphSig"),
      self.m_one_pop_graph_ctrl.printGraphSlot)
    self.disconnect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("saveImagesSig"),
      self.saveImagesSlot)
    self.disconnect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("exportAnalyzeSig"),
      self.m_one_pop_graph_ctrl.exportSlot)

    workspace_ctrl.popview_controlStartAction.setVisible(False)
    workspace_ctrl.popview_controlNext_UpdateAction.setVisible(False)
    workspace_ctrl.popview_controlRestart_ExpAction.setVisible(False)

  def aboutToBeRaised(self, workspace_ctrl):
    """Connects menu items to One-Pop Graph controller."""
    descr()
    self.connect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("printSig"),
      self.printSlot)
    self.connect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("printPetriDishSig"),
      self.m_one_pop_petri_dish_ctrl.printPetriDishSlot)
    self.connect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("printGraphSig"),
      self.m_one_pop_graph_ctrl.printGraphSlot)    
    self.connect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("saveImagesSig"),
      self.saveImagesSlot)
    self.connect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("exportAnalyzeSig"),
      self.m_one_pop_graph_ctrl.exportSlot)

    workspace_ctrl.popview_controlStartAction.setVisible(True)
    workspace_ctrl.popview_controlNext_UpdateAction.setVisible(True)
    workspace_ctrl.popview_controlRestart_ExpAction.setVisible(True)

  def dragEnterEvent( self, e ):

    freezer_item_list = QString()

    # freezer_item_list is a string...tab delimited list of file names

    if ( QTextDrag.decode( e, freezer_item_list ) ) :
      freezer_item_list = str(e.encodedData("text/plain"))
      errors = False
      for freezer_item_name in freezer_item_list.split("\t")[1:]:
        if os.path.exists(freezer_item_name) == False:
            errors = True
            warningNoMethodName(freezer_item_name + " does not exist.")
      if not(errors):
        e.acceptAction(True)

  def dropEvent( self, e ):
    freezer_item_list = QString()

    #freezer_item_name is a string...the file name 

    if ( QTextDrag.decode( e, freezer_item_list ) ) :
      freezer_item_list = str(e.encodedData("text/plain"))
      errors = False
      for freezer_item_name in freezer_item_list.split("\t")[1:]:
        if os.path.exists(freezer_item_name) == False:
            errors = True
            warningNoMethodName(freezer_item_name + " does not exist.")
      if not(errors): 
        self.emit(PYSIGNAL("petriDishDroppedInPopViewSig"), (e,))
    
    # Check if item is icon

    if (pyNewIconView.canDecode(e)):
      info("You can not drop Ancestors here")

  def freezerItemDoubleClickedSlot(self, freezer_item_name):
   if self.isVisible():
     self.emit(PYSIGNAL("freezerItemDoubleClickedOnInOnePopSig"), 
       (freezer_item_name,))

  def restartPopulationSlot(self, session_mdl):
    descr()
    self.m_one_pop_petri_dish_ctrl.restart(self.m_session_mdl)
    self.m_one_pop_graph_ctrl.restart()
    self.m_one_pop_stats_ctrl.restart()
    self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("doSyncSig"), ())
    # self.m_session_mdl.m_session_mdtr.emit(
    #   PYSIGNAL("doInitializeAvidaPhaseISig"), (self.m_session_mdl.m_tempdir,))

  def saveImagesSlot(self):
    "Save petri dish or graph to image file"
    dlg = pyButtonListDialog("Export Graphics", "Choose object to save",
                             ["Petri Dish", "Graph"])
    res = dlg.showDialog()
    if len(res) == 0:
      return

    img_dlg = pyImageFileDialog()

    # get population name
    name = str(self.m_one_pop_petri_dish_ctrl.PopulationTextLabel.text())

    # get update
    stats = update = None
    if self.m_one_pop_petri_dish_ctrl.m_avida:
      stats = self.m_one_pop_petri_dish_ctrl.m_avida.m_population.GetStats()
    if stats: update = stats.GetUpdate()

    # Save the image
    if res[0] == "Petri Dish":
      filename, type = img_dlg.saveImageDialog(
        name + "-" + "petridish-update" + str(update))
      p = self.m_one_pop_petri_dish_ctrl.getPetriDishPixmap()
      if filename:
        p.save(filename, type, 100)
    else:
      filename, type = img_dlg.saveImageDialog(
        name + "-" + "graph-update" + str(update))
      if filename:
        self.m_one_pop_graph_ctrl.m_graph_ctrl.saveImage(filename, type)

  def printSlot(self):
    "Let user choose what object to print and send signal to appropriate slot"
    dlg = pyButtonListDialog("Print", "Choose object to print",
                             ["Petri Dish", "Graph"])
    res = dlg.showDialog()
    if res[0] == "Petri Dish":
      self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("printPetriDishSig"), ())
    elif res[0] == "Graph":
      self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("printGraphSig"), ())
