from qt import *

#from pyHideShowButtonView import pyHideShowButtonView
#
#class pyHideShowButtonCtrl(pyHideShowButtonView):
#  def __init__(self,parent = None,name = None,fl = 0):
#    pyHideShowButtonView.__init__(self,parent,name,fl) 
#    if not name: self.setName("pyHideShowButtonCtrl")

class pyHideShowButton(QWidget):
  def __init__(self,parent = None,name = None,fl = 0):
    QWidget.__init__(self,parent,name,fl)
    if not name: self.setName("pyHideShowButtonView")

  def mouseReleaseEvent(self, mouse_event):
    self.emit(PYSIGNAL("clicked"),())

class pyHideShowButtonCtrl(QWidget):
    def __init__(self,parent = None,name = None,fl = 0):
        QWidget.__init__(self,parent,name,fl)
        if not name: self.setName("pyHideShowButtonView")


        pyHideShowButtonViewLayout = QVBoxLayout(self,0,0,"pyHideShowButtonViewLayout")

        self.m_widget_stack = QWidgetStack(self,"m_widget_stack")

        self.m_is_hidden_page = QWidget(self.m_widget_stack,"m_is_hidden_page")
        m_is_hidden_pageLayout = QVBoxLayout(self.m_is_hidden_page,0,0,"m_is_hidden_pageLayout")


        self.m_is_hidden_button = pyHideShowButton(self.m_is_hidden_page,"m_is_hidden_button")
        self.m_is_hidden_button.setPaletteBackgroundPixmap(QPixmap("closed.png"))
        #self.m_is_hidden_button.setMinimumSize(QSize(11, 11))
        #self.m_is_hidden_button.setMaximumSize(QSize(11, 11))

        #self.m_is_hidden_button = QToolButton(Qt.RightArrow, self.m_is_hidden_page,"m_is_hidden_button")
        #self.m_is_hidden_button.setAutoRaise(False)

        #self.m_is_hidden_button = QRadioButton(self.m_is_hidden_page,"m_is_hidden_button")
        #self.m_is_hidden_button.setPixmap(QPixmap("closed.png"))

        m_is_hidden_pageLayout.addWidget(self.m_is_hidden_button)
        self.m_widget_stack.addWidget(self.m_is_hidden_page,0)

        self.m_is_shown_page = QWidget(self.m_widget_stack,"m_is_shown_page")
        m_is_shown_pageLayout = QVBoxLayout(self.m_is_shown_page,0,0,"m_is_shown_pageLayout")


        self.m_is_shown_button = pyHideShowButton(self.m_is_shown_page,"m_is_shown_button")
        self.m_is_shown_button.setPaletteBackgroundPixmap(QPixmap("open.png"))

        #self.m_is_shown_button = QToolButton(Qt.DownArrow, self.m_is_shown_page,"m_is_shown_button")
        #self.m_is_shown_button.setPixmap(QPixmap("open.png"))
        #self.m_is_shown_button.setBackgroundMode(Qt.FixedPixmap)

        #self.m_is_shown_button.setAutoRaise(False)
        #self.m_is_shown_button = QRadioButton(self.m_is_shown_page,"m_is_shown_button")
        #self.m_is_shown_button.setPixmap(QPixmap("open.png"))

        m_is_shown_pageLayout.addWidget(self.m_is_shown_button)
        self.m_widget_stack.addWidget(self.m_is_shown_page,1)
        pyHideShowButtonViewLayout.addWidget(self.m_widget_stack)

        #self.resize(QSize(149,149).expandedTo(self.minimumSizeHint()))
        #self.clearWState(Qt.WState_Polished)


