# -*- coding: utf-8 -*-

from qt import *
from pyRightClickDialogView import pyRightClickDialogView
import shutil, os, os.path

# class to pop up a dialog box when the user quits
# to ask for what to freeze and to
# return the name of a file to save information to be frozen

class pyRightClickDialogCtrl (pyRightClickDialogView):
  def __init__(self):
    pyRightClickDialogView.__init__(self)
    self.connect(self.CancelPushButton, SIGNAL("clicked()"), self.DownCancelSlot)
    self.connect(self.ConfirmPushButton, SIGNAL("clicked()"), self.DownConfirmSlot)
    self.ConfirmFlag = 1
    self.CancelFlag = 2
    
  def DownCancelSlot(self):
    self.QuitCancelButton.setDown(True)
    
  def DownConfirmSlot(self):
    self.ConfirmPushButton.setDown(True)
    
  def showDialog(self):
    dialog_result = 1
    while (dialog_result > 0):
      while (self.exec_loop() and self.result() == 0):
        pass
      dialog_result = self.result()
      if dialog_result == 0:
        return self.CancelFlag
      elif self.ConfirmPushButton.isDown():
        return self.ConfirmFlag
      else:
        return self.CancelFlag
