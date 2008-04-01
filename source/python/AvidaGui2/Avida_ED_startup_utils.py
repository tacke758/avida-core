
import qt

# Make a splash screen to give feedback to the user about progress in
# launchning Avida-ED.
#
s_pixmap = qt.QPixmap("avida-ED-logo.png")
s_splash = qt.QSplashScreen(s_pixmap)
font = s_splash.font()
font.setPointSize(9)
s_splash.setFont(font)
del font
s_splash.show()
s_splash.message("Loading core Avida modules ...")

import AvidaCore

import os
import sys

from descr import descr, info, question, warning, critical

# For developers : this asks the pyTemporaryReloads class to reload
# certain parts of the python source code for the Avida gui, picking up
# any changes since the last launch of the gui. This can be called
# without restarting the application (convenient for quickly checking
# the results of changes to the source code).
#
def Reload():
  import AvidaGui2.pyTemporaryReloads
  reload(AvidaGui2.pyTemporaryReloads)
  # Ask the linecache module to refresh its cache with new versions of
  # the source code.
  import linecache
  linecache.checkcache()

# Runs unit tests. Can be called while gui is running, and will
# instantiate a new copy of Avida for testing (but this will disturb
# Avida's random number generator).
#
def UTs():
  Reload()
  AvidaGui2.pyTemporaryReloads.UTs()

# Instantiate (or reinstantiate) the Avida-ED gui. Return controller for
# new gui instance.
#
def AvidaEd():
  sys.exc_clear()
  sys.exc_traceback = sys.last_traceback = None
  s_splash.message("Loading Avida-ED user-interface ...")
  s_splash.show()

  try:
    Reload()
    from AvidaGui2.pyEduMainCtrl import pyEduMainCtrl
    s_splash.message("Assembling Avida-ED user-interface ...")

    edu_main_controller = pyEduMainCtrl()
    edu_main_controller.construct()
    edu_main_controller.m_main_mdl.m_main_mdtr.m_main_controller_factory_mdtr.emit(
      qt.PYSIGNAL("newMainControllerSig"), ("pySessionCtrl",))

    # Get the edu_workspace_ctrl to browse interactively via
    # 'avida_ed.m_edu_workspace_ctrl'
    
    # BDB -- When I commented out the creation of the contoller for
    #        pyEduMainMenuBarHdlr the array index for the session contoller
    #        changed from 1 to 0

    session_ctrl = edu_main_controller.m_main_controller_factory.m_main_controllers_list[0]

    # Get the workspace from the session controller factory

    edu_main_controller.m_edu_workspace_ctrl = session_ctrl.m_session_controller_factory.m_session_controllers_list[1]

    # Get the prompt for new/existing workspace from the session controller 
    # factory

    edu_main_controller.m_prompt_for_workspace_ctrl = session_ctrl.m_session_controller_factory.m_session_controllers_list[2]

    #launching the loading of the default petri dish from here
    edu_main_controller.m_edu_workspace_ctrl.m_session_mdl.m_session_mdtr.emit( 
      qt.PYSIGNAL("doInitializeAvidaPhaseISig"),
      (edu_main_controller.m_edu_workspace_ctrl.m_session_mdl.m_tempdir,))


  finally:
    s_splash.clear()
    s_splash.hide()

    print """
    
    To disable debugging messages, type 'AvidaGui2.descr.DEBUG = False'.

    Type 'avida_ed=AvidaEd()' to reload the AvidaEd user interface.
    
    """
  return edu_main_controller


res_dir = os.getcwd()

def AvidaConsole():
  sys.exc_clear()
  sys.exc_traceback = sys.last_traceback = None
  s_splash.message("Loading Avida-ED in console mode ...")
  s_splash.show()

  avida = None

  genesis_file_name = "./genesis.default"

  try:
    Reload()
    from pyAvida import pyAvida

    s_splash.message("Running Avida-ED in console mode ...")

    os.chdir(os.path.join(res_dir, "console_work"))
    print "os.getcwd()", os.getcwd()
    genesis = AvidaCore.cGenesis()
    genesis.Open(AvidaCore.cString(genesis_file_name))
    if 0 == genesis.IsOpen():
      print "Unable to find file " + genesis_file_name
      return None
    avida = pyAvida()
    avida.construct(genesis)

  finally:
    s_splash.clear()
    s_splash.hide()

    print """
    
    To disable debugging messages, type 'AvidaGui2.descr.DEBUG = False'.

    Type 'avida_ed=AvidaEd()' to reload the AvidaEd user interface.
    
    """
  return avida

def LoopTest():
  for i in range(10):
    avida = AvidaConsole()
    for j in range(200):
      avida.m_avida_threaded_driver.ProcessUpdate()
    os.rename('average.dat', 'average.dat.%d' % i)

# Instantiate (or reinstantiate) the Avida-ED gui. Return controller for
# new gui instance.
#
def AvidaEdDriverRefactor():
  sys.exc_clear()
  sys.exc_traceback = sys.last_traceback = None
  s_splash.message("Loading Avida-ED user-interface ...")
  s_splash.show()

  try:
    Reload()
    from AvidaGui2.pyEduMainCtrl import pyEduMainCtrl
    s_splash.message("Driver- Refactor Test...")

  finally:
    s_splash.clear()
    s_splash.hide()

  return

