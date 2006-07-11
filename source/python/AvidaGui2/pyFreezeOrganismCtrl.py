# -*- coding: utf-8 -*-

from qt import *
from pyFreezeOrganismView import pyFreezeOrganismView
from pyBeforeStartingCtrl import pyBeforeStartingCtrl
import shutil, os, os.path

# class to pop up a dialog box to ask for what to freeze and to
# return the name of a file to save information to be frozen

class pyFreezeOrganismCtrl (pyFreezeOrganismView):
  def __init__(self):
    pyFreezeOrganismView.__init__(self)
    
  def showDialog(self, session_mdl):
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

    while (found_valid_name == False and dialog_result > 0):
      self.exec_loop()
      dialog_result = self.result()
      tmp_name = str(self.FileNameLineEdit.text())

      if dialog_result == 0:
        return ''
      elif (tmp_name == ''):
        found_valid_name = False
        self.MainMessageTextLabel.setText("Enter a Non-Blank Name of Organism to Freeze")
      else:
          if (tmp_name.endswith(".organism") == False):
            tmp_name = tmp_name + ".organism"
          tmp_name = os.path.join(session_mdl.m_current_freezer, tmp_name)
          if os.path.exists(tmp_name):
            found_valid_name = False
            self.MainMessageTextLabel.setText("Organism Name Exists, Please Enter a Different Name")
          else:
            found_valid_name = True
            return tmp_name
      
