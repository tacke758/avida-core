from qt import *
from pyAboutScreenView import pyAboutScreenView
from Get_Avida_ED_version import avida_ed_version_string

# Simple class to pop up about message. Need to add author list.

class pyAboutScreenCtrl (pyAboutScreenView):
  def __init__(self):
    pyAboutScreenView.__init__(self)

    # in the original about screen the location for version is held with "xxx"
    # find this string and replace it with the actual version number

    tmp_help_str = str(self.AboutInfoTextLabel.text())
    version_loc = tmp_help_str.find("xxx")
    if (version_loc > 1):
      new_help_str = tmp_help_str[:version_loc] + avida_ed_version_string + tmp_help_str[version_loc+3:]
      self.AboutInfoTextLabel.setText(new_help_str)

  def showDialog(self):
    self.exec_loop()
