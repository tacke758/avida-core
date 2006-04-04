from qt import *
from pyBufferCtrl import pyBufferCtrl
from pyHideShowCtrl import pyHideShowCtrl

class pyHideShowFactory:
  def setParent(self, parent):
    self.m_parent = parent
    self.m_layout = QVBoxLayout(parent,5,0)
  def newHideShow(self):
    buffer = pyHideShowCtrl(self.m_parent)
    self.m_layout.addWidget(buffer)
    return buffer
  def layout(self):
    return self.m_layout

class pyBufferCtrlFactory:
  def setParent(self, parent):
    self.m_parent = parent
    self.m_layout = QVBoxLayout(parent,0,0)
  def newBuffer(self):
    buffer = pyBufferCtrl(self.m_parent)
    self.m_layout.addWidget(buffer)
    return buffer

class pyOrganismDataView(QWidget):
  def __init__(self,parent = None,name = None,fl = 0):
    QWidget.__init__(self,parent,name,fl)
    if not name: self.setName("pyOrganismDataView")

    #self.top_layout = QVBoxLayout(self,5,0,"pyOrganismDataView,top_layout")

    #self.vb_layout = QVBoxLayout(None,0,0,"pyOrganismDataView,layout1")
    #self.top_layout.addLayout(vb_layout)
    #self.top_layout.addStretch()

    hs_factory = pyHideShowFactory()
    b_factory = pyBufferCtrlFactory()

    hs_factory.setParent(self)

    self.m_hideshow_inputs = hs_factory.newHideShow()
    self.m_hideshow_inputs.getLabel().setText("input array")
    b_factory.setParent(self.m_hideshow_inputs.getSubwidget())
    self.m_buffer_in0 = b_factory.newBuffer()
    self.m_buffer_in1 = b_factory.newBuffer()
    self.m_buffer_in2 = b_factory.newBuffer()

    self.m_hideshow_ouput = hs_factory.newHideShow()
    self.m_hideshow_ouput.getLabel().setText("output")
    b_factory.setParent(self.m_hideshow_ouput.getSubwidget())
    self.m_buffer_out = b_factory.newBuffer()

    self.m_hideshow_regs = hs_factory.newHideShow()
    self.m_hideshow_regs.getLabel().setText("registers A, B, C")
    b_factory.setParent(self.m_hideshow_regs.getSubwidget())
    self.m_buffer_regA = b_factory.newBuffer()
    self.m_buffer_regB = b_factory.newBuffer()
    self.m_buffer_regC = b_factory.newBuffer()

    hs_factory.layout().addStretch()

