from descr import descr

from pyTaskDescriptionCtrl import pyTaskDescriptionCtrl

class pyStdTaskDescriptionCtrl(pyTaskDescriptionCtrl):
  def __init__(self,parent = None,name = None,fl = 0):
    pyTaskDescriptionCtrl.__init__(self,parent,name,fl)

    self.m_task_renamings = {
      'not':'Not',
      'nand':'Nan',
      'and':'And',
      'orn':'Orn',
      'or':'Oro',
      'andn':'Ant',
      'nor':'Nor',
      'xor':'Xor',
      'equ':'Equ',
    }

