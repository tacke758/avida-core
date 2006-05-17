from pyBufferCtrl import pyBufferCtrl
from pyInstructionDescriptionCtrl import pyInstructionDescriptionCtrl
from pyHideShowCtrl import pyHideShowCtrl
from descr import descr
from qt import *
from AvidaCore import *

class WidgetFactory:
  def setParentAndLayout(self, parent, layout):
    self.parent = parent
    self.layout = layout
  def setParent(self, parent):
    self.setParentAndLayout(parent, self.layout_fn(parent))
  def setLayoutFn(self, layout_fn):
    self.layout_fn = layout_fn
  def setWidgetFn(self, widget_fn):
    self.widget_fn = widget_fn
  def newWidget(self, name=None):
    widget = self.widget_fn(self.parent, self.layout, name)
    return widget
  def getParent(self):
    return self.parent
  def getLayout(self):
    return self.layout

def hideShowSetup(parent, layout, name):
  buffer = pyHideShowCtrl(parent, name)
  layout.addWidget(buffer)
  return buffer
def bufferSetup(parent, layout, name):
  buffer = pyBufferCtrl(parent.getSubwidget(), name)
  layout.addWidget(buffer)
  parent.updateMinWidth(buffer.maximumWidth())
  return buffer
def instDescrSetup(parent, layout, name):
  inst_descr = pyInstructionDescriptionCtrl(parent.getSubwidget(), name)
  layout.addWidget(inst_descr)
  parent.updateMinWidth(inst_descr.maximumWidth())
  return inst_descr
def labelSetup(parent, layout, name):
  font = QFont(qApp.font())
  font.setPointSize(9)
  label = QLabel(parent.getSubwidget(), name)
  label.setFont(font)
  layout.addWidget(label)
  parent.updateMinWidth(label.maximumWidth())
  return label


class pyOrganismDataCtrl(QScrollView):
#class pyOrganismDataCtrl(QWidget):
  def __init__(self,parent = None,name = None,fl = 0):
    QScrollView.__init__(self,parent,name,fl)
    #QWidget.__init__(self,parent,name,fl)
    if not name: self.setName("pyOrganismDataCtrl")
    QToolTip.add(self,"Shows current state of the organism's hardware")

    #font = QFont(qApp.font())
    #font.setPointSize(9)
    #self.setFont(font)

    self.setPaletteBackgroundColor(Qt.white)

    self.setResizePolicy(QScrollView.AutoOneFit)
    self.viewport().setPaletteBackgroundColor(Qt.white)
    self.setVScrollBarMode(QScrollView.AlwaysOn)
    self.setHScrollBarMode(QScrollView.AlwaysOff)

    layout = QVBoxLayout(self.viewport(),5,0)
    #layout = QVBoxLayout(self,5,0)

    self.old_frame_no = -1

    self.hideshow_factory = WidgetFactory()
    self.hideshow_factory.setWidgetFn(hideShowSetup)
    self.hideshow_factory.setLayoutFn(lambda parent: layout)
    self.hideshow_factory.setParent(self.viewport())
    #self.hideshow_factory.setParent(self)

    self.hideshow_all_hardware = self.hideshow_factory.newWidget("pyOrganismDataCtrl.hideshow_all_hardware")
    self.hideshow_all_hardware.getLabel().setText("Hardware")
    self.hideshow_factory.setLayoutFn(lambda parent: QVBoxLayout(parent,0,0))
    self.hideshow_factory.setParent(self.hideshow_all_hardware.getSubwidget())

    self.widget_factory = WidgetFactory()
    self.widget_factory.setWidgetFn(bufferSetup)
    self.widget_factory.setLayoutFn(lambda parent: QVBoxLayout(parent.getSubwidget(),0,0))

    self.hideshow_inputs = self.hideshow_factory.newWidget("pyOrganismDataCtrl.hideshow_inputs")
    self.hideshow_inputs.getLabel().setText("Input array")
    self.widget_factory.setParent(self.hideshow_inputs)
    self.buffer_in0 = self.widget_factory.newWidget("pyOrganismDataCtrl.buffer_in0").setReadFn(self,
      lambda f, fn: f.m_ibuf_0_info[fn])
    self.buffer_in1 = self.widget_factory.newWidget("pyOrganismDataCtrl.buffer_in1").setReadFn(self,
      lambda f, fn: f.m_ibuf_1_info[fn])
    self.buffer_in2 = self.widget_factory.newWidget("pyOrganismDataCtrl.buffer_in2").setReadFn(self,
      lambda f, fn: f.m_ibuf_2_info[fn])
    self.hideshow_all_hardware.updateMinWidth(self.hideshow_inputs.minimumWidth())

    self.hideshow_output = self.hideshow_factory.newWidget("pyOrganismDataCtrl.hideshow_output")
    self.hideshow_output.getLabel().setText("Output")
    self.widget_factory.setParent(self.hideshow_output)
    self.buffer_out = self.widget_factory.newWidget("pyOrganismDataCtrl.buffer_out").setReadFn(self,
      lambda f, fn: f.m_obuf_0_info[fn])
    self.hideshow_all_hardware.updateMinWidth(self.hideshow_output.minimumWidth())

    self.hideshow_regs = self.hideshow_factory.newWidget("pyOrganismDataCtrl.hideshow_regs")
    self.hideshow_regs.getLabel().setText("Registers A, B, C")
    self.widget_factory.setParent(self.hideshow_regs)
    self.buffer_regA = self.widget_factory.newWidget("pyOrganismDataCtrl.buffer_regA").setReadFn(self,
      lambda f, fn: f.getThreadsSnapshotAt(fn)[0].GetRegister(cHardwareCPUDefs.s_REG_AX))
    self.buffer_regB = self.widget_factory.newWidget("pyOrganismDataCtrl.buffer_regB").setReadFn(self,
      lambda f, fn: f.getThreadsSnapshotAt(fn)[0].GetRegister(cHardwareCPUDefs.s_REG_BX))
    self.buffer_regC = self.widget_factory.newWidget("pyOrganismDataCtrl.buffer_regC").setReadFn(self,
      lambda f, fn: f.getThreadsSnapshotAt(fn)[0].GetRegister(cHardwareCPUDefs.s_REG_CX))
    self.hideshow_all_hardware.updateMinWidth(self.hideshow_regs.minimumWidth())

    self.hideshow_stackA = self.hideshow_factory.newWidget("pyOrganismDataCtrl.hideshow_stackA")
    self.hideshow_stackA.getLabel().setText("Stack A")
    self.widget_factory.setParent(self.hideshow_stackA)
    ##### I don't know why this doesn't work (@kgn):
    #self.buflist_stackA = [self.widget_factory.newWidget("pyOrganismDataCtrl.buflist_stackA[%i]"%i).setReadFn(self,
    #  lambda f, fn: f.getHardwareSnapshotAt(fn).pyGetGlobalStack().Get(i)) for i in range(10)]
    l = lambda i: lambda f, fn: f.getHardwareSnapshotAt(fn).pyGetGlobalStack().Get(i)
    self.buflist_stackB = [self.widget_factory.newWidget("pyOrganismDataCtrl.buflist_stackA[%i]"%i).setReadFn(self, l(i))
      for i in range(10)]
    self.hideshow_all_hardware.updateMinWidth(self.hideshow_stackA.minimumWidth())

    self.hideshow_stackB = self.hideshow_factory.newWidget("pyOrganismDataCtrl.hideshow_stackB")
    self.hideshow_stackB.getLabel().setText("Stack B")
    self.widget_factory.setParent(self.hideshow_stackB)
    ##### I don't know why this doesn't work (@kgn):
    #self.buflist_stackB = [self.widget_factory.newWidget("pyOrganismDataCtrl.buflist_stackB[%i]"%i).setReadFn(self,
    #  lambda f, fn: f.getThreadsSnapshotAt(fn)[0].stack.Get(i)) for i in range(10)]
    l = lambda i: lambda f, fn: f.getThreadsSnapshotAt(fn)[0].stack.Get(i)
    self.buflist_stackB = [self.widget_factory.newWidget("pyOrganismDataCtrl.buflist_stackB[%i]"%i).setReadFn(self, l(i))
      for i in range(10)]
    self.hideshow_all_hardware.updateMinWidth(self.hideshow_stackB.minimumWidth())

    self.hideshow_factory.setLayoutFn(lambda parent: layout)
    self.hideshow_factory.setParent(self.viewport())
    #self.hideshow_factory.setParent(self)

    self.widget_factory.setWidgetFn(labelSetup)

    self.hideshow_inst_legend = self.hideshow_factory.newWidget()
    self.hideshow_inst_legend.getLabel().setText("Instructions legend")
    self.widget_factory.setParent(self.hideshow_inst_legend)
    self.no_legend_entries = self.widget_factory.newWidget()
    self.no_legend_entries.setText("(no instruction set)")
    self.hideshow_all_hardware.updateMinWidth(self.hideshow_inst_legend.minimumWidth())

    self.widget_factory.setWidgetFn(instDescrSetup)

    self.hideshow_cur_inst = self.hideshow_factory.newWidget()
    self.hideshow_cur_inst.getLabel().setText("Current instruction")
    self.widget_factory.setParent(self.hideshow_cur_inst)
    self.curinst_descr = self.widget_factory.newWidget().setReadFn(self, None)
    self.curinst_descr.setText("(no instruction)")
    self.hideshow_all_hardware.updateMinWidth(self.hideshow_cur_inst.minimumWidth())

    layout.addStretch(1)

  def frameShownSlot(self, frames, frame_no):
    if frames is None:
      if self.old_frame_no != -1:
        self.old_frame_no = -1
        self.emit(PYSIGNAL("propagated-FrameShownSig"), (frames, -1))
    elif self.old_frame_no != frame_no:
      self.old_frame_no = frame_no
      self.emit(PYSIGNAL("propagated-FrameShownSig"), (frames, frame_no))
