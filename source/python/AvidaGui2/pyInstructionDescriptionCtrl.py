
from descr import descr
from qt import *
#from AvidaCore import cHardwareDefs, cHardwareCPUDefs
from AvidaCore import *

descriptions_dict = {
  'a':"""The instructions nop-A (a), nop-B (b), and nop-C (c) are no-operation instructions, and will not do anything when executed. They will, however, modifiy the behavior of the instruction preceeding it (by changing the CPU component that it affects; see also nop-register notation and nop-head notation) or act as part of a template to denote positions in the genome.""",
  
  'b':"""The instructions nop-A (a), nop-B (b), and nop-C (c) are no-operation instructions, and will not do anything when executed. They will, however, modifiy the behavior of the instruction preceeding it (by changing the CPU component that it affects; see also nop-register notation and nop-head notation) or act as part of a template to denote positions in the genome.""",
  
  'c':"""The instructions nop-A (a), nop-B (b), and nop-C (c) are no-operation instructions, and will not do anything when executed. They will, however, modifiy the behavior of the instruction preceeding it (by changing the CPU component that it affects; see also nop-register notation and nop-head notation) or act as part of a template to denote positions in the genome.""",
  
  'd':"""This instruction compares the BX register to its complement. If they are not equal, the next instruction (after a modifying no-operation instruction, if one is present) is executed. If they are equal, that next instruction is skipped.""",
  
  'e':"""This instruction compares the BX register to its complement. If BX is the lesser of the pair, the next instruction (after a modifying no-operation instruction, if one is present) is executed. If it is greater or equal, then that next instruction is skipped.""",
  
  'f':"""This instruction removes the top element from the active stack, and places it into the BX register.""",
  
  'g':"""This instruction reads in the contents of the BX register, and places it as a new entry at the top of the active stack. The BX register itself remains unchanged.""",
  
  'h':"""This instruction toggles the active stack in the CPU. All other instructions that use a stack will always use the active one.""",
  
  'i':"""This instruction swaps the contents of the BX register with its complement.""",
  
  'j':"""This instruction reads in the contents of the BX register, and shifts all of the bits in that register to the right by one. In effect, it divides the value stored in the register by two, rounding down.""",
  
  'k':"""This instruction reads in the contents of the BX register, and shifts all of the bits in that register to the left by one, placing a zero as the new rightmost bit, and trunkating any bits beyond the 32 maximum. For values that require fewer than 32 bits, it effectively multiplies that value by two.""",
  
  'l':"""The inc (l) and dec (m) instructions read in the contents of the BX register and increment or decrement it by one.""",
  
  'm':"""The inc (l) and dec (m) instructions read in the contents of the BX register and increment or decrement it by one.""",
  
  'n':"""The add (n) and sub (o) instructions read in the contents of the BX and CX registers and either sums them together or subtracts CX from BX (respectively). The result of this operation is then placed in the BX register.""",
  
  'o':"""The add (n) and sub (o) instructions read in the contents of the BX and CX registers and either sums them together or subtracts CX from BX (respectively). The result of this operation is then placed in the BX register.""",
  
  'p':"""This instruction reads in the contents of the BX and CX registers (each of which are 32-bit numbers) and performs a bitwise nand operation on them. The result of this operation is placed in the BX register. Note that this is the only logic operation provided in the basic avida instruction set.""",
  
  'q':"""This is the input/output instruction. It takes the contents of the BX register and outputs it, checking it for any tasks that may have been performed. It will then place a new input into BX.""",
  
  'r':"""This instruction allocates additional memory for the organism up to the maximum it is allowed to use for its offspring.""",
  
  's':"""This instruction is used for an organism to divide off an finnished offspring. The original organism keeps the state of its memory up until the read-head. The offspring's memory is initialized to everything between the read-head and the write-head. All memory past the write-head is removed entirely.""",
  
  't':"""This instruction reads the contents of the organism's memory at the position of the read-head, and copy that to the position of the write-head. If a non-zero copy mutation rate is set, a test will be made based on this probability to determine if a mutation occurs. If so, a random instruction (chosen from the full set with equal probability) will be placed at the write-head instead.""",
  
  'u':"""This instruction will read in the template the follows it, and find the location of a complement template in the code. The BX register will be set to the distance to the complement from the current position of the instruction-pointer, and the CX register will be set to the size of the template. The flow-head will also be placed at the beginning of the complement template. If no template follows, both BX and CX will be set to zero, and the flow-head will be placed on the instruction immediatly following the h-search.""",
  
  'v':"""This instruction will cause the IP to jump to the position in memory of the flow-head.""",
  
  'w':"""This instruction will read in the value of the CX register, and the move the IP by that fixed amount through the organism's memory.""",
  
  'x':"""This instruction will copy the position of the IP into the CX register.""",
  
  'y':"""This instruction reads in the template that follows it, and tests if its complement template was the most recent series of instructions copied. If so, it executed the next instruction, otherwise it skips it. This instruction is commonly used for an organism to determine when it has finished producing its offspring.""",
  
  'z':"""This instruction moves the flow-head to the memory position denoted in the CX register.""",
}

class pyInstructionDescriptionCtrl(QTextEdit):
  #def __init__(self,parent = None,name = None,fl = 0):
  #  QLabel.__init__(self,parent,name,fl)
  def __init__(self,parent = None,name = None):
    QTextEdit.__init__(self,parent,name)
    if not name: self.setName("pyInstructionDescriptionCtrl")

    font = QFont(qApp.font())
    font.setPointSize(9)
    self.setFont(font)

    self.setAlignment(Qt.WordBreak)
    self.setReadOnly(True)
    #self.read_fn = None

  def setReadFn(self, sender, read_fn):
    #self.read_fn = read_fn
    self.connect(sender, PYSIGNAL("propagated-FrameShownSig"), self.frameShownSlot)
    return self

  def frameShownSlot(self, frames, frame_no):
    #if frames is not None and frame_no < frames.m_gestation_time and self.read_fn is not None:
    label_text = "(no instruction)"
    if frames is not None and frame_no < frames.m_gestation_time:
      #self.read_fn(frames, frame_no)
      inst_set = frames.getHardwareSnapshotAt(frame_no).GetInstSet()
      short_name = frames.m_genome_info[frame_no][frames.m_ihead_info[frame_no]]
      #self.setText(short_name)
      inst = cInstruction()
      inst.SetSymbol(short_name)
      long_name = inst_set.GetName(inst)
      description = descriptions_dict.has_key(short_name) and descriptions_dict[short_name] or ""
      label_text = "%s: %s\n%s" % (short_name, long_name, description)
      #self.setText("%s: %s" % (short_name, long_name))
    #else: 
    #  self.setText("(no instruction)")
    self.setText(label_text)

