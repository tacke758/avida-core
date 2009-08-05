# -*- coding: utf-8 -*-

from descr import *

from pyAvida import pyAvida
from pyFreezeDialogCtrl import pyFreezeDialogCtrl
from pyPetriConfigureView import pyPetriConfigureView
from pyWriteAvidaCfgEvent import pyWriteAvidaCfgEvent
from pyWriteToFreezer import pyWriteToFreezer
from pyReadFreezer import pyReadFreezer
from pyNewIconView import pyNewIconViewItem

from AvidaCore import cGenesis, cString

from qt import *

import os, os.path, shutil, traceback
import math

def nullFunction():
  pass

class pyPetriConfigureCtrl(pyPetriConfigureView):

  def __init__(self,parent = None,name = None,fl = 0):
    pyPetriConfigureView.__init__(self,parent,name,fl)
    self.setAcceptDrops(1)
    self.AncestorIconView.setVScrollBarMode(QIconView.AlwaysOff)

  def setAvidaSlot(self, avida):
    old_avida = self.m_avida
    self.m_avida = avida
    if old_avida:
      del old_avida
    
  def construct(self, session_mdl):
    self.m_session_mdl = session_mdl
    self.m_session_petri_view = pyPetriConfigureView()
    self.m_avida = None
    self.full_petri_dict = {}
    self.DishDisabled = False
    self.connect(self.MutationSlider, SIGNAL("valueChanged(int)"), 
      self.ChangeMutationTextSlot)
    self.connect(self.MutationRateLineEdit, SIGNAL("returnPressed()"), 
      self.ChangeMutationSliderSlot)
    self.connect(self.WorldSizeSlider, SIGNAL("valueChanged(int)"), 
      self.ChangeWorldSizeTextSlot)
    self.connect(self.WorldSizeLineEdit, SIGNAL("returnPressed()"), 
      self.ChangeWorldSizeSliderSlot)
    self.connect(self.StopManuallyRadioButton, SIGNAL("clicked()"), 
      self.ChangeStopSpinBoxEnabledSlot)
    self.connect(self.StopAtRadioButton, SIGNAL("clicked()"), 
      self.ChangeStopSpinBoxEnabledSlot)
    self.connect(self.StopAtSpinBox, SIGNAL("valueChanged(int)"), 
      self.ChangeStopSpinBoxSlot)

    # Before freezing be sure the population is paused

    self.connect(self.SavePetriPushButton, SIGNAL("clicked()"), 
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("doPauseAvidaSig"))
    self.connect(self.SavePetriPushButton, SIGNAL("clicked()"), 
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("freezeDishPhaseISig"))
    self.connect(self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("freezeDishPhaseIISig"), self.FreezePetriSlot)
    self.connect(self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("FillDishSig"), self.FillDishSlot)
    self.connect(self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("doStartAvidaSig"), self.DisablePetriConfigureSlot)
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
      PYSIGNAL("petriDishDroppedInPetriConfigSig"), self.petriDroppedSlot)

    # If the user drops something in the Ancestor Box

    self.connect(self.AncestorIconView, PYSIGNAL("DroppedOnNewIconViewSig"),
      self.petriAncestorDroppedSlot)
    self.connect( self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("petriDishDroppedAncestorSig"), self.petriAncestorDroppedSlot)

    # If an ancestor was dropped into the trash can

    self.connect(self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("DeleteFromAncestorViewSig"), self.deleteAncestorSlot)

    self.ChangeMutationTextSlot()
    self.ChangeWorldSizeTextSlot()
    self.m_session_mdl.m_session_mdtr.emit(
      PYSIGNAL("setAvidaSig"),
      (self.m_avida,))

    self.populated = False
    
  def destruct(self):
    self.m_session_petri_view = None
    self.m_avida = None
    self.full_petri_dict = {}
    # @@kgn; was:
    # self.DishDisabled = False
    self.DishDisabled = True
    self.disconnect(self.MutationSlider, SIGNAL("valueChanged(int)"), 
      self.ChangeMutationTextSlot)
    self.disconnect(self.WorldSizeSlider, SIGNAL("valueChanged(int)"), 
      self.ChangeWorldSizeTextSlot)
    self.disconnect(self.StopManuallyRadioButton, SIGNAL("clicked()"), 
      self.ChangeStopSpinBoxEnabledSlot)
    self.disconnect(self.StopAtRadioButton, SIGNAL("clicked()"), 
      self.ChangeStopSpinBoxEnabledSlot)
    self.disconnect(self.StopAtSpinBox, SIGNAL("valueChanged(int)"), 
      self.ChangeStopSpinBoxSlot)
    self.disconnect(self.SavePetriPushButton, SIGNAL("clicked()"), 
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("freezeDishPhaseISig"))
    self.disconnect(self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("freezeDishPhaseIISig"), self.FreezePetriSlot)
    self.disconnect(self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("FillDishSig"), self.FillDishSlot)
    self.disconnect(self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("doStartAvidaSig"), self.DisablePetriConfigureSlot)
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

  
  # When the user changes the mutation slider (which has a log scale) change the
  # text next to it (which is liner)

  def ChangeMutationTextSlot(self):
    slide_value = float(self.MutationSlider.value())/10000.0
    slide_value = pow(10,slide_value)
    if slide_value < 0.0011:
      slide_value = 0.0
    if slide_value >= 1 or slide_value < 0.00001:
      slide_value_txt = ("%1.1f" % (slide_value)) # + "%"
    elif slide_value > 0.1:
      slide_value_txt = ("%1.2f" % (slide_value)) # + "%"
    elif slide_value > 0.01:
      slide_value_txt = ("%1.3f" % (slide_value)) # + "%"
    else:
      slide_value_txt = ("%1.4f" % (slide_value)) # + "%"
    self.MutationRateLineEdit.setText(slide_value_txt)
  
  # When the user changes the mutation slider (which has a log scale) change the
  # text next to it (which is liner). Must check if the text entered is a 
  # valid real number

  def ChangeMutationSliderSlot(self):
    validNumEntered = True
    mutText = str(self.MutationRateLineEdit.text())
    mutText = (mutText.rstrip('% ')).lstrip('+- ')
    decPtLocation = mutText.find('.')
    if (decPtLocation == -1):
      if (not mutText.isdigit()):
        validNumEntered = False
      else:
        mutValue = float(mutText)
    else:
      if (not (mutText[0:decPtLocation].isdigit() and 
          mutText[decPtLocation+1:].isdigit())):
        validNumEntered = False
      else:
        decPlace = float(len(mutText[decPtLocation+1:]))
        divisor = pow(10.0,decPlace)
        mutValue = float(mutText[0:decPtLocation]) + \
                  (float(mutText[decPtLocation+1:])/divisor)

    # if they have entered a bad number pull the current value off the slider

    if (not validNumEntered):
      self.ChangeMutationTextSlot()
    else:
      if (mutValue < 0.0):
        mutValue = 0.0
      elif (mutValue > 100.0):
        mutValue = 100.0
      if mutValue > 0.00001:
        self.MutationSlider.setValue(int(math.log10(mutValue) * 10000))
      else:
        self.MutationSlider.setValue(-30000)

  def ChangeWorldSizeTextSlot(self):
    slide_value = str(self.WorldSizeSlider.value())
    slide_value_txt = "x " + slide_value + " cells"
    self.WorldSizeTextLabel.setText(slide_value_txt)
    self.WorldSizeLineEdit.setText(slide_value)

  # Move the world size slider based on the WorldSizeLineEdit widget
  # that widget will be sure that a 1 to 3 digit integer is returned

  def ChangeWorldSizeSliderSlot(self):
    if (not self.WorldSizeLineEdit.text()):
      tmp_value = self.WorldSizeSlider.value()
    else:
      tmp_value = int(str(self.WorldSizeLineEdit.text()))
    if (tmp_value < self.WorldSizeSlider.minValue()):
      tmp_value = self.WorldSizeSlider.minValue()
    elif (tmp_value > self.WorldSizeSlider.maxValue()):
      tmp_value = self.WorldSizeSlider.maxValue()
    self.WorldSizeSlider.setValue(tmp_value)

  def ChangeStopSpinBoxEnabledSlot(self):
    if self.StopManuallyRadioButton.isChecked() == True:
      self.StopAtSpinBox.setEnabled(False)
      # BDB -- hack for pause at given update
      self.m_session_mdl.m_update_to_pause = -99
    else:
      self.StopAtSpinBox.setEnabled(True)
      # BDB -- hack for pause at given update
      self.m_session_mdl.m_update_to_pause = self.StopAtSpinBox.value()
  
  def ChangeStopSpinBoxSlot(self):
    # BDB -- hack for pause at given update
    self.m_session_mdl.m_update_to_pause = self.StopAtSpinBox.value()
     
  def FillDishSlot(self, dish_name, petri_dict):
    
    self.full_petri_dict = petri_dict.dictionary
    settings_dict =  petri_dict.dictionary["SETTINGS"]

    # Erase all items for the ancestor list 

    self.AncestorIconView.clearGenomeDict()
    self.AncestorIconView.clear()

    # Find all ancestors with the name of the form START_CREATUREi

    i = 0
    while(settings_dict.has_key("START_CREATURE" + str(i))):
      try:
        start_creature = settings_dict["START_CREATURE" + str(i)]

        # Read the genome from the petri dish file

        org_string = settings_dict["START_GENOME" + str(i)]
        tmp_name=self.AncestorIconView.addGenomeToDict(start_creature, org_string)
        tmp_item = pyNewIconViewItem(self.AncestorIconView, tmp_name )
      except Exception, details:
        traceback.print_exc()
      i = i + 1
    if settings_dict.has_key("MAX_UPDATES") == True:
      max_updates = int(settings_dict["MAX_UPDATES"])
    else:
      max_updates = -1
    self.StopAtSpinBox.setValue(max_updates)
    if max_updates < 0:
       self.StopManuallyRadioButton.setChecked(True)
       self.StopAtRadioButton.setChecked(False)
       self.StopAtSpinBox.setEnabled(False)
       # BDB -- hack for pause at given update
       self.m_session_mdl.m_update_to_pause = -99
    else:
       self.StopManuallyRadioButton.setChecked(False)
       self.StopAtRadioButton.setChecked(True)
       self.StopAtSpinBox.setEnabled(True)
       # BDB -- hack for pause at given update
       self.m_session_mdl.m_update_to_pause = self.StopAtSpinBox.value()
    if settings_dict.has_key("WORLD-X") == True:
      self.WorldSizeSlider.setValue(int(settings_dict["WORLD-X"]))
    else:
      self.WorldSizeSlider.setValue(30)
    if settings_dict.has_key("RANDOM_SEED") == True:
      seed = int(settings_dict["RANDOM_SEED"])
    else:
      seed = 0
    if seed == 0:
       self.RandomGeneratedRadioButton.setChecked(True)
       self.RandomFixedRadioButton.setChecked(False)
    else:
       self.RandomGeneratedRadioButton.setChecked(False)
       self.RandomFixedRadioButton.setChecked(True)
    if settings_dict.has_key("COPY_MUT_PROB") == True:
      copy_mutation_percent = float(settings_dict["COPY_MUT_PROB"]) * 100;
    else:
      copy_mutation_percent = 0.5
    if copy_mutation_percent > 0.00000001:
      self.MutationSlider.setValue(int(math.log10(copy_mutation_percent) * 10000))
    else:
      self.MutationSlider.setValue(-30000)
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
    self.ConfigTitleTextLabel.setText("Environmental Settings Disabled During Run")

    # Turn off the controls 

    self.AncestorIconView.setEnabled(False)

    # BDB -- while using pause at hack don't turn off these settings

    # self.StopAtSpinBox.setEnabled(False) 
    # self.StopManuallyRadioButton.setEnabled(False)
    # self.StopAtRadioButton.setEnabled(False)
    # self.StopHeadTextLabel.setEnabled(False)

    self.RandomGeneratedRadioButton.setEnabled(False)
    self.RandomFixedRadioButton.setEnabled(False)
    self.MutationSlider.setEnabled(False)
    self.LocalBirthRadioButton.setEnabled(False)
    self.MassActionRadioButton.setEnabled(False)
    self.EasyTextLabel.setEnabled(False)
    self.ModerateTextLabel.setEnabled(False)
    self.HardTextLabel.setEnabled(False)
    self.VHardTextLabel.setEnabled(False)
    self.BrutalTextLabel.setEnabled(False)
    # self.line1.setEnabled(False)
    # self.line2.setEnabled(False)
    # self.line3.setEnabled(False)
    # self.line4.setEnabled(False)
    self.NotCheckBox.setEnabled(False)
    self.NandCheckBox.setEnabled(False)
    self.AndCheckBox.setEnabled(False)
    self.OrnCheckBox.setEnabled(False)
    self.OrCheckBox.setEnabled(False)
    self.AndnCheckBox.setEnabled(False)
    self.NorCheckBox.setEnabled(False)
    self.XorCheckBox.setEnabled(False)
    self.EquCheckBox.setEnabled(False)
    self.MutationRateLineEdit.setEnabled(False)
    self.MutationRateHeadTextLabel.setEnabled(False)
    self.WorldSizeTextLabel.setEnabled(False)
    self.WorldSizeHeadTextLable.setEnabled(False)
    self.WorldSizeSlider.setEnabled(False)
    self.WorldSizeLineEdit.setEnabled(False)
    self.RewardTextLabel.setEnabled(False)
    self.RandomHeadTextLabel.setEnabled(False)
    self.AncestorHeadTextLabel.setEnabled(False)
    self.BirthHeadTextLabel.setEnabled(False)
    self.DishDisabled = True
    self.m_session_mdl.m_session_mdtr.emit(
      PYSIGNAL("doDisablePetriDishSig"), ())

  def EnablePetriConfigureSlot(self):
    self.ConfigTitleTextLabel.setText("Environmental Settings")

    # Turn on the controls 
    
    self.AncestorIconView.setEnabled(True)
    self.StopAtSpinBox.setEnabled(True)
    self.StopManuallyRadioButton.setEnabled(True)
    self.StopAtRadioButton.setEnabled(True)
    self.WorldSizeSlider.setEnabled(True)
    self.RandomGeneratedRadioButton.setEnabled(True)
    self.RandomFixedRadioButton.setEnabled(True)
    self.MutationSlider.setEnabled(True)
    self.LocalBirthRadioButton.setEnabled(True)
    self.MassActionRadioButton.setEnabled(True)
    self.EasyTextLabel.setEnabled(True)
    self.ModerateTextLabel.setEnabled(True)
    self.HardTextLabel.setEnabled(True)
    self.VHardTextLabel.setEnabled(True)
    self.BrutalTextLabel.setEnabled(True)
    # self.line1.setEnabled(True)
    # self.line2.setEnabled(True)
    # self.line3.setEnabled(True)
    # self.line4.setEnabled(True)
    self.NotCheckBox.setEnabled(True)
    self.NandCheckBox.setEnabled(True)
    self.AndCheckBox.setEnabled(True)
    self.OrnCheckBox.setEnabled(True)
    self.OrCheckBox.setEnabled(True)
    self.AndnCheckBox.setEnabled(True)
    self.NorCheckBox.setEnabled(True)
    self.XorCheckBox.setEnabled(True)
    self.EquCheckBox.setEnabled(True)
    self.MutationRateLineEdit.setEnabled(True)
    self.WorldSizeTextLabel.setEnabled(True)
    self.MutationRateHeadTextLabel.setEnabled(True)
    self.WorldSizeHeadTextLable.setEnabled(True)
    self.WorldSizeLineEdit.setEnabled(True)
    self.RewardTextLabel.setEnabled(True)
    self.RandomHeadTextLabel.setEnabled(True)
    self.AncestorHeadTextLabel.setEnabled(True)
    self.BirthHeadTextLabel.setEnabled(True)
    self.StopHeadTextLabel.setEnabled(True)
    self.DishDisabled = False


  def CreateFilesFromPetriSlot(self, out_dir = None):

    # The input files will be placed in a python generated temporary directory
    # ouput files will be stored in tmp_dir/output until the data is frozen
    # (ie saved)

    self.full_petri_dict["SETTINGS"] = self.Form2Dictionary()
    write_object = pyWriteAvidaCfgEvent(self.full_petri_dict,
      self.m_session_mdl, 
      self.m_session_mdl.m_current_workspace,
      self.m_session_mdl.m_current_freezer, self.m_session_mdl.m_tempdir,
      self.m_session_mdl.m_tempdir_out)
    self.m_session_mdl.m_session_mdtr.emit(
      PYSIGNAL("doInitializeAvidaPhaseIISig"), 
      (os.path.join(self.m_session_mdl.m_tempdir, "avida_cfg.avida"),))
      
  def Form2Dictionary(self):
    settings_dict = {}
    
    # Write START_CREATUREi for all the organisms in the Ancestor Icon View

    curr_item = self.AncestorIconView.firstItem()
    i = 0
    while (curr_item):
      settings_dict["START_CREATURE" + str(i)] = str(curr_item.text())
 
      settings_dict["START_GENOME" + str(i)] = self.AncestorIconView.getGenomeFromDict(str(curr_item.text()))
      i = i + 1
      curr_item = curr_item.nextItem()
    if (self.StopAtRadioButton.isChecked() == True):
      settings_dict["MAX_UPDATES"] = self.StopAtSpinBox.value()
    else:
      settings_dict["MAX_UPDATES"] = -1
    settings_dict["WORLD-X"] = self.WorldSizeSlider.value()
    settings_dict["WORLD-Y"] = self.WorldSizeSlider.value()
    if self.RandomFixedRadioButton.isChecked() == True:
      settings_dict["RANDOM_SEED"] = 777
    else:
      settings_dict["RANDOM_SEED"] = 0
    slide_value = float(self.MutationSlider.value())/10000.0
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
    
  def FreezePetriSlot(self, population_dict = None, ancestor_dict = None, send_reset_signal = False, send_quit_signal = False):
    if len(population_dict) == 0:
      freeze_empty_only_flag = True;
    else:
      freeze_empty_only_flag = False;
    tmp_dict = {}
    tmp_dict["SETTINGS"] = self.Form2Dictionary()
    m_pop_up_freezer_file_name = pyFreezeDialogCtrl()
    file_name = m_pop_up_freezer_file_name.showDialog(self.m_session_mdl, freeze_empty_only_flag)
    file_name_len = len(file_name.rstrip())

    # If the user is saving a full population expand the name and insert
    # the population dictionary into the temporary dictionary

    if (file_name_len > 0):
      is_empty_dish = m_pop_up_freezer_file_name.isEmpty()
      is_organism = m_pop_up_freezer_file_name.isOrganism()
      if is_organism:
        tmp_dict = {1:self.m_session_mdl.m_current_cell_genome} 
      elif (not is_empty_dish):
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
        tmp_dict["ANCESTOR_NAMES"] = self.m_session_mdl.m_ancestors_dict
        tmp_dict["ANCESTOR_LINKS"] = ancestor_dict
      freezer_file = pyWriteToFreezer(tmp_dict, file_name)
      if (is_empty_dish):
        self.m_session_mdl.saved_empty_dish = True
      else:
        self.m_session_mdl.saved_full_dish = True

    
      # Have program redraw the freezer menu pane

      self.m_session_mdl.m_session_mdtr.emit(
        PYSIGNAL("doRefreshFreezerInventorySig"), ())

      if send_reset_signal:
        self.m_session_mdl.m_session_mdtr.emit(
          PYSIGNAL("restartPopulationSig"), (self.m_session_mdl, ))

      # If the send_quit_signal flag was sent to this routine kill the 
      # application (Instead of killing the application directly a signal 
      # should be sent upto the workspace moderator)

      if send_quit_signal:
        qApp.quit()


  # Routine to load data from Avida-ED into Avida Core

  def doLoadPetriDishConfigFileSlot(self, avida_cfgFileName = None):
    avida_cfg = cGenesis()
    avida_cfg.Open(cString(avida_cfgFileName))
    if 0 == avida_cfg.IsOpen():
      warningNoMethodName("Unable to find file " +  avida_cfgFileName)
      return
    descr("self.setAvidaSlot(None) ...")
    self.setAvidaSlot(None)
    descr("self.setAvidaSlot(None) done.")
    avida = pyAvida()
    avida.construct(avida_cfg)
    descr("self.setAvidaSlot(avida) ...")
    self.setAvidaSlot(avida)
    descr("self.setAvidaSlot(avida) done.")

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

    # @@kgn
    self.DishDisabled = False

  def dragEnterEvent( self, e ):
    freezer_item_list = QString()
 
    # freezer_item_list is a string...tab delimited list of file names 

    if ( QTextDrag.decode( e, freezer_item_list ) ) :
      freezer_item_list = str(e.encodedData("text/plain"))
      errors = False
      for freezer_item_name in freezer_item_list.split("\t")[1:]:
        if os.path.exists(freezer_item_name) == False:
            errors = True
            warningNoMethodName(freezer_item_name + " does not exist.")
      if not(errors):
        e.acceptAction(True)

  # When an item (or list of items) is dropped checked that all the items
  # exist before sending out the signal to process

  def dropEvent( self, e ):
    freezer_item_list = QString()
    if ( QTextDrag.decode( e, freezer_item_list ) ) :
      freezer_item_list = str(e.encodedData("text/plain"))
      errors = False
      for freezer_item_name in freezer_item_list.split("\t")[1:]:
        if os.path.exists(freezer_item_name) == False:
            errors = True
            warningNoMethodName(freezer_item_name + " does not exist.")
      if not(errors): 
        # self.emit(PYSIGNAL("petriDishDroppedInPopViewSig"), (e,))
        self.petriDroppedSlot(e)

  def petriDroppedSlot(self, e):
    # Try to decode to the data you understand...
    freezer_item_list = QString()

#    if ( QTextDrag.decode( e, freezer_item_list ) and not self.DishDisabled) :
    if ( QTextDrag.decode( e, freezer_item_list )) :
      freezer_item_list = str(e.encodedData("text/plain"))
      freezer_item_names = freezer_item_list.split("\t")[1:]
      if (len(freezer_item_names) > 1):
         info("Only one petri dish can be dragged here")
      else:
        freezer_item_name = freezer_item_names[0]
        if freezer_item_name[-8:] == 'organism':
          if (not self.DishDisabled) :
            info("Organisms should be placed in the Ancestor Box")
          else :
            try:
              raise Exception
            except Exception, details:
              traceback.print_exc()
            info("Organisms can't be dragged into a running population")
          return
        elif freezer_item_name[-4:] == 'full':
          freezer_item_name_temp = os.path.join(freezer_item_name, 'petri_dish')
          self.m_session_mdl.new_full_dish = True
        else:
          freezer_item_name_temp = freezer_item_name
          self.m_session_mdl.new_empty_dish = True
        thawed_item = pyReadFreezer(freezer_item_name_temp)
        self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("doDefrostDishSig"),
          (os.path.splitext((os.path.split(str(freezer_item_name))[1]))[0], thawed_item,))

        # initialize Avida (which repaints the dish)

        self.m_session_mdl.m_session_mdtr.emit(
          PYSIGNAL("doInitializeAvidaPhaseISig"),
          (self.m_session_mdl.m_tempdir,))

  def petriAncestorDroppedSlot(self, e):
    # Try to decode to the data you understand...
    freezer_item_list = QString()
    if (QTextDrag.decode( e, freezer_item_list ) and self.DishDisabled) :
      try:
        raise Exception
      except Exception, details:
        traceback.print_exc()
      info("Organisms can't be dragged into a running population")
    elif ( QTextDrag.decode( e, freezer_item_list ) and not self.DishDisabled) :
      freezer_item_list = str(e.encodedData("text/plain"))
      freezer_item_names = freezer_item_list.split("\t")[1:]
      for freezer_item_name in freezer_item_list.split("\t")[1:]:
        if freezer_item_name[-8:] == 'organism':
          core_name = freezer_item_name[:-9]
          core_name = os.path.basename(str(freezer_item_name[:-9]))

          # Read the genome from the organism file

          org_file = open(os.path.join(self.m_session_mdl.m_current_freezer,
                          core_name+".organism"))
          org_string = org_file.readline()
          org_string = org_string.rstrip()
          org_string = org_string.lstrip()
          org_file.close

          tmp_name = self.AncestorIconView.addGenomeToDict(core_name, org_string)
          tmp_item = pyNewIconViewItem(self.AncestorIconView, tmp_name)
        else:
          info("Dishes should not be dragged into Ancestor Box")

      # initialize Avida (which repaints the dish)

      self.m_session_mdl.m_session_mdtr.emit(
        PYSIGNAL("doInitializeAvidaPhaseISig"),
        (self.m_session_mdl.m_tempdir,))


  # Find the first item in the AncestorView with the name ancestor_item_name
  # and remove it. Items in AncestorVeiw should have unique names so the 
  # correct item should be deleted.

  def deleteAncestorSlot(self, ancestor_item_name):
    curr_item = self.AncestorIconView.firstItem()
    while (curr_item):
      curr_item_name = str(curr_item.text())
      if (str(curr_item_name) == ancestor_item_name):
        self.AncestorIconView.takeItem(curr_item)
        # curr_item.~QIconViewItem()
        self.AncestorIconView.removeGenomeFromDict(curr_item_name)
        break
      curr_item = curr_item.nextItem()

