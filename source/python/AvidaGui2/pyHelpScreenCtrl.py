from descr import *
from pyHelpScreenView import pyHelpScreenView
from qt import *

# Simple class to pop up help message. Place holder for a real help sysem

class pyHelpScreenCtrl (pyHelpScreenView):
  def __init__(self):
    pyHelpScreenView.__init__(self)

  def showDialog(self):
    self.exec_loop()

#class pyHelpScreenCtrl(QMainWindow):
#  def __init__(self, home, path, parent = None, name = None, fl = 0):
#    QMainWindow.__init__(self,parent,name,fl)
#
#    self.browser = QTextBrowser(self)
#    #descr("path:", path)
#    #self.browser.mimeSourceFactory().setFilePath(QString(path))
#    #self.connect(self.browser, PYSIGNAL("sourceChanged"), self.sourceChangedSlot)
#    self.setCentralWidget(self.browser)
#
#    if not home == "":
#      self.browser.setSource(home)
#
#    self.resize(800, 600)
#
#    file = QPopupMenu(self)
#    file.insertItem("&Close", self.hide, QKeySequence("Ctrl+W"))
#
#
#    self.menuBar().insertItem("&File", file)
#
#    toolbar = QToolBar(self)
#    self.addDockWindow(toolbar, "Toolbar")
#
#    back_button = QToolButton(Qt.LeftArrow, toolbar)
#    self.connect(back_button, SIGNAL("clicked()"), self.browser.backward)
#
#    forward_button = QToolButton(Qt.RightArrow, toolbar)
#    self.connect(forward_button, SIGNAL("clicked()"), self.browser.forward)
#
#    home_button = QToolButton(toolbar)
#    home_button.setTextLabel("Reload")
#    home_button.setUsesTextLabel(True)
#    self.connect(home_button, SIGNAL("clicked()"), self.browser.reload)
#
#    #toolbar.addSeparator()
#
#    #self.path_combo = QComboBox(True, toolbar)
#    #toolbar.setStretchableWidget(path_combo)
#    self.setRightJustification(True)
#    self.setDockEnabled(Qt.DockLeft, False)
#    #self.setDockEnabled(Qt.DockRight, False)
#    #
#
#    #self.path_combo.insertItem(home)
#    self.browser.setFocus()
#
#  def setSource(self, source):
#    self.browser.setSource(source)
#
#  def __del__(self):
#    descr("deleting...")
