from qt import *
from pyHelpScreenView import pyHelpScreenView

# Simple class to pop up help message. Place holder for a real help sysem

class pyHelpScreenCtrl (pyHelpScreenView):
  def __init__(self):
    pyHelpScreenView.__init__(self)

  def showDialog(self):
    self.exec_loop()
