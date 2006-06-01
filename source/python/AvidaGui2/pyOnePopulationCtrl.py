# -*- coding: utf-8 -*-

from descr import descr

from pyAvida import pyAvida
from qt import *
from pyOnePopulationView import pyOnePopulationView
from pyButtonListDialog import pyButtonListDialog
from pyGraphCtrl import PrintFilter
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
      PYSIGNAL("printGraphSig"),
      self.m_one_pop_graph_ctrl.printGraphSlot)
    self.disconnect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("printPetriDishSig"),
      self.m_one_pop_petri_dish_ctrl.printPetriDishSlot)
    self.disconnect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("saveImagesSig"),
      self.saveImagesSlot)
  def aboutToBeRaised(self):
    """Connects menu items to One-Pop Graph controller."""
    descr()
    self.connect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("printGraphSig"),
      self.m_one_pop_graph_ctrl.printGraphSlot)
    self.connect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("printPetriDishSig"),
      self.m_one_pop_petri_dish_ctrl.printPetriDishSlot)
    self.connect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("saveImagesSig"),
      self.saveImagesSlot)

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
    # Let user select which image to save
    dlg = pyButtonListDialog("Save Image", "Choose object to save",
                             ["Petri Dish", "Graph"])
    res = dlg.showDialog()

    # Let user select file format
    dialog_caption = "Export Image"
    fd = QFileDialog.getSaveFileName("", "JPEG (*.jpg);;PNG (*.png)", None,
                                     "Save As", dialog_caption)
    filename = str(fd)
    if filename == "":
      return

    if filename[-4:].lower() == ".jpg":
      type = "JPEG"
    elif filename[-4:].lower() == ".png":
      type = "PNG"
    else:
      filename += ".jpg"
      type = "JPEG"

    # Save the image
    if res[0] == "Petri Dish":
      p = self.m_one_pop_petri_dish_ctrl.getPetriDishPixmap()
      p.save(filename, type, 100)
    else:
      p = QPixmap.grabWidget(self.m_one_pop_graph_ctrl.m_graph_ctrl, 0, 0,
                             self.m_one_pop_graph_ctrl.m_graph_ctrl.width(),
                             self.m_one_pop_graph_ctrl.m_graph_ctrl.height())
      p.save(filename, type, 100)
