from qt import *

from pyHideShowButtonCtrl import pyHideShowButtonCtrl

class pyHideShowView(QWidget):
  def __init__(self,parent = None,name = None,fl = 0):
    QWidget.__init__(self,parent,name,fl)
    if not name: self.setName("pyHideShowView")

    self.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)

    top_layout = QVBoxLayout(self,0,0,"pyHideShowView,top_layout")

    h_layout1 = QHBoxLayout(None,0,0,"pyHideShowView,h_layout1")
    #self.m_hide_show_btn = QWidget(self)
    self.m_hide_show_btn = pyHideShowButtonCtrl(self)
    self.m_hide_show_btn.setMinimumSize(11, 11)
    self.m_hide_show_btn.setMaximumSize(11, 11)
    h_layout1.addWidget(self.m_hide_show_btn)
    # temporary
    self.m_label = QLabel(self)
    font = QFont(qApp.font())
    font.setPointSize(10)
    self.m_label.setFont(font)
    h_layout1.addWidget(self.m_label)

    h_layout2 = QHBoxLayout(None,0,0,"pyHideShowView,h_layout2")
    # temporary
    self.m_spacer_widget = QWidget(self)
    self.m_spacer_widget.setMinimumWidth(11)
    self.m_spacer_widget.setMaximumWidth(11)
    h_layout2.addWidget(self.m_spacer_widget)
    # temporary
    self.m_subwidget = QWidget(self)
    h_layout2.addWidget(self.m_subwidget)
    # temporary
    #self.m_spacer_widget2 = QWidget(self)
    #h_layout3 = QHBoxLayout(self.m_spacer_widget2,11,0,"pyHideShowView,h_layout2")
    #self.m_spacer_widget2.setMinimumWidth(11)
    #self.m_spacer_widget2.setMaximumWidth(11)
    #h_layout2.addWidget(self.m_spacer_widget2)

    top_layout.addLayout(h_layout1)
    top_layout.addLayout(h_layout2)
    top_layout.addStretch()

  def getLabel(self):
    return self.m_label

  def getSubwidget(self):
    return self.m_subwidget
