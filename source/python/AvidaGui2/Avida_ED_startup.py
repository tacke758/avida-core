def NonInteractive():
  import sys
  import qt
  a = qt.QApplication(sys.argv)
  from AvidaGui2 import Avida_ED_startup_utils as Avida_ED_startup_utils
  Avida_ED_startup_utils.AvidaCore.cConfig.InitGroupList()
  ctrl = Avida_ED_startup_utils.AvidaEd()
  default_font = qt.QFont("Arial", 12)
  a.setFont(default_font, True)
  a.exec_loop()

def Interactive():
  import IPython
  import __builtin__

  startup_sequence = "import iqt; import AvidaGui2; from AvidaGui2.Avida_ED_startup_utils import *; AvidaCore.cConfig.InitGroupList(); avida_ed = AvidaEd();"
  #startup_sequence = "import iqt; import AvidaGui2; from AvidaGui2.Avida_ED_startup_utils import *; AvidaCore.cConfig.InitGroupList();"

  ipython_shell = IPython.Shell.IPShell(argv=["-c", startup_sequence])
  ipython_shell.mainloop()

