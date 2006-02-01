# -*- coding: utf-8 -*-

from descr import descr

from pyAvida import pyAvida
from pyFreezeDialogCtrl import pyFreezeDialogCtrl
from pyPetriConfigureView import pyPetriConfigureView
from pyWriteGenesisEvent import pyWriteGenesisEvent
from pyWriteToFreezer import pyWriteToFreezer

from AvidaCore import cGenesis, cString

from qt import *

import os, os.path, shutil
import math

class pyPetriConfigureCtrl(pyPetriConfigureView):

  def __init__(self,parent = None,name = None,fl = 0):
    pyPetriConfigureView.__init__(self,parent,name,fl)
    self.setAcceptDrops(1)

  def setAvidaSlot(self, avida):
    old_avida = self.m_avida
    self.m_avida = avida
    if old_avida:
      del old_avida
    if self.m_avida:
      pass
    
  def construct(self, session_mdl):
    descr()
    self.m_session_mdl = session_mdl
    self.m_session_petri_view = pyPetriConfigureView()
    self.m_avida = None
    self.full_petri_dict = {}
    self.DishDisabled = False
    self.connect(self.MutationSlider, SIGNAL("valueChanged(int)"), 
      self.ChangeMutationTextSlot)
    self.connect(self.WorldSizeSlider, SIGNAL("valueChanged(int)"), 
      self.ChangeWorldSizeTextSlot)
    self.connect(self.RewardNonePushButton, SIGNAL("clicked()"), 
      self.RewardNoneSlot)
    self.connect(self.RewardAllPushButton, SIGNAL("clicked()"), 
      self.RewardAllSlot)
    self.connect(self.RadomGeneratedRadioButton, SIGNAL("clicked()"), 
      self.ChangeRandomSpinBoxSlot)
    self.connect(self.RandomFixedRadioButton, SIGNAL("clicked()"), 
      self.ChangeRandomSpinBoxSlot)
    self.connect(self.StopManuallyRadioButton, SIGNAL("clicked()"), 
      self.ChangeStopSpinBoxSlot)
    self.connect(self.StopAtRadioButton, SIGNAL("clicked()"), 
      self.ChangeStopSpinBoxSlot)
    self.connect(self.SavePetriPushButton, SIGNAL("clicked()"), 
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("freezeDishPhaseISig"))
    self.connect(self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("freezeDishPhaseIISig"), self.FreezePetriSlot)
    self.connect(self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("FillDishSig"), self.FillDishSlot)
    self.connect(self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("doInitializeAvidaPhaseISig"), self.DisablePetriConfigureSlot)
    self.connect(self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("doEnablePetriDishSig"), self.EnablePetriConfigureSlot)
    self.connect(self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("doInitializeAvidaPhaseSyncSig"), self.CreateFilesFromPetriSlot)
    self.connect(self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("doInitializeAvidaPhaseISig"), self.CreateFilesFromPetriSlot)
    self.connect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("setAvidaSig"), 
      self.setAvidaSlot)
    self.connect(self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("doInitializeAvidaPhaseIISig"),
      self.doLoadPetriDishConfigFileSlot)
    self.connect( self, PYSIGNAL("petriDishDroppedInPopViewSig"), 
      self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("petriDishDroppedInPopViewSig"))
    self.connect( self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("petriDishDroppedInPopViewSig"), self.petriDropped)
    self.ChangeMutationTextSlot()
    self.ChangeWorldSizeTextSlot()
    self.populated = False
    
  def destruct(self):
    descr()
    self.m_session_petri_view = None
    self.m_avida = None
    self.full_petri_dict = {}
    self.DishDisabled = False
    self.disconnect(self.MutationSlider, SIGNAL("valueChanged(int)"), 
      self.ChangeMutationTextSlot)
    self.disconnect(self.WorldSizeSlider, SIGNAL("valueChanged(int)"), 
      self.ChangeWorldSizeTextSlot)
    self.disconnect(self.RewardAllPushButton, SIGNAL("clicked()"), 
      self.RewardAllSlot)
    self.disconnect(self.RewordNonePushButton, SIGNAL("clicked()"), 
      self.RewardNoneSlot)
    self.disconnect(self.RadomGeneratedRadioButton, SIGNAL("clicked()"), 
      self.ChangeRandomSpinBoxSlot)
    self.disconnect(self.RandomFixedRadioButton, SIGNAL("clicked()"), 
      self.ChangeRandomSpinBoxSlot)
    self.disconnect(self.StopManuallyRadioButton, SIGNAL("clicked()"), 
      self.ChangeStopSpinBoxSlot)
    self.disconnect(self.StopAtRadioButton, SIGNAL("clicked()"), 
      self.ChangeStopSpinBoxSlot)
    self.disconnect(self.SavePetriPushButton, SIGNAL("clicked()"), 
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("freezeDishPhaseISig"))
    self.disconnect(self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("freezeDishPhaseIISig"), self.FreezePetriSlot)
    self.disconnect(self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("FillDishSig"), self.FillDishSlot)
    self.disconnect(self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("doInitializeAvidaPhaseISig"), self.DisablePetriConfigureSlot)
    self.disconnect(self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("doEnablePetriDishSig"), self.EnablePetriConfigureSlot)
    self.disconnect(self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("doInitializeAvidaPhaseISig"), self.CreateFilesFromPetriSlot)
    self.disconnect(self.m_session_mdl.m_session_mdtr, PYSIGNAL("setAvidaSig"), 
      self.setAvidaSlot)
    self.disconnect(self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("doInitializeAvidaPhaseIISig"),
      self.doLoadPetriDishConfigFileSlot)
    self.disconnect( self, PYSIGNAL("petriDishDroppedInPopViewSig"), 
      self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("petriDishDroppedInPopViewSig"))
    self.populated = False
    self.m_session_mdl = None

  
  def ChangeMutationTextSlot(self):
    slide_value = float(self.MutationSlider.value())/100.0
    slide_value = pow(10,slide_value)
    if slide_value < 0.0011:
      slide_value = 0.0
    if slide_value > 1 or slide_value < 0.00001:
      slide_value_txt = ("%1.1f" % (slide_value)) + "%"
    elif slide_value > 0.1:
      slide_value_txt = ("%1.2f" % (slide_value)) + "%"
    elif slide_value > 0.01:
      slide_value_txt = ("%1.3f" % (slide_value)) + "%"
    else:
      slide_value_txt = ("%1.4f" % (slide_value)) + "%"
    self.MutationPercentTextLabel.setText(slide_value_txt)

  def ChangeWorldSizeTextSlot(self):
    slide_value = str(self.WorldSizeSlider.value())
    slide_value_txt = slide_value + " x " + slide_value + " cells"
    self.WorldSizeTextLabel.setText(slide_value_txt)

  def RewardAllSlot(self):
    self.NotCheckBox.setChecked(True)
    self.NandCheckBox.setChecked(True)
    self.AndCheckBox.setChecked(True)
    self.OrnCheckBox.setChecked(True)
    self.OrCheckBox.setChecked(True)
    self.AndnCheckBox.setChecked(True)
    self.NorCheckBox.setChecked(True)
    self.XorCheckBox.setChecked(True)
    self.EquCheckBox.setChecked(True)

  def RewardNoneSlot(self):
    self.NotCheckBox.setChecked(False)
    self.NandCheckBox.setChecked(False)
    self.AndCheckBox.setChecked(False)
    self.OrnCheckBox.setChecked(False)
    self.OrCheckBox.setChecked(False)
    self.AndnCheckBox.setChecked(False)
    self.NorCheckBox.setChecked(False)
    self.XorCheckBox.setChecked(False)
    self.EquCheckBox.setChecked(False)

  def ChangeRandomSpinBoxSlot(self):
    if self.RadomGeneratedRadioButton.isChecked() == True:
      self.RandomSpinBox.setEnabled(False)
    else:
      self.RandomSpinBox.setEnabled(True)
  
  def ChangeStopSpinBoxSlot(self):
    if self.StopManuallyRadioButton.isChecked() == True:
      self.StopAtSpinBox.setEnabled(False)
    else:
      self.StopAtSpinBox.setEnabled(True)
  
  def FillDishSlot(self, dish_name, petri_dict):
    descr()
    
    self.full_petri_dict = petri_dict.dictionary
    settings_dict =  petri_dict.dictionary["SETTINGS"]

    # Erase all items for the ancestor list (largest to smallest index)

    for i in range((self.AncestorComboBox.count() - 1), -1, -1):
      self.AncestorComboBox.removeItem (i)

    # Find all ancestors with the name of the form START_CREATUREx

    i = 0
    while(settings_dict.has_key("START_CREATURE" + str(i))):
      start_creature = settings_dict["START_CREATURE" + str(i)]
      self.AncestorComboBox.insertItem(start_creature)
      i = i + 1
    if settings_dict.has_key("MAX_UPDATES") == True:
      max_updates = int(settings_dict["MAX_UPDATES"])
    else:
      max_updates = -1
    self.StopAtSpinBox.setValue(max_updates)
    if max_updates < 0:
       self.StopManuallyRadioButton.setChecked(True)
       self.StopAtRadioButton.setChecked(False)
    else:
       self.StopManuallyRadioButton.setChecked(False)
       self.StopAtRadioButton.setChecked(True)
    if settings_dict.has_key("WORLD-X") == True:
      self.WorldSizeSlider.setValue(int(settings_dict["WORLD-X"]))
    else:
      self.WorldSizeSlider.setValue(30)
    if settings_dict.has_key("RANDOM_SEED") == True:
      seed = int(settings_dict["RANDOM_SEED"])
    else:
      seed = 0
    self.RandomSpinBox.setValue(seed)
    if seed == 0:
       self.RadomGeneratedRadioButton.setChecked(True)
       self.RandomFixedRadioButton.setChecked(False)
    else:
       self.RadomGeneratedRadioButton.setChecked(False)
       self.RandomFixedRadioButton.setChecked(True)
    if settings_dict.has_key("COPY_MUT_PROB") == True:
      copy_mutation_percent = float(settings_dict["COPY_MUT_PROB"]) * 100;
    else:
      copy_mutation_percent = 0.01
    if copy_mutation_percent > 0.00000001:
      self.MutationSlider.setValue(int(math.log10(copy_mutation_percent) * 100))
    else:
      self.MutationSlider.setValue(-300)
    if (settings_dict.has_key("BIRTH_METHOD") and 
        int(settings_dict["BIRTH_METHOD"])) in [0, 1, 2, 3]:
       self.LocalBirthRadioButton.setChecked(True)
       self.MassActionRadioButton.setChecked(False)
    else:
       self.LocalBirthRadioButton.setChecked(False)
       self.MassActionRadioButton.setChecked(True)

    if (settings_dict.has_key("REWARD_NOT") and 
        settings_dict["REWARD_NOT"] == "YES"):
      self.NotCheckBox.setChecked(True)
    else:
      self.NotCheckBox.setChecked(False)
    if (settings_dict.has_key("REWARD_NAND") and 
        settings_dict["REWARD_NAND"] == "YES"):
      self.NandCheckBox.setChecked(True)
    else:
      self.NandCheckBox.setChecked(False)
    if (settings_dict.has_key("REWARD_AND") and 
        settings_dict["REWARD_AND"] == "YES"):
      self.AndCheckBox.setChecked(True)
    else:
      self.AndCheckBox.setChecked(False)
    if (settings_dict.has_key("REWARD_ORN") and 
        settings_dict["REWARD_ORN"] == "YES"):
      self.OrnCheckBox.setChecked(True)
    else:
      self.OrnCheckBox.setChecked(False)
    if (settings_dict.has_key("REWARD_OR") and 
        settings_dict["REWARD_OR"] == "YES"):
      self.OrCheckBox.setChecked(True)
    else:
      self.OrCheckBox.setChecked(False)
    if (settings_dict.has_key("REWARD_ANDN") and 
        settings_dict["REWARD_ANDN"] == "YES"):
      self.AndnCheckBox.setChecked(True)
    else:
      self.AndnCheckBox.setChecked(False)
    if (settings_dict.has_key("REWARD_NOR") and 
        settings_dict["REWARD_NOR"] == "YES"):
      self.NorCheckBox.setChecked(True)
    else:
      self.NorCheckBox.setChecked(False)
    if (settings_dict.has_key("REWARD_XOR") and 
        settings_dict["REWARD_XOR"] == "YES"):
      self.XorCheckBox.setChecked(True)
    else:
      self.XorCheckBox.setChecked(False)
    if (settings_dict.has_key("REWARD_EQU") and 
        settings_dict["REWARD_EQU"] == "YES"):
      self.EquCheckBox.setChecked(True)
    else:
      self.EquCheckBox.setChecked(False)

    self.m_session_mdl.new_empty_dish = True
    self.m_session_mdl.m_session_mdtr.emit(
      PYSIGNAL("finishedPetriDishSig"), ())

  def DisablePetriConfigureSlot(self):
    descr()
    self.ConfigTitleTextLabel.setText("Environmental Settings Disabled During Run")

    # Turn off the controls 

    self.AncestorComboBox.setEnabled(False)
    self.StopAtSpinBox.setEnabled(False)
    self.StopManuallyRadioButton.setEnabled(False)
    self.StopAtRadioButton.setEnabled(False)
    self.WorldSizeSlider.setEnabled(False)
    self.RandomSpinBox.setEnabled(False)
    self.RadomGeneratedRadioButton.setEnabled(False)
    self.RandomFixedRadioButton.setEnabled(False)
    self.MutationSlider.setEnabled(False)
    self.LocalBirthRadioButton.setEnabled(False)
    self.MassActionRadioButton.setEnabled(False)
    self.RewardAllPushButton.setEnabled(False)
    self.RewardNonePushButton.setEnabled(False)
    self.NotCheckBox.setEnabled(False)
    self.NandCheckBox.setEnabled(False)
    self.AndCheckBox.setEnabled(False)
    self.OrnCheckBox.setEnabled(False)
    self.OrCheckBox.setEnabled(False)
    self.AndnCheckBox.setEnabled(False)
    self.NorCheckBox.setEnabled(False)
    self.XorCheckBox.setEnabled(False)
    self.EquCheckBox.setEnabled(False)
    self.MutationPercentTextLabel.setEnabled(False)
    self.WorldSizeTextLabel.setEnabled(False)
    self.MutationRateHeadTextLabel.setEnabled(False)
    self.WorldSizeHeadTextLable.setEnabled(False)
    self.RewardTextLabel.setEnabled(False)
    self.RandomHeadTextLabel.setEnabled(False)
    self.AncestorHeadTextLabel.setEnabled(False)
    self.BirthHeadTextLabel.setEnabled(False)
    self.StopHeadTextLabel.setEnabled(False)
    self.DishDisabled = True
    self.m_session_mdl.m_session_mdtr.emit(
      PYSIGNAL("doDisablePetriDishSig"), ())

  def EnablePetriConfigureSlot(self):
    descr()
    self.ConfigTitleTextLabel.setText("Environmental Settings")

    # Turn on the controls 
    
    self.AncestorComboBox.setEnabled(True)
    self.StopAtSpinBox.setEnabled(True)
    self.StopManuallyRadioButton.setEnabled(True)
    self.StopAtRadioButton.setEnabled(True)
    self.WorldSizeSlider.setEnabled(True)
    self.RandomSpinBox.setEnabled(True)
    self.RadomGeneratedRadioButton.setEnabled(True)
    self.RandomFixedRadioButton.setEnabled(True)
    self.MutationSlider.setEnabled(True)
    self.LocalBirthRadioButton.setEnabled(True)
    self.MassActionRadioButton.setEnabled(True)
    self.RewardAllPushButton.setEnabled(True)
    self.RewardNonePushButton.setEnabled(True)
    self.NotCheckBox.setEnabled(True)
    self.NandCheckBox.setEnabled(True)
    self.AndCheckBox.setEnabled(True)
    self.OrnCheckBox.setEnabled(True)
    self.OrCheckBox.setEnabled(True)
    self.AndnCheckBox.setEnabled(True)
    self.NorCheckBox.setEnabled(True)
    self.XorCheckBox.setEnabled(True)
    self.EquCheckBox.setEnabled(True)
    self.MutationPercentTextLabel.setEnabled(True)
    self.WorldSizeTextLabel.setEnabled(True)
    self.MutationRateHeadTextLabel.setEnabled(True)
    self.WorldSizeHeadTextLable.setEnabled(True)
    self.RewardTextLabel.setEnabled(True)
    self.RandomHeadTextLabel.setEnabled(True)
    self.AncestorHeadTextLabel.setEnabled(True)
    self.BirthHeadTextLabel.setEnabled(True)
    self.StopHeadTextLabel.setEnabled(True)
    self.DishDisabled = False


  def CreateFilesFromPetriSlot(self, out_dir = None):
    descr()

    # The input files will be placed in a python generated temporary directory
    # ouput files will be stored in tmp_dir/output until the data is frozen
    # (ie saved)

    self.full_petri_dict["SETTINGS"] = self.Form2Dictionary()
    write_object = pyWriteGenesisEvent(self.full_petri_dict, 
      self.m_session_mdl.m_current_workspace,
      self.m_session_mdl.m_current_freezer, self.m_session_mdl.m_tempdir,
      self.m_session_mdl.m_tempdir_out)
    self.m_session_mdl.m_session_mdtr.emit(
      PYSIGNAL("doInitializeAvidaPhaseIISig"), (os.path.join(self.m_session_mdl.m_tempdir, "genesis.avida"),))
      
  def Form2Dictionary(self):
    descr()
    settings_dict = {}
    
    # Write START_CREATUREx for all the organisms in the Ancestor Combo Box

    for i in range(self.AncestorComboBox.count()):
      settings_dict["START_CREATURE" + str(i)] = str(self.AncestorComboBox.text(i))
    if (self.StopAtRadioButton.isChecked() == True):
      settings_dict["MAX_UPDATES"] = self.StopAtSpinBox.value()
    else:
      settings_dict["MAX_UPDATES"] = -1
    settings_dict["WORLD-X"] = self.WorldSizeSlider.value()
    settings_dict["WORLD-Y"] = self.WorldSizeSlider.value()
    if self.RandomFixedRadioButton.isChecked() == True:
      settings_dict["RANDOM_SEED"] = self.RandomSpinBox.value()
    else:
      settings_dict["RANDOM_SEED"] = 0
    slide_value = float(self.MutationSlider.value())/100.0
    slide_value = pow(10,slide_value)
    if slide_value < 0.0011:
      slide_value = 0.0
    settings_dict["COPY_MUT_PROB"] = slide_value/100.0
    if self.LocalBirthRadioButton.isChecked() == True:
      settings_dict["BIRTH_METHOD"] = 0
    else:
      settings_dict["BIRTH_METHOD"] = 4

    if self.NotCheckBox.isChecked() == True:
      settings_dict["REWARD_NOT"] = "YES"
    else:
      settings_dict["REWARD_NOT"] = "NO"
    if self.NandCheckBox.isChecked() == True:
      settings_dict["REWARD_NAND"] = "YES"
    else:
      settings_dict["REWARD_NAND"] = "NO"
    if self.AndCheckBox.isChecked() == True:
      settings_dict["REWARD_AND"] = "YES"
    else:
      settings_dict["REWARD_AND"] = "NO"
    if self.OrnCheckBox.isChecked() == True:
      settings_dict["REWARD_ORN"] = "YES"
    else:
      settings_dict["REWARD_ORN"] = "NO"
    if self.OrCheckBox.isChecked() == True:
      settings_dict["REWARD_OR"] = "YES"
    else:
      settings_dict["REWARD_OR"] = "NO"
    if self.AndnCheckBox.isChecked() == True:
      settings_dict["REWARD_ANDN"] = "YES"
    else:
      settings_dict["REWARD_ANDN"] = "NO"
    if self.NorCheckBox.isChecked() == True:
      settings_dict["REWARD_NOR"] = "YES"
    else:
      settings_dict["REWARD_NOR"] = "NO"
    if self.XorCheckBox.isChecked() == True:
      settings_dict["REWARD_XOR"] = "YES"
    else:
      settings_dict["REWARD_XOR"] = "NO"
    if self.EquCheckBox.isChecked() == True:
      settings_dict["REWARD_EQU"] = "YES"
    else:
      settings_dict["REWARD_EQU"] = "NO"

    return settings_dict
    
  def FreezePetriSlot(self, population_dict = None, send_reset_signal = False, send_quit_signal = False):
    descr()
    if len(population_dict) == 0:
      freeze_empty_only_flag = True;
    else:
      freeze_empty_only_flag = False;
    tmp_dict = {}
    tmp_dict["SETTINGS"] = self.Form2Dictionary()
    m_pop_up_freezer_file_name = pyFreezeDialogCtrl()
    file_name = m_pop_up_freezer_file_name.showDialog(self.m_session_mdl.m_current_freezer,
         freeze_empty_only_flag)
    file_name_len = len(file_name.rstrip())

    # If the user is saving a full population expand the name and insert
    # the population dictionary into the temporary dictionary

    if (file_name_len > 0):
      is_empty_dish = m_pop_up_freezer_file_name.isEmpty()
      if (not is_empty_dish):
        os.mkdir(file_name)

        # Copy the average and count files from the teporary output directory
        # to the Freezer directory
        
        tmp_ave_file = os.path.join(self.m_session_mdl.m_tempdir_out, "average.dat")
        if (os.path.exists(tmp_ave_file)):
          shutil.copyfile(tmp_ave_file, os.path.join(file_name, "average.dat"))
        tmp_count_file = os.path.join(self.m_session_mdl.m_tempdir_out, "count.dat")
        if (os.path.exists(tmp_count_file)):
          shutil.copyfile(tmp_count_file, os.path.join(file_name, "count.dat"))
        file_name = os.path.join(file_name, "petri_dish")
        tmp_dict["POPULATION"] = population_dict
      freezer_file = pyWriteToFreezer(tmp_dict, file_name)
      if (is_empty_dish):
        self.m_session_mdl.saved_empty_dish = True
      else:
        self.m_session_mdl.saved_full_dish = True
    
    self.m_session_mdl.m_session_mdtr.emit(
      PYSIGNAL("doRefreshFreezerInventorySig"), ())
    if send_reset_signal:
      self.m_session_mdl.m_session_mdtr.emit(
        PYSIGNAL("restartPopulationSig"), (self.m_session_mdl, ))

    # If the send_quit_signal flag was sent to this routine kill the application
    # (Instead of killing the application directly a signal should be sent
    # upto the workspace moderator)

    if send_quit_signal:
      qApp.quit()


  def doLoadPetriDishConfigFileSlot(self, genesisFileName = None):
    descr()
    genesis = cGenesis()
    genesis.Open(cString(genesisFileName))
    if 0 == genesis.IsOpen():
      print "Warning: Unable to find file '", genesisFileName
      return
    avida = pyAvida()
    avida.construct(genesis)
    self.setAvidaSlot(avida)

    # Stops self from hearing own setAvidaSig signal

    self.disconnect(
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("setAvidaSig"),
      self.setAvidaSlot)
    self.m_session_mdl.m_session_mdtr.emit(
      PYSIGNAL("setAvidaSig"),
      (self.m_avida,))
    self.connect(
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("setAvidaSig"),
      self.setAvidaSlot)

  def dragEnterEvent( self, e ):
    descr(e)

    freezer_item_name = QString()
    if ( QTextDrag.decode( e, freezer_item_name ) ) : #freezer_item_name is a string...the file name 
      if os.path.exists(str(freezer_item_name)) == False:
        descr("that was not a valid path (1)")
      else: 
        e.acceptAction(True)
        descr("accepted.")

      
  def dropEvent( self, e ):
    descr()
    freezer_item_name = QString()
    if ( QTextDrag.decode( e, freezer_item_name ) ) :
      if os.path.exists(str(freezer_item_name)) == False:
        print "that was not a valid path (2)" 
      else: 
        self.emit(PYSIGNAL("petriDishDroppedInPopViewSig"), (e,))

  def petriDropped(self, e):
    descr()
    # Try to decode to the data you understand...
    freezer_item_name = QString()
    if ( QTextDrag.decode( e, freezer_item_name ) and not self.DishDisabled) :
      if freezer_item_name[-8:] == 'organism':
        core_name = freezer_item_name[:-9]
        core_name = os.path.basename(str(freezer_item_name[:-9]))
        self.AncestorComboBox.insertItem(core_name)
        return
