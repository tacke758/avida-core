//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "cpu.hh"
#include "head.hh"
#include "../main/inst_lib.hh"
#include "../main/genotype.hh"
#include "../main/population.hh"
#include "../main/genebank.hh"
#include "environment.hh"
#include "head.ii"
#include "hardware.ii"
#include "cpu.ii"

#include <iomanip.h>

///////////////
//  cBaseCPU
///////////////

cBaseCPU::cBaseCPU(int in_cpu_test) : hardware(this)
{
  g_memory.Add(C_BASE_CPU);

  hardware.SetInstLib(cInstLib::InitInstructions());

  cpu_stats.Setup(hardware.GetNumInst());

#ifdef INSTRUCTION_COSTS
  // instruction cost arrays
  num_inst_cost = hardware.GetNumInst();
  inst_cost = new int[num_inst_cost];
  inst_ft_cost = new int[num_inst_cost];
  for( int i=0; i<num_inst_cost; ++i ){
    inst_cost[i] = hardware.GetInstLib().GetCost(i);
    inst_ft_cost[i] = hardware.GetInstLib().GetFTCost(i);
  }
#endif

  // hardware.Reset(this);   // Link this CPU to its hardware.

  info.active_genotype = NULL;
  info.max_executed = -1;
  info.next = NULL;
  info.prev = NULL;
  info.neutral_metric = 0;  // random walk variable
  info.lineage_label = -1; // a label that can be used to keep track of lineages
  info.lineage = 0;

  if (in_cpu_test) {
    environment = new cTestEnvironment(this);
  } else {
    environment = new cMainEnvironment(this);
  }

  phenotype = new cPhenotype(cConfig::GetDefaultTaskLib()); // @TCC - hack
  cpu_stats.genotype_id = -1;
  ResetVariables();
}

cBaseCPU::~cBaseCPU()
{
  delete environment;
  delete phenotype;
  g_memory.Remove(C_BASE_CPU);
}

double cBaseCPU::GetFitness()
{
  return phenotype->GetMerit().CalcFitness(GetGestationTime());
}



//////////////////////////
// And the instructions...
//////////////////////////

void cBaseCPU::Inst_Nop()          // Do Nothing.
{
}

void cBaseCPU::Inst_If0()          // Execute next if ?bx? ==0.
{
  int reg_used = FindModifiedRegister(REG_BX);

  if (hardware.GetRegister(reg_used) != 0) {
    hardware.AdvanceIP();
  }
}

void cBaseCPU::Inst_IfNot0()       // Execute next if ?bx? != 0.
{
  int reg_used = FindModifiedRegister(REG_BX);

  if (hardware.GetRegister(reg_used) == 0) {
    hardware.AdvanceIP();
  }
}

void cBaseCPU::Inst_IfEqu()      // Execute next if bx == ?cx?
{
  int reg_used = FindModifiedRegister(REG_BX);
  int reg_used2 = reg_used + 1;
  if (reg_used2 == NUM_REGISTERS) reg_used2 = REG_AX;

  if (hardware.GetRegister(reg_used) != hardware.GetRegister(reg_used2)) {
    hardware.AdvanceIP();
  }
}

void cBaseCPU::Inst_IfNEqu()     // Execute next if bx != ?cx?
{
  int reg_used = FindModifiedRegister(REG_BX);
  int reg_used2 = reg_used + 1;
  if (reg_used2 == NUM_REGISTERS) reg_used2 = REG_AX;

  if (hardware.GetRegister(reg_used) == hardware.GetRegister(reg_used2)) {
    hardware.AdvanceIP();
  }
}

void cBaseCPU::Inst_IfGr0()       // Execute next if ?bx? ! < 0.
{
  int reg_used = FindModifiedRegister(REG_BX);

  if (hardware.GetRegister(reg_used) <= 0) {
    hardware.AdvanceIP();
  }
}

void cBaseCPU::Inst_IfGr()       // Execute next if bx > ?cx?
{
  int reg_used = FindModifiedRegister(REG_BX);
  int reg_used2 = reg_used + 1;
  if (reg_used2 == NUM_REGISTERS) reg_used2 = REG_AX;

  if (hardware.GetRegister(reg_used) <= hardware.GetRegister(reg_used2)) {
    hardware.AdvanceIP();
  }
}

void cBaseCPU::Inst_IfGrEqu0()       // Execute next if ?bx? != 0.
{
  int reg_used = FindModifiedRegister(REG_BX);

  if (hardware.GetRegister(reg_used) < 0) {
    hardware.AdvanceIP();
  }
}

void cBaseCPU::Inst_IfGrEqu()       // Execute next if bx > ?cx?
{
  int reg_used = FindModifiedRegister(REG_BX);
  int reg_used2 = reg_used + 1;
  if (reg_used2 == NUM_REGISTERS) reg_used2 = REG_AX;

  if (hardware.GetRegister(reg_used) < hardware.GetRegister(reg_used2)) {
    hardware.AdvanceIP();
  }
}

void cBaseCPU::Inst_IfLess0()       // Execute next if ?bx? != 0.
{
  int reg_used = FindModifiedRegister(REG_BX);

  if (hardware.GetRegister(reg_used) >= 0) {
    hardware.AdvanceIP();
  }
}

void cBaseCPU::Inst_IfLess()       // Execute next if ?bx? < ?cx?
{
  int reg_used = FindModifiedRegister(REG_BX);
  int reg_used2 = reg_used + 1;
  if (reg_used2 == NUM_REGISTERS) reg_used2 = REG_AX;

  if (hardware.GetRegister(reg_used) >=  hardware.GetRegister(reg_used2)) {
    hardware.AdvanceIP();
  }
}

void cBaseCPU::Inst_IfLsEqu0()       // Execute next if ?bx? != 0.
{
  int reg_used = FindModifiedRegister(REG_BX);

  if (hardware.GetRegister(reg_used) > 0) {
    hardware.AdvanceIP();
  }
}

void cBaseCPU::Inst_IfLsEqu()       // Execute next if bx > ?cx?
{
  int reg_used = FindModifiedRegister(REG_BX);
  int reg_used2 = reg_used + 1;
  if (reg_used2 == NUM_REGISTERS) reg_used2 = REG_AX;

  if (hardware.GetRegister(reg_used) >  hardware.GetRegister(reg_used2)) {
    hardware.AdvanceIP();
  }
}

void cBaseCPU::Inst_IfBit1()
{
  int reg_used = FindModifiedRegister(REG_BX);

  if ((hardware.GetRegister(reg_used) & 1) == 0) {

    hardware.AdvanceIP();
  }
}

void cBaseCPU::Inst_IfANotEqB()     // Execute next if AX != BX
{
  if (hardware.GetRegister(REG_AX) == hardware.GetRegister(REG_BX) ) {
    hardware.AdvanceIP();
  }
}

void cBaseCPU::Inst_IfBNotEqC()     // Execute next if BX != CX
{
  if (hardware.GetRegister(REG_BX) == hardware.GetRegister(REG_CX) ) {
    hardware.AdvanceIP();
  }
}

void cBaseCPU::Inst_IfANotEqC()     // Execute next if AX != BX
{
  if (hardware.GetRegister(REG_AX) == hardware.GetRegister(REG_CX) ) {
    hardware.AdvanceIP();
  }
}

void cBaseCPU::Inst_JumpF()
{
  cCPUHead jump_location;

  hardware.ReadLabel();
  hardware.ComplementLabel();

  // If there is no label, jump BX steps.
  if (hardware.GetLabelSize() == 0) {
    hardware.JumpHead(hardware.GetRegister(REG_BX), hardware.GetCurHead());
  }

  // Otherwise, jump to the complement label.
  else if ((jump_location = hardware.FindLabel(1)).GetPosition() != -1) {
    hardware.SetHead(jump_location, hardware.GetCurHead());
  }
}


void cBaseCPU::Inst_JumpB()
{
  cCPUHead jump_location;

  hardware.ReadLabel();
  hardware.ComplementLabel();

  // If there is no label, jump BX steps.
  if (hardware.GetLabelSize() == 0) {
    hardware.JumpHead(-(hardware.GetRegister(REG_BX)), hardware.GetCurHead());
  }

  // otherwise jump to the complement label.
  else if ((jump_location = hardware.FindLabel(-1)).GetPosition() != -1) {
    hardware.SetHead(jump_location, hardware.GetCurHead());
  }
}

void cBaseCPU::Inst_JumpP()
{
  if (environment->InPopulation() == FALSE) {
    // A test CPU has a hard time determining a parasite; mark it as one...
    // but we have no idea how successful it is though.  If it has no
    // complement label in the surrounding area, it won't really become a
    // parasite...  This is the best we can do for now.
    SetParasite();
    return;
  }

  cCPUHead jump_location;

  hardware.ReadLabel();
  hardware.ComplementLabel();

  // If there is no label, jump to line BX in creature.
  if (hardware.GetLabelSize() == 0) {
    hardware.SetIP(hardware.GetRegister(REG_BX), environment->GetFacing());
  }

  // otherwise jump to the complement label.
  else if ((jump_location = environment->GetFacing()->FindFullLabel(hardware.GetLabel())).GetPosition() != -1) {
    hardware.SetIP(jump_location);
  }

  // otherwise if complement label was not found; record an error.
  else {
    Fault(FAULT_LOC_JUMP, FAULT_TYPE_WARNING, "jump-p: No complement label");
    return;
  }

  if (hardware.TestParasite()) {
    SetParasite();
  }
}

void cBaseCPU::Inst_JumpSelf()
{
  hardware.ReadLabel();
  hardware.ComplementLabel();

  cCPUHead jump_location;

  // If there is no label, jump to line BX in creature.
  if (hardware.GetLabelSize() == 0) {
    hardware.SetIP(hardware.GetRegister(REG_BX), this);
  }

  // otherwise jump to the complement label.
  else if ( (jump_location =
	    FindFullLabel(hardware.GetLabel())).GetPosition() != -1) {
    hardware.SetIP(jump_location);
  }

  // otherwise if complement label was not found; record and error.
  else {
    Fault(FAULT_LOC_JUMP, FAULT_TYPE_ERROR, "jump-slf: no complement label");
    return;
  }
}

void cBaseCPU::Inst_Call()
{
  cCPUHead jump_location;

  // Put the starting location onto the stack
  int location = hardware.GetInstPointer().GetPosition();
  hardware.StackPush(location);

  // Jump to the compliment label (or by the ammount in the bx register)
  hardware.ReadLabel();
  hardware.ComplementLabel();

  if (hardware.GetLabelSize() == 0) {
    hardware.JumpIP(hardware.GetRegister(REG_BX));
  }
  else if ((jump_location = hardware.FindLabel(1)).GetPosition() != -1) {
    hardware.SetIP(jump_location);
  }
  TestSearchSize(location);
}

void cBaseCPU::Inst_Return()
{
  hardware.SetIP(hardware.StackPop());
}

void cBaseCPU::Inst_Pop()
{
  int reg_used = FindModifiedRegister(REG_BX);
  hardware.Reg_Set(reg_used, hardware.StackPop());
}

void cBaseCPU::Inst_Push()
{
  int reg_used = FindModifiedRegister(REG_BX);
  hardware.StackPush(hardware.GetRegister(reg_used));
}

void cBaseCPU::Inst_HeadPop()
{
  const int head_used = FindModifiedHead(HEAD_IP);
  hardware.SetHead(hardware.StackPop(), this, head_used);
}

void cBaseCPU::Inst_HeadPush()
{
  const int head_used = FindModifiedHead(HEAD_IP);
  hardware.StackPush(hardware.GetHeadPosition(head_used));
}

void cBaseCPU::Inst_PopA() { hardware.Reg_Set(REG_AX, hardware.StackPop()); }

void cBaseCPU::Inst_PopB() { hardware.Reg_Set(REG_BX, hardware.StackPop()); }

void cBaseCPU::Inst_PopC() { hardware.Reg_Set(REG_CX, hardware.StackPop()); }

void cBaseCPU::Inst_PushA() {
  hardware.StackPush(hardware.GetRegister(REG_AX));
}

void cBaseCPU::Inst_PushB() {
  hardware.StackPush(hardware.GetRegister(REG_BX));
}

void cBaseCPU::Inst_PushC() {
  hardware.StackPush(hardware.GetRegister(REG_CX));
}

void cBaseCPU::Inst_SwitchStack()
{
  hardware.SwitchStack();
}

void cBaseCPU::Inst_FlipStack()
{
  hardware.StackFlip();
}

void cBaseCPU::Inst_Swap()
{
  int reg_used = FindModifiedRegister(REG_BX);
  int other_reg = reg_used + 1;
  if (other_reg == NUM_REGISTERS) other_reg = 0;

  int tmp_value  = hardware.GetRegister(reg_used);
  hardware.Reg_Set(reg_used, hardware.GetRegister(other_reg));
  hardware.Reg_Set(other_reg, tmp_value);
}

void cBaseCPU::Inst_SwapAB()
{
  int tmp_value  = hardware.GetRegister(REG_AX);
  hardware.Reg_Set(REG_AX, hardware.GetRegister(REG_BX));
  hardware.Reg_Set(REG_BX, tmp_value);
}

void cBaseCPU::Inst_SwapBC()
{
  int tmp_value  = hardware.GetRegister(REG_CX);
  hardware.Reg_Set(REG_CX, hardware.GetRegister(REG_BX));
  hardware.Reg_Set(REG_BX, tmp_value);
}

void cBaseCPU::Inst_SwapAC()
{
  int tmp_value  = hardware.GetRegister(REG_AX);
  hardware.Reg_Set(REG_AX, hardware.GetRegister(REG_CX));
  hardware.Reg_Set(REG_CX, tmp_value);
}

void cBaseCPU::Inst_CopyReg()
{
  int reg_used = FindModifiedRegister(REG_BX);
  int other_reg = reg_used + 1;
  if (other_reg == NUM_REGISTERS) other_reg = 0;

  hardware.Reg_Set(other_reg, hardware.GetRegister(reg_used));
}


void cBaseCPU::Inst_CopyRegAB()
{
  hardware.Reg_Set(REG_AX, hardware.GetRegister(REG_BX));
}


void cBaseCPU::Inst_CopyRegAC()
{
  hardware.Reg_Set(REG_AX, hardware.GetRegister(REG_CX));
}


void cBaseCPU::Inst_CopyRegBA()
{
  hardware.Reg_Set(REG_BX, hardware.GetRegister(REG_AX));
}


void cBaseCPU::Inst_CopyRegBC()
{
  hardware.Reg_Set(REG_BX, hardware.GetRegister(REG_CX));
}


void cBaseCPU::Inst_CopyRegCA()
{
  hardware.Reg_Set(REG_CX, hardware.GetRegister(REG_AX));
}


void cBaseCPU::Inst_CopyRegCB()
{
  hardware.Reg_Set(REG_CX, hardware.GetRegister(REG_BX));
}


void cBaseCPU::Inst_Reset()
{
  hardware.Reg_Set(REG_AX,0);
  hardware.Reg_Set(REG_BX,0);
  hardware.Reg_Set(REG_CX,0);

  hardware.StackClear();
}

void cBaseCPU::Inst_ShiftR()
{
  int reg_used = FindModifiedRegister(REG_BX);
  hardware.Reg_ShiftR(reg_used);
}

void cBaseCPU::Inst_ShiftL()
{
  int reg_used = FindModifiedRegister(REG_BX);
  hardware.Reg_ShiftL(reg_used);
}

void cBaseCPU::Inst_Bit1()
{
  int reg_used = FindModifiedRegister(REG_BX);

  if ((hardware.GetRegister(reg_used) & 1) == 0) {
    hardware.Reg_Inc(reg_used);
  }
}

void cBaseCPU::Inst_SetNum()
{
  hardware.ReadLabel();
  hardware.Reg_Set(REG_BX, hardware.GetTrinaryLabel());
}

void cBaseCPU::Inst_Inc()
{
  int reg_used = FindModifiedRegister(REG_BX);
  hardware.Reg_Inc(reg_used);
}

void cBaseCPU::Inst_Dec()
{
  int reg_used = FindModifiedRegister(REG_BX);
  hardware.Reg_Dec(reg_used);
}

void cBaseCPU::Inst_Zero()
{
  int reg_used = FindModifiedRegister(REG_BX);
  hardware.Reg_Set(reg_used, 0);
}

void cBaseCPU::Inst_Neg()
{
  int reg_used = FindModifiedRegister(REG_BX);
  hardware.Reg_Set(reg_used, 0-hardware.GetRegister(reg_used));
}

void cBaseCPU::Inst_Square()
{
  int reg_used = FindModifiedRegister(REG_BX);
  hardware.Reg_Set(reg_used,
	   hardware.GetRegister(reg_used) * hardware.GetRegister(reg_used) );
}

void cBaseCPU::Inst_Sqrt()
{
  int reg_used = FindModifiedRegister(REG_BX);
  int value = hardware.GetRegister(reg_used);
  if (value > 1) hardware.Reg_Set(reg_used, (int) sqrt((double) value));
}

void cBaseCPU::Inst_Add()
{
  int reg_used = FindModifiedRegister(REG_BX);
  hardware.Reg_Set(reg_used,
     hardware.GetRegister(REG_BX) + hardware.GetRegister(REG_CX));
}

void cBaseCPU::Inst_Sub()
{
  int reg_used = FindModifiedRegister(REG_BX);
  hardware.Reg_Set(reg_used,
     hardware.GetRegister(REG_BX) - hardware.GetRegister(REG_CX));
}

void cBaseCPU::Inst_Mult()
{
  int reg_used = FindModifiedRegister(REG_BX);
  hardware.Reg_Set(reg_used,
     hardware.GetRegister(REG_BX) * hardware.GetRegister(REG_CX));
}

void cBaseCPU::Inst_Div()
{
  int reg_used = FindModifiedRegister(REG_BX);
  if (hardware.GetRegister(REG_CX) != 0) {
    hardware.Reg_Set(reg_used,
	hardware.GetRegister(REG_BX) / hardware.GetRegister(REG_CX));
  }
}

void cBaseCPU::Inst_Mod()
{
  int reg_used = FindModifiedRegister(REG_BX);
  if (hardware.GetRegister(REG_CX) != 0) {
    hardware.Reg_Set(reg_used,
       hardware.GetRegister(REG_BX) % hardware.GetRegister(REG_CX));
  }
}

void cBaseCPU::Inst_Nand()
{
  int reg_used = FindModifiedRegister(REG_BX);
  hardware.Reg_Set(reg_used,
     ~(hardware.GetRegister(REG_BX) & hardware.GetRegister(REG_CX)));
}

void cBaseCPU::Inst_Nor()
{
  int reg_used = FindModifiedRegister(REG_BX);
  hardware.Reg_Set(reg_used,
      ~(hardware.GetRegister(REG_BX) | hardware.GetRegister(REG_CX)));
}

void cBaseCPU::Inst_And()
{
  int reg_used = FindModifiedRegister(REG_BX);
  hardware.Reg_Set(reg_used,
       (hardware.GetRegister(REG_BX) & hardware.GetRegister(REG_CX)));
}

void cBaseCPU::Inst_Not()
{
  int reg_used = FindModifiedRegister(REG_BX);
  hardware.Reg_Set(reg_used, ~(hardware.GetRegister(reg_used)));
}

void cBaseCPU::Inst_Order()
{
  int temp_reg;

  if (hardware.GetRegister(REG_BX) > hardware.GetRegister(REG_CX)) {
    temp_reg = hardware.GetRegister(REG_CX);
    hardware.Reg_Set(REG_CX, hardware.GetRegister(REG_BX));
    hardware.Reg_Set(REG_BX, temp_reg);
  }
}

void cBaseCPU::Inst_Xor()
{
  int reg_used = FindModifiedRegister(REG_BX);
  hardware.Reg_Set(reg_used,
    hardware.GetRegister(REG_BX) ^ hardware.GetRegister(REG_CX));
}

void cBaseCPU::Inst_Copy()
{
  cCPUHead from(this, hardware.GetRegister(REG_BX));
  cCPUHead to(this,
     hardware.GetRegister(REG_AX) + hardware.GetRegister(REG_BX));

  if (environment->InPopulation() && environment->TestCopyMut()) {
    to.SetInst(hardware.GetRandomInst());
    to.SetFlag(INST_FLAG_MUTATED);  // Mark this instruction as mutated...
    to.SetFlag(INST_FLAG_COPY_MUT); // Mark this instruction as copy mut...
    ++cpu_stats.mut_stats.copy_mut_count;
  } else {
    to.SetInst(from.GetInst());
    to.UnsetFlag(INST_FLAG_MUTATED);  // UnMark
    to.UnsetFlag(INST_FLAG_COPY_MUT); // UnMark
  }

  to.SetFlag(INST_FLAG_COPIED);  // Set the copied flag.
  ++cpu_stats.mut_stats.copies_exec;
}

void cBaseCPU::Inst_ReadInst()
{
  int reg_used = FindModifiedRegister(REG_CX);
  cCPUHead from(this, hardware.GetRegister(REG_BX));

  // Dis-allowing mutations on read, for the moment (write only...)
  hardware.Reg_Set(reg_used, from.GetInst().GetOp());
}

void cBaseCPU::Inst_WriteInst()
{
  cCPUHead to(this,
     hardware.GetRegister(REG_AX) + hardware.GetRegister(REG_BX));
  int reg_used = FindModifiedRegister(REG_CX);
  int value = hardware.GetRegister(reg_used);

  // Change value on a mutation...
  if (value < 0 || value >= hardware.GetNumInst() ||
      (environment->InPopulation() && environment->TestCopyMut())) {
    if (environment->InPopulation() == FALSE) value = 0;
    else value = hardware.GetRandomInst().GetOp();
  }

  to.SetInst(cInstruction(value));
  to.SetFlag(INST_FLAG_COPIED);  // Set the copied flag.
}

void cBaseCPU::Inst_StackReadInst()
{
  int reg_used = FindModifiedRegister(REG_CX);
  cCPUHead from(this, hardware.GetRegister(reg_used));

  hardware.StackPush(from.GetInst().GetOp());
}

void cBaseCPU::Inst_StackWriteInst()
{
  int reg_used = FindModifiedRegister(REG_BX);
  cCPUHead to(this,
     hardware.GetRegister(REG_AX) + hardware.GetRegister(reg_used));
  int value = hardware.StackPop();

  // Change value on a mutation...
  if (value < 0 || value >= hardware.GetNumInst() ||
      (environment->InPopulation() && environment->TestCopyMut())) {
    if (environment->InPopulation() == FALSE) value = 0;
    else value = hardware.GetRandomInst().GetOp();
  }

  to.SetInst(cInstruction(value));
  to.SetFlag(INST_FLAG_COPIED);  // Set the copied flag.
}

void cBaseCPU::Inst_Compare()
{
  int reg_used = FindModifiedRegister(REG_CX);
  cCPUHead from(this, hardware.GetRegister(REG_BX));
  cCPUHead to(this, hardware.GetRegister(REG_AX) + hardware.GetRegister(REG_BX));

  if (environment->TestCopyMut()) {
    from.SetInst(hardware.GetRandomInst());
    to.SetInst(hardware.GetRandomInst());
  }

  hardware.Reg_Set(reg_used,
		   (int) from.GetInst().GetOp() - (int) to.GetInst().GetOp());
}

void cBaseCPU::Inst_IfNCpy()
{
  cCPUHead from(this, hardware.GetRegister(REG_BX));
  cCPUHead to(this,
     hardware.GetRegister(REG_AX) + hardware.GetRegister(REG_BX));

  if (environment->TestCopyMut()) {
    if (from.GetInst() != to.GetInst()) hardware.AdvanceIP();
  } else {
    if (from.GetInst() == to.GetInst()) hardware.AdvanceIP();
  }
}

void cBaseCPU::Inst_Allocate()   // Allocate bx more space...
{
  int size = hardware.GetMemorySize();
  if( Allocate_Main(hardware.GetRegister(REG_BX)) ){
    hardware.Reg_Set(REG_AX, size);
  }
}

void cBaseCPU::Inst_Divide()
{
  Divide_Main(hardware.GetRegister(REG_AX));
}

void cBaseCPU::Inst_CDivide()
{
  Divide_Main(hardware.GetMemorySize() / 2);
}

void cBaseCPU::Inst_CAlloc()   // Double the allocated space.
{
  Allocate_Main(hardware.GetMemorySize());
}


void cBaseCPU::Inst_MaxAlloc()   // Allocate maximal more
{
  const int cur_size = hardware.GetMemorySize();
  const int alloc_size = Min((int) (cConfig::GetChildSizeRange() * cur_size),
			     MAX_CREATURE_SIZE - cur_size);

  if( Allocate_Main(alloc_size) )  hardware.Reg_Set(REG_AX, cur_size);
}

void cBaseCPU::Inst_Bin_Repro(){ // The Binary Inst Set Reproduce...
  assert(false);

  // Set the merit
  // @TCC  ---
  // Stupid method... to the power of size

  static cCodeArray ref_code;
  const int ref_size = 25;
  if( ref_code.GetSize() == 0 ){  // if uninitialized...
    ref_code.Reset( ref_size );
    for( int i=0; i<ref_size; ++i ){
      ref_code.Set(i, cInstruction(GetInstLib().GetInstID("rep-1")));
    }
  }

  const int max_m = 25;
  int m = max_m - GetMemory().FindHammingDistance(ref_code);
  if( m<1 )  m = 1;
  // cMerit bonus = 1;
  // bonus.Bonus(1 << m);
  // phenotype->SetMerit(bonus);  // NEED TO SETUP METHOD @TCC
  Inst_Repro();
}


void cBaseCPU::Inst_Repro()
{
   int child_size = hardware.GetMemorySize();

  // Make sure this divide will produce a viable offspring.
   //if (!Divide_CheckViable(child_size, divide_point)) return;
  // SET EXECUTED & COPIED SIZE
  repro_data.copied_size = child_size;
  repro_data.executed_size = 0;
  for( int i = 0; i<hardware.GetMemorySize(); i++ ){
    if( hardware.GetMemFlag(i, INST_FLAG_EXECUTED) )
      repro_data.executed_size++;
  }

  // Since the divide will now succeed, update the relevant information.
  // Divide_RecordInfo();
  info.num_divides++;
  info.gestation_time = hardware.GetTimeUsed() - info.gestation_start;
  info.gestation_start = hardware.GetTimeUsed();

  // Set up the information to be sent to the new CPU
  //Divide_SetupChild(divide_point);
  // ACTUALLY COPY THE PARENTS CODE ARRAY
  repro_data.child_memory.Reset(hardware.GetMemory());
  repro_data.parent_cpu = this;
  repro_data.parent_genotype = info.active_genotype;
  repro_data.parent_generation = environment->GetGeneration();
  repro_data.parent_merit = phenotype->GetCurMerit();
  repro_data.neutral_metric = info.neutral_metric;
  repro_data.lineage_label = info.lineage_label;
  repro_data.lineage = info.lineage;

  // Perform Mutations...
  if (environment->InPopulation()) { // Skip this if not in population...
    for (int i = 0; i < hardware.GetMemorySize(); i++) {
      if (environment->TestCopyMut()) {
  	repro_data.child_memory[i] = hardware.GetRandomInst();
      }
    }
  }

  // Information for the phenotype
  repro_data.gestation_time = info.gestation_time;
  repro_data.parent_merit_base = phenotype->GetMeritBase();
  repro_data.parent_bonus = phenotype->GetBonus();
  if( repro_data.tasks == NULL ||
      repro_data.tasks->GetSize() != phenotype->GetNumTasks() ){
    if( repro_data.tasks == NULL ) delete repro_data.tasks;
    repro_data.tasks = new tArray<int>(phenotype->GetNumTasks());
  }
  for( int i=0; i<repro_data.tasks->GetSize(); ++i ){
    (*repro_data.tasks)[i] = phenotype->GetTaskCount(i);
  }

  // DON'T NEED TO DO THIS
  // Cut off everything in this CPU's memory past the divide point.
  // hardware.ResizeMemory(divide_point);

  // Find out if there is a divide mutation, and handle it.
  if (environment->InPopulation()) Divide_DoMutations();

  // Find out if an exact copy of the CPU was made.
  Divide_CheckCopyTrue();

  // Re-adjust the phenotype due to the divide
  if (environment->InPopulation()) phenotype->DivideReset(repro_data);
  environment->AdjustTimeSlice();

  // Finally, activate the child!
  environment->ActivateChild(repro_data);
#ifdef INSTRUCTION_COSTS
  // reset first time instruction costs
  for( int i=0; i<num_inst_cost; ++i ){
    inst_ft_cost[i] = hardware.GetInstLib().GetFTCost(i);
  }
#endif
}

// The "inject" instruction has been updated to use the heads-based
// instruction set.  The inject instruction can be used instead of a divide
// command, paired with an allocate.  Note that for an inject to work, one
// needs to have a broad range for sizes allowed to be allocated.
//
// This command will cut out from read-head to write-head.
// It will then look at the template that follows the command and inject it
// into the complement template found in a neighboring organism.

void cBaseCPU::Inst_Inject()
{
  hardware.AdjustHeads();
  const int new_size = hardware.GetHeadPosition(HEAD_READ);
  const int end_position = hardware.GetHeadPosition(HEAD_WRITE);
  const int inject_size = end_position - new_size;

  cBaseCPU * host_cpu = environment->GetFacing();

  const int new_host_size =
    inject_size + (host_cpu ? host_cpu->GetMemorySize() : 0);

  // Make sure the creature will still be above the minimum size,
  // and the the would-be injected creature is below max.  For errors
  // due only to a host, still delete the material to be injected.
  if (inject_size <= 0 || new_size < MIN_CREATURE_SIZE) {
    Fault(FAULT_LOC_INJECT, FAULT_TYPE_ERROR, "inject: size too small");
    return; // (inject fails)
  }
  if (new_host_size > MAX_CREATURE_SIZE) {
    hardware.ResizeMemory(new_size);
    mal_active = false;
    Fault(FAULT_LOC_INJECT, FAULT_TYPE_WARNING, "inject: target too large");
    return; // (inject fails)
  }

  // Find the inject_line

  hardware.ReadLabel();
  hardware.ComplementLabel();

  // If there is no label, abort.
  if (hardware.GetLabelSize() == 0) {
    Fault(FAULT_LOC_INJECT, FAULT_TYPE_ERROR, "inject: label required");
    return; // (inject fails)
  }

  // If this is just a test CPU, this is as far as we have to go.  Resize,
  // and be done with it.
  if (!host_cpu) {
    hardware.ResizeMemory(new_size);
    mal_active = false;
    return; // (inject doesn't happen)
  }

  // Otherwise, inject at the compliment.
  int inject_line = host_cpu->FindFullLabel(hardware.GetLabel()).GetPosition();

  // Cut off injectable code and abort if no compliment is found.
  if (inject_line == -1) {
    hardware.ResizeMemory(new_size);
    mal_active = false;
    Fault(FAULT_LOC_INJECT, FAULT_TYPE_WARNING, "inject: label not found");
    return; // (inject fails)
  }

  // Construct the code to be injected.
  cCodeArray inject_code(inject_size);
  for (int i = 0; i < inject_size; i++) {
    inject_code[i] = hardware.GetMemData(new_size + i);
  }
  hardware.ResizeMemory(new_size);
  mal_active = false;

  // And inject it!
  host_cpu->InjectCode(&inject_code, inject_line+1);

  // Set the relevent flags.
  inst_inject = true;
}

void cBaseCPU::Inst_TaskGet()
{
  int reg_used = FindModifiedRegister(REG_CX);

  int value = hardware.GetInput();
  hardware.Reg_Set(reg_used, value);
  DoInput(value);
}

void cBaseCPU::Inst_TaskStackGet()
{
  int value = hardware.GetInput();
  hardware.StackPush(value);
  DoInput(value);
}

void cBaseCPU::Inst_TaskStackLoad()
{
  for (int i=0; i<IO_SIZE; i++) { hardware.StackPush(hardware.GetInput()); }
}

void cBaseCPU::Inst_TaskPut()
{
  int reg_used = FindModifiedRegister(REG_BX);
  int value = hardware.GetRegister(reg_used);
  hardware.Reg_Set(reg_used, 0);

  // Check for tasks compleated.
  DoOutput(value);
}

void cBaseCPU::Inst_TaskIO()
{
  int reg_used = FindModifiedRegister(REG_BX);

  // Do the "put" component
  int value = hardware.GetRegister(reg_used);
  DoOutput(value);  // Check for tasks compleated.

  // Do the "get" component
  value = hardware.GetInput();
  hardware.Reg_Set(reg_used, value);
  DoInput(value);
}

void cBaseCPU::Inst_SearchF()
{
  hardware.ReadLabel();
  hardware.ComplementLabel();
  int search_size = hardware.FindLabel(1).GetPosition()
                    - hardware.GetIPPosition();
  hardware.Reg_Set(REG_BX, search_size);
  hardware.Reg_Set(REG_CX, hardware.GetLabelSize());
  TestSearchSize(search_size);
}

void cBaseCPU::Inst_SearchB()
{
  hardware.ReadLabel();
  hardware.ComplementLabel();
  int search_size = hardware.GetIPPosition() -
		    hardware.FindLabel(-1).GetPosition();
  hardware.Reg_Set(REG_BX, search_size);
  hardware.Reg_Set(REG_CX, hardware.GetLabelSize());
  TestSearchSize(search_size);
}

void cBaseCPU::Inst_MemSize(){
  hardware.Reg_Set(FindModifiedRegister(REG_BX),
		   hardware.GetMemorySize());
}


void cBaseCPU::Inst_RotateL()
{
   // If this CPU has no one else in the environment, ignore this.
  if (environment->GetNeighborhoodSize() == 0) return;

  hardware.ReadLabel();

  // Mark the original facing, and rotate once.
  cBaseCPU * start_facing = environment->GetFacing();
  environment->RotateL();

  // If there was no label, then the one rotation was all we want.
  if (!hardware.GetLabelSize()) return;

  // Rotate until a complement label is found (or all have been checked).

  hardware.ComplementLabel();
  while (environment->GetFacing() != start_facing) {
    // If this facing has the full label, break here.
    if (environment->GetFacing()->FindFullLabel(hardware.GetLabel()).InMemory()) {
      break;
    }

    // Otherwise keep rotating...
    environment->RotateL();
  }
}

void cBaseCPU::Inst_RotateR()
{
  // If this CPU has no one else in the environment, ignore this.
  if (environment->GetNeighborhoodSize() == 0) return;

  hardware.ReadLabel();

  // Mark the original facing, and rotate once.
  cBaseCPU * start_facing = environment->GetFacing();
  environment->RotateR();

  // If there was no label, then the one rotation was all we want.
  if (!hardware.GetLabelSize()) return;

  // Rotate until a complement label is found (or all have been checked).

  hardware.ComplementLabel();
  while (environment->GetFacing() != start_facing) {
    // If this facing has the full label, break here.
    if (environment->GetFacing()->FindFullLabel(hardware.GetLabel()).InMemory()) {
      break;
    }

    // Otherwise keep rotating...
    environment->RotateR();
  }
}

void cBaseCPU::Inst_SetCopyMut()
{
  int reg_used = FindModifiedRegister(REG_BX);

  int new_mut_rate = hardware.GetRegister(reg_used);
  if (new_mut_rate < 1) new_mut_rate = 1;
  environment->SetCopyMutProb(((double) new_mut_rate) / 10000.0);
}

void cBaseCPU::Inst_ModCopyMut()
{
  int reg_used = FindModifiedRegister(REG_BX);

  double new_mut_rate = environment->GetCopyMutProb() +
    ((double) hardware.GetRegister(reg_used)) / 10000.0;
  if (new_mut_rate > 0.0) environment->SetCopyMutProb(new_mut_rate);
}


// Multi-threading.

void cBaseCPU::Inst_ForkThread()
{
  hardware.AdvanceIP();
  if (!hardware.ForkThread()) Fault(FAULT_LOC_THREAD_FORK, FAULT_TYPE_FORK_TH);
}

void cBaseCPU::Inst_KillThread()
{
  if (!hardware.KillThread()) {
    Fault(FAULT_LOC_THREAD_KILL, FAULT_TYPE_KILL_TH);
  } else {
    advance_ip = false;
  }
}

void cBaseCPU::Inst_ThreadID()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  hardware.Reg_Set(reg_used, hardware.GetCurThreadID());
}

// Head-based instructions

void cBaseCPU::Inst_SetHead()
{
  const int head_used = FindModifiedHead(HEAD_IP);
  hardware.SetActiveHead(head_used);
}

void cBaseCPU::Inst_AdvanceHead()
{
  const int head_used = FindModifiedHead(HEAD_WRITE);
  hardware.AdvanceHead(head_used);
}

void cBaseCPU::Inst_MoveHead()
{
  const int head_used = FindModifiedHead(HEAD_IP);
  hardware.SetHead(hardware.GetHead(HEAD_FLOW), head_used);
  if (head_used == HEAD_IP) advance_ip = false;
}

void cBaseCPU::Inst_JumpHead()
{
  const int head_used = FindModifiedHead(HEAD_IP);
  //  int old_pos = hardware.GetHead(head_used).GetPosition();
  hardware.JumpHead(hardware.GetRegister(REG_CX), head_used);
  //  hardware.Reg_Set(REG_CX, old_pos);
}

void cBaseCPU::Inst_GetHead()
{
  const int head_used = FindModifiedHead(HEAD_IP);
  hardware.Reg_Set(REG_CX, hardware.GetHead(head_used).GetPosition());
}

void cBaseCPU::Inst_IfLabel()
{
  hardware.ReadLabel();
  hardware.ComplementLabel();
  if (hardware.GetLabel() != hardware.GetReadLabel()) {
    hardware.AdvanceIP();
  }
}

void cBaseCPU::Inst_HeadDivide()
{
  hardware.AdjustHeads();
  const int divide_pos = hardware.GetHeadPosition(HEAD_READ);
  int child_end =  hardware.GetHeadPosition(HEAD_WRITE);
  if (child_end == 0) child_end = hardware.GetMemorySize();
  const int extra_lines = hardware.GetMemorySize() - child_end;
  Divide_Main(divide_pos, extra_lines);

  // Re-adjust heads.
  hardware.AdjustHeads();
}

void cBaseCPU::Inst_HeadRead()
{
  const int head_id = FindModifiedHead(HEAD_READ);
  hardware.AdjustHead(head_id);

  // Mutations only occur on the read, for the moment.
  int read_inst = 0;
  if (environment->InPopulation() && environment->TestCopyMut()) {
    read_inst = hardware.GetRandomInst().GetOp();
    ++cpu_stats.mut_stats.copy_mut_count;  // @CAO, hope this is good!
  } else {
    read_inst = hardware.GetCurInst(head_id).GetOp();
  }
  hardware.Reg_Set(REG_BX, read_inst);
  hardware.ReadInst(read_inst);

  ++cpu_stats.mut_stats.copies_exec;  // @CAO, this too..
  hardware.AdvanceHead(head_id);
}

void cBaseCPU::Inst_HeadWrite()
{
  const int head_id = FindModifiedHead(HEAD_WRITE);
  hardware.AdjustHead(head_id);

  int value = hardware.GetRegister(REG_BX);
  if (value < 0 || value >= hardware.GetNumInst()) {
    if (environment->InPopulation() == FALSE) value = 0;
    else value = hardware.GetRandomInst().GetOp();
  }

  hardware.SetCurInst(cInstruction(value), head_id);
  hardware.SetHeadFlag(INST_FLAG_COPIED, head_id);  // Set the copied flag.

  hardware.AdvanceHead(head_id);
}

void cBaseCPU::Inst_HeadCopy()
{
  // For the moment, this cannot be nop-modified.
  const int read_head = HEAD_READ; //FindModifiedHead(HEAD_READ);
  const int write_head = HEAD_WRITE; //(read_head + 1) % NUM_HEADS;
  hardware.AdjustHead(read_head);
  hardware.AdjustHead(write_head);

  // Do mutations.
  bool mut = false;
  cInstruction read_inst = hardware.GetCurInst(read_head);
  if (environment->InPopulation() && environment->TestCopyMut()) {
    read_inst = hardware.GetRandomInst();
    ++cpu_stats.mut_stats.copy_mut_count;  // @CAO, hope this is good!
    mut = true;
  }
  hardware.ReadInst(read_inst.GetOp());

  ++cpu_stats.mut_stats.copies_exec;  // @CAO, this too..

  hardware.SetCurInst(read_inst, write_head);
  hardware.SetHeadFlag(INST_FLAG_COPIED, write_head);  // Set the copied flag.
  if (mut == true) {
    // Mark instruction as mutated with a copy mutation.
    hardware.SetHeadFlag(INST_FLAG_MUTATED, write_head);
    hardware.SetHeadFlag(INST_FLAG_COPY_MUT, write_head);
  }

  hardware.AdvanceHead(read_head);
  hardware.AdvanceHead(write_head);
}

void cBaseCPU::Inst_HeadSearch()
{
  hardware.ReadLabel();
  hardware.ComplementLabel();
  cCPUHead found_pos = hardware.FindLabel(0);
  int search_size = found_pos.GetPosition() - hardware.GetIPPosition();
  hardware.Reg_Set(REG_BX, search_size);
  hardware.Reg_Set(REG_CX, hardware.GetLabelSize());
  hardware.SetHead(found_pos, HEAD_FLOW);
  hardware.AdvanceHead(HEAD_FLOW);
  TestSearchSize(search_size);
}

void cBaseCPU::Inst_SetFlow()
{
  int reg_used = FindModifiedRegister(REG_CX);
  hardware.SetHead(hardware.GetRegister(reg_used), this, HEAD_FLOW);
}

// Direct Matching Templates

void cBaseCPU::Inst_DMJumpF(){
  cCPUHead jump_location;
  hardware.ReadLabel();
  // If there is no label, jump BX steps.
  if (hardware.GetLabelSize() == 0) {
    hardware.JumpIP(hardware.GetRegister(REG_BX));
  }
  // Otherwise, jump to the label.
  else if ((jump_location = hardware.FindLabel(1)).GetPosition() != -1) {
    hardware.SetIP(jump_location);
  }
}

void cBaseCPU::Inst_DMJumpB()
{
  cCPUHead jump_location;
  hardware.ReadLabel();
  // If there is no label, jump BX steps.
  if (hardware.GetLabelSize() == 0) {
    hardware.JumpIP(-(hardware.GetRegister(REG_BX)));
  }
  // otherwise jump to the label.
  else if ((jump_location = hardware.FindLabel(-1)).GetPosition() != -1) {
    hardware.SetIP(jump_location);
  }
}

void cBaseCPU::Inst_DMCall()
{
  cCPUHead jump_location;

  // Put the starting location onto the stack
  int location = hardware.GetInstPointer().GetPosition();
  hardware.StackPush(location);

  // Jump to the label (or by the ammount in the bx register)
  hardware.ReadLabel();

  if (hardware.GetLabelSize() == 0) {
    hardware.JumpIP(hardware.GetRegister(REG_BX));
  }
  else if ((jump_location = hardware.FindLabel(1)).GetPosition() != -1) {
    hardware.SetIP(jump_location);
  }
  TestSearchSize(location);
}

void cBaseCPU::Inst_DMSearchF()
{
  hardware.ReadLabel();
  int search_size = hardware.FindLabel(1).GetPosition()
		    - hardware.GetIPPosition();
  hardware.Reg_Set(REG_BX, search_size);
  hardware.Reg_Set(REG_CX, hardware.GetLabelSize());
  TestSearchSize(search_size);
}

void cBaseCPU::Inst_DMSearchB() {
  hardware.ReadLabel();
  int search_size = hardware.GetIPPosition() -
		    hardware.FindLabel(-1).GetPosition();
  hardware.Reg_Set(REG_BX, search_size);
  hardware.Reg_Set(REG_CX, hardware.GetLabelSize());
  TestSearchSize(search_size);
}


// Relavie Addressed Instructions

void cBaseCPU::Inst_REJumpF(){ // DEFAULT REG_CX
  hardware.JumpIP(FindModifiedRegister(REG_CX));
}

void cBaseCPU::Inst_REJumpB(){ // DEFAULT REG_CX
  hardware.JumpIP(-(hardware.GetRegister(
      FindModifiedRegister(REG_CX)) +1 ));
}


// Absolute Addressed Instructions

void cBaseCPU::Inst_ABSJump(){
  hardware.SetIP(FindModifiedRegister(REG_BX) + 1);
}



// --- Biologically oriented allocate, copy, and divide ---

void cBaseCPU::Inst_BCAlloc(){
  // Bio - Const size - Allocate

  // must do divide before second mal
  if( mal_active == true ) {
    Fault(FAULT_LOC_ALLOC, FAULT_TYPE_ERROR);
    return;
  }

  int memory_size = hardware.GetMemorySize();

  mal_active = true;
  bio_mal_active = true;

  cBaseCPU * neighbor = environment->GetNeighbor();
  int neighbor_size = (neighbor != NULL) ? neighbor->GetMemorySize() : 0;

  hardware.ResetChildMemory(hardware.GetMemorySize());

  // If we don't have a neighbor, initialize to zero...
  if( neighbor_size == 0 ||
      cConfig::GetAllocMethod() == ALLOC_METHOD_DEFAULT ||
      cConfig::GetAllocMethod() == ALLOC_METHOD_RANDOM ){
  }else{
    // Otherwise initialize using this neighbors memory (ALLOC_METHOD_NECRO)
    int offset = g_random.GetUInt(neighbor_size);
    for (int i = 0; i < memory_size; i++) {
      hardware.SetChildMemory(i,
		 neighbor->GetMemory().Get((i + offset) % neighbor_size));
    }
  }
}


void cBaseCPU::Inst_BCopy() {
  Inst_BCopy_Main( environment->TestCopyMut() );
}


void cBaseCPU::Inst_BCopyDiv2() {
  Inst_BCopy_Main( g_random.P(environment->GetCopyMutProb() / 2) );
}

void cBaseCPU::Inst_BCopyDiv3() {
  Inst_BCopy_Main( g_random.P(environment->GetCopyMutProb() / 3) );
}

void cBaseCPU::Inst_BCopyDiv4() {
  Inst_BCopy_Main( g_random.P(environment->GetCopyMutProb() / 4) );
}

void cBaseCPU::Inst_BCopyDiv5() {
  Inst_BCopy_Main( g_random.P(environment->GetCopyMutProb() / 5) );
}

void cBaseCPU::Inst_BCopyDiv6() {
  Inst_BCopy_Main( g_random.P(environment->GetCopyMutProb() / 6) );
}

void cBaseCPU::Inst_BCopyDiv7() {
  Inst_BCopy_Main( g_random.P(environment->GetCopyMutProb() / 7) );
}

void cBaseCPU::Inst_BCopyDiv8() {
  Inst_BCopy_Main( g_random.P(environment->GetCopyMutProb() / 8) );
}

void cBaseCPU::Inst_BCopyDiv9() {
  Inst_BCopy_Main( g_random.P(environment->GetCopyMutProb() / 9) );
}

void cBaseCPU::Inst_BCopyDiv10() {
  Inst_BCopy_Main( g_random.P(environment->GetCopyMutProb() / 10) );
}


void cBaseCPU::Inst_BCopyPow2() {
  Inst_BCopy_Main( g_random.P(environment->GetCopyMutProb() *
			      environment->GetCopyMutProb()) );
}


void cBaseCPU::Inst_BCopy_Main(double mut_rate) {
  // Verify a good Bio Allocate
  if (bio_mal_active == false ||
      hardware.GetChildMemorySize() < MIN_CREATURE_SIZE ){
    Fault(FAULT_LOC_BCOPY, FAULT_TYPE_ERROR);
    return;
  }
  int pos = hardware.GetRegister(REG_BX);
  int child_pos = pos % hardware.GetChildMemorySize();
  int child_size = hardware.GetChildMemorySize();
  // Make child position in range
  while( child_pos < 0 ){ child_pos += child_size; }
  while( child_pos > child_size ){ child_pos -= child_size; }
  assert( child_pos >= 0 && child_pos < child_size );

  cCPUHead from(this,pos);

  if( environment->InPopulation() && g_random.P(mut_rate) ){
    hardware.SetChildMemory(child_pos, hardware.GetRandomInst());
    hardware.SetChildMemFlag(child_pos,INST_FLAG_MUTATED); // Mark as mutated.
    hardware.SetChildMemFlag(child_pos,INST_FLAG_COPY_MUT); // Mark as copy mut
    ++cpu_stats.mut_stats.copy_mut_count;
  } else {
    hardware.SetChildMemory(child_pos, from.GetInst());
    hardware.UnsetChildMemFlag(child_pos,INST_FLAG_MUTATED); // UnMark
    hardware.UnsetChildMemFlag(child_pos,INST_FLAG_COPY_MUT); // UnMark
  }
  hardware.SetChildMemFlag(child_pos, INST_FLAG_COPIED);  // Set copied flag.
  ++cpu_stats.mut_stats.copies_exec;
}

void cBaseCPU::Inst_BDivide() {
  // Make sure this divide will produce a viable offspring.
  // Make sure an allocate has occured.
  if( bio_mal_active == false){
    Fault(FAULT_LOC_DIVIDE, FAULT_TYPE_ERROR);
    return; //  (divide fails)
  }

  int child_size = hardware.GetChildMemorySize();
  int parent_size = hardware.GetMemorySize();

  if( child_size < MIN_CREATURE_SIZE || child_size > MAX_CREATURE_SIZE ||
      child_size  > cConfig::GetChildSizeRange() * parent_size ||
      parent_size > cConfig::GetChildSizeRange() * child_size) {
    Fault(FAULT_LOC_DIVIDE, FAULT_TYPE_ERROR);
    g_debug.Comment("BDiv: Viable Failed (sizecheck): child_size=%d, parent_size=%d", child_size, parent_size);
    return; // (divide fails)
  }
  // Count the number of lines executed in the parent.
  repro_data.executed_size =
    hardware.GetMemory().CountFlag(INST_FLAG_EXECUTED);
  // If less than 50% the lines in the parent were executed, divide fails...
  if (repro_data.executed_size <= parent_size * cConfig::GetMinExeLines()) {
    Fault(FAULT_LOC_DIVIDE, FAULT_TYPE_ERROR);
    g_debug.Comment("BDiv: Viable Failed (exec_size): exec_size=%d, parnet_size=%d", repro_data.executed_size, parent_size);
    return; // (divide fails)
  }
  // Count the number of lines which were copied into the child.
  repro_data.copied_size =
    hardware.GetChildMemory().CountFlag(INST_FLAG_COPIED);
  // If less than half the lines were copied into the child, divide fails...
  if (repro_data.copied_size <= child_size * cConfig::GetMinCopiedLines()) {
    Fault(FAULT_LOC_DIVIDE, FAULT_TYPE_ERROR);
    g_debug.Comment("BDiv: Viable Failed (copy_size): copy_size=%d, parnet_size=%d", repro_data.copied_size, parent_size);
    return; // (divide fails)
  }
	
  // Since the divide will now succeed, set up the information to be sent
  // to the new CPU
  // Determine the gestation time of the child.
  info.gestation_time = hardware.GetTimeUsed() - info.gestation_start;
  info.gestation_start = hardware.GetTimeUsed();
  repro_data.child_memory.Reset(hardware.GetChildMemory());
  repro_data.parent_cpu = this;
  repro_data.parent_genotype = info.active_genotype;
  repro_data.parent_generation = environment->GetGeneration();
  repro_data.parent_merit = phenotype->GetCurMerit();
  repro_data.neutral_metric = info.neutral_metric;
  repro_data.lineage_label = info.lineage_label;
  repro_data.lineage = info.lineage;

  // Information for the phenotype
  repro_data.gestation_time = info.gestation_time;
  repro_data.parent_merit_base = phenotype->GetMeritBase();
  repro_data.parent_bonus = phenotype->GetBonus();
  if( repro_data.tasks == NULL ||
      repro_data.tasks->GetSize() != phenotype->GetNumTasks() ){
    if( repro_data.tasks == NULL ) delete repro_data.tasks;
    repro_data.tasks = new tArray<int>(phenotype->GetNumTasks());
  }
  for( int i=0; i<repro_data.tasks->GetSize(); ++i ){
    (*repro_data.tasks)[i] = phenotype->GetTaskCount(i);
  }

  // Find out if there is a divide mutation, and handle it.
  if (environment->InPopulation()) Divide_DoMutations();

  // Find out if an exact copy of the CPU was made.
  Divide_CheckCopyTrue();

  // Collect all of the stats about the divide.
  Divide_RecordInfo();

  // Re-adjust the phenotype due to the divide
  if (environment->InPopulation()) phenotype->DivideReset(repro_data);
  environment->AdjustTimeSlice();

  // Unset Flags
  mal_active = false;
  bio_mal_active = false;

  // activate the child!
  environment->ActivateChild(repro_data);
#ifdef INSTRUCTION_COSTS
  // reset first time instruction costs
  for( int i=0; i<num_inst_cost; ++i ){
    inst_ft_cost[i] = hardware.GetInstLib().GetFTCost(i);
  }
#endif
}

// Bio Error Correcting
void cBaseCPU::Inst_BIfNotCopy() {
  // Verify a good Bio Allocate
  if (bio_mal_active == false ||
      hardware.GetChildMemorySize() < MIN_CREATURE_SIZE ){
    Fault(FAULT_LOC_BCOPY, FAULT_TYPE_ERROR);
    return;
  }
  int pos = hardware.GetRegister(REG_BX);
  int child_pos = pos % hardware.GetChildMemorySize();
  int child_size = hardware.GetChildMemorySize();
  // Make child position in range
  while( child_pos < 0 ){ child_pos += child_size; }
  while( child_pos > child_size ){ child_pos -= child_size; }
  assert( child_pos >= 0 && child_pos < child_size );

  cCPUHead from(this,pos);

  if( environment->TestCopyMut() ){
    if( from.GetInst() != hardware.GetChildInst(child_pos) )
      hardware.AdvanceIP();
  } else {
    if( from.GetInst() == hardware.GetChildInst(child_pos) )
      hardware.AdvanceIP();
  }
}

void cBaseCPU::Inst_BIfCopy() {
  // Verify a good Bio Allocate
  if (bio_mal_active == false ||
      hardware.GetChildMemorySize() < MIN_CREATURE_SIZE ){
    Fault(FAULT_LOC_BCOPY, FAULT_TYPE_ERROR);
    hardware.AdvanceIP();
    return;
  }

  int pos = hardware.GetRegister(REG_BX);
  int child_pos = pos % hardware.GetChildMemorySize();
  int child_size = hardware.GetChildMemorySize();
  // Make child position in range
  while( child_pos < 0 ){ child_pos += child_size; }
  while( child_pos > child_size ){ child_pos -= child_size; }
  assert( child_pos >= 0 && child_pos < child_size );

  cCPUHead from(this,pos);

  if( environment->TestCopyMut() ){
    if( from.GetInst() == hardware.GetChildInst(child_pos) )
      hardware.AdvanceIP();
  } else {
    if( from.GetInst() != hardware.GetChildInst(child_pos) )
      hardware.AdvanceIP();
  }
}


//// Placebo insts ////
void cBaseCPU::Inst_Skip() {
  hardware.AdvanceIP();
}


//       -=-=  Instructions w/ Arguments  =-=-

void cBaseCPU::InstArg_Add()
{
  LoadArgs();

  WriteArg(1, ReadArgValue(0) + ReadArgValue(1));
}

void cBaseCPU::InstArg_And()
{
  LoadArgs();

  WriteArg( 1, ReadArgValue(0) & ReadArgValue(1) );
}

void cBaseCPU::InstArg_Nand()
{
  LoadArgs();

  WriteArg(1, ~( ReadArgValue(0) & ReadArgValue(1) ));
}

void cBaseCPU::InstArg_IfEqu()
{
  LoadArgs();

  if (ReadArgValue(0) != ReadArgValue(1)) hardware.AdvanceIP();
}

void cBaseCPU::InstArg_IfLess()
{
  LoadArgs();

  if (ReadArgValue(0) >= ReadArgValue(1)) hardware.AdvanceIP();
}

void cBaseCPU::InstArg_Copy()
{
  LoadArgs();

  WriteArg(1, ReadArg(0));
}

void cBaseCPU::InstArg_Swap()
{
  LoadArgs();

  cInstruction out_val0 = ReadArg(0);
  cInstruction out_val1 = ReadArg(1);
  WriteArg(0, out_val1);
  WriteArg(1, out_val0);
}

void cBaseCPU::InstArg_Shift()
{
  LoadArgs();

  int val0 = ReadArgValue(0);
  int val1 = ReadArgValue(1);
  WriteArg(0, val0 << val1);
}

void cBaseCPU::InstArg_Nop()
{
  LoadArgs();
}

void cBaseCPU::InstArg_Allocate()
{
  LoadArgs();

  Allocate_Main(ReadArgValue(0));
}

void cBaseCPU::InstArg_Divide()
{
  LoadArgs();
  Divide_Main(ReadArgValue(0));
}

void cBaseCPU::InstArg_Rotate()
{
  LoadArgs();

  int num_neighbors = environment->GetNeighborhoodSize();
  int rot_val = ReadArgValue(0);

  if (num_neighbors == 0) return;

  rot_val %= num_neighbors;

  for (int i = 0; i < rot_val; i++) environment->RotateR();
  for (int j = 0; j > rot_val; j--) environment->RotateL();
}

void cBaseCPU::InstArg_Label()
{
  LoadArgs();
}


//  -= Redcode Instructions =-

void cBaseCPU::InstRed_Nop()
{
  LoadRedcodeArgs();
}

void cBaseCPU::InstRed_Data()
{
  LoadRedcodeArgs();
  if (!hardware.KillThread()) Fault(FAULT_LOC_THREAD_KILL, FAULT_TYPE_WARNING);
}

void cBaseCPU::InstRed_Move()
{
  LoadRedcodeArgs();

  cInstruction out_inst = ReadRedcodeArg(0);

  if (environment->InPopulation() && injected == false &&
      environment->TestCopyMut()) {
    // Lets do a full mutations of this instruction. (this is how I
    // *think* Steen does them in Venus...
    // @CAO in the future, we should do more intelligent mutations...
    if (out_inst == cInstruction::GetInstNone()) {
      out_inst.SetArgValue(0, hardware.GetRandomInst().GetArg(0).GetValue());
    } else {
      out_inst = hardware.GetRandomInst();
    }
  }

  WriteRedcodeArg(1, out_inst);
}

void cBaseCPU::InstRed_Add()
{
  LoadRedcodeArgs();

  WriteRedcodeArg(1, ReadRedcodeArgValue(0) + ReadRedcodeArgValue(1));
}

void cBaseCPU::InstRed_Sub()
{
  LoadRedcodeArgs();

  WriteRedcodeArg(1, ReadRedcodeArgValue(0) - ReadRedcodeArgValue(1));
}

void cBaseCPU::InstRed_Nand()
{
  LoadRedcodeArgs();

  WriteRedcodeArg(1, ~(ReadRedcodeArgValue(0) & ReadRedcodeArgValue(1)));
}

void cBaseCPU::InstRed_Jump()
{
  LoadRedcodeArgs();
  hardware.AbsJumpIP(ReadRedcodeArgValue(0) - 1);
}

void cBaseCPU::InstRed_JumpZero()
{
  LoadRedcodeArgs();
  if (ReadRedcodeArgValue(1) == 0) hardware.JumpIP(ReadRedcodeArgValue(0) - 1);
}

void cBaseCPU::InstRed_JumpNZero()
{
  LoadRedcodeArgs();
  if (ReadRedcodeArgValue(1) != 0) hardware.JumpIP(ReadRedcodeArgValue(0) - 1);
}

void cBaseCPU::InstRed_DecJumpNZero()
{
  LoadRedcodeArgs();
  cInstruction inst_b = ReadRedcodeArg(1);
  inst_b.SetArgValue(0, inst_b.GetArg(0).GetValue() - 1);
  if (ReadRedcodeArgValue(1) != 0) hardware.JumpIP(ReadRedcodeArgValue(0) - 1);
}

void cBaseCPU::InstRed_Compare()
{
  LoadRedcodeArgs();
  if (ReadRedcodeArgValue(0) == ReadRedcodeArgValue(1)) hardware.AdvanceIP();
}

void cBaseCPU::InstRed_Split()
{
  LoadRedcodeArgs();
  if (!hardware.ForkThread()) Fault(FAULT_LOC_THREAD_FORK, FAULT_TYPE_WARNING);
  hardware.JumpIP(ReadRedcodeArgValue(0));
}

void cBaseCPU::InstRed_SkipLess()
{
  LoadRedcodeArgs();
  if (ReadRedcodeArgValue(0) < ReadRedcodeArgValue(1)) hardware.AdvanceIP();
}

void cBaseCPU::InstRed_Allocate()
{
  LoadRedcodeArgs();

  Allocate_Main(ReadRedcodeArgValue(0));
}

void cBaseCPU::InstRed_Divide()
{
  LoadRedcodeArgs();

  Divide_Main(ReadRedcodeArgValue(0));
}

void cBaseCPU::InstRed_Get()
{
  int value = hardware.GetInput();
  WriteRedcodeArg(0, value);
  DoInput(value);
}

void cBaseCPU::InstRed_Put()
{
  int value = ReadRedcodeArgValue(0);
  WriteRedcodeArg(0, 0);

  // Check for tasks compleated.
  DoOutput(value);
}


// This method notifies the viewer _before_ it executes each instruction.
void cBaseCPU::Notify()
{
  hardware.AdjustIP();
  environment->Notify();

  // And execute it.
  const cInstruction & cur_inst = hardware.GetCurInst();
  tCPUMethod inst_ptr = hardware.GetInstLib().GetFunction(cur_inst);

  if (cur_inst.GetOp() >= hardware.GetInstLib().GetSize()) {
    Fault (FAULT_LOC_INSTRUCTION, FAULT_TYPE_ERROR);
    inst_ptr = hardware.GetInstLib().
      GetActiveFunction(cInstruction::GetInstDefault());
  }

  (this->*(inst_ptr))();
}


cCodeArray cBaseCPU::LoadCode(const char * filename, const cInstLib & inst_lib)
{
  cInitFile input_file(filename);
  if (!input_file.IsOpen()) {
    cString error_str("Cannot load file: ");
    error_str += filename;
    fprintf(stderr, "%s\n", error_str()); // environment->NotifyError(error_str);
    return cCodeArray(0);
  }
  input_file.Load();
  input_file.Compress();
  input_file.Close();

  // Setup the code array...
  cCodeArray in_code(input_file.GetNumLines());

  for (int line_num = 0; line_num < in_code.GetSize(); line_num++) {
    cString cur_line = input_file.GetLine();
    in_code[line_num] = inst_lib.GetInst(cur_line);

    if (in_code[line_num] == cInstruction::GetInstError()) {
      // You're using the wrong instruction set!  YOU FOOL!
      cString error_str("Cannot load creature \"");
      error_str += filename;
      error_str += "\"\n       Unknown line: ";
      error_str += input_file.GetLine()();
      error_str += "       ";

      fprintf(stderr, "%s\n", error_str()); // environment->NotifyError(error_str);
    }
    input_file.RemoveLine();
  }

  return in_code;
}

cGenotype * cBaseCPU::LoadCode(const char * filename)
{
#ifdef DEBUG
  if (environment->InPopulation() == FALSE) {
    g_debug.Warning("Trying to LoadCode into a test CPU.");
    return NULL;
  }
#endif

  return LoadCode( LoadCode (filename, hardware.GetInstLib() ) );
}


/*
cGenotype * cBaseCPU::LoadCode(const char * filename)
{
#ifdef DEBUG
  if (environment->InPopulation() == FALSE) {
    g_debug.Warning("Trying to LoadCode into a test CPU.");
    return NULL;
  }
#endif

  cInitFile input_file(filename);
  if (!input_file.IsOpen()) {
    cString error_str("Cannot load file: ");
    error_str += filename;
    environment->NotifyError(error_str);
    return NULL;
  }
  input_file.Load();
  input_file.Compress();
  input_file.Close();

  // Setup the code array...
  const cInstLib & inst_lib = hardware.GetInstLib();

  cCodeArray in_code(input_file.GetNumLines());

  for (int line_num = 0; line_num < in_code.GetSize(); line_num++) {
    cString cur_line = input_file.GetLine();
    in_code[line_num] = inst_lib.GetInst(cur_line);

    if (in_code[line_num] == cInstruction::GetInstError()) {
      // You're using the wrong instruction set!  YOU FOOL!
      cString error_str("Cannot load creature \"");
      error_str += filename;
      error_str += "\"\n       Unknown line: ";
      error_str += input_file.GetLine()();
      error_str += "       ";

      environment->NotifyError(error_str);
    }
    input_file.RemoveLine();
  }

  return LoadCode(in_code);
}

*/


cGenotype * cBaseCPU::LoadCode(const cCodeArray & in_code)
{
  cCPUTestInfo test_info;
  cTestCPU::TestCode(test_info, in_code);

  cGenotype * new_genotype = environment->AddGenotype(in_code);
  ChangeGenotype(new_genotype);


  SetParentTrue(    test_info.GetGenotypeCopyTrue()  );
  SetCopiedSize(    test_info.GetGenotypeCopiedSize());
  SetExecutedSize(  test_info.GetGenotypeExeSize()   );
  SetGestationTime( test_info.GetGenotypeGestation() );
  SetFitness(       test_info.GetGenotypeFitness()   );

  // @TCC -- This is setting merit based on TEST? Bad...
  //int merit_size = cPhenotype::CalcSizeMerit(in_code.GetSize(),
	 //(int) test_info.CalcCopiedSize(), (int) test_info.CalcExeSize());
  //phenotype->Clear(merit_size, repro_data.parent_merit);

  // Change these to something more meaningful??? @CAO
  GetEnvironment()->SetGeneration(0);
  SetNeutralMetric(0);
  SetLineageLabel(-1);
  SetLineage(0);

  // Initialize the time-slice for this new creature.
  environment->AdjustTimeSlice();



  // Update stats for the cpu...
  SetParentTrue(TRUE);
  SetCopiedSize(info.active_genotype->GetLength());

  // Initialize the phenotype.
  phenotype->Clear(info.active_genotype->GetLength());

  injected = true;

  return new_genotype;
}

void cBaseCPU::ChangeGenotype(cGenotype * in_genotype)
{
  // If we only have a test CPU, make this quick...
  if (environment->InPopulation() == FALSE) {
    ResetVariables();
    if (in_genotype != NULL) {
      hardware.Memory_CopyData(in_genotype->GetCode());
    } else {
      hardware.ResizeMemory(0);
    }
    info.active_genotype = in_genotype;
    return;
  }

  // If this CPU already had a genotype, handle its removal.
  if (info.active_genotype) {
    environment->Stats_RemoveCreature(info.num_divides, info.age);

    // if this creature was a parasite, let the genotype know.
    if (parasite == true) info.active_genotype->AddParasite();

    // If we are actually changeing genotypes, adjust the cpu counts.
    if (in_genotype != info.active_genotype) {
      info.active_genotype->RemoveCPU();

      // Adjust the genebank for this genotype.  If it has been removed,
      // make sure to destroy our reference to it.
      if (!environment->AdjustGenotype(*(info.active_genotype))) {
	info.active_genotype = NULL;
      }
    }
  }

  // Now that we have cleaned up all the statistics to do with the old
  // genotype, reset the CPU fpr the new one....
  ResetVariables();

  // If we are actually adding in a new genotype, set it up.
  if (in_genotype) {
    hardware.Memory_CopyData(in_genotype->GetCode());
    if (in_genotype != info.active_genotype) {
      in_genotype->AddCPU();
      info.active_genotype = in_genotype;
      environment->AdjustGenotype(*in_genotype);
    } else {
      in_genotype->SwapCPU();
    }
    environment->Stats_AddCreature(in_genotype->GetID());

    switch (cConfig::GetDeathMethod()) {
    case DEATH_METHOD_OFF:
      info.max_executed = -1;
      break;
    case DEATH_METHOD_CONST:
      info.max_executed = cConfig::GetAgeLimit();
      break;
    case DEATH_METHOD_MULTIPLE:
      info.max_executed = cConfig::GetAgeLimit() * in_genotype->GetLength();
      break;
    }

    cpu_stats.genotype_id = GetActiveGenotype()->GetID();
  }

  // If we are not adding in a new genotype, make the CPU empty.
  else {
    info.active_genotype = NULL;
    hardware.ResetMemory(0);
    environment->Stats_AddCreature(-1);
    cpu_stats.genotype_id = -1;
  }

  cStats::ReportNewCreature(cpu_stats);
}


int cBaseCPU::OK()
{
  int result = TRUE;

  // First Check all of the environment and hardware.

  if (!environment->OK()) result = FALSE;
  if (!hardware.OK()) result = FALSE;

  if (hardware.GetInstPointer().GetMainCPU() != this) {
    result = FALSE;
    g_debug.Warning("Inst pointer for CPU %d does not identify proper owner",
		    environment->GetID());
  }

  // Some quick info checks...

  if (info.num_divides < 0 || info.num_errors < 0) {
    g_debug.Warning("Execution structure corrupt in CPU");
    result = FALSE;
  }

  return result;
}

void cBaseCPU::ActivateChild()
{
#ifdef DEBUG
  if (environment->InPopulation()) {
    g_debug.Warning("Trying to ActivateChild() on a non-test CPU!");
    return;
  }
#endif

  // @CAO should give the option to make the child active in this CPU, and
  // run it for a gestation cycle.

  SetParentTrue(repro_data.copy_true);
}

void cBaseCPU::Kill()   // Prepare creature for death (stats & such)
{
  cStats::ReportDeath(cpu_stats);
  environment->Stats_AddDeath();
}

void cBaseCPU::ResetVariables()
{
  hardware.Reset(this);
  environment->CPUReset();
  if (environment->InPopulation() == FALSE) hardware.SetupTestInputs();
  else hardware.RandomizeInputs();

#ifdef INSTRUCTION_COSTS
  // reset first time instruction costs
  for( int i=0; i<num_inst_cost; ++i ){
    inst_ft_cost[i] = hardware.GetInstLib().GetFTCost(i);
  }
#endif

  // Info...

  info.copied_size = 0;
  info.executed_size = 0;

  // Info...

  info.num_divides = 0;
  info.num_errors = 0;
  info.gestation_start = 0;
  info.age = 0;
  info.gestation_time = 0;
  info.fitness = 0.0;

  mal_active = false;
  bio_mal_active = false;
  full_trace = false;
  injected = false;
  parasite = false;
  inst_inject = false;
  modified = false;
  point_mut = false;
  search_approx_size = false;
  fertile = true;

  // Repro data...
  repro_data.copy_true = FALSE;

}

int cBaseCPU::Allocate_Main(const int allocated_size)
{
  int old_memory_size = hardware.GetMemorySize();
  int new_memory_size = old_memory_size + allocated_size;

  // must do divide before second allocate & must allocate positive amount...
  if (mal_active == true) {
    Fault(FAULT_LOC_ALLOC, FAULT_TYPE_ERROR, "allocate already active");
    return FALSE;
  }
  if (allocated_size < 1) {
    Fault(FAULT_LOC_ALLOC, FAULT_TYPE_ERROR, "must allocate > 0");
    return FALSE;
  }

  // Make sure that the new size is in range.
  if (new_memory_size > MAX_CREATURE_SIZE   ||
      new_memory_size < MIN_CREATURE_SIZE   ||
      allocated_size > old_memory_size * cConfig::GetChildSizeRange()  ||
      old_memory_size > allocated_size * cConfig::GetChildSizeRange()) {
    Fault(FAULT_LOC_ALLOC, FAULT_TYPE_ERROR, "Invalid allocate size");
    return FALSE;
  }

  hardware.ResizeMemory(new_memory_size);
  mal_active = true;

  cBaseCPU * neighbor = environment->GetNeighbor();
  int neighbor_size = (neighbor != NULL) ? neighbor->GetMemorySize() : 0;

  // If we don't have a neighbor, initialize to zero...
  if( neighbor_size != 0 && cConfig::GetAllocMethod() == ALLOC_METHOD_NECRO ){
    int offset = g_random.GetUInt(neighbor_size);
    for (int i = old_memory_size; i < new_memory_size; i++) {
      hardware.SetMemory(i,
		 neighbor->GetMemory().Get((i + offset) % neighbor_size));
    }
  }else{
    if( cConfig::GetAllocMethod() == ALLOC_METHOD_RANDOM ){
      for (int i = old_memory_size; i < new_memory_size; i++) {
	hardware.SetMemory(i, hardware.GetInstLib().GetRandomInst());
      }
    }else{ // Assume (ALLOC_METHOD_DEFAULT)
      for (int i = old_memory_size; i < new_memory_size; i++) {
	hardware.SetMemory(i, cInstruction::GetInstDefault());
      }
    }
  }

  return TRUE;
}

void cBaseCPU::Divide_Main(const int divide_point, const int extra_lines)
{
  const int child_size = hardware.GetMemorySize() - divide_point - extra_lines;

  // Make sure this divide will produce a viable offspring.
  if (!Divide_CheckViable(child_size, divide_point, extra_lines)) return;

  // Since the divide will now succeed, set up the information to be sent
  // to the new CPU
  Divide_SetupChild(divide_point, child_size);

  // Cut off everything in this CPU's memory past the divide point.
  hardware.ResizeMemory(divide_point);

  // Find out if there is a divide mutation, and handle it.
  if (environment->InPopulation()) Divide_DoMutations();

  // Find out if an exact copy of the CPU was made.
  Divide_CheckCopyTrue();

  // Many tests will require us to run the offspring through a test CPU;
  // this is, for example, to see if mutations need to be reverted or if
  // lineages need to be updated.
  Divide_TestFitnessMeasures();

  // Collect all of the stats about the divide.
  Divide_RecordInfo(); // mutation info transferred here

  // Re-adjust the phenotype due to the divide
  if (environment->InPopulation()) phenotype->DivideReset(repro_data);
  environment->AdjustTimeSlice();


  // If the parent is counted as a second child, reset much of it as
  // well.
  if (cConfig::GetDivideMethod() == DIVIDE_METHOD_SPLIT) {
    hardware.Reset(this);

    if (environment->InPopulation() == FALSE) hardware.SetupTestInputs();
    else hardware.RandomizeInputs();
    advance_ip = false;
  }

  // increase generation count of the parent if requested
  if (cConfig::GetGenerationCountMethod() == GENERATION_COUNT_METHOD_BOTH ){
    environment->SetGeneration( environment->GetGeneration() + 1 );
  }


#ifdef INSTRUCTION_COSTS
  // reset first time instruction costs
  for( int i=0; i<num_inst_cost; ++i ){
    inst_ft_cost[i] = hardware.GetInstLib().GetFTCost(i);
  }
#endif

  // Activate the child!
  // This should be the last thing in this function, because we are not
  // sure that we still exist after this call.
  environment->ActivateChild(repro_data);
}

bool cBaseCPU::Divide_CheckViable(const int child_size, const int parent_size,
				  int extra_lines)
{
#ifndef REMOVE_SIZE_RANGE_FIX
  extra_lines = 0;
#endif
  
  // Make sure the parent is fertile and an allocate has occured.
  if (fertile == false) {
    Fault(FAULT_LOC_DIVIDE, FAULT_TYPE_ERROR, "infertile organism");
    return false; //  (divide fails)
  }
  if (cConfig::GetRequireAllocate() && mal_active == false) {
    Fault(FAULT_LOC_DIVIDE, FAULT_TYPE_ERROR, "must allocate before divide");
    return false; //  (divide fails)
  }


  // Make sure that neither parent nor child will be below the minimum size.

  if (child_size < MIN_CREATURE_SIZE || parent_size < MIN_CREATURE_SIZE ||
      child_size + extra_lines > cConfig::GetChildSizeRange() * parent_size ||
      parent_size + extra_lines > cConfig::GetChildSizeRange() * child_size) {
    Fault(FAULT_LOC_DIVIDE, FAULT_TYPE_ERROR, "invalid offspring length");
    // g_debug.Comment("Viable Failed (sizecheck): child_size=%d, parent_size=%d", child_size, parent_size);
    return false; // (divide fails)
  }

  // Count the number of lines executed in the parent.

  repro_data.executed_size = 0;
  int i;
  for (i = 0; i < parent_size; i++) {
    if (hardware.GetMemFlag(i, INST_FLAG_EXECUTED)) {
      repro_data.executed_size++;
    }
  }

  // If less than 50% the lines in the parent were executed, divide fails...
  // @CAO, make % copied adjustable!

  if (repro_data.executed_size <= parent_size * cConfig::GetMinExeLines()) {
    Fault(FAULT_LOC_DIVIDE, FAULT_TYPE_ERROR, "too few executed commands");
    // g_debug.Comment("Viable Failed (exec_size): exec_size=%d, parnet_size=%d", repro_data.executed_size, parent_size);
    return false; // (divide fails)
  }
	
  // Count the number of lines which were copied into the child.

  repro_data.copied_size = 0;
  for (i = 0; i < child_size; i++) {
    if (hardware.GetMemFlag(parent_size + i, INST_FLAG_COPIED)) {
      repro_data.copied_size++;
    }
  }

  // If less than half the lines were copied into the child, divide fails...
  // @CAO, make % copied adjustable!

  if (repro_data.copied_size <= child_size * cConfig::GetMinCopiedLines()) {
    Fault(FAULT_LOC_DIVIDE, FAULT_TYPE_ERROR, "too few copied commands");
    // g_debug.Comment("UD %d: Viable Failed (copy_size): copy_size=%d, parnet_size=%d", cStats::GetUpdate(), repro_data.copied_size, parent_size);
    return false; // (divide fails)
  }

  // g_debug.Comment("UD %d: Viable Passed!", cStats::GetUpdate());
	
  return true;
}

void cBaseCPU::Divide_SetupChild(const int parent_size, const int child_size)
{
  // Determine the gestation time of the child.
  info.gestation_time = hardware.GetTimeUsed() - info.gestation_start;
  info.gestation_start = (cConfig::GetDivideMethod() == DIVIDE_METHOD_SPLIT)
    ? 0 : hardware.GetTimeUsed();

  repro_data.child_memory.Reset(hardware.GetMemory(), parent_size, child_size);
  repro_data.parent_cpu = this;
  repro_data.parent_genotype = info.active_genotype;
  repro_data.parent_generation = environment->GetGeneration();
  repro_data.parent_merit = phenotype->GetCurMerit();
  repro_data.neutral_metric = info.neutral_metric;
  repro_data.lineage_label = info.lineage_label;
  repro_data.lineage = info.lineage;
  repro_data.child_fertile = true;

  // Information for the phenotype
  repro_data.gestation_time = info.gestation_time;
  repro_data.parent_merit_base = phenotype->GetMeritBase();
  repro_data.parent_bonus = phenotype->GetBonus();
  if( repro_data.tasks == NULL ||
      repro_data.tasks->GetSize() != phenotype->GetNumTasks() ){
    if( repro_data.tasks == NULL ) delete repro_data.tasks;
    repro_data.tasks = new tArray<int>(phenotype->GetNumTasks());
  }
  for( int i=0; i<repro_data.tasks->GetSize(); ++i ){
    (*repro_data.tasks)[i] = phenotype->GetTaskCount(i);
  }
}


// For QSORT in Divide_DoMutations ... MOVE THIS --@TCC
int IntCompareFunction(const void * a, const void * b){
  if( *((int*)a) > *((int*)b) ) return 1;
  if( *((int*)a) < *((int*)b) ) return -1;
  return 0;
}


void cBaseCPU::Divide_DoMutations(){
  int i;
  int num_mut;
  static int mut_sites[MAX_CREATURE_SIZE];

  // Divide Mutations
  if (environment->TestDivideMut()) {
    UINT mut_line = g_random.GetUInt(repro_data.child_memory.GetSize());
    repro_data.child_memory[mut_line] = hardware.GetRandomInst();
    ++cpu_stats.mut_stats.divide_mut_count;
  }

  // Divide Insertions
  if (environment->TestDivideIns() &&
      repro_data.child_memory.GetSize() < MAX_CREATURE_SIZE) {
    UINT mut_line = g_random.GetUInt(repro_data.child_memory.GetSize() + 1);
    repro_data.child_memory.Insert(mut_line, hardware.GetRandomInst());
    ++cpu_stats.mut_stats.divide_insert_mut_count;
  }

  // Divide Deletions
  if (environment->TestDivideDel() &&
      repro_data.child_memory.GetSize() > MIN_CREATURE_SIZE) {
    UINT mut_line = g_random.GetUInt(repro_data.child_memory.GetSize());
    if( repro_data.child_memory.GetFlag(mut_line, INST_FLAG_COPIED) ){
      repro_data.copied_size--;
    }
    repro_data.child_memory.Remove(mut_line);
    ++cpu_stats.mut_stats.divide_delete_mut_count;
  }

  // Insert Mutations (per site)
  if( environment->GetInsMutProb() > 0 ){
    num_mut = g_random.GetRandBinomial(repro_data.child_memory.GetSize(),
				       environment->GetInsMutProb());
    // If would make creature to big, insert up to MAX_CREATURE_SIZE
    if( num_mut + repro_data.child_memory.GetSize() > MAX_CREATURE_SIZE ){
      num_mut = MAX_CREATURE_SIZE - repro_data.child_memory.GetSize();
    }
    // If we have lines to insert...
    if( num_mut > 0 ){
      // Build a list of the sites where mutations occured
      for( i=0; i<num_mut; ++i ){
	mut_sites[i] = g_random.GetUInt(repro_data.child_memory.GetSize() + 1);
      }
      // Sort the list
      qsort( (void*)mut_sites, num_mut, sizeof(int), &IntCompareFunction );
      // Actually do the mutations (in reverse sort order)
      for( i=num_mut-1; i>=0; --i ){
	repro_data.child_memory.Insert(mut_sites[i], hardware.GetRandomInst());
	++cpu_stats.mut_stats.insert_mut_count;
      }
    }
  }

  // Delete Mutations (per site)
  if( environment->GetDelMutProb() > 0 ){
    num_mut = g_random.GetRandBinomial(repro_data.child_memory.GetSize(),
				       environment->GetDelMutProb());
    // If would make creature to small, delete down to MIN_CREATURE_SIZE
    if( repro_data.child_memory.GetSize() - num_mut < MIN_CREATURE_SIZE ){
      num_mut = repro_data.child_memory.GetSize() - MIN_CREATURE_SIZE;
    }
    // If we have lines to delete...
    if( num_mut > 0 ){
      // Build a list of the sites where mutations occured
      for( i=0; i<num_mut; ++i ){
	mut_sites[i] = g_random.GetUInt(repro_data.child_memory.GetSize());
      }
      // Sort the list
      qsort( (void*)mut_sites, num_mut, sizeof(int), &IntCompareFunction );
      // Actually do the mutations (in reverse sort order)
      for( i=num_mut-1; i>=0; --i ){
	// Hack to solve problem of multiple deletions on the last line.
	if( mut_sites[i] == repro_data.child_memory.GetSize() ){
	  mut_sites[i] = repro_data.child_memory.GetSize() - 1;
	}
	if( repro_data.child_memory.GetFlag(mut_sites[i], INST_FLAG_COPIED) ){
	  repro_data.copied_size--;
	}
	repro_data.child_memory.Remove(mut_sites[i]);
	++cpu_stats.mut_stats.delete_mut_count;
      }
    }
  }

  //// Crossover!! ////
  //if( environment->GetCrossMutProb() > 0 ){
  if( environment->TestCrossover() ){
    DoCrossoverMutation( g_random.GetUInt(GetMemorySize()),
			 g_random.GetUInt(GetMemorySize()),
			 repro_data.child_memory,
			 g_random.GetUInt(repro_data.child_memory.GetSize()),
			 g_random.GetUInt(repro_data.child_memory.GetSize()) );
  }
  if( environment->TestAlignedCross() ){
    int startpt = g_random.GetUInt(
	   ( GetMemorySize() < repro_data.child_memory.GetSize() ) ?
	     GetMemorySize() : repro_data.child_memory.GetSize() );
    DoCrossoverMutation( startpt,
			 g_random.GetUInt(GetMemorySize()),
			 repro_data.child_memory,
			 startpt,
			 g_random.GetUInt(repro_data.child_memory.GetSize()) );
  }

  // Drift on neutral metirc
  repro_data.neutral_metric += g_random.GetRandNormal(0,1);

  // Count up mutated lines
  for( i=0; i<hardware.GetMemorySize(); ++i ){
    if( hardware.GetMemFlag(i,INST_FLAG_POINT_MUT) ){
      ++cpu_stats.mut_stats.point_mut_line_count;
    }
  }
  for( i=0; i<hardware.GetChildMemorySize(); ++i ){
    if( hardware.GetChildMemFlag(i,INST_FLAG_COPY_MUT) ){
      ++cpu_stats.mut_stats.copy_mut_line_count;
    }
  }
}


void cBaseCPU::DoCrossoverMutation(int start1, int end1,
				   cCodeArray & in, int start2, int end2){
  const cCodeArray & c1 = GetMemory();
  const cCodeArray & c2 = in;
  assert( start1 >= 0  &&  start1 < c1.GetSize() );
  assert( end1   >= 0  &&  end1   < c1.GetSize() );
  assert( start2 >= 0  &&  start2 < c2.GetSize() );
  assert( end2   >= 0  &&  end2   < c2.GetSize() );

  // Allocate new arrays
  // calc size of bits crossingover
  int sz1 = (start1 <= end1) ? end1-start1+1 : c1.GetSize()-start1+end1+1;
  int sz2 = (start2 <= end2) ? end2-start2+1 : c2.GetSize()-start2+end2+1;

  int nsz1 = c1.GetSize() - sz1 + sz2;
  int nsz2 = c2.GetSize() - sz2 + sz1;

  if( nsz1 < MIN_CREATURE_SIZE || nsz1 > MAX_CREATURE_SIZE ||
      nsz2 < MIN_CREATURE_SIZE || nsz2 > MAX_CREATURE_SIZE ){
    // Don't Crossover because offspring will be illegal!!!
    return;
  }else{
    cCodeArray nc1(nsz1);
    cCodeArray nc2(nsz2);

    nc1.CrossoverInto(c1, start1, end1, c2, start2, end2);
    nc2.CrossoverInto(c2, start2, end2, c1, start1, end1);

    SetMemory(nc1);
    in = nc2;
  }
}



void cBaseCPU::Divide_CheckCopyTrue()
{
#ifdef INST_ARGS
  // Strip out all un-used arguments.
  const cInstLib & inst_lib = hardware.GetInstLib();
  const int max_args = inst_lib.GetNumArgs();
  for (int i = 0; i < repro_data.child_memory.GetSize(); i++) {
    cInstruction & cur_inst = repro_data.child_memory[i];
    const int num_args = inst_lib.GetNumArgs(cur_inst);
    for (int j = num_args; j < max_args; j++) {
      cur_inst.SetArg(j, 0, 0);
    }
  }
#endif

  repro_data.copy_true =
    (info.active_genotype->GetCode() == repro_data.child_memory);
}



// test whether the offspring creature contains an advantageous mutation.
void cBaseCPU::Divide_TestFitnessMeasures()
{
  // If we're in a test CPU already, or we didn't have a genotype
  // change, we can stop right here.
  if (!environment->InPopulation() || repro_data.copy_true == true) return;

  double child_fitness = -1.0; // Only calculate the child's fitness once.

  // If we need to run a test CPU on each divide, do so.  Follow this by
  // all of the tests that need to be done with that CPU.
  if (cConfig::GetTestOnDivide() == true) {
    double parent_fitness = info.active_genotype->GetTestFitness();

    cCPUTestInfo test_info;
    cTestCPU::TestCode(test_info, repro_data.child_memory);
    if (child_fitness == -1.0) child_fitness = test_info.GetGenotypeFitness();

    double neut_min = parent_fitness * LANDSCAPE_NEUTRAL_MIN;
    double neut_max = parent_fitness * LANDSCAPE_NEUTRAL_MAX;
    bool revert = false;
    bool reset = false;

    // If implicit mutations are turned off, make sure this won't spawn one.
    if (cConfig::GetFailImplicit() == true) {
      if (test_info.GetMaxDepth() > 0) reset = true;
    }

    if (child_fitness == 0.0) {
      // Fatal mutation... test for reversion.
      if (g_random.P(cConfig::GetRevertFatal())) revert = true;
      if (g_random.P(cConfig::GetResetFatal())) reset = true;
    } else if (child_fitness < neut_min) {
      if (g_random.P(cConfig::GetRevertNeg())) revert = true;
      if (g_random.P(cConfig::GetResetNeg())) reset = true;
    } else if (child_fitness <= neut_max) {
      if (g_random.P(cConfig::GetRevertNeut())) revert = true;
      if (g_random.P(cConfig::GetResetNeut())) reset = true;
    } else {
      if (g_random.P(cConfig::GetRevertPos())) revert = true;
      if (g_random.P(cConfig::GetResetPos())) reset = true;
    }

    // Ideally, we won't have reversions and resets turned on at the same
    // time, but if we do, give resets the priority.
    if (reset == true) {
      repro_data.child_fertile = false;
      return; // Child is now infertile.
    } else if (revert == true) {
      repro_data.child_memory = info.active_genotype->GetCode();
      return; // Parent and child are now identical; skip remaining tests.
    }
  }

}

void cBaseCPU::Divide_RecordInfo()
{
  mal_active = false;
  info.num_divides++;

  info.fitness = phenotype->GetCurMerit().CalcFitness(info.gestation_time);

//    static int print_count;
//    if (info.fitness > 2048 && print_count < 10) {
//      print_count++;
//      cString filename;
//      filename.Set("creature.EQU.%s", print_count);
//      ofstream fp(filename);
//      hardware.GetInstLib().PrintCode(info.active_genotype->GetCode(), fp);
//    }


  info.executed_size = repro_data.executed_size;
  info.copied_size = repro_data.copied_size;
  repro_data.fitness = info.fitness;

  environment->GStats_SetGestationTime(info.gestation_time);
  environment->GStats_SetFitness(info.fitness);
  environment->GStats_SetMerit(phenotype->GetCurMerit());
  environment->GStats_SetExecutedSize(info.executed_size);
  environment->GStats_SetCopiedSize(info.copied_size);

  //// Setup info in cpu_stats on divide
  cpu_stats.genotype_id = repro_data.parent_genotype->GetID();
  cpu_stats.size = GetMemorySize();
  cpu_stats.executed_size = repro_data.executed_size;
  cpu_stats.copied_size = repro_data.copied_size;
  cpu_stats.child_size = repro_data.child_memory.GetSize();
  cpu_stats.parent_dist =
    GetActiveGenotype()->FindGeneticDistance(repro_data.child_memory);
}

void cBaseCPU::Fault(int fault_loc, int fault_type, cString fault_desc)
{
  (void) fault_loc;
  (void) fault_type;
  (void) fault_desc;

#ifdef FATAL_ERRORS
  if (fault_type == FAULT_TYPE_ERROR) {
    environment->KillCreature( this );
  }
#endif

#ifdef FATAL_WARNINGS
  if (fault_type == FAULT_TYPE_WARNING) {
    environment->KillCreature( this );
  }
#endif

#ifdef BREAKPOINTS
  info.fault_desc = fault_desc;
#endif

  info.num_errors++;
}



int cBaseCPU::Mutate(int mut_point)
{
  // Point Mutations only ???  -- @TCC
#ifdef DEBUG
  if (environment->InPopulation() == FALSE) {
    g_debug.Warning("Trying to Mutate(int) a test cpu");
  }
#endif

  if (mut_point >= hardware.GetMemorySize()) {
    return 0;
  }

  hardware.SetMemory(mut_point, hardware.GetRandomInst());
  hardware.SetMemFlag(mut_point, INST_FLAG_MUTATED);
  hardware.SetMemFlag(mut_point, INST_FLAG_POINT_MUT);
  point_mut = true;
  ++cpu_stats.mut_stats.point_mut_count;

  return 1;
}


void cBaseCPU::TestSearchSize(const int search_size){
#ifdef TEST_SEARCH_SIZE
  if( mal_active == false &&
      search_size > GetMemorySize()*(1-TEST_SEARCH_SIZE) &&
      search_size < GetMemorySize()*(1+TEST_SEARCH_SIZE) ){
    search_approx_size = true;
  }
#endif
}


//// Save and Load ////
void cBaseCPU::SaveState(ofstream & fp)
{
  assert(fp.good());

  fp<<"cBaseCPU"<<endl;

  //// Save If it is alive ////
  if( GetActiveGenotype() == NULL ){
    fp<<false<<endl;
  }else{
    fp<<true<<endl;

    //// Save Genotype Genome ////
    fp<<info.active_genotype->GetLength()<<endl;
    hardware.GetInstLib().PrintCode(info.active_genotype->GetCode(), fp);

    //// Save Actual Creature Memory & MemFlags ////
    fp<<hardware.GetMemorySize()<<endl;
    hardware.GetInstLib().PrintCode(hardware.GetMemory(), fp);
    fp<<"|"; // marker
    for( int i=0; i<hardware.GetMemorySize(); ++i ){
      fp<<hardware.GetMemory().GetFlags(i);
    }
    fp<<endl;

    //// Save Creature Child Memory & MemFlags ////
    fp<<hardware.GetChildMemorySize()<<endl;
    if( hardware.GetChildMemorySize() > 0 ){
      hardware.GetInstLib().PrintCode(hardware.GetChildMemory(), fp);
      fp<<"|"; // marker
      for( int i=0; i<hardware.GetChildMemorySize(); ++i ){
	fp<<hardware.GetChildMemory().GetFlags(i);
      }
      fp<<endl;
    }

    //// Save Hardware (Inst_Pointer, Stacks, and the like)
    hardware.SaveState(fp);

    //// Save Flags & Statistics//// @TCC === THIS PART STILL TO DO
    {
      flags.SaveState(fp);
      fp<<GetEnvironment()->GetGeneration()<<endl;
    }

    //// Creature info ////
    info.SaveState(fp);

    //// Save Phenotype  ////
    GetPhenotype()->SaveState(fp);

  } // if there is a genotype here (ie. not dead)
}


void cBaseCPU::LoadState(ifstream & fp)
{
#ifdef DEBUG
  if (environment->InPopulation() == FALSE) {
    g_debug.Warning("Trying to LoadState into a test CPU.");
    return;
  }
#endif

  ResetVariables();

  assert(fp.good());

  cString foo;
  fp>>foo;
  assert( foo == "cBaseCPU" );

  //// Is there a creature there ////
  bool alive_flag = 0;
  fp>>alive_flag;
  if( alive_flag ){

    //// Load Genotype ////
    cCodeArray in_code = LoadCodeFromStream(fp);
    cGenotype * new_genotype = environment->AddGenotype(in_code);
    ChangeGenotype(new_genotype);

    //// Load Actual Creature Memory & MemFlags ////
    {	
      in_code = LoadCodeFromStream(fp);	
      hardware.Memory_CopyData(in_code);
      CA_FLAG_TYPE flags;
      char marker;  fp>>marker;  assert( marker == '|' );
      for( int i=0; i<hardware.GetMemorySize(); ++i ){
	fp.get(flags);
	hardware.SetMemFlags(i, flags);
      }
    }

    //// Load Creature Child Memory & MemFlags ////
    {
      in_code = LoadCodeFromStream(fp);	
      if( in_code.GetSize() > 0 ){
	CA_FLAG_TYPE flags;
	char marker;  fp>>marker;  assert( marker == '|' );
	for( int i=0; i<in_code.GetSize(); ++i ){
	  fp.get(flags);
	  in_code.SetFlags(i, flags);
	}
      }
      hardware.SetChildMemory(in_code);
    }


    //// Load Hardware (Inst_Pointer, Stacks, and the like)
    hardware.LoadState(fp);

    //// Load Flags & Statistics////
    {
      flags.LoadState(fp);
      assert(fp.good());
      int tmp_int;
      fp>>tmp_int; GetEnvironment()->SetGeneration(tmp_int);
    }

    //// Creature info ////
    info.LoadState(fp);

    //// Load Phenotype  ////
    assert(fp.good());
    phenotype->Clear(info.active_genotype->GetLength());
    GetPhenotype()->LoadState(fp);
    GetPhenotype()->ReCalcMerit();

    //// Just set other CPU flags & state (some based on phenotype) ////
    injected = true;
    SetFitness(GetPhenotype()->GetFitness());
    SetGestationTime(GetPhenotype()->GetGestationTime());

    //// Adjust Time Slice ////
    environment->AdjustTimeSlice();

  } // end if not a dead creature
}



cCodeArray cBaseCPU::LoadCodeFromStream(ifstream & fp){
  assert(fp.good());
  const cInstLib & inst_lib = hardware.GetInstLib();

  int num_lines = -1;
  fp>>num_lines;

  if( num_lines <= 0 ){
    cCodeArray in_code;
    return in_code;
  }

  // Setup the code array...
  cCodeArray in_code(num_lines);
  cString cur_line;
  for (int line_num = 0; line_num < in_code.GetSize(); line_num++) {
    assert(fp.good());

    fp>>cur_line;
    in_code[line_num] = inst_lib.GetInst(cur_line);

    if (in_code[line_num] == cInstruction::GetInstError()) {
      // You're using the wrong instruction set!  YOU FOOL!
      cString error_str("Cannot load creature memory in LoadSate\"");
      error_str += "\"\n       Unknown line: ";
      error_str += line_num;
      error_str += "       ";
      environment->NotifyError(error_str);
    }
  }
  return in_code;
}	
