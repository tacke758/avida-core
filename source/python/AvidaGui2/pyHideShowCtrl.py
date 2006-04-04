from qt import *

from pyHideShowView import pyHideShowView

class pyHideShowCtrl(pyHideShowView):
  def __init__(self,parent = None,name = None,fl = 0):
    pyHideShowView.__init__(self,parent,name,fl)
    if not name: self.setName("pyHideShowCtrl")

    self.min_width = 0

    self.connect(self.m_hide_show_btn.m_is_shown_button, PYSIGNAL("clicked"), self.hideSlot)
    self.connect(self.m_hide_show_btn.m_is_hidden_button, PYSIGNAL("clicked"), self.showSlot)
    self.showSlot()

  def updateMinWidth(self, child_width):
    width = self.m_hide_show_btn.maximumWidth() + child_width + 2*self.layout().margin()
    #width = self.m_hide_show_btn.maximumWidth() + child_width + 30
    if self.min_width < width: self.min_width = width
    self.setMinimumWidth(self.min_width)
    
  def hideSlot(self):
    self.m_spacer_widget.hide()
    self.m_subwidget.hide()
    self.m_hide_show_btn.m_widget_stack.raiseWidget(self.m_hide_show_btn.m_is_hidden_page)

  def showSlot(self):
    self.m_spacer_widget.show()
    self.m_subwidget.show()
    self.m_hide_show_btn.m_widget_stack.raiseWidget(self.m_hide_show_btn.m_is_shown_page)
