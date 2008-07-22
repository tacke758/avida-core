# -*- coding: utf-8 -*-

from descr import *

# from Get_Avida_ED_version import avida_ed_version_string
from DefaultExportDir import DefaultExportDir
from pyEduWorkspaceView import pyEduWorkspaceView
from pyMdtr import pyMdtr
from pyOneAnalyzeCtrl import pyOneAnalyzeCtrl
from pyOneOrganismCtrl import pyOneOrganismCtrl
from pyOnePopulationCtrl import pyOnePopulationCtrl
from pyTwoAnalyzeCtrl import pyTwoAnalyzeCtrl
from pyTwoOrganismCtrl import pyTwoOrganismCtrl
from pyTwoPopulationCtrl import pyTwoPopulationCtrl
from pyPetriConfigureCtrl import pyPetriConfigureCtrl
from pyQuitDialogCtrl import pyQuitDialogCtrl
from pyDefaultFiles import pyDefaultFiles
from pyReadFreezer import pyReadFreezer
from pyWarnAboutTrashCtrl import pyWarnAboutTrashCtrl
# from pyImportItemCtrl import pyImportItemCtrl
from pyBeforeStartingCtrl import pyBeforeStartingCtrl
from pyHelpScreenCtrl import pyHelpScreenCtrl
from pyAboutScreenCtrl import pyAboutScreenCtrl
from pyRenameDialogCtrl import pyRenameDialogCtrl
import pyNewIconView
import os.path, shutil
from qt import *

class pyEduWorkspaceCtrl(pyEduWorkspaceView):

  def construct(self, session_mdl):
    self.m_session_mdl = session_mdl
    self.m_avida = None
    self.startStatus = True
    self.m_nav_bar_ctrl.construct(session_mdl)
    # self.m_nav_bar_list_view.construct(session_mdl)
    self.m_freezer_ctrl.construct(session_mdl)
    self.m_cli_to_ctrl_dict = {}
    self.m_ctrl_to_cli_dict = {}
    self.curr_sel_files = ""
    self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("initializeWithDefaultPetriDishSig"),
       self.Restart_ExpActionSlot)
    self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("freezerItemsSelectedSig"),
       self.ModifyFreezerItemMenuItemsSlot)



    if (session_mdl.directory_chosen == False):
      self.setCaption('%s - %s' % ("Avida-ED", "Unnamed  Workspace") )
    else:
      self.setCaption('%s - %s' % ("Avida-ED", os.path.basename(os.path.splitext(self.m_session_mdl.m_current_workspace)[0])) )
   
    while self.m_widget_stack.visibleWidget():
      self.m_widget_stack.removeWidget(self.m_widget_stack.visibleWidget())

    self.m_one_population_ctrl  = pyOnePopulationCtrl(self.m_widget_stack, "m_one_population_ctrl")
    self.m_one_organism_ctrl    = pyOneOrganismCtrl(self.m_widget_stack, "m_one_organism_ctrl")
    self.m_one_analyze_ctrl     = pyOneAnalyzeCtrl(self.m_widget_stack, "m_one_analyze_ctrl")

    for (cli, ctrl) in (
      (self.m_nav_bar_ctrl.m_one_population_cli, self.m_one_population_ctrl),
      (self.m_nav_bar_ctrl.m_one_organism_cli,   self.m_one_organism_ctrl),
      (self.m_nav_bar_ctrl.m_one_analyze_cli,    self.m_one_analyze_ctrl),
    ):
      self.m_cli_to_ctrl_dict[cli] = ctrl
      self.m_ctrl_to_cli_dict[ctrl] = cli

    #self.m_nav_bar_ctrl.m_one_population_cli.setState(QCheckListItem.On)
    #self.m_nav_bar_ctrl.m_one_population_cli.setState(QCheckListItem.Off)
    #self.m_nav_bar_ctrl.m_one_population_cli.setSelected(True)
    #self.m_nav_bar_ctrl.m_one_population_cli.setSelected(False)

    #for ctrl in self.m_ctrl_to_cli_dict.keys():
    #  ctrl.construct(self.m_session_mdl)
    self.m_one_population_ctrl.construct(self.m_session_mdl)
    self.m_one_organism_ctrl.construct(self.m_session_mdl, self)
    self.m_one_analyze_ctrl.construct(self.m_session_mdl)
        
    self.connect(
      self, PYSIGNAL("quitAvidaPhaseISig"), self.startQuitProcessSlot)
    self.connect(self, PYSIGNAL("quitAvidaPhaseIISig"), qApp, SLOT("quit()"))
    self.connect(
      self.m_nav_bar_ctrl.m_list_view, SIGNAL("clicked(QListViewItem *)"), 
      self.navBarItemClickedSlot)

    # @kgn : desperate hacks to get drag & drop working.
    self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("raisePopViewSig"), self.raisePopViewSlot)
    self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("raiseOrgViewSig"), self.raiseOrgViewSlot)
    self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("raiseAnaViewSig"), self.raiseAnaViewSlot)

    # self.connect(
    #   self.fileOpenFreezerAction,SIGNAL("activated()"),self.freezerOpenSlot)
    self.connect(self.popview_controlNext_UpdateAction,SIGNAL("activated()"), self.next_UpdateActionSlot)
    self.connect(self.popview_controlStartAction,SIGNAL("activated()"),self.startActionSlot)

    # Next three connects  are for where a user selects a veiwer from 
    # the View menu

    self.connect( self.viewPopulationAction,SIGNAL("activated()"), 
      self.viewPopulationActionSlot)
    self.connect( self.viewOrganismAction,SIGNAL("activated()"), 
      self.viewOrganismActionSlot)
    self.connect( self.viewAnalysisAction,SIGNAL("activated()"), 
      self.viewAnalysisActionSlot)
    
    self.connect(
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("setAvidaSig"),
      self.setAvidaSlot)
    self.connect(
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("doPauseAvidaSig"),
      self.doPauseAvidaSlot)
    self.connect(
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("doStartAvidaSig"),
      self.doStartAvidaSlot)

    self.connect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("doStartSig"),
      self.doStartAvidaSlot)
    self.connect(
      self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("doPauseSig"),
      self.doPauseAvidaSlot)
    # self.connect(
    #   self.m_session_mdl.m_session_mdtr,
    #   PYSIGNAL("doNextUpdateSig"),
    #   self.updatePBClickedSlot)

    self.connect(self.popview_controlRestart_ExpAction,SIGNAL("activated()"),
      self.Restart_ExpActionSlot)
    self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("addStatusBarWidgetSig"), self.addStatusBarWidgetSlot)
    self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("removeStatusBarWidgetSig"), self.removeStatusBarWidgetSlot)
    self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("statusBarMessageSig"), self.statusBarMessageSlot)
    self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("statusBarClearSig"), self.statusBarClearSlot)
    self.connect(
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("workspaceNewSig"),
      self.fileNew)
    self.connect(
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("workspaceOpenSig"),
      self.fileOpen)

    # If the user drags something into the trash can

    self.connect(self.TrashCanIconView, PYSIGNAL("DroppedOnNewIconViewSig"),
      self.DroppedInTrashSlot)

    # Start the program with the population viewer

    self.navBarItemClickedSlot(self.m_nav_bar_ctrl.m_one_population_cli)
    self.m_nav_bar_ctrl.m_list_view.setSelected(self.m_nav_bar_ctrl.m_one_population_cli, True)
    self.splitter1.setSizes([150,500,100])

    # set up the trash can to have one trash can icon that can not be selected

    # self.TrashCanIconView.setItemTextPos(QIconView.Right)
    # self.TrashCanIconView.setSpacing(1)
    self.TrashCanIconView.setVScrollBarMode(QIconView.AlwaysOff)
    self.TrashCanIconView.setHScrollBarMode(QIconView.AlwaysOff)
    self.TrashCanIconView.setSelectionMode(QIconView.NoSelection)
    self.TrashCanIconView.setAutoArrange(False)
    # TCIcon = pyNewIconView.TrashIconViewItem(self.TrashCanIconView)

    self.show()

  def __del__(self):
    for key in self.m_cli_to_ctrl_dict.keys():
      del self.m_cli_to_ctrl_dict[key]
    for key in self.m_ctrl_to_cli_dict.keys():
      del self.m_ctrl_to_cli_dict[key]

  # When user clicks on one of the icons in the navigation bar go to the 
  # correct viewer

  def navBarItemClickedSlot(self, item):
    if item:
      if self.m_cli_to_ctrl_dict.has_key(item):
        controller = self.m_cli_to_ctrl_dict[item]
        old_controller = self.m_widget_stack.visibleWidget()
        if old_controller is not None:
          old_controller.aboutToBeLowered(self)
        controller.aboutToBeRaised(self)
        self.m_widget_stack.raiseWidget(controller)

  # @kgn : desperate hacks to get drag & drop working.
  def switchToView(self, cli):
    self.m_nav_bar_ctrl.m_list_view.setSelected(self.m_nav_bar_ctrl.m_one_population_cli, False)
    self.m_nav_bar_ctrl.m_list_view.setSelected(self.m_nav_bar_ctrl.m_one_organism_cli, False)
    self.m_nav_bar_ctrl.m_list_view.setSelected(self.m_nav_bar_ctrl.m_one_analyze_cli, False)
    self.m_nav_bar_ctrl.m_list_view.setSelected(cli, True)
    self.navBarItemClickedSlot(cli)
  def raisePopViewSlot(self):
    self.switchToView(self.m_nav_bar_ctrl.m_one_population_cli)
  def raiseOrgViewSlot(self):
    self.switchToView(self.m_nav_bar_ctrl.m_one_organism_cli)
  def raiseAnaViewSlot(self):
    self.switchToView(self.m_nav_bar_ctrl.m_one_analyze_cli)

  # when user clicks on an item (or items) in the freezer list change the 
  # menu to reflect the choice

  def ModifyFreezerItemMenuItemsSlot(self, file_list):
    self.curr_sel_files = file_list
    itemsPicked = file_list.split("\t")[1:]
    if (len(itemsPicked) == 0):
      file_name_str = "Freezer Item"
      item_enable_flag = False
    elif (len(itemsPicked) == 1):
      file_name_str = itemsPicked[0].strip()
      file_name_str = os.path.basename(file_name_str)
      if (file_name_str.endswith("empty")):
        file_name_str = "Dish " + file_name_str[:-6]
      elif (file_name_str.endswith("full")):
        file_name_str = "Dish " + file_name_str[:-5]
      elif (file_name_str.endswith("organism")):
        file_name_str = "Organism " + file_name_str[:-9]
      item_enable_flag = True
    else:
      file_name_str = str(len(itemsPicked)) + " Freezer Items"
      item_enable_flag = True
    self.fileRenameItemAction.setMenuText("Rename " + file_name_str)
    self.fileExportItemAction.setMenuText("Export " + file_name_str)
    self.fileDeleteItemAction.setMenuText("Delete " + file_name_str)
    self.fileOpenItemAction.setMenuText("Open " + file_name_str)

    self.fileRenameItemAction.setEnabled(item_enable_flag)
    self.fileExportItemAction.setEnabled(item_enable_flag)
    self.fileDeleteItemAction.setEnabled(item_enable_flag)
    self.fileOpenItemAction.setEnabled(item_enable_flag)
    if ((item_enable_flag == True) and (len(itemsPicked) > 1)):
      self.fileRenameItemAction.setEnabled(False)
      self.fileExportItemAction.setEnabled(False)
      self.fileOpenItemAction.setEnabled(False)

  def close(self, also_delete = False):
    self.emit(PYSIGNAL("quitAvidaPhaseISig"), ())
    return False


  def __del__(self):
    pass
    # print "pyEduWorkspaceCtrl.__del__(): Not implemented yet"

  def __init__(self, parent = None, name = None, fl = 0):
    pyEduWorkspaceView.__init__(self,parent,name,fl)
    self.help_screen = None

    self.popview_controlStartAction.setVisible(False)
    self.popview_controlNext_UpdateAction.setVisible(False)
    self.popview_controlRestart_ExpAction.setVisible(False)

    self.orgview_controlStartAction.setVisible(False)
    self.orgview_controlNext_InstructionAction.setVisible(False)
    self.orgview_controlPrevious_InstructionAction.setVisible(False)
    self.orgview_controlGo_To_StartAction.setVisible(False)
    self.orgview_controlGo_To_EndAction.setVisible(False)

    self.anaview_controlNo_controls_available_in_Analysis_ViewAction.setVisible(False)

  # public slot

  def fileNew(self):

    # loop till the users selects a directory name that does not exist or
    # chooses the cancel button

    created = False
    dialog_caption = "Enter the name of a new workspace"
    while (created == False):

      # if user has not yet selected a dir try for the ~/Documents on the Mac
      # ~ on Linux and $HOME/My Documents on Windows

      if not self.m_session_mdl.directory_chosen:
        initial_dir = os.path.expanduser("~")
        if os.path.exists(os.path.join(initial_dir,"Documents")):
          initial_dir = os.path.join(initial_dir,"Documents")
        elif os.path.exists(os.path.join(initial_dir,"My Documents")):
          initial_dir = os.path.join(initial_dir,"My Documents")
      else:
        initial_dir = ""
      new_dir = QFileDialog.getSaveFileName(
                    initial_dir,
                    "Workspace (*.workspace)",
                    None,
                    "new workspace",
                    dialog_caption);
      new_dir = str(new_dir)
      if (new_dir.strip() == ""):
        created = True
      else:
        if (new_dir.endswith(".workspace") != True):
          new_dir = new_dir + ".workspace"
          if os.path.exists(new_dir):
            dialog_caption = new_dir + " already exists"
          else:
            os.mkdir(new_dir)
            os.mkdir(os.path.join(new_dir,"freezer"))
            os.mkdir(os.path.join(new_dir,"freezer","@example.full"))
            filesToCopy = ["environment.default", "inst_set.default", 
                           "events.default", "avida_cfg.default", 
                           os.path.join("freezer", "@default.empty"), 
                           os.path.join("freezer", "@example.empty"), 
                           os.path.join("freezer", "@ancestor.organism"),
                     os.path.join("freezer", "@example.full", "average.dat"),
                     os.path.join("freezer", "@example.full", "count.dat"),
                     os.path.join("freezer", "@example.full", "petri_dish")]
            for fileName in filesToCopy:
              sourceName = os.path.join(self.m_session_mdl.m_current_workspace,
                           fileName)
              destName = os.path.join(new_dir, fileName)
              if (os.path.exists(sourceName)):
                shutil.copyfile(sourceName, destName)
              else:
                pyDefaultFiles(fileName, destName)
            self.m_session_mdl.m_current_workspace = str(new_dir)
            self.m_session_mdl.m_current_freezer = os.path.join(new_dir, "freezer")
            self.m_session_mdl.directory_chosen = True
            self.setCaption('%s - %s' % ("Avida-ED", os.path.basename(os.path.splitext(self.m_session_mdl.m_current_workspace)[0])) )
            self.m_session_mdl.m_session_mdtr.emit(
              PYSIGNAL("doRefreshFreezerInventorySig"), ())
            created = True

  # public slot
  def fileOpen(self):

    tmpDialogCap = "Choose a workspace (folder with name ending .workspace)"
    foundFile = False
    workspace_dir = ""

    # self.m_session_mdl.m_current_workspace does not work correctly in this
    # context (at least on the Mac).  It is a relative path where 
    # getExistingDirectory seems to require an absult path

    while (foundFile == False):

      # if user has not yet selected a dir try for the ~/Documents on the Mac
      # ~ on Linux and $HOME/My Documents on Windows

      if not self.m_session_mdl.directory_chosen:
        initial_dir = os.path.expanduser("~")
        if os.path.exists(os.path.join(initial_dir,"Documents")):
          initial_dir = os.path.join(initial_dir,"Documents")
        elif os.path.exists(os.path.join(initial_dir,"My Documents")):
          initial_dir = os.path.join(initial_dir,"My Documents")
      else:
        initial_dir = ""

      workspace_dir = QFileDialog.getExistingDirectory(
                    # self.m_session_mdl.m_current_workspace,
                    initial_dir,
                    None,
                    "get existing workspace",
                    tmpDialogCap,
                    True);
      workspace_dir = str(workspace_dir)              
      workspace_dir = workspace_dir.strip()

      # If a valid name is found or the user hit cancel leave the loop
      # otherwise prompt for the wrong name and continue
      
      if (workspace_dir == "") or (workspace_dir.endswith(".workspace")):
        foundFile = True
      else:
        foundFile = False
        tmpDialogCap = "Name did not end with .workspace, please try again"

    if workspace_dir != "":
      self.m_session_mdl.m_current_workspace = str(workspace_dir)
      self.m_session_mdl.m_current_freezer = os.path.join(self.m_session_mdl.m_current_workspace, "freezer")
      self.m_session_mdl.directory_chosen = True
      self.setCaption('%s - %s' % ("Avida-ED", os.path.basename(os.path.splitext(self.m_session_mdl.m_current_workspace)[0])) )
      self.m_session_mdl.m_session_mdtr.emit(
        PYSIGNAL("doRefreshFreezerInventorySig"), ())

  # public slot

  def freezerOpenSlot(self):
    freezer_dir = QFileDialog.getExistingDirectory(
                    self.m_session_mdl.m_current_freezer,
                    None,
                    "get existing directory",
                    "Choose a directory",
                    True);
    freezer_dir = str(freezer_dir)
    if freezer_dir.strip() != "":
      self.m_session_mdl.m_current_freezer = str(freezer_dir)
      self.m_session_mdl.m_session_mdtr.emit(
        PYSIGNAL("doRefreshFreezerInventorySig"), ())

  # public slot

  def fileSave(self):
    print "pyEduWorkspaceCtrl.fileSave(): Not implemented yet"

  # public slot

  def fileSaveAs(self):

    # loop till the users selects a directory name that does not exist or
    # choses the cancel button

    created = False
    dialog_caption = "Save workspace as:"
    while (created == False):
      new_dir = QFileDialog.getSaveFileName(
                    "",
                    "Workspace (*.workspace)",
                    None,
                    "new workspace",
                    dialog_caption);
      new_dir = str(new_dir)
      if (new_dir.strip() == ""):
        created = True
      else:
        if (new_dir.endswith(".workspace") != True):
          new_dir = new_dir + ".workspace"
          if os.path.exists(new_dir):
            dialog_caption = new_dir + " already exists"
          else:
            shutil.copytree(self.m_session_mdl.m_current_workspace, new_dir)
            self.m_session_mdl.m_current_workspace = str(new_dir)
            self.m_session_mdl.m_current_freezer = os.path.join(new_dir, "freezer")
            #self.setCaption(self.m_session_mdl.m_current_workspace)
            self.setCaption('%s - %s' % ("Avida-ED", os.path.basename(os.path.splitext(self.m_session_mdl.m_current_workspace)[0])) )
            self.m_session_mdl.m_session_mdtr.emit(
              PYSIGNAL("doRefreshFreezerInventorySig"), ())
            created = True

  # public slot

  def filePrint(self):
    "Print petri dish or graph"
    self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("printSig"), ())

  # public slot

  def fileExport(self):
    "Export Avida analysis to a file"
    self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("exportAnalyzeSig"), ())
 
  # public slot

  def fileSaveImages(self):
    "Export images to a file"
    self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("saveImagesSig"), ())

  def fileImportItem(self):
    "Import a freezer item that was previously exported"

    # self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("importFreezerItemSig"), ())
    # m_import_freeze_item = pyImportItemCtrl()
    # item_to_import = m_import_freeze_item.showDialog(self.m_session_mdl)

    # If the user has not already chosen an active workspace for this session
    # make them do so now. If they chose not to pick a workspace, don't let
    # them import the file

    if (self.m_session_mdl.directory_chosen == False):
      m_prompt_dir = pyBeforeStartingCtrl()
      m_prompt_dir.construct(self.m_session_mdl)
      if (m_prompt_dir.showDialog("import") == 0):
        return ''

    tmpDialogCap = "Select an exported dish or organism (*.aex)"
    foundFile = False
    input_item_name = ""
    initial_dir = ""

    while (foundFile == False):

      # start on the ~/Documents on the Mac ~ on Linux and $HOME/My 
      # Documents on Windows

      if (len(initial_dir.strip()) == 0):
        initial_dir = os.path.expanduser("~")
        if os.path.exists(os.path.join(initial_dir,"Documents")):
          initial_dir = os.path.join(initial_dir,"Documents")
        elif os.path.exists(os.path.join(initial_dir,"My Documents")):
          initial_dir = os.path.join(initial_dir,"My Documents")

      input_item_name = QFileDialog.getOpenFileName(
                    initial_dir,
                    "Avida-ED Export Files (*.aex)",
                    None,
                    "Import Item",
                    tmpDialogCap,
                    "Configured Dishes",
                    True)
      input_item_name = str(input_item_name)
      input_item_name = input_item_name.strip()

      # If a valid name is found or the user hit cancel leave the loop
      # otherwise prompt for the wrong name and continue

      if (input_item_name == "") or (input_item_name.endswith(".aex")):
        foundFile = True
      else:
        foundFile = False
        tmpDialogCap = "Not a valid file.  Please try again"

    if input_item_name != "":
      junk, input_core_file_name = os.path.split(input_item_name)
      input_name_no_ext, junk = os.path.splitext(input_core_file_name) 
      input_file =  open(input_item_name,"r")
      try:
        for line in input_file:

          # If we find a *File line open a new file

          foundFile = False
          suffix = ""
          while ((line.startswith("*File:")) and (foundFile == False)):
            if (line.find(".empty") > -1):
              new_file_name = os.path.join(self.m_session_mdl.m_current_freezer,
                         input_name_no_ext + suffix + ".empty")
            elif (line.find(".organism") > -1):
              new_file_name = os.path.join(self.m_session_mdl.m_current_freezer,
                         input_name_no_ext + suffix + ".organism")
            elif (line.find(".full") > -1):
              new_dir_name = os.path.join(self.m_session_mdl.m_current_freezer,
                         input_name_no_ext + suffix + ".full")
              if (not os.path.exists(new_dir_name)):
                os.mkdir(new_dir_name)
              sub_file_name = line[line.rfind(":")+2:]
              sub_file_name = sub_file_name.strip()
              new_file_name = os.path.join(new_dir_name, sub_file_name)
              
            # Do not allow duplicate files

            if (os.path.exists(new_file_name) == True):
              foundFile = False
              if (suffix == ""):
                suffix = "_1"
              else:
                suffix = "_" + str(int(suffix[1:]) + 1)
            else:
              foundFile = True
              new_file = open(new_file_name.strip(),'w')
          if (not line.startswith("*File:")):
            new_file.write(line)
      finally:
        input_file.close()
        new_file.close()
      self.m_session_mdl.m_session_mdtr.emit(
           PYSIGNAL("doRefreshFreezerInventorySig"), ())

  # public slot

  def fileRenameItem(self):
    if (self.m_session_mdl.directory_chosen == False):
      m_prompt_dir = pyBeforeStartingCtrl()
      m_prompt_dir.construct(self.m_session_mdl)
      if (m_prompt_dir.showDialog("rename") == 0):
        return ''

    files2process = self.curr_sel_files.split("\t")
    rename_dialog = pyRenameDialogCtrl()
    dialog_result = rename_dialog.showDialog(files2process[1:])
    if (dialog_result > 0):
      self.m_session_mdl.m_session_mdtr.emit(
          PYSIGNAL("doRefreshFreezerInventorySig"), ())

  def fileExportItemSlot(self):
    files2process = self.curr_sel_files.split("\t")
    file_name = files2process[1]

    initial_dir = DefaultExportDir(self.m_session_mdl.export_directory, file_name)
    no_ext_name, file_ext = os.path.splitext(os.path.basename(file_name))
    initial_file_name = os.path.join(initial_dir,no_ext_name + ".aex")
  
    export_file_name = QFileDialog.getSaveFileName(
      initial_file_name,
      "Avida_ED export (*.aex);;",
      None,
      "Export Item",
      "Export dish or organism")

    self.m_session_mdl.export_directory = os.path.dirname(str(export_file_name))
    export_file = open(str(export_file_name), "w")
    if (file_ext == '.full'):
      files_in_full = os.listdir(file_name)
      for indiv_file_name in files_in_full:
      
        # Read each file, write the name and content of the file
        # into the export file
      
        if (not indiv_file_name.startswith('.')):
          individual_file = open(os.path.join(file_name,indiv_file_name), "r")
          export_file.write("*File: " + file_ext + ": " + indiv_file_name + "\n")
          lines = individual_file.readlines()
          for line in lines:
            export_file.write(line)
          individual_file.close()
    else:
      individual_file = open(file_name, "r")
      export_file.write("*File: " + file_ext + "\n")
      lines = individual_file.readlines()
      for line in lines:
        export_file.write(line)
      individual_file.close()
    export_file.close()

  # allow user to delete one or more freezer items

  def fileDeleteItemSlot(self):
      if (self.m_session_mdl.directory_chosen == False):
        m_prompt_dir = pyBeforeStartingCtrl()
        m_prompt_dir.construct(self.m_session_mdl)
        if (m_prompt_dir.showDialog("delete") == 0):
          return ''

      for freezer_item_name in self.curr_sel_files.split("\t")[1:]:
        delete_item = False
        if self.m_session_mdl.m_warn_about_trash:
           tmp_prompt = pyWarnAboutTrashCtrl(freezer_item_name)
           prompt_result = tmp_prompt.showDialog()
           if prompt_result == tmp_prompt.DeleteAllCode:
             self.m_session_mdl.m_warn_about_trash = False
           if (prompt_result == tmp_prompt.DeleteAllCode) or (prompt_result == tmp_prompt.DeleteCode):
             delete_item = True
        else:
          delete_item = True
        if delete_item:
          self.m_session_mdl.m_session_mdtr.emit(
            PYSIGNAL("DeleteFromFreezerSig"), (freezer_item_name, ))

  # allow user to open up the first freezer item in the currently selected
  # file list
 
  def fileOpenItemSlot(self):
    file_name = self.curr_sel_files.split("\t")[1]
    if (file_name.endswith(".full")) or (file_name.endswith(".empty")):
      self.m_session_mdl.m_session_mdtr.emit(
        PYSIGNAL("openFreezerItemSig"), (file_name,))
    else:
      self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("raiseOrgViewSig"),())
      self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("parseOrganismFileSig"), 
        (str(file_name),))

  # public slot

  def fileExit(self):
    info("pyEduWorkspaceCtrl.fileExit(): Not implemented yet")

  # public slot

  def editUndo(self):
    info("pyEduWorkspaceCtrl.editUndo(): Not implemented yet")

  # public slot

  def editRedo(self):
    info("pyEduWorkspaceCtrl.editRedo(): Not implemented yet")

  # public slot

  def editCut(self):
    info("pyEduWorkspaceCtrl.editCut(): Not implemented yet")

  # public slot

  def editCopy(self):
    info("pyEduWorkspaceCtrl.editCopy(): Not implemented yet")

  # public slot

  def editPaste(self):
    info("pyEduWorkspaceCtrl.editPaste(): Not implemented yet")

  # public slot

  def editFind(self):
    info("pyEduWorkspaceCtrl.editFind(): Not implemented yet")

  # public slot

  def helpIndex(self):
    self.help_dialog = pyHelpScreenCtrl()
    self.help_dialog.showDialog()
  #  home = QDir("./documentation/index.html").absPath()
  #  if self.help_screen is not None:
  #    self.help_screen.setSource(home)
  #  else:
  #    self.help_screen = pyHelpScreenCtrl(home, ".", None)
  #  #help_dialog.showDialog()
  #  self.help_screen.show()

  #def helpChanges(self):
  #  home = QDir("./documentation/changes.html").absPath()
  #  if self.help_screen is not None:
  #    self.help_screen.setSource(home)
  #  else:
  #    self.help_screen = pyHelpScreenCtrl(home, ".", None)
  #  self.help_screen.show()

  #def helpKnownBugs(self):
  #  home = QDir("./documentation/bugs.html").absPath()
  #  if self.help_screen is not None:
  #    descr("calling setSource")
  #    self.help_screen.setSource(home)
  #  else:
  #    self.help_screen = pyHelpScreenCtrl(home, ".", None)
  #  self.help_screen.show()

  # public slot

  def helpContents(self):
    self.help_dialog = pyHelpScreenCtrl()
    self.help_dialog.showDialog()

  # public slot

  def helpAbout(self):
    about_dialog = pyAboutScreenCtrl()
    about_dialog.showDialog()

  def next_UpdateActionSlot(self):
    self.m_session_mdl.m_session_mdtr.emit(
      PYSIGNAL("fromLiveCtrlUpdateAvidaSig"), ())

  def startActionSlot(self):
    if self.startStatus:
      if (self.m_session_mdl.m_global_num_of_ancestors == 0):
        warningNoMethodName("There were no starting organisms in this population. " + \
                            "Please drag in at least one from the freezer.")
      else:
        self.m_session_mdl.m_session_mdtr.emit(
          PYSIGNAL("fromLiveCtrlStartAvidaSig"), ())
    else:
      self.m_session_mdl.m_session_mdtr.emit(
        PYSIGNAL("fromLiveCtrlPauseAvidaSig"), ())
    
  def setAvidaSlot(self, avida):
    descr("pyEduWorkspaceCtrl.setAvidaSlot() ...")
    old_avida = self.m_avida
    self.m_avida = avida
    if old_avida:
      self.disconnect(
        old_avida.m_avida_thread_mdtr, PYSIGNAL("AvidaUpdatedSig"),
        self.avidaUpdatedSlot)
      del old_avida
    if self.m_avida:
      self.connect(
        self.m_avida.m_avida_thread_mdtr, PYSIGNAL("AvidaUpdatedSig"),
        self.avidaUpdatedSlot)

  def avidaUpdatedSlot(self):
    pass
    
  def doPauseAvidaSlot(self):
    self.popview_controlStartAction.setText("Run")
    self.popview_controlStartAction.setMenuText("Run")
    self.popview_controlStartAction.setAccel(QKeySequence("Ctrl+R"))
    self.startStatus = True
    
  def doStartAvidaSlot(self):
    self.popview_controlStartAction.setText("Pause")
    self.popview_controlStartAction.setMenuText("Pause")
    self.popview_controlStartAction.setAccel(QKeySequence("Ctrl+."))
    self.startStatus = False
    
  def startQuitProcessSlot(self):

    # Be sure that the session is paused before quitting (to reduce confusion
    # if the user decides to save before quiting)

    self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("doPauseAvidaSig"), ())


    # Check if there unsaved petri dishes if there are ask the user if they 
    # want to save them, just quit the program or cancel the quit.  If there
    # are no unsaved populations just quit.
    # (actually only works with one population will need to expand to
    # two populations in the future)

    if (not self.m_one_population_ctrl.m_session_mdl.saved_full_dish and
        not self.m_one_population_ctrl.m_session_mdl.new_full_dish):
      m_quit_avida_ed = pyQuitDialogCtrl()
      quit_return = m_quit_avida_ed.showDialog()
      if quit_return == m_quit_avida_ed.QuitFlag:
        self.emit(PYSIGNAL("quitAvidaPhaseIISig"), ())
      elif quit_return == m_quit_avida_ed.FreezeQuitFlag:
        self.m_one_population_ctrl.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("freezeDishPhaseISig"), (False, True, ))
    else:
      self.emit(PYSIGNAL("quitAvidaPhaseIISig"), ())

  def Restart_ExpActionSlot(self):
    # If the user clicks the repopulate button pretend that they double
    # click the default empty petri dish from the freezer

    file_name = os.path.join(self.m_session_mdl.m_current_freezer, 
      "@default.empty")
    if not(os.path.exists(file_name)):
      short_file_name = os.path.join("freezer", "@default.empty")
      pyDefaultFiles(short_file_name, file_name)
      #warningNoMethodName(file_name + 
      # " does not exist -- please start experiment by dragging dish")
      #return
    thawed_item = pyReadFreezer(file_name)
    self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("startNewExperimentSig"),
      ())
    self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("doDefrostDishSig"),
      ("@default", thawed_item,))
    self.m_session_mdl.m_session_mdtr.emit(
      PYSIGNAL("freezerItemDoubleClicked"), (file_name, ))

  # Next three methods are for where a user selects a veiwer from the View menu

  def viewPopulationActionSlot(self):
    self.navBarItemClickedSlot(self.m_nav_bar_ctrl.m_one_population_cli)

  def viewOrganismActionSlot(self):
    self.navBarItemClickedSlot(self.m_nav_bar_ctrl.m_one_organism_cli)

  def viewAnalysisActionSlot(self):
    self.navBarItemClickedSlot(self.m_nav_bar_ctrl.m_one_analyze_cli)


  def addStatusBarWidgetSlot(self, *args):
    widget = args[0]
    pt = QPoint()
    widget.reparent(self, pt)
    apply(QStatusBar.addWidget,(self.statusBar(),) + args)
  def removeStatusBarWidgetSlot(self, *args):
    apply(QStatusBar.removeWidget,(self.statusBar(),) + args)
  def statusBarMessageSlot(self, *args):
    apply(QStatusBar.message,(self.statusBar(),) + args)
  def statusBarClearSlot(self):
    self.statusBar().clear()

  # Routine to process item dropped in the trash can

  def DroppedInTrashSlot(self, e):

    # Try to decode to the data you understand...

    freezer_item_list = QString()
    if ( QTextDrag.decode( e, freezer_item_list)) :
      freezer_item_list = str(e.encodedData("text/plain"))

      # for each item check if the flag for prompting is true then check to
      # see if the user answer yes/no to deleting item and if they turn off
      # the prompt

      for freezer_item_name in freezer_item_list.split("\t")[1:]:
        delete_item = False
        if self.m_session_mdl.m_warn_about_trash:
           tmp_prompt = pyWarnAboutTrashCtrl(freezer_item_name)
           prompt_result = tmp_prompt.showDialog()
           if prompt_result == tmp_prompt.DeleteAllCode:
             self.m_session_mdl.m_warn_about_trash = False 
           if (prompt_result == tmp_prompt.DeleteAllCode) or (prompt_result == tmp_prompt.DeleteCode):
             delete_item = True
        else:
          delete_item = True
        if delete_item:
          self.m_session_mdl.m_session_mdtr.emit(
            PYSIGNAL("DeleteFromFreezerSig"), (freezer_item_name, ))

    elif (pyNewIconView.canDecode(e)):
      ancestor_item_name = str(e.encodedData("application/x-qiconlist"))
      if self.m_session_mdl.m_warn_about_trash:
         tmp_prompt = pyWarnAboutTrashCtrl(ancestor_item_name)
         prompt_result = tmp_prompt.showDialog()
         if prompt_result == tmp_prompt.DeleteAllCode:
           self.m_session_mdl.m_warn_about_trash = False
         if (prompt_result == tmp_prompt.DeleteAllCode) or (prompt_result == tmp_prompt.DeleteCode):
           delete_item = True
      else:
        delete_item = True
      if delete_item:
        self.m_session_mdl.m_session_mdtr.emit(
          PYSIGNAL("DeleteFromAncestorViewSig"),
          (ancestor_item_name,) )
 
