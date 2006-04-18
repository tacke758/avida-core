# -*- coding: utf-8 -*-

from qt import *
from pyBeforeStartingView import pyBeforeStartingView

# class to pop up a dialog box before AvidaEd starts

class pyBeforeStartingCtrl (pyBeforeStartingView):
  def __init__(self):
    pyBeforeStartingView.__init__(self)

  def construct(self, session_mdl):
    self.m_session_mdl = session_mdl
    self.connect(self.OpenExistingPushButton, SIGNAL("clicked()"),
                 self.m_session_mdl.m_session_mdtr, 
                 PYSIGNAL("workspaceOpenSig"))
    self.connect(self.OpenExistingPushButton, SIGNAL("clicked()"),
                self.closeDialog)
    self.connect(self.CreateNewPushButton, SIGNAL("clicked()"),
                 self.m_session_mdl.m_session_mdtr, 
                 PYSIGNAL("workspaceNewSig"))
    self.connect(self.CreateNewPushButton, SIGNAL("clicked()"),
                self.closeDialog)
    self.connect(self.CancelPushButton, SIGNAL("clicked()"),
                self.closeDialog)

  def showDialog(self):
    self.exec_loop()
    dialog_result = self.result()

  def closeDialog(self):
    self.done(1)
