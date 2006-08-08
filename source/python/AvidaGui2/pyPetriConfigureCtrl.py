# -*- coding: utf-8 -*-

from descr import descr

from pyAvida import pyAvida
from pyFreezeDialogCtrl import pyFreezeDialogCtrl
from pyPetriConfigureView import pyPetriConfigureView
from pyWriteGenesisEvent import pyWriteGenesisEvent
from pyWriteToFreezer import pyWriteToFreezer

from AvidaCore import cGenesis, cString

from qt import *

imageAncestor_data = [
"24 16 23 1",
"t c #3c553e",
"n c #42a985",
"l c #782424",
"f c #7e9a3e",
"i c #8c7c75",
"d c #914c54",
"q c #a1ca5d",
"o c #a6a8a3",
"r c #b3b5b2",
"s c #b5868b",
"c c #bba4a6",
"m c #c02a30",
"p c #c0bebe",
"a c #cbcccb",
"j c #d7d7d6",
"# c #e64d4a",
"e c #e7c9cc",
"u c #eb3c88",
"b c #efefef",
"k c #f2ddde",
"g c #f4756e",
"h c #f7a798",
". c #ffffff",
"....................#a..",
"..................bcde..",
"................bcdcfb..",
"...........becghcdijca..",
"..........kclliml#dejn..",
"........jeddefoolooi###c",
".......kpdjqaooipjaooole",
".bbgbb.pdiaariosb.barr..",
"biddstm#gdsens#s........",
"jcatoisdsdmgggsk........",
".renaatdooosdsb.........",
".ba.qrl....b............",
".jarsdg.................",
"..roupb.................",
".baae...................",
"..bj...................."
]

import os, os.path, shutil
import math

class pyPetriConfigureCtrl(pyPetriConfigureView):

  def __init__(self,parent = None,name = None,fl = 0):
    pyPetriConfigureView.__init__(self,parent,name,fl)
    self.imageAncestor = QPixmap(imageAncestor_data)
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
      PYSIGNAL("petriDishDroppedInPopViewSig"), self.petriDroppedSlot)
    self.connect(self.AncestorIconView, 
      SIGNAL("dropped(QDropEvent*,const QValueList<QIconDragItem>&)"),
      self.petriAncestorDroppedSlot)
    # self.connect(self.AncestorIconView, SIGNAL("clicked(QIconViewItem*)"),
    #   self.setAncestorDragSlot)
    self.ChangeMutationTextSlot()
    self.ChangeWorldSizeTextSlot()
    self.m_session_mdl.m_session_mdtr.emit(
      PYSIGNAL("setAvidaSig"),
      (self.m_avida,))

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
    descr()
    
    self.full_petri_dict = petri_dict.dictionary
    settings_dict =  petri_dict.dictionary["SETTINGS"]

    # Erase all items for the ancestor list (largest to smallest index)

    self.AncestorIconView.clear()

    # Find all ancestors with the name of the form START_CREATUREi

    i = 0
    while(settings_dict.has_key("START_CREATURE" + str(i))):
      start_creature = settings_dict["START_CREATURE" + str(i)]
      tmp_item = QIconViewItem(self.AncestorIconView, start_creature, 
                self.imageAncestor)
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
    descr()
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
    self.line1.setEnabled(False)
    self.line2.setEnabled(False)
    self.line3.setEnabled(False)
    self.line4.setEnabled(False)
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
    descr()
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
    self.line1.setEnabled(True)
    self.line2.setEnabled(True)
    self.line3.setEnabled(True)
    self.line4.setEnabled(True)
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
    descr()

    # The input files will be placed in a python generated temporary directory
    # ouput files will be stored in tmp_dir/output until the data is frozen
    # (ie saved)

    self.full_petri_dict["SETTINGS"] = self.Form2Dictionary()
    write_object = pyWriteGenesisEvent(self.full_petri_dict,
      self.m_session_mdl, 
      self.m_session_mdl.m_current_workspace,
      self.m_session_mdl.m_current_freezer, self.m_session_mdl.m_tempdir,
      self.m_session_mdl.m_tempdir_out)
    self.m_session_mdl.m_session_mdtr.emit(
      PYSIGNAL("doInitializeAvidaPhaseIISig"), 
      (os.path.join(self.m_session_mdl.m_tempdir, "genesis.avida"),))
      
  def Form2Dictionary(self):
    descr()
    settings_dict = {}
    
    # Write START_CREATUREi for all the organisms in the Ancestor Icon View

    curr_item = self.AncestorIconView.firstItem()
    i = 0
    while (curr_item):
      settings_dict["START_CREATURE" + str(i)] = str(curr_item.text())
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
    descr()
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
    descr("BDB")
    descr(e)
    descr("BDB")

    freezer_item_name = QString()
 
    # freezer_item_name is a string...the file name 

    if ( QTextDrag.decode( e, freezer_item_name ) ) :
      freezer_item_name = str(e.encodedData("text/plain"))
      if os.path.exists(freezer_item_name) == False:
        descr("that was not a valid path (1)")
      else: 
        e.acceptAction(True)
        descr("accepted.")

      
  def dropEvent( self, e ):
    descr()
    freezer_item_name = QString()
    if ( QTextDrag.decode( e, freezer_item_name ) ) :
      freezer_item_name = str(e.encodedData("text/plain"))
      if os.path.exists(freezer_item_name) == False:
        print "that was not a valid path (2)" 
      else: 
        self.emit(PYSIGNAL("petriDishDroppedInPopViewSig"), (e,))

  # The function petriDroppedSlot and petriAncestorDroppedSlot are identical
  # at some point petriDroppedSlot should not allow creatures dropped outside
  # the AncestorIconView to be added to the Ancestort List

  def petriDroppedSlot(self, e):
    descr()
    # Try to decode to the data you understand...
    freezer_item_name = QString()
    if ( QTextDrag.decode( e, freezer_item_name ) and not self.DishDisabled) :
      freezer_item_name = str(e.encodedData("text/plain"))
      if freezer_item_name[-8:] == 'organism':
        core_name = freezer_item_name[:-9]
        core_name = os.path.basename(str(freezer_item_name[:-9]))
        tmp_item = QIconViewItem(self.AncestorIconView, core_name, self.imageAncestor)
        #initialize Avida (which repaints the dish)
      print "ABOUT TO SEND INIT"
      self.m_session_mdl.m_session_mdtr.emit(
        PYSIGNAL("doInitializeAvidaPhaseISig"),
        (self.m_session_mdl.m_tempdir,))
      print "INIT SENT"
#        if you are reading this, the next line doesn't matter and should die
#        return
        

  def petriAncestorDroppedSlot(self, e):
    descr()
    # Try to decode to the data you understand...
    freezer_item_name = QString()
    if ( QTextDrag.decode( e, freezer_item_name ) and not self.DishDisabled) :
      freezer_item_name = str(e.encodedData("text/plain"))
      if freezer_item_name[-8:] == 'organism':
        core_name = freezer_item_name[:-9]
        core_name = os.path.basename(str(freezer_item_name[:-9]))
        tmp_item = QIconViewItem(self.AncestorIconView, core_name, self.imageAncestor)
        return

  # def setAncestorDragSlot(self, item):
  #   descr()
  #   
  #   # if the user clicks on a portion of the ancestor icon view that does not
  #   # have an actual icon quit this subroutine

  #   print type(item)
  #   if (not item):
  #     return
  #   dragHolder = QTextDrag("ancestor." + str(item.text()), self.AncestorIconView, "dragname")
  #   dragHolder.dragCopy()
