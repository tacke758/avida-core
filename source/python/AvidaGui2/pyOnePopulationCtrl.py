# -*- coding: utf-8 -*-

from descr import descr

from pyAvida import pyAvida
from qt import *
from pyOnePopulationView import pyOnePopulationView
from pyButtonListDialog import pyButtonListDialog
from pyGraphCtrl import PrintFilter
from pyImageFileDialog import pyImageFileDialog
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
      PYSIGNAL("freezerItemDoubleClicked"), self.freezerItemDoubleClicked)
    self.connect(self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("restartPopulationSig"), self.restartPopulationSlot)

  def aboutToBeLowered(self):
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
  def aboutToBeRaised(self):
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

  def dragEnterEvent( self, e ):
    descr(e)
    #e.acceptAction(True)
    #if e.isAccepted():
    #  descr("isAccepted.")
    #else:
    #  descr("not isAccepted.")

    freezer_item_name = QString()
    if ( QTextDrag.decode( e, freezer_item_name ) ) : #freezer_item_name is a string...the file name 
      freezer_item_name = str(e.encodedData("text/plain"))
      if os.path.exists(freezer_item_name) == False:
        descr("that was not a valid path (1)")
      else: 
        e.acceptAction(True)
        descr("accepted.")


  def dropEvent( self, e ):
    descr(e)
    freezer_item_name = QString()
    if ( QTextDrag.decode( e, freezer_item_name ) ) : #freezer_item_name is a string...the file name 
      freezer_item_name = str(e.encodedData("text/plain"))
      if os.path.exists(freezer_item_name) == False:
        print "that was not a valid path (1)" 
      else: 
        self.emit(PYSIGNAL("petriDishDroppedInPopViewSig"), (e,))

  def freezerItemDoubleClicked(self, freezer_item_name):
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
    dlg = pyButtonListDialog("Save Image", "Choose object to save",
                             ["Petri Dish", "Graph"])
    res = dlg.showDialog()

    img_dlg = pyImageFileDialog()
    filename, type = img_dlg.saveImageDialog()

    # Save the image
    if res[0] == "Petri Dish":
      p = self.m_one_pop_petri_dish_ctrl.getPetriDishPixmap()
      p.save(filename, type, 100)
    else:
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
