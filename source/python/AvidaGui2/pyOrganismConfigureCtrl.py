
from pyOrganismConfigureView import pyOrganismConfigureView
from descr import *
from qt import *
import math

class pyOrganismConfigureCtrl(pyOrganismConfigureView):
  def __init__(self, parent = None, name = None, fl = 0):
    pyOrganismConfigureView.__init__(self, parent, name, fl)

  def construct(self, session_mdl):
    self.m_session_mdl = session_mdl

    self.connect(self.m_mutation_slider, SIGNAL("valueChanged(int)"), 
      self.ChangeMutationTextSlot)
    self.connect(self.m_mutation_rate_lineedit, SIGNAL("returnPressed()"), 
      self.ChangeMutationSliderSlot)

    self.connect(self.RandomGeneratedRadioButton, SIGNAL("toggled(bool)"), 
      self.ChangeRandomGeneratedRadioButtonSlot)

    #self.connect(self.m_mutation_slider, SIGNAL("valueChanged(int)"), self.m_session_mdl.m_session_mdtr,
    #  PYSIGNAL("ScopeConfig_MutationSliderValueChangedSig"))

    self.m_mutation_slider.setValue(-30000)
    #self.ChangeMutationTextSlot()

    #self.connect(self.m_heads_type_cb, SIGNAL("activated(int)"), self.m_session_mdl.m_session_mdtr,
    #  PYSIGNAL("ScopeConfig_HeadsTypeCBActivatedSig"))

    #self.connect(self.m_show_task_tests_cb, SIGNAL("toggled(bool)"), self.m_session_mdl.m_session_mdtr,
    #  PYSIGNAL("ScopeConfig_ShowTaskTestsCBToggledSig"))
    #self.connect(self.m_show_registers_cb, SIGNAL("toggled(bool)"), self.m_session_mdl.m_session_mdtr,
    #  PYSIGNAL("ScopeConfig_ShowRegistersCBToggledSig"))
    #self.connect(self.m_animate_head_movement_cb, SIGNAL("toggled(bool)"), self.m_session_mdl.m_session_mdtr,
    #  PYSIGNAL("ScopeConfig_AnimateHeadMovementCBToggledSig"))
    #self.connect(self.m_show_stacks_cb, SIGNAL("toggled(bool)"), self.m_session_mdl.m_session_mdtr,
    #  PYSIGNAL("ScopeConfig_ShowStacksCBToggledSig"))
    #self.connect(self.m_show_heads_as_letters_cb, SIGNAL("toggled(bool)"), self.m_session_mdl.m_session_mdtr,
    #  PYSIGNAL("ScopeConfig_ShowHeadsAsLettersCBToggledSig"))
    #self.connect(self.m_show_instruction_names_cb, SIGNAL("toggled(bool)"), self.m_session_mdl.m_session_mdtr,
    #  PYSIGNAL("ScopeConfig_ShowInstructionNamesCBToggledSig"))
    #self.connect(self.m_show_inputs_and_outputs_cb, SIGNAL("toggled(bool)"), self.m_session_mdl.m_session_mdtr,
    #  PYSIGNAL("ScopeConfig_ShowInputsAndOutputsCBToggledSig"))
    #self.connect(self.m_show_full_stacks_cb, SIGNAL("toggled(bool)"), self.m_session_mdl.m_session_mdtr,
    #  PYSIGNAL("ScopeConfig_ShowFullStacksCBToggledSig"))
    #self.connect(self.m_animate_instruction_copy_cb, SIGNAL("toggled(bool)"), self.m_session_mdl.m_session_mdtr,
    #  PYSIGNAL("ScopeConfig_AnimateInstructionCopyCBToggledSig"))
    #self.connect(self.m_show_hardware_cb, SIGNAL("toggled(bool)"), self.m_session_mdl.m_session_mdtr,
    #  PYSIGNAL("ScopeConfig_ShowHardwareCBToggledSig"))
    #self.connect(self.m_animate_organism_divide_cb, SIGNAL("toggled(bool)"), self.m_session_mdl.m_session_mdtr,
    #  PYSIGNAL("ScopeConfig_AnimateOrganismDivideCBToggledSig"))

    #self.connect(self.m_layout_spacing_sb, SIGNAL("valueChanged(int)"), self.m_session_mdl.m_session_mdtr,
    #  PYSIGNAL("ScopeConfig_LayoutSpacingSBValueChangedSig"))
    #self.connect(self.m_hardware_indicator_size_sb, SIGNAL("valueChanged(int)"), self.m_session_mdl.m_session_mdtr,
    #  PYSIGNAL("ScopeConfig_HardwareIndicatorSBValueChangedSig"))



  # When the user changes the mutation slider (which has a log scale) change the
  # text next to it (which is liner)

  def ChangeMutationTextSlot(self):
    slide_value = float(self.m_mutation_slider.value())/10000.0
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
    self.m_mutation_rate_lineedit.setText(slide_value_txt)
    self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("ScopeConfig_MutationSliderValueChangedSig"),(slide_value,))

  # When the user changes the mutation slider (which has a log scale) change the
  # text next to it (which is liner). Must check if the text entered is a 
  # valid real number

  def ChangeMutationSliderSlot(self):
    validNumEntered = True
    mutText = str(self.m_mutation_rate_lineedit.text())
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
        self.m_mutation_slider.setValue(int(math.log10(mutValue) * 10000))
      else:
        self.m_mutation_slider.setValue(-30000)

    self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("ScopeConfig_MutationSliderValueChangedSig"),(mutValue,))
  
  def SetRandomGeneratedRadioButton(self, is_down):
    self.RandomGeneratedRadioButton.setDown(is_down);

  def ChangeRandomGeneratedRadioButtonSlot(self, value):
    self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("ScopeConfig_RandomSeedSig"),(value,))
