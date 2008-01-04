from pyAvidaCoreData import pyAvidaCoreData
from pyAvidaThreadedDriver import pyAvidaThreadedDriver
from pyEduSessionMenuBarHdlr import pyEduSessionMenuBarHdlr
from pyMdl import pyMdl
from pyMdtr import pyMdtr
from pySessionControllerFactory import pySessionControllerFactory
from pySessionDumbCtrl import pySessionDumbCtrl
from pySessionWorkThreadHdlr import pySessionWorkThreadHdlr

from AvidaCore import cString

import qt

class pySessionCtrl(qt.QObject):
  tmp_session_key_idx = 0
  def __init__(self, main_mdl):
    print("XXX blah! pySessionCtrl.__init__()...:")
    print("""
    FIXME : class pyMainControllerData : AvidaGui/py_main_controller_data.py :
    I'm using the wrong locking model in the driver threads...
    I need to lock on access to the Avida core library, rather than on
    per-thread locks (in order to protect static data in the library).
    """)
    # create information model
    self.session_mdl = pyMdl()
    self.session_mdl.main_mdl = main_mdl
    # XXX
    self.session_mdl.session_key = self.tmp_session_key_idx
    self.session_mdl.main_mdl.sessions_dict[self.session_mdl.session_key] = self
    self.tmp_session_key_idx += 1
    # create session mediator
    self.session_mdl.session_mdtr = pyMdtr()
    # create session controller factory
    self.session_mdl.session_controller_factory = pySessionControllerFactory(self.session_mdl)
    self.session_mdl.session_controllers_list = []
    # XXX temporary. in future, name will be given by user.
    self.session_mdl.genesis_filename = cString('genesis')
    print("XXX pySessionCtrl.__init__() done.")
  def construct(self):
    print("XXX pySessionCtrl.construct()...:")
    # create an avida processing thread
    self.session_mdl.session_controller_factory.construct()
    self.session_mdl.avida_core_data = pyAvidaCoreData(self.session_mdl.genesis_filename)
    self.session_mdl.avida_core_data.construct()

    # connect various session controller creators to the controller
    # factory.
    self.session_mdl.session_controller_factory.addControllerCreator("pyEduSessionMenuBarHdlr", pyEduSessionMenuBarHdlr)
    self.session_mdl.session_controller_factory.addControllerCreator("pySessionDumbCtrl", pySessionDumbCtrl)
    self.session_mdl.session_controller_factory.addControllerCreator("pySessionWorkThreadHdlr", pySessionWorkThreadHdlr)

    self.session_mdl.session_mdtr.session_controller_factory_mdtr.emit(
      qt.PYSIGNAL("newSessionControllerSig"), ("pyEduSessionMenuBarHdlr",))
    ## XXX temporary.
    self.session_mdl.session_mdtr.session_controller_factory_mdtr.emit(
      qt.PYSIGNAL("newSessionControllerSig"), ("pySessionWorkThreadHdlr",))
    ## XXX temporary. cause instantiation of a dumb gui for testing.
    self.session_mdl.session_mdtr.session_controller_factory_mdtr.emit(
      qt.PYSIGNAL("newSessionControllerSig"), ("pySessionDumbCtrl",))
  def destruct(self):
    print("pySessionCtrl.destruct()...")
    print("""
    FIXME : class pySessionCtrl : AvidaGui/pySessionCtrl.py :
    destruct() doesn't clean-up correctly (halt the processing thread,
    then delete it before exiting), probably because I'm locking
    per-thread rather than on the Avida library.

    I'm using the wrong locking model in the driver threads...
    I need to lock on access to the Avida core library, rather than on
    per-thread locks (in order to protect static data in the library).
    """)
    # Explicit deletion, rather than reliance on refcounting, permits
    # the thread to delete itself in a thread-safe way.
    self.session_mdl.session_controllers_list.reverse()
    #self.avida_threaded_driver = self.session_mdl.avida_threaded_driver
    #for controller in self.session_mdl.session_controllers_list:
    #  print "pySessionCtrl.destruct() deleting controller:", controller
    #  controller.destruct()
    #  del controller

    while 0 < len(self.session_mdl.session_controllers_list):
      print "pySessionCtrl.destruct() againg deleting controller:", self.session_mdl.session_controllers_list[0]
      del self.session_mdl.session_controllers_list[0]

    if hasattr(self.session_mdl, "avida_threaded_driver"):
      del self.session_mdl.avida_threaded_driver
    else:
      print("pySessionCtrl.destruct() self.session_mdl.avida_threaded_driver missing.")

    del self.session_mdl.avida_core_data
    del self.session_mdl.session_controller_factory
    del self.session_mdl.session_mdtr
    del self.session_mdl.main_mdl

    if hasattr(self, "session_mdl"):
      del self.session_mdl
    else:
      print("pySessionCtrl.destruct() self.session_mdl missing.")

    print("pySessionCtrl.destruct() done.")

  def __del__(self):
    print("pySessionCtrl.__del__()...")
    self.destruct()
    print("pySessionCtrl.__del__() done.")
