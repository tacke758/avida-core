# -*- coding: utf-8 -*-

from qt import *
from pyFreezeDialogView import pyFreezeDialogView
from pyBeforeStartingCtrl import pyBeforeStartingCtrl
import shutil, os, os.path

# class to pop up a dialog box to ask for what to freeze and to
# return the name of a file to save information to be frozen

class pyFreezeDialogCtrl (pyFreezeDialogView):
  def __init__(self):
    pyFreezeDialogView.__init__(self)
    
  def showDialog(self, session_mdl, freeze_empty_only_flag = False):
    found_valid_name = False
    dialog_result = 1

    # If the user has not already chosen an active workspace for this session
    # make them do so now. If they chose not to pick a workspace, don't let
    # them save their work

    if (session_mdl.directory_chosen == False):
      m_prompt_dir = pyBeforeStartingCtrl()
      m_prompt_dir.construct(session_mdl)
      if (m_prompt_dir.showDialog() == 0):
        return ''
    
    # If the user selects to open existing/create new workspace then hits
    # the cancel button in the choose dir dialog box the program will end
    # up saving into the program default workspace -- to prevent that
    # double check that a directory has actually been chosen

    if (session_mdl.directory_chosen == False):
      return ''

    if freeze_empty_only_flag == True:
      self.FullRadioButton.setEnabled(False)
      self.FullRadioButton.setChecked(False)
      self.EmptyRadioButton.setChecked(True)
    else:
      self.FullRadioButton.setEnabled(True)
      self.EmptyRadioButton.setChecked(False)
      self.FullRadioButton.setChecked(True)
    while (found_valid_name == False and dialog_result > 0):
      self.exec_loop()
      dialog_result = self.result()
      tmp_name = str(self.FileNameLineEdit.text())
      if dialog_result == 0:
        return ''
      elif (tmp_name == ''):
        found_valid_name = False
        self.MainMessageTextLabel.setText("Enter a Non-Blank Name of Item to Freeze")
      else:

        # Save empty petri dish

        if self.EmptyRadioButton.isChecked():
          if (tmp_name.endswith(".empty") == False):
            tmp_name = tmp_name + ".empty"
          tmp_name = os.path.join(session_mdl.m_current_freezer, tmp_name)
          if os.path.exists(tmp_name):
            found_valid_name = False
            self.MainMessageTextLabel.setText("Petri Dish Exists, Please Enter a Different Name")
          else:
            found_valid_name = True
            return tmp_name

        # Save full petri dish

        elif self.FullRadioButton.isChecked():
          if (tmp_name.endswith(".full") == False):
            tmp_name = tmp_name + ".full"
          tmp_name = os.path.join(session_mdl.m_current_freezer, tmp_name)
          if os.path.exists(tmp_name):
            found_valid_name = False
            self.MainMessageTextLabel.setText("Petri Dish Exists, Please Enter a Different Name")
          else:
            found_valid_name = True
            return tmp_name

        # Save selected organism

        else:
          # BDB -- code for saving selected organism needs to be added here
          pass

  def isEmpty(self):
    if self.EmptyRadioButton.isChecked():
      return True
    else:
      return False

