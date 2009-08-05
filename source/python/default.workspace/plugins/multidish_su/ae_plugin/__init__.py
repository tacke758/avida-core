from qt import *
from qtnetwork import *

#from testclient import Form1
from time import *

import os
import sys
import stat
import tempfile

import sqlite3

from apihelper import info

# from socktools import *
from avidacomm import *


# MultiDish UI
#from pyMultiDishView import *
#from pyMultiDish_PetriDishCtrl import pyMultiDish_PetriDishCtrl
#from pyPetriConfigureCtrl import *
# from pyMultiDish_TimelineCtrl import *



print """
Plugin MultiDish SU in __init__.py
"""

image_md_icon_data = \
    "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x00\x00\x00\x0d\x49\x48\x44\x52" \
    "\x00\x00\x00\x20\x00\x00\x00\x20\x08\x02\x00\x00\x00\xfc\x18\xed" \
    "\xa3\x00\x00\x00\x01\x73\x52\x47\x42\x00\xae\xce\x1c\xe9\x00\x00" \
    "\x00\x09\x70\x48\x59\x73\x00\x00\x0b\x13\x00\x00\x0b\x13\x01\x00" \
    "\x9a\x9c\x18\x00\x00\x00\x07\x74\x49\x4d\x45\x07\xd9\x08\x05\x01" \
    "\x25\x08\x19\xa1\x6e\x56\x00\x00\x01\x4b\x49\x44\x41\x54\x48\xc7" \
    "\xb5\x55\xb1\x6e\xc2\x40\x0c\xf5\x9d\xa2\x16\x95\x25\x19\x40\x4c" \
    "\x9d\x8a\x04\xbf\x03\x23\xfd\xbc\x32\xc2\xef\xb4\x12\x53\x59\x8a" \
    "\xba\x24\x0b\x12\xb0\x84\x21\xd2\xd5\x71\xee\x6c\xdf\x5d\xb1\x32" \
    "\x9c\x2f\xc9\x7b\xb6\x9f\x7d\x67\xca\x5d\x0d\x6a\x6b\xd6\x15\x00" \
    "\xfc\xbc\xbd\xe8\x7f\xb1\x90\x64\xb3\xc3\xf9\xb1\x04\x7a\x8e\x82" \
    "\x7f\x5d\xaf\xca\x9e\xdf\xb6\x84\xe3\x34\x1f\x67\x11\x54\xfb\xc6" \
    "\xab\x41\x17\xbe\x88\x9e\x55\x22\x0d\x7a\x04\x01\xa9\x95\x12\x5d" \
    "\x4b\x40\x95\x88\x31\x23\xce\x01\x8f\x2e\xa6\x22\x67\xe0\x74\xae" \
    "\xf6\x8d\x31\xc6\x18\x73\x9a\x8f\xdd\x93\x5e\x22\x1c\x38\xe9\xa5" \
    "\xa8\x69\xb0\xca\x52\x30\x1c\x3c\xab\xc5\xe8\x39\x62\x12\x74\xc7" \
    "\x61\x09\x6e\x26\x07\x8e\xbd\x5b\x17\x2e\xfd\x7a\x55\x46\xd5\x81" \
    "\x69\x2a\xff\x11\xd2\xf6\xcf\x19\xe2\x86\xcc\xfb\x19\xde\xb4\xf0" \
    "\x60\x2b\x9e\x3f\x16\xce\xc1\xeb\xa1\x0b\x00\xd7\xf7\x2f\x00\xb8" \
    "\x7e\x4f\x7a\x9b\x7d\x97\x6c\x7a\x4e\xd3\xcb\xe6\x13\x2f\x46\xdb" \
    "\xa5\x37\xb4\xa7\xd7\x5f\xbc\xb8\x1d\xa7\xda\x41\xc3\x88\x21\x74" \
    "\x82\x18\x42\x0f\x6a\xd0\xe1\x32\xe8\x18\x97\x41\xe7\x44\x16\xd1" \
    "\xc5\xd8\x29\x81\x12\x91\x08\x20\xb2\xa6\xb7\x29\xcf\xf1\x77\x1f" \
    "\x0c\x7b\xd1\xdb\x51\xc9\x25\x12\x08\x42\x73\xc0\xb7\x66\xca\x9d" \
    "\x3c\x4c\x45\x83\xae\x25\x48\x46\x57\x95\x28\x53\x06\x39\x03\xd7" \
    "\xbe\xa3\xed\xb2\xbb\x93\x6f\xc7\x29\x7e\xfe\xa1\x44\x51\x23\x92" \
    "\x28\x72\x32\xc7\x1d\x40\xd9\x9f\x3e\x94\xa4\x0e\xa9\x00\x00\x00" \
    "\x00\x49\x45\x4e\x44\xae\x42\x60\x82"



class MultiDish(QWidget):
  def printFlush(self,thestr):
    print thestr
    sys.stdout.flush()

  def __init__(self, workspace_ctrl, name = None, fl = 0):
    QWidget.__init__(self, workspace_ctrl.m_widget_stack, name, fl)

    # @WRE: taking OnePop as a guide
    self.setEnabled(1)

    # ---------- Symbolics (from client.py)
    self.APPNAME = "Avida-Ed Client"
    self.APPVER = "v0.2"
    
    self.bufsize = 4096
    self.host = 'localhost'
    self.port = 6712
    self.rxbuffer = ""
    self.connected = 0
    self.running = 1

    self.ip_address = ""
    self.connected = False
    # Polling interval in ms
    self.socket_poll_interval = 1000
    self.socket_timeout = 0.1        # Has to be small to retain responsiveness of GUI

    self.m_workspace_ctrl = workspace_ctrl
    # This is the title of this viewer's entry in the nav bar
    self.m_plugin_title = "Multi-Dish"
    # Add self to nav bar's list view's dictionary of viewers
    self.m_workspace_ctrl.m_nav_bar_ctrl.m_list_view.m_viewer_dict[self.m_plugin_title] = self
    # Create an entry in the nav bar for this new viewer
    cli = QListViewItem(self.m_workspace_ctrl.m_nav_bar_ctrl.m_list_view, ' ' + self.m_plugin_title)
    # cli.setText(0, self.m_plugin_title)
    # TODO@kgn: make an image for an icon
    print "adding an icon for the plugin"
    self.m_plugin_icon_img = QPixmap()
    self.m_plugin_icon_img.loadFromData(image_md_icon_data,"PNG")

    # This works...
    # self.m_plugin_icon_img = QPixmap(image_pop)
    if (None != self.m_plugin_icon_img):
      cli.setPixmap(0,self.m_plugin_icon_img)
    else:
      print "Error in loading image!"
    print "Added %s to %s" % (self.m_plugin_icon_img,cli)

    # Attach to nav bar's cli to ctrl dicts.
    self.m_workspace_ctrl.m_cli_to_ctrl_dict[cli] = self
    self.m_workspace_ctrl.m_ctrl_to_cli_dict[self] = cli

    # Attach QActions for this viewer to the m_workspace_ctrl
    self.viewMultiDishAction = QAction(self.m_workspace_ctrl, "viewMultiDishAction")
    self.viewMultiDishAction.addTo(self.m_workspace_ctrl.View)
    self.viewMultiDishAction.setText(self.m_plugin_title)
    self.viewMultiDishAction.setMenuText(self.m_plugin_title)
    self.viewMultiDishAction.setToolTip('Open ' + self.m_plugin_title + ' Viewer')

    self.connect( self.viewMultiDishAction,SIGNAL("activated()"), self.viewActionSlot)

    print "widget setup"

    # Adding some widgets to poke
    self.v_box_layout = QVBoxLayout(self,0,6)

    # @WRE: taking OnePop as a guide
    self.v_box_layout.setResizeMode(QLayout.Minimum)

    # Don't need this anymore?
    #self.push_button = QPushButton("Hi.", self)
    #self.v_box_layout.addWidget(self.push_button)

    # self.md_form = pyMultiDishView(self)
    # from pyMDBUint1 import MDBUForm
    
    print "MD Setup form"
    from pyMDSetupFormView import md_setup_Form

    self.md_form = md_setup_Form(self)
    self.v_box_layout.addWidget(self.md_form)

    # @WRE: taking OnePop as a guide
    self.resize(QSize(528,540).expandedTo(self.minimumSizeHint()))
    self.clearWState(Qt.WState_Polished)

    # Put together the dictionary of command functions for communication
    #self.commandDict = {
    #  0:self.doSendRequest,
    #  1:self.doJoinRequest,
    #  2:self.doRemoveRequest,
    #  3:self.doSubmitRequest, 
    #  4:self.doUpdateRequest
    #  }
    #self.commandDictStr = {
    #  "Send Request":self.doSendRequest,
    #  "Join Request":self.doJoinRequest,
    #  "Update Request":self.doUpdateRequest, 
    #  "Remove Request":self.doRemoveRequest,
    #  "Submit Request":self.doSubmitRequest 
    #  }

  def construct(self, avida_ed_session_model):
    """
    MultiDish.construct(...)
    """
    print "MultiDish[v.0.0.1b].construct(avida_ed_session_model:", avida_ed_session_model, ")"
    self.m_session_mdl = avida_ed_session_model

    # Setup form UI logic
    from pyMDSetupFormCtrl import pyMDSetupFormCtrl
    self.m_md_setup_ctrl = pyMDSetupFormCtrl()
    self.m_md_setup_ctrl.construct(self.md_form,self.m_session_mdl)

    # Initialize widgets as needed
    self.md_form.MDDiversity_textEdit.clear()
    #self.md_form.MDDiversity_textEdit.append("Testing append.")
    self.m_session_mdl.md_diversity = {}

    self.md_form.MDRunStatistics_textEdit.clear()
    #self.md_form.MDRunStatistics_textEdit.append("Testing append.")
    self.m_session_mdl.md_runstats = {}

    # m_session_mdl.m_current_workspace

    print "MultiDish.construct setting up signals and slots\n"
    #self.connect(self.push_button, SIGNAL("clicked()"),
    #             self.PushButtonClickSlot)
    #self.connect(self.md_form.IPPushButton, SIGNAL("clicked()"),
    #             self.IPPushButtonSlot)
    #self.connect(self.md_form.SendPushButton, SIGNAL("clicked()"),
    #             self.SendPushButtonSlot)

    self.connect(self.md_form.md_set_values_pushButton, SIGNAL("clicked()"),
                 self.m_md_setup_ctrl.MDSSetValuesButtonClick)
    #self.connect(self.md_form.md_set_values_pushButton, SIGNAL("clicked()"),
    #             self.MDSSetValuesButtonSlot)

    self.connect(self.md_form.sd_list_refresh_pushButton, SIGNAL("clicked()"),
                 self.m_md_setup_ctrl.MDSSubDishListRefreshButtonClick)

    self.connect(self.md_form.sd_add_pushButton, SIGNAL("clicked()"),
                 self.m_md_setup_ctrl.MDSSubDishAddButtonClick)

    self.connect(self.md_form.md_generate_pushButton, SIGNAL("clicked()"),
                 self.m_md_setup_ctrl.MDSGenerateMDButtonClick)

    self.connect(self.m_session_mdl.m_session_mdtr,PYSIGNAL("haveMDDiversityUpdate"),
                self.m_md_setup_ctrl.updateMDDiversitySlot)
    self.connect(self.m_session_mdl.m_session_mdtr,PYSIGNAL("haveMDRunStatsUpdate"),
                self.m_md_setup_ctrl.updateMDRunStatsSlot)

    self.connect(self.m_session_mdl.m_session_mdtr,PYSIGNAL("refreshSubDishList"),
                self.m_md_setup_ctrl.MDSSubDishListRefreshButtonClick)

    # Connect UI elements to Avida controller methods
    #self.connect(self.md_form.avidaRunPushButton, SIGNAL("clicked()"),
    #             self.doPostClick)

    # self.connect(self.md_form.avidaRunPushButton, SIGNAL("clicked()"),
    #             self.m_avida_ctrl.doStartAvidaSlot)


    # Create a temporary subdirectory for general use in this session. Add a                                                                                                                                                              
    # subdirectory to that for output files -- these files will only get                                                                                                                                                                  
    # put into named directories when frozen                                                                                                                                                                                              

    self.m_session_mdl.m_md_tempdir = tempfile.mkdtemp('','AvidaEd-MD-pid%d-'%os.getpid())
    self.m_session_mdl.m_md_tempdir_out = os.path.join(self.m_session_mdl.m_md_tempdir, "output")
    os.mkdir(self.m_session_mdl.m_md_tempdir_out)


    self.printFlush("MultiDish.construct done.\n")


  # The other part of having a timer, throwing signals
  # def timerEvent(self, ev):
  #  self.emit(PYSIGNAL("TestClientTimeSignal"), ())

  # 
  # Standard methods for reporting on plugin status
  #
  def aboutToBeRaised(self, workspace_ctrl):
    self.printFlush('MultiDish.aboutToBeRaised.')

  def aboutToBeLowered(self, workspace_ctrl):
    self.printFlush('MultiDish.aboutToBeLowered.')

  def viewActionSlot(self):
    self.printFlush('MultiDish.viewActionSlot.')
    self.m_workspace_ctrl.navBarItemClickedSlot(self.m_workspace_ctrl.m_ctrl_to_cli_dict[self])

  def HandleDropEvent(self, dropped_item_name):
    self.printFlush('MultiDish.HandleDropEvent: dropped_item_name:' +  dropped_item_name)
    self.m_session_mdl.m_md_session_mdtr.emit(PYSIGNAL("raiseViewSig"),(self,))
  #
  # End of standard plugin methods section
  # 


  #
  # Begin plugin-specific method declarations
  #

  #def PushButtonClickSlot(self):
  #  print "TestClient.PushButtonClickSlot.\n"
  #  # self.status_label.setText("<p align=\"right\">Status Label: button clicked</p>")
  #  self.tc_form.ConsoleTextEdit.insert("IPPushButton clicked: " + self.tc_form.IPLineEdit.displayText())

  #def MDSSetValuesButtonSlot(self):
  #  print "MDSSetValuesButtonSlot"
  #  self.md_form.md_values_textLabel.setText("Testing")

  # 
  # End plugin-specific method declarations
  #

class AEPlugin(object):
  def __init__(self, *args, **kw):
    """
    AEPlugin.__init__(...)
    args: (pyEduWorkspaceCtrl instance)
    kw (an empty dictionary): {}
    """
    print "AEPlugin.__init__(args:", args, "kw:", kw, ")",
    print "hello."
    print "(plugin MultiDish Setup version 0.0.1...)"

    # Expects keyword argument named 'workspace_ctrl'.
    self.m_workspace_ctrl = kw['workspace_ctrl']
    self.m_multidish_ctrl = MultiDish(self.m_workspace_ctrl, "m_multidish_ctrl")

    # reload(pyMultiDish_TimelineCtrl)

  def construct(self, avida_ed_session_model):
    """
    AEPlugin.construct(...)
    """
    print "AEPlugin.construct(avida_ed_session_model:", avida_ed_session_model, ")"
    print "freezer directory:", avida_ed_session_model.m_current_freezer
    self.m_session_mdl = avida_ed_session_model
    self.m_multidish_ctrl.construct(self.m_session_mdl)


