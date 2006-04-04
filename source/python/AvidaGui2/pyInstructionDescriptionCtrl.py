
from descr import descr
from qt import *
#from AvidaCore import cHardwareDefs, cHardwareCPUDefs
from AvidaCore import *

class pyInstructionDescriptionCtrl(QLabel):
  def __init__(self,parent = None,name = None,fl = 0):
    QLabel.__init__(self,parent,name,fl)
    if not name: self.setName("pyInstructionDescriptionCtrl")

    font = QFont(qApp.font())
    font.setPointSize(9)
    self.setFont(font)

    #self.read_fn = None

  def setReadFn(self, sender, read_fn):
    #self.read_fn = read_fn
    self.connect(sender, PYSIGNAL("propagated-FrameShownSig"), self.frameShownSlot)
    return self

  def frameShownSlot(self, frames, frame_no):
    #if frames is not None and frame_no < frames.m_gestation_time and self.read_fn is not None:
    if frames is not None and frame_no < frames.m_gestation_time:
      #self.read_fn(frames, frame_no)
      inst_set = frames.getHardwareSnapshotAt(frame_no).GetInstSet()
      short_name = frames.m_genome_info[frame_no][frames.m_ihead_info[frame_no]]
      #self.setText(short_name)
      inst = cInstruction()
      inst.SetSymbol(short_name)
      long_name = inst_set.GetName(inst)
      self.setText("%s: %s" % (short_name, long_name))
    else: 
      self.setText("(no instruction)")
      pass

