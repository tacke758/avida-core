
from pyOrganismConfigureView import pyOrganismConfigureView

from qt import *

class pyOrganismConfigureCtrl(pyOrganismConfigureView):
  def __init__(self, parent = None, name = None, fl = 0):
    pyOrganismConfigureView.__init__(self, parent, name, fl)
  def construct(self, session_mdl):
    self.m_session_mdl = session_mdl

    self.connect(self.m_heads_type_cb, SIGNAL("activated(int)"), self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("ScopeConfig_HeadsTypeCBActivatedSig"))

    self.connect(self.m_show_task_tests_cb, SIGNAL("toggled(bool)"), self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("ScopeConfig_ShowTaskTestsCBToggledSig"))
    self.connect(self.m_show_registers_cb, SIGNAL("toggled(bool)"), self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("ScopeConfig_ShowRegistersCBToggledSig"))
    self.connect(self.m_animate_head_movement_cb, SIGNAL("toggled(bool)"), self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("ScopeConfig_AnimateHeadMovementCBToggledSig"))
    self.connect(self.m_show_stacks_cb, SIGNAL("toggled(bool)"), self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("ScopeConfig_ShowStacksCBToggledSig"))
    self.connect(self.m_show_heads_as_letters_cb, SIGNAL("toggled(bool)"), self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("ScopeConfig_ShowHeadsAsLettersCBToggledSig"))
    self.connect(self.m_show_instruction_names_cb, SIGNAL("toggled(bool)"), self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("ScopeConfig_ShowInstructionNamesCBToggledSig"))
    self.connect(self.m_show_inputs_and_outputs_cb, SIGNAL("toggled(bool)"), self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("ScopeConfig_ShowInputsAndOutputsCBToggledSig"))
    self.connect(self.m_show_full_stacks_cb, SIGNAL("toggled(bool)"), self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("ScopeConfig_ShowFullStacksCBToggledSig"))
    self.connect(self.m_animate_instruction_copy_cb, SIGNAL("toggled(bool)"), self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("ScopeConfig_AnimateInstructionCopyCBToggledSig"))
    self.connect(self.m_show_hardware_cb, SIGNAL("toggled(bool)"), self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("ScopeConfig_ShowHardwareCBToggledSig"))
    self.connect(self.m_animate_organism_divide_cb, SIGNAL("toggled(bool)"), self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("ScopeConfig_AnimateOrganismDivideCBToggledSig"))

    self.connect(self.m_layout_spacing_sb, SIGNAL("valueChanged(int)"), self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("ScopeConfig_LayoutSpacingSBValueChangedSig"))
    self.connect(self.m_hardware_indicator_size_sb, SIGNAL("valueChanged(int)"), self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("ScopeConfig_HardwareIndicatorSBValueChangedSig"))
