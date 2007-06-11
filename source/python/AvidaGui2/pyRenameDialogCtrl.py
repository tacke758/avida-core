from qt import *
from descr import *
from pyRenameDialogView import pyRenameDialogView
import os, os.path

# class to pop up a dialog box to allow a user to rename one or more files

class pyRenameDialogCtrl (pyRenameDialogView):
  def __init__(self):
    pyRenameDialogView.__init__(self)

  def showDialog(self, files2process):
    if (len(files2process) == 1):
      file_name = files2process[0]
      short_name_str = os.path.basename(file_name)
      prefix = str(os.path.dirname(file_name))
      if (short_name_str.endswith("empty")):
        short_name_str = short_name_str[:-6]
        suffix = ".empty"
      elif (short_name_str.endswith("full")):
        short_name_str = short_name_str[:-5]
        suffix = ".full"
      elif (short_name_str.endswith("organism")):
        short_name_str = short_name_str[:-9]
        suffix = ".organism"
      new_prompt_str = "Enter new name for " + str(short_name_str)
      self.MainMessageTextLabel.setText(new_prompt_str)
      self.newNameLineEdit.setText(str(short_name_str))
      dialog_result = self.exec_loop()
      if ((dialog_result == 1) and (self.newNameLineEdit.text() != "")):
        new_name = os.path.join(prefix, str(self.newNameLineEdit.text()) + suffix)
        if (os.path.exists(new_name) == False):
          os.rename(file_name, new_name)
      return(dialog_result)
    else:
      warning("Currently you can only rename one item at a time")
      return(0)
