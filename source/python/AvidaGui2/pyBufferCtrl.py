
from descr import descr
from pyBufferView import pyBufferView
from qt import *
from AvidaCore import cHardwareDefs, cHardwareCPUDefs

class pyBufferCtrl(pyBufferView):
  def __init__(self,parent = None,name = None,fl = 0):
    pyBufferView.__init__(self,parent,name,fl)
    if not name: self.setName("pyBufferCtrl")
    self.read_fn = None

  def setReadFn(self, sender, read_fn):
    self.read_fn = read_fn
    self.connect(sender, PYSIGNAL("propagated-FrameShownSig"), self.frameShownSlot)
    return self

  def frameShownSlot(self, frames, frame_no):
    if frames is not None and frame_no < frames.m_gestation_time and self.read_fn is not None:
      self.setBits(self.read_fn(frames, frame_no))
    else: 
      self.setBits(0)
