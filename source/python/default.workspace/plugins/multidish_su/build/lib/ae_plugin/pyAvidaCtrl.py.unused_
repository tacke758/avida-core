"""
pyAvidaCtrl.py

A Python Avida control that does not depend on user interface elements.

Based on pyPetriConfigureCtrl.py
  and pyPetriDishCtrl.py

"""

from descr import *

from pyMDAvida import pyMDAvida
from pyWriteAvidaCfgEvent import pyWriteAvidaCfgEvent
from pyWriteToFreezer import pyWriteToFreezer
from pyReadFreezer import pyReadFreezer

from AvidaCore import cGenesis, cString

from qt import *
from pyMdtr import *
import tempfile

import os, os.path, shutil, traceback
import math
import time

class AvidaCtrl(QWidget):
    def __init__(self):
        pass

    def setAvidaSlot(self, avida):
        descr("setAvidaSlot()")
        old_avida = self.m_avida
        self.m_avida = avida
        if old_avida:
            del old_avida
        descr("setAvidaSlot() done")

    def construct(self, session_mdl):
        self.m_session_mdl = session_mdl
        self.m_avida = None
        self.full_petri_dict = {}
        self.DishDisabled = False
        #self.connect(self.m_session_mdl.m_session_mdtr, 
        #             PYSIGNAL("MDfreezeDishPhaseIISig"), self.FreezePetriSlot)
        #self.connect(self.m_session_mdl.m_session_mdtr,
        #             PYSIGNAL("FillDishSig"), self.FillDishSlot)
        #self.connect(self.m_session_mdl.m_session_mdtr, 
        #             PYSIGNAL("doStartAvidaSig"), self.DisablePetriConfigureSlot)
        #self.connect(self.m_session_mdl.m_session_mdtr, 
        #             PYSIGNAL("doEnablePetriDishSig"), self.EnablePetriConfigureSlot)
        #self.connect(self.m_session_mdl.m_session_mdtr, 
        #             PYSIGNAL("doInitializeAvidaPhaseSyncSig"), self.CreateFilesFromPetriSlot)
        #self.connect(self.m_session_mdl.m_session_mdtr, 
        #             PYSIGNAL("doInitializeAvidaPhaseISig"), self.CreateFilesFromPetriSlot)
        self.connect(self.m_session_mdl.m_md_session_mdtr, PYSIGNAL("MDsetAvidaSig"), 
                     self.setAvidaSlot)
        #self.connect(self.m_session_mdl.m_session_mdtr, 
        #             PYSIGNAL("doInitializeAvidaPhaseIISig"),
        #             self.doLoadPetriDishConfigFileSlot)
        #self.connect( self, PYSIGNAL("petriDishDroppedInPopViewSig"), 
        #              self.m_session_mdl.m_session_mdtr, 
        #              PYSIGNAL("petriDishDroppedInPopViewSig"))
        #self.connect( self.m_session_mdl.m_session_mdtr, 
        #              PYSIGNAL("petriDishDroppedInPetriConfigSig"), self.petriDroppedSlot)

        # If the user drops something in the Ancestor Box

        #self.connect(self.AncestorIconView, PYSIGNAL("DroppedOnNewIconViewSig"),
        #             self.petriAncestorDroppedSlot)
        #self.connect( self.m_session_mdl.m_session_mdtr, 
        #              PYSIGNAL("petriDishDroppedAncestorSig"), self.petriAncestorDroppedSlot)
        
        # If an ancestor was dropped into the trash can

        #self.connect(self.m_session_mdl.m_session_mdtr,
        #             PYSIGNAL("DeleteFromAncestorViewSig"), self.deleteAncestorSlot)
        
        #self.ChangeMutationTextSlot()
        #self.ChangeWorldSizeTextSlot()
        #self.m_session_mdl.m_session_mdtr.emit(
        #    PYSIGNAL("setAvidaSig"),
        #    (self.m_avida,))

        self.populated = False
        
    def destruct(self):
        self.m_avida = None
        self.full_petri_dict = {}
        # @@kgn; was:
        # self.DishDisabled = False
        self.DishDisabled = True
        #self.disconnect(self.m_session_mdl.m_session_mdtr, 
        #                PYSIGNAL("freezeDishPhaseIISig"), self.FreezePetriSlot)
        #self.disconnect(self.m_session_mdl.m_session_mdtr,
        #                PYSIGNAL("FillDishSig"), self.FillDishSlot)
        #self.disconnect(self.m_session_mdl.m_session_mdtr, 
        #                PYSIGNAL("doStartAvidaSig"), self.DisablePetriConfigureSlot)
        #self.disconnect(self.m_session_mdl.m_session_mdtr, 
        #                PYSIGNAL("doEnablePetriDishSig"), self.EnablePetriConfigureSlot)
        #self.disconnect(self.m_session_mdl.m_session_mdtr, 
        #                PYSIGNAL("doInitializeAvidaPhaseISig"), self.CreateFilesFromPetriSlot)
        self.disconnect(self.m_session_mdl.m_md_session_mdtr, PYSIGNAL("MDsetAvidaSig"), 
                        self.setAvidaSlot)
        self.disconnect(self.m_session_mdl.m_md_session_mdtr, 
                        PYSIGNAL("MDdoInitializeAvidaPhaseIISig"),
                        self.doLoadPetriDishConfigFileSlot)
        #self.disconnect( self, PYSIGNAL("petriDishDroppedInPopViewSig"), 
        #                 self.m_session_mdl.m_session_mdtr, 
        #                 PYSIGNAL("petriDishDroppedInPopViewSig"))
        self.populated = False
        self.m_session_mdl = None
    
    def doStartAvidaSlot (self):
        print "doStartAvidaSlot()"
        self.fileOpen()
        
        if (self.m_session_mdl.directory_chosen):
            print "doStartAvidaSlot(): directory_chosen == True"
            orig_avida_cfg_filename = os.path.join(self.m_session_mdl.m_current_workspace, "genesis.default")
            self.setupAvida(orig_avida_cfg_filename)
            # self.m_session_mdl.m_avida_has_started = True
            self.m_avida.m_avida_thread_mdtr.emit(PYSIGNAL("MDdoStartAvidaSig"), ())
        else:
            print "doStartAvidaSlot(): directory_chosen == False"
            

    def restart(self):
        self.m_cell_info = None
        self.m_changed_cell_items = []
        self.m_change_list = None
        self.m_occupied_cells_ids = []

    def extractPopulationSlot(self, send_reset_signal = False, send_quit_signal = False):

        # If there is an active Avida object find all the cells that are occupied
        # and place them in a dictionary.  Fire off the signal for the next freezer
        # phase with that signal.

        population_dict = {}
        ancestor_dict = {}
        if self.m_avida != None:
            for x in range(self.m_world_w):
                for y in range(self.m_world_h):
                    cell = self.m_avida.m_population.GetCell(x + self.m_world_w*y)
                    if cell.IsOccupied() == True:
                        organism = cell.GetOrganism()
                        genome = organism.GetGenome()
                        population_dict[cell.GetID()] = str(genome.AsString())
                        ancestor_dict[cell.GetID()] = str(organism.GetLineageLabel())
        self.m_session_mdl.m_md_session_mdtr.emit(PYSIGNAL("MDfreezeDishPhaseIISig"), 
                                               (population_dict, ancestor_dict, send_reset_signal, send_quit_signal, ))


    def setupAvida(self, avida_cfgFileName = None):
        """
        setupAvida() 
        Takes a valid Avida configuration file as its argument.
        Only succeeds if the configuration files are accessible.
        """
        print "pyAvidaCtrl.setupAvida()"
        # Internal preparation

        print "pyAvidaCtrl.setupAvida(): mkDefaultDictionary()"
        self.full_petri_dict["SETTINGS"] = self.mkDefaultDictionary()
        print "pyAvidaCtrl.setupAvida(): pyWriteAvidaCfgEvent(" + self.m_session_mdl.m_current_workspace + "," + self.m_session_mdl.m_current_freezer + "," + self.m_session_mdl.m_md_tempdir + "," + self.m_session_mdl.m_md_tempdir_out + ")"
        write_object = pyWriteAvidaCfgEvent(self.full_petri_dict,
                                            self.m_session_mdl, 
                                            self.m_session_mdl.m_current_workspace,
                                            self.m_session_mdl.m_current_freezer, 
                                            self.m_session_mdl.m_md_tempdir,
                                            self.m_session_mdl.m_md_tempdir_out)


        # Create a Genesis object
        #print "pyAvidaCtrl.setupAvida(): creating Genesis object"
        #avida_cfg = cGenesis()
        # Use the Genesis object to open the config file
        print "pAvidaCtrl.setupAvida(): opening Avida config file " + avida_cfgFileName
        # Pass in newly created Avida configuration filename
        
        # avida_cfg.Open(cString(avida_cfgFileName))
        # write_object.avida_cfg.Open(cString(self.m_session_mdl.m_md_tempdir))
        # Bail out if the config file didn't actually get opened
        if 0 == write_object.avida_cfg.IsOpen():
            warningNoMethodName("Unable to find file " + write_object.avida_cfg_file_name)
            return
        # Output a little diagnostic info
        descr("self.setAvidaSlot(None) ...")
        self.setAvidaSlot(None)
        # Output a little diagnostic info
        descr("self.setAvidaSlot(None) done.")
        # Create the Avida object instance
        # This refers to methods in the C++ library.
        avida = pyMDAvida()
        # Perform setup on the new Avida object
        avida.construct(write_object.avida_cfg)
        # Output a little diagnostic info
        descr("self.setAvidaSlot(avida) ...")
        # Very important -- this next line associates the Avida object with the
        # Avida slot, and we only henceforth know of it because of the persistence
        # of the Avida slot.
        self.setAvidaSlot(avida)
        # Output a little diagnostic info
        descr("self.setAvidaSlot(avida) done.")

        # Stops self from hearing own setAvidaSig signal

        # This is made necessary by the use of the threading driver. First, disconnect
        # the signal from our setAvidaSlot. Then, emit the signal for other parts of 
        # the Qt application to handle what they need to on instantiation of a new
        # Avida instance. Finally, reconnect our setAvidaSlot to the signal so that
        # we can pay attention in the usual manner.
        self.disconnect(
            self.m_session_mdl.m_md_session_mdtr, PYSIGNAL("MDsetAvidaSig"),
            self.setAvidaSlot)
        self.m_session_mdl.m_md_session_mdtr.emit(
            PYSIGNAL("MDsetAvidaSig"),
            (self.m_avida,))
        self.connect(
            self.m_session_mdl.m_md_session_mdtr, PYSIGNAL("MDsetAvidaSig"),
            self.setAvidaSlot)

        self.connect(
            self.m_avida.m_avida_thread_mdtr, PYSIGNAL("MDAvidaUpdatedSig"),
            self.avidaUpdatedSlot)

        descr("setupAvida(): emit MDdoStartAvidaSig")
        self.m_session_mdl.m_md_session_mdtr.emit(PYSIGNAL("MDdoStartAvidaSig"), ())

        # @@kgn
        self.DishDisabled = False
        descr("setupAvida() done.")
        
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
            #self.setCaption('%s - %s' % ("Avida-ED", os.path.basename(os.path.splitext(self.m_session_mdl.m_current_workspace)[0])) )
            #self.m_session_mdl.m_session_mdtr.emit(
            #    PYSIGNAL("doRefreshFreezerInventorySig"), ())

    def mkDefaultDictionary(self):

        settings_dict = {}
    
        # Default ancestor
        i = 0
        settings_dict["START_CREATURE" + str(i)] = "@ancestor"
        settings_dict["START_GENOME" + str(i)] = "rucavcqgfcqapqcccccccccccccccccccccccccccutycasvab"

        # Stop after not too many updates
        settings_dict["MAX_UPDATES"] = 200

        # X and Y as in MultiDish settings
        settings_dict["WORLD-X"] = 300
        settings_dict["WORLD-Y"] = 300

        settings_dict["RANDOM_SEED"] = 0

        settings_dict["COPY_MUT_PROB"] = 0.01

        settings_dict["BIRTH_METHOD"] = 0

        settings_dict["REWARD_NOT"] = "YES"
        
        settings_dict["REWARD_NAND"] = "YES"

        settings_dict["REWARD_AND"] = "YES"

        settings_dict["REWARD_ORN"] = "YES"

        settings_dict["REWARD_OR"] = "YES"

        settings_dict["REWARD_ANDN"] = "YES"

        settings_dict["REWARD_NOR"] = "YES"

        settings_dict["REWARD_XOR"] = "YES"

        settings_dict["REWARD_EQU"] = "YES"

        return settings_dict


    def avidaUpdatedSlot(self):
        descr("avidaUpdatedSlot()")
        self.stats = self.m_avida.m_population.GetStats()
        #print "Sleeping 1..."
        #time.sleep(5)
        #print "awake 1..."

        # Print out some stats
        age_now ="%d" %(self.stats.GetUpdate())
        print "At update %s" % (age_now)
        
        #print "Sleeping 2..."
        #time.sleep(5)

        #print "awake 2..."

        #print "mdtr emit\n"
        #sys.stdout.flush()
        #self.m_avida.m_avida_thread_mdtr.emit(PYSIGNAL("MDAvidaRunUpdateSig"), ())
        #print "mdtr emitted, now trying avida_ctrl emit\n"
        #sys.stdout.flush()
        #self.emit(PYSIGNAL("MDAvidaRunUpdateSig"), ())
        #print "avida_ctrl emitted\n"
        #sys.stdout.flush()
        descr("avidaUpdatedSlot(): done.")


    def doOnlyStartAvidaSlot(self):
        self.m_avida.m_avida_thread_mdtr.emit(PYSIGNAL("MDdoStartAvidaSig"), ())
