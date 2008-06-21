# -*- coding: utf-8 -*-

from qt import *
from pyBeforeStartingView import pyBeforeStartingView

# class to pop up a dialog box before someone saves for the first time
# (assumeing the user has not already opened/created a workspace)

class pyBeforeStartingCtrl (pyBeforeStartingView):
  def __init__(self):
    pyBeforeStartingView.__init__(self)

  def construct(self, session_mdl):
    self.m_session_mdl = session_mdl
    self.Existing_Pushed = False
    self.New_Pushed = False
    self.connect(self.OpenExistingPushButton, SIGNAL("clicked()"),
                 self.OpenExistingSlot)
    self.connect(self.CreateNewPushButton, SIGNAL("clicked()"),
                 self.CreateNewSlot)

  def showDialog(self, verb):

    # Change the propt depending if the user is trying to import or freeze

    self.BeforeStartTextLabel.setText("<p align=\"center\">Before you can " + verb + " a petri dish or organism<br> you must choose a workspace to work with.</p>")

    dialog_result = 1
    while (dialog_result > 0):
      self.exec_loop()
      dialog_result = self.result()
      if dialog_result == 0:
        return 0
      elif self.Existing_Pushed:
        self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("workspaceOpenSig"),())
        return 1
      elif self.New_Pushed:
        self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("workspaceNewSig"),())
        return 1
      else:
        pass

  def OpenExistingSlot(self):
    self.Existing_Pushed = True

  def CreateNewSlot(self):
    self.New_Pushed = True

 
