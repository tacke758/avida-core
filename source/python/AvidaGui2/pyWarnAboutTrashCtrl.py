# -*- coding: utf-8 -*-

from qt import *
from pyWarnAboutTrashView import pyWarnAboutTrashView

# class to pop up a dialog box when the user quits
# to ask for what to freeze and to
# return the name of a file to save information to be frozen

class pyWarnAboutTrashCtrl (pyWarnAboutTrashView):
  def __init__(self,item_name):
    pyWarnAboutTrashView.__init__(self)
    self.DeleteAllFlag = False
    self.DeleteFlag = False
    self.IgnoreFlag = False
    self.DeleteAllCode = 1
    self.DeleteCode = 2
    self.IgnoreCode = 3
    self.connect(self.DeleteAllPushButton, SIGNAL("clicked()"),
                 self.DeleteAllSlot)
    self.connect(self.DeletePushButton, SIGNAL("clicked()"),
                 self.DeleteSlot)
    self.connect(self.IgnorePushButton, SIGNAL("clicked()"),
                 self.IgnoreSlot)
    self.WarningLabel.setText("<p align=\"center\">There is no way to recover something from the trash.  <br>Do you want to delete " + str(item_name) + "?</p>")
    
  def showDialog(self):
    dialog_result = 1
    while (dialog_result > 0):
      self.exec_loop()
      if self.DeleteAllFlag:
        return self.DeleteAllCode
      elif self.DeleteFlag:
        return self.DeleteCode
      elif self.IgnoreFlag:
        return self.IgnoreCode

  def DeleteAllSlot(self):
    self.DeleteAllFlag = True

  def DeleteSlot(self):
    self.DeleteFlag = True

  def IgnoreSlot(self):
    self.IgnoreFlag = True
