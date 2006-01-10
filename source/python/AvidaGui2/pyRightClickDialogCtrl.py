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
    self.file_name = file_name
    self.NewNameLineEdit.setText(item_name)
    self.connect(self.ConfirmPushButton, SIGNAL("clicked()"), 
                 self.DoneDialogSlot)

    (self.file_dir, self.file_core_name)  = os.path.split(self.file_name)
    self.file_ext = self.file_core_name[self.file_core_name.rfind('.'):]
    self.NameTextLabel.setText(item_name)
    if (self.file_ext == '.organism'):
      self.TypeTextLabel.setText(self.file_ext.lstrip('.') + ":")
    else:
      self.TypeTextLabel.setText(self.file_ext.lstrip('.') + " dish:")
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
    self.change = False
    self.open_obj = False
    self.show()
    self.exec_loop()
    dialog_result = self.result()
    new_name_core = str(self.NewNameLineEdit.text())
    new_name_core = new_name_core.strip()

    # if the user chose to delete file or directory

    if dialog_result == self.DeleteFlag:
      self.change = True
      if (self.file_ext == '.full'):
        tmp_file_list = os.listdir(self.file_name)
        for file in tmp_file_list:
          os.remove(os.path.join(self.file_name,file))
        os.removedirs(self.file_name)
      else:
        os.remove(self.file_name)

    # if the user chose to rename the file/directory and they gave an actual
    # check if that file already exists if it does not rename the file

    elif dialog_result == self.RenameFlag and new_name_core != "":
      new_name = os.path.join(self.file_dir, new_name_core + self.file_ext)
      if (os.path.exists(new_name) == False):
        os.rename(self.file_name, new_name)
        self.change = True

    elif dialog_result == self.SaveAsFlag:
      file_dialog = QFileDialog (os.path.join(self.file_dir, '..'), 
        '.junk1234junk', self, 'Export', False)
      file_dialog.setCaption('Export ' + self.file_ext.lstrip('.') + " " + 
        str(self.NameTextLabel.text()) + ' to:')
      file_dialog.setMode(QFileDialog.DirectoryOnly)
      # file_dialog.setSelection (self.file_core_name)
      file_dialog.show()
      file_dialog.exec_loop()
      if file_dialog.result() == True:
        print "BDB dir name = " + str(file_dialog.dir().selectedFile())
        export_file_name = os.path.join(str(file_dialog.dir().selectedFile()), 
          self.file_core_name)
        print "BDB export file name = " + str(export_file_name)
        # shutil.copytree(self.file_name, str(file_dialog.dir().path()))

    elif dialog_result == self.OpenFlag:
      self.open_obj = True

    return(self.change, self.open_obj)
 
