# -*- coding: utf-8 -*-

from descr import descr

from qt import *
from pyOneOrganismView import pyOneOrganismView


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
    """does nothing yet."""
    descr()
  def aboutToBeRaised(self):
    """does nothing yet."""
    descr()

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

