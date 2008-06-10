# -*- coding: utf-8 -*-

from descr import descr, info

from qt import *
from pyOneOrganismView import pyOneOrganismView
from pyButtonListDialog import pyButtonListDialog
from pyImageFileDialog import pyImageFileDialog

import os.path

class pyOneOrganismCtrl(pyOneOrganismView):

  def __init__(self,parent = None,name = None,fl = 0):
    pyOneOrganismView.__init__(self,parent,name,fl)

  def construct(self, session_mdl):
    self.m_session_mdl = session_mdl
    self.m_one_org_scope_ctrl.construct(self.m_session_mdl)
    self.m_organism_configure_ctrl.construct(self.m_session_mdl)

    self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("setOneOrganismViewNameLabelTextSig"),
      self.setOneOrganismViewNameLabelTextSlot)
    self.connect(self.m_organism_scope_toggle, SIGNAL("clicked()"),
      self.ToggleScopeSlot)

    self.m_organism_configure_ctrl.SetRandomGeneratedRadioButton(True);
    self.m_organism_configure_ctrl.ChangeMutationSliderSlot();
    self.m_organism_configure_ctrl.setAnalysisNeeded(False)

  def aboutToBeLowered(self):
    """Disconnects menu items from organism viewer controller."""
    descr()
    self.disconnect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("saveImagesSig"),
      self.saveImagesSlot)
    self.disconnect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("printSig"),
      self.printOrgSlot)

  def aboutToBeRaised(self):
    """Connects menu items from organism viewer controller."""
    descr()
    self.connect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("saveImagesSig"),
      self.saveImagesSlot)
    self.connect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("printSig"),
      self.printOrgSlot)


  def generatePixmaps(self, objects):
    pixmap = QPixmap()

    # Size the pixmap to hold the requested objects.
    if 'Organism' in objects:
      org_pix = QPixmap.grabWidget(
        self.m_one_org_scope_ctrl.m_organism_scope_ctrl,
        0, 0,
        self.m_one_org_scope_ctrl.m_organism_scope_ctrl.width(),
        self.m_one_org_scope_ctrl.m_organism_scope_ctrl.height()
      )
      pixmap.resize(
        pixmap.width() + org_pix.width(),
        pixmap.height() + org_pix.height()
      )
    if 'Statistics' in objects:
      data_pix = QPixmap.grabWidget(
        self.m_one_org_scope_ctrl.m_organism_data_ctrl,
        0, 0,
        self.m_one_org_scope_ctrl.m_organism_data_ctrl.width(),
        self.m_one_org_scope_ctrl.m_organism_data_ctrl.height()
      )
      data_pix_x = pixmap.width()
      pixmap.resize(
        data_pix_x + data_pix.width(),
        max(pixmap.height(), data_pix.height())
      )
    if 'Timeline' in objects:
      timeline_pix = QPixmap.grabWidget(
        self.m_one_org_scope_ctrl.m_timeline,
        0, 0,
        self.m_one_org_scope_ctrl.m_timeline.width(),
        self.m_one_org_scope_ctrl.m_timeline.height()
      )
      timeline_pix_y = pixmap.height()
      pixmap.resize(
        max(pixmap.width(), timeline_pix.width()),
        timeline_pix_y + timeline_pix.height()
      )
      slider_pix = QPixmap.grabWidget(
        self.m_one_org_scope_ctrl.m_execution_step_slider,
        0, 0,
        self.m_one_org_scope_ctrl.m_execution_step_slider.width(),
        self.m_one_org_scope_ctrl.m_execution_step_slider.height()
      )
      slider_pix_y = pixmap.height()
      pixmap.resize(
        max(pixmap.width(), slider_pix.width()),
        slider_pix_y + slider_pix.height()
      )

    
    # Clear the pixmap.
    pixmap.fill()

    # Paint the pixmap.
    painter = QPainter(pixmap)
    if 'Organism' in objects:
      descr('Organism in objects:', objects)
      painter.drawPixmap(0, 0, org_pix)
    if 'Statistics' in objects:
      descr('Statistics in objects:', objects)
      painter.drawPixmap(data_pix_x, 0, data_pix)
    if 'Timeline' in objects:
      descr('Timeline in objects:', objects)
      painter.drawPixmap(0, timeline_pix_y, timeline_pix)
      painter.drawPixmap(0, slider_pix_y, slider_pix)

    return pixmap


  def saveImagesSlot(self):
    "Save organism viewer to image file"
    dlg = pyButtonListDialog(
      "Export Graphics",
      "Choose objects to save",
      ["Organism", "Statistics", "Timeline"],
      True
    )
    objects = dlg.showDialog()
    if len(objects) == 0:
      return

    # Generate pixmap containing requested objects.
    pixmap = self.generatePixmaps(objects)

    # Ask user for name of image file to save.
    img_dlg = pyImageFileDialog()
    name = str(self.m_one_org_scope_ctrl.m_organism_scope_ctrl.m_organism_name)
    execution_step = self.m_one_org_scope_ctrl.m_execution_step_slider.value()
    filename, type = img_dlg.saveImageDialog(
      name +
      "-organism-step-" +
      str(execution_step)
    )

    # Save pixmap to image file.
    if filename:
      pixmap.save(filename, type, 100)


  def printOrgSlot(self):
    """Print image of organism viewer"""
    dlg = pyButtonListDialog(
      "Export Graphics",
      "Choose objects to save",
      ["Organism", "Statistics", "Timeline"],
      True
    )
    objects = dlg.showDialog()
    if len(objects) == 0:
      return

    # Generate pixmap containing requested objects.
    pixmap = self.generatePixmaps(objects)

    # Print pixmap.
    printer = QPrinter()
    if printer.setup():
      painter = QPainter(printer)
      painter.drawPixmap(0, 0, pixmap)
      painter.end()


  def setOneOrganismViewNameLabelTextSlot(self, name):
    self.m_organism_name_label.setText(name)

  def ToggleScopeSlot (self):
    current_page = self.m_organism_scope_widget_stack.visibleWidget()
    current_page_int = self.m_organism_scope_widget_stack.id(current_page)
    if (current_page_int == 0):
       self.m_organism_scope_widget_stack.raiseWidget(1)
       self.m_flip_button_text_label.setText("<p align=\"right\">Flip to<br>Viewer</p>")
       QToolTip.remove(self.m_organism_scope_toggle)
       QToolTip.add(self.m_organism_scope_toggle,"Flip to see <b><i>Organism Viewer</i></b>")
    else:
       self.m_organism_scope_widget_stack.raiseWidget(0)
       self.m_flip_button_text_label.setText("<p align=\"right\">Flip to<br>Settings</p>")
       QToolTip.remove(self.m_organism_scope_toggle)
       QToolTip.add(self.m_organism_scope_toggle,"Flip to see <b><i>Organism Viewer Settings</i></b>")
       if self.m_organism_configure_ctrl.isAnalysisNeeded():
          self.m_one_org_scope_ctrl.m_organism_scope_ctrl.analyzeLoadedOrganism()
          self.m_organism_configure_ctrl.setAnalysisNeeded(False)

