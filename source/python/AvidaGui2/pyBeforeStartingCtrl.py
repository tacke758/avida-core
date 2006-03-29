# -*- coding: utf-8 -*-

from qt import *
from pyBeforeStartingView import pyBeforeStartingView

# class to pop up a dialog box before AvidaEd starts

class pyBeforeStartingCtrl (pyBeforeStartingView):
  def __init__(self):
    pyBeforeStartingView.__init__(self)
    # self.connect(self.OpenExistingPushButton, SIGNAL("clicked()"),
    #              self.m_session_mdl.m_session_mdtr, 
    #              PYSIGNAL("workspaceOpenSig"))
    # self.connect(self.CreateNewPushButton, SIGNAL("clicked()"),
    #              self.m_session_mdl.m_session_mdtr, 
    #              PYSIGNAL("workspaceOpenSig"))

  def showDialog(self):
    self.show()
    self.exec_loop()
    dialog_result = self.result()
    print "BDB pyBeforeStgartingCtrl:showDialog dialog_result = " + str(dialog_result)
