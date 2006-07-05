from descr import descr
from qt import *
#from AvidaCore import cHardwareDefs, cHardwareCPUDefs
from AvidaCore import *

class pyTaskDescriptionCtrl(QLabel):
  def __init__(self,parent = None,name = None,fl = 0):
    QLabel.__init__(self,parent,name,fl)
    if not name: name = "pyTaskDescriptionCtrl"
    self.name = name
    self.setName(self.name)
    self.setText(self.name)

    self.normal_font = QFont(qApp.font())
    self.normal_font.setPointSize(9)
    self.normal_font.setBold(False)

    self.bold_font = QFont(qApp.font())
    self.bold_font.setPointSize(9)
    self.bold_font.setBold(True)

    self.setFont(self.normal_font)

    self.read_fn = None

  def setReadFn(self, sender, read_fn):
    self.read_fn = read_fn
    self.connect(sender, PYSIGNAL("propagated-FrameShownSig"), self.frameShownSlot)
    return self

  def frameShownSlot(self, frames, frame_no):
    #descr()
    task_text = self.name
    if frames is not None and frame_no < frames.m_gestation_time and self.read_fn is not None:
      task_num = self.read_fn(frames, frame_no)
      task_name = frames.m_task_names[task_num]
      task_text = '%s' % task_name
      tasks_frame = frames.m_tasks_info[frame_no]
      task_count = tasks_frame[task_num]
      self.setFont(task_count > 0 and self.bold_font or self.normal_font)
      task_text = task_text + ": " + str(task_count)
    self.setText("%s" % task_text)

