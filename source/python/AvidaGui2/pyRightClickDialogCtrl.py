# -*- coding: utf-8 -*-

from qt import *
from pyRightClickDialogView import pyRightClickDialogView
import shutil, os, os.path

# class to pop up a dialog box when the user quits
# to ask for what to freeze and to
# return the name of a file to save information to be frozen

class pyRightClickDialogCtrl (pyRightClickDialogView):
  def __init__(self, item_name, file_name):
    pyRightClickDialogView.__init__(self)
    self.connect(self.ConfirmPushButton, SIGNAL("clicked()"), self.DoneDialogSlot)

    print "BDB item_name = " + str(item_name) + " file_name = " + str(file_name)
    self.DeleteFlag = 1
    self.RenameFlag = 2
    self.SaveAsFlag = 3
    self.OpenFlag = 4
    
  def DoneDialogSlot(self):
    if self.DeleteRadioButton.isChecked():
      self.done(self.DeleteFlag)
    elif self.RenameRadioButton.isChecked():
      self.done(self.RenameFlag)
    elif self.SaveAsRadioButton.isChecked():
      self.done(self.SaveAsFlag)
    elif self.OpenRadioButton.isChecked():
      self.done(self.OpenFlag)

  def showDialog(self):
    self.show()
    junk = self.exec_loop()
    print "BDB junk = " + str(junk)
    dialog_result = self.result()
    print "BDB dialog_results = " + str(dialog_result)
