//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          //
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
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
#include "cpu.ii"

///////////////
//  cBaseCPU
///////////////

cBaseCPU::cBaseCPU(int in_cpu_test)
{
  g_memory.Add(C_BASE_CPU);

  inst_lib = InitInstructions();

  hardware.Reset(this); // Move to hardware object...

  info.active_genotype = NULL;
  info.max_executed = -1;
  info.next = NULL;
  info.prev = NULL;
  info.cpu_test = in_cpu_test;

  if (info.cpu_test) {
    environment = new cTestEnvironment(this);
  } else {
    environment = new cMainEnvironment(this);
  }

  ResetVariables();
}

cBaseCPU::~cBaseCPU()
{
  g_memory.Remove(C_BASE_CPU);
  delete environment;
}

double cBaseCPU::GetFitness()
{
  return phenotype.GetMerit().CalcFitness(GetGestationTime());
}



//////////////////////////
// And the instructions...
//////////////////////////

void cBaseCPU::Inst_Nop()          // Do Nothing.
{
}

void cBaseCPU::Inst_IfNot0()       // Execute next if ?bx? != 0.
{
  int reg_used = hardware.FindModifiedRegister(REG_BX);

  if (hardware.GetRegister(reg_used) == 0) {
    hardware.AdvanceIP();
  }
}

void cBaseCPU::Inst_IfNEqu()     // Execute next if bx != ?cx?
{
  int reg_used = hardware.FindModifiedRegister(REG_BX);
  int reg_used2 = reg_used + 1;
  if (reg_used2 == NUM_REGISTERS) reg_used2 = REG_AX;

  if (hardware.GetRegister(reg_used) == hardware.GetRegister(reg_used2)) {
    hardware.AdvanceIP();
  }
}

void cBaseCPU::Inst_IfBit1()
{
  int reg_used = hardware.FindModifiedRegister(REG_BX);

  if ((hardware.GetRegister(reg_used) & 1) == 0) {
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
    hardware.JumpIP(hardware.GetRegister(REG_BX));
  }

  // Otherwise, jump to the complement label.
  else if ((jump_location = hardware.FindLabel(1)).GetPosition() != -1) {
    hardware.SetIP(jump_location);
  }
}


void cBaseCPU::Inst_JumpB()
{
  cCPUHead jump_location;

  hardware.ReadLabel();
  hardware.ComplementLabel();

  // If there is no label, jump BX steps.
  if (hardware.GetLabelSize() == 0) {
    hardware.JumpIP(-(hardware.GetRegister(REG_BX)));
  }    

  // otherwise jump to the complement label.
  else if ((jump_location = hardware.FindLabel(-1)).GetPosition() != -1) {
    hardware.SetIP(jump_location);
  }
}

void cBaseCPU::Inst_JumpP()
{
  if (info.cpu_test) {
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

  // otherwise template was not found; reccord and error.
  else {
    info.num_errors++;
    return;
  }

  if (hardware.TestParasite()) {
    SetParasite();
  }
}

void cBaseCPU::Inst_Call()
{
  cCPUHead jump_location;

  // Put the starting location onto the stack
  hardware.StackPush(hardware.GetInstPointer().GetPosition());

  // Jump to the compliment label (or by the ammount in the bx register)
  hardware.ReadLabel();
  hardware.ComplementLabel();

  if (hardware.GetLabelSize() == 0) {
    hardware.JumpIP(hardware.GetRegister(REG_BX));
  }
  else if ((jump_location = hardware.FindLabel(1)).GetPosition() != -1) {
    hardware.SetIP(jump_location);
  }
}

void cBaseCPU::Inst_Return()
{
  hardware.SetIP(hardware.StackPop());
}

void cBaseCPU::Inst_ShiftR()
{
  int reg_used = hardware.FindModifiedRegister(REG_BX);
  hardware.Reg_ShiftR(reg_used);
}

void cBaseCPU::Inst_ShiftL()
{
  int reg_used = hardware.FindModifiedRegister(REG_BX);
  hardware.Reg_ShiftL(reg_used);
}

void cBaseCPU::Inst_Bit1()
{
  int reg_used = hardware.FindModifiedRegister(REG_BX);

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
  int reg_used = hardware.FindModifiedRegister(REG_BX);
  hardware.Reg_Inc(reg_used);
}

void cBaseCPU::Inst_Dec()
{
  int reg_used = hardware.FindModifiedRegister(REG_BX);
  hardware.Reg_Dec(reg_used);
}

void cBaseCPU::Inst_Zero()
{
  int reg_used = hardware.FindModifiedRegister(REG_BX);
  hardware.Reg_Set(reg_used, 0);
}

void cBaseCPU::Inst_Pop()
{
  int reg_used = hardware.FindModifiedRegister(REG_BX);
  hardware.Reg_Set(reg_used, hardware.StackPop());
}

void cBaseCPU::Inst_Push()
{
  int reg_used = hardware.FindModifiedRegister(REG_BX);
  hardware.StackPush(hardware.GetRegister(reg_used));
}

void cBaseCPU::Inst_Add()
{
  int reg_used = hardware.FindModifiedRegister(REG_BX);
  hardware.Reg_Set(reg_used,
     hardware.GetRegister(REG_BX) + hardware.GetRegister(REG_CX));
}

void cBaseCPU::Inst_Sub()
{
  int reg_used = hardware.FindModifiedRegister(REG_BX);
  hardware.Reg_Set(reg_used,
     hardware.GetRegister(REG_BX) - hardware.GetRegister(REG_CX));
}

void cBaseCPU::Inst_Nand()
{
  int reg_used = hardware.FindModifiedRegister(REG_BX);
  hardware.Reg_Set(reg_used,
     ~(hardware.GetRegister(REG_BX) & hardware.GetRegister(REG_CX)));
}

void cBaseCPU::Inst_Nor()
{
  int reg_used = hardware.FindModifiedRegister(REG_BX);
  hardware.Reg_Set(reg_used,
      ~(hardware.GetRegister(REG_BX) | hardware.GetRegister(REG_CX)));
}

void cBaseCPU::Inst_And()
{
  int reg_used = hardware.FindModifiedRegister(REG_BX);
  hardware.Reg_Set(reg_used,
       (hardware.GetRegister(REG_BX) & hardware.GetRegister(REG_CX)));
}

void cBaseCPU::Inst_Not()
{
  int reg_used = hardware.FindModifiedRegister(REG_BX);
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
  int reg_used = hardware.FindModifiedRegister(REG_BX);
  hardware.Reg_Set(reg_used,
    hardware.GetRegister(REG_BX) ^ hardware.GetRegister(REG_CX));
}

void cBaseCPU::Inst_Copy()
{
  cCPUHead from(this, hardware.GetRegister(REG_BX));
  cCPUHead to(this,
     hardware.GetRegister(REG_AX) + hardware.GetRegister(REG_BX));

  if (!info.cpu_test && environment->TestCopyMut()) {
    to.SetInst(g_random.GetUInt(inst_lib->GetSize()));
    to.SetFlag(INST_FLAG_MUTATED);  // Mark this instruction as mutated...
  } else {
    to.SetInst(from.GetInst());
  }

  to.SetFlag(INST_FLAG_COPIED);  // Set the copied flag.
}

void cBaseCPU::Inst_ReadInst()
{
  int reg_used = hardware.FindModifiedRegister(REG_CX);
  cCPUHead from(this, hardware.GetRegister(REG_BX));
  
  if (!info.cpu_test && environment->TestCopyMut()) {
    hardware.Reg_Set(reg_used, g_random.GetUInt(inst_lib->GetSize()));
  } else {
    hardware.Reg_Set(reg_used, from.GetInst());
  }
}

void cBaseCPU::Inst_WriteInst()
{
  cCPUHead to(this,
     hardware.GetRegister(REG_AX) + hardware.GetRegister(REG_BX));
  int reg_used = hardware.FindModifiedRegister(REG_CX);
  int value = hardware.GetRegister(reg_used);

  if (value < 0 || value >= inst_lib->GetSize() ||
      (!info.cpu_test && environment->TestCopyMut())) {
    if (info.cpu_test) value = 0;
    else value = g_random.GetUInt(inst_lib->GetSize());
  } else {
    to.SetInst(value);
  }    

  to.SetFlag(INST_FLAG_COPIED);  // Set the copied flag.
}

void cBaseCPU::Inst_Compare()
{
  int reg_used = hardware.FindModifiedRegister(REG_CX);
  cCPUHead from(this, hardware.GetRegister(REG_BX));
  cCPUHead to(this, hardware.GetRegister(REG_AX) + hardware.GetRegister(REG_BX));

  if (environment->TestCopyMut()) {
    from.SetInst(g_random.GetUInt(inst_lib->GetSize()));
    to.SetInst(g_random.GetUInt(inst_lib->GetSize()));
  }

  hardware.Reg_Set(reg_used, (int) from.GetInst() - (int) to.GetInst());
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
  // must do divide before second mal & must mal positive amount...
  if (flags.GetFlag(CPU_FLAG_MAL_ACTIVE) || hardware.GetRegister(REG_BX) < 1) {
    info.num_errors++;
    return;
  }

  int allocated_size = hardware.GetRegister(REG_BX);
  int new_memory_size = hardware.GetMemorySize() + allocated_size;

  // Make sure that the new size is in range.
  if (new_memory_size > MAX_CREATURE_SIZE             ||
      new_memory_size < MIN_CREATURE_SIZE             ||
      allocated_size > hardware.GetMemorySize() * 2  ||
      allocated_size < hardware.GetMemorySize() / 2) {
    info.num_errors++;
    return;
  }

  hardware.Reg_Set(REG_AX, GetMemorySize());
  hardware.ResizeMemory(new_memory_size);
  flags.SetFlag(CPU_FLAG_MAL_ACTIVE);


  cBaseCPU * neighbor = environment->GetNeighbor();

  // If we don't have a neighbor, initialize to zero...

  if (!neighbor || neighbor->GetMemorySize() == 0) {
    for (int i = hardware.GetRegister(REG_AX); i < new_memory_size; i++) {
      hardware.SetMemory(i, 0);
    }
  }

  // Otherwise initialize using this neighbors memory (should be done better
  // later!)


  else {
    int offset = g_random.GetUInt(neighbor->GetMemory().GetSize());
    for (int i = hardware.GetRegister(REG_AX); i < new_memory_size; i++) {
      hardware.SetMemory(i,
        neighbor->GetMemory().Get((i + offset) % neighbor->GetMemorySize()));
    }
  }
}

void cBaseCPU::Inst_Divide()
{
  int new_parent_size = hardware.GetRegister(REG_AX);
  int child_size = hardware.GetMemorySize() - new_parent_size;

  // Make sure this divide will produce a viable offspring.
  if (!Divide_CheckViable(child_size, new_parent_size)) return;

  // Since the divide will now succeed, update the relevant information.
  Divide_RecordInfo();

  // Set up the information to be sent to the new CPU
  Divide_SetupChild(new_parent_size);

  // Cut off everything in this CPU's memory past the divide point.
  hardware.ResizeMemory(new_parent_size);

  // Find out if there is a divide mutation, and handle it.
  if (!info.cpu_test) Divide_DoMutations();

  // Find out if an exact copy of the CPU was made.
  Divide_CheckCopyTrue();

  // Re-adjust the phenotype due to the divide
  if (!info.cpu_test) {
    phenotype.DivideReset(repro_data);
  }
  environment->AdjustTimeSlice();

  // Finally, activate the child!
  environment->ActivateChild(&repro_data);
} // End: cBaseCPU::Inst_Divide()

void cBaseCPU::Inst_CDivide()
{
  int new_parent_size = hardware.GetMemorySize() / 2;
  int child_size = hardware.GetMemorySize() - new_parent_size;

  // Make sure this divide will produce a viable offspring.
  if (!Divide_CheckViable(child_size, new_parent_size)) return;

  // Since the divide will now succeed, update the relevant information.
  Divide_RecordInfo();

  // Set up the information to be sent to the new CPU
  Divide_SetupChild(new_parent_size);

  // Cut off everything in this CPU's memory past the divide point.
  hardware.ResizeMemory(new_parent_size);

  // Find out if there is a divide mutation, and handle it.
  if (!info.cpu_test) Divide_DoMutations();

  // Find out if an exact copy of the CPU was made.
  Divide_CheckCopyTrue();

  // Re-adjust the phenotype due to the divide
  if (!info.cpu_test) phenotype.DivideReset(repro_data);

  // Finally, activate the child!
  environment->ActivateChild(&repro_data);
} // End: cBaseCPU::Inst_CDivide()

void cBaseCPU::Inst_CAlloc()   // Double the allocated space.
{
  // must do divide before second mal
  if (flags.GetFlag(CPU_FLAG_MAL_ACTIVE)) {
    info.num_errors++;
    return;
  }
  flags.SetFlag(CPU_FLAG_MAL_ACTIVE);

  hardware.Reg_Set(REG_AX, hardware.GetMemorySize());
  int new_memory_size = 2 * hardware.GetMemorySize();

  if (new_memory_size > MAX_CREATURE_SIZE) {
    info.num_errors++;
    return;
  }

  hardware.ResizeMemory(new_memory_size);

  cBaseCPU * neighbor = environment->GetNeighbor();

  // If we don't have a neighbor, initialize to zero...

  if (!neighbor || neighbor->GetMemory().GetSize() == 0) {
    for (int i = hardware.GetRegister(REG_AX); i < new_memory_size; i++) {
      hardware.SetMemory(i, 0);
    }
  }

  // Otherwise initialize using this neighbors memory (should be done better
  // later!)


  else {
    int offset = g_random.GetUInt(neighbor->GetMemory().GetSize());
    for (int i = hardware.GetRegister(REG_AX); i < new_memory_size; i++) {
      hardware.SetMemory(i,
	  neighbor->GetMemory().Get((i + offset) % neighbor->GetMemorySize()));
    }
  }
}

void cBaseCPU::Inst_Inject()
{
  int new_size = hardware.GetRegister(REG_AX);
  int inject_size = hardware.GetMemorySize() - new_size;
  cBaseCPU * host_cpu = environment->GetFacing();

  int new_host_size = inject_size + (host_cpu ? host_cpu->GetMemorySize() : 0);

  // Make sure the creature will still be above the minimum size,
  // and the the would-be injected creature is below max.
  if (inject_size <= 0 || new_size < MIN_CREATURE_SIZE ||
      new_host_size > MAX_CREATURE_SIZE) {
    info.num_errors++;
    return; // (inject fails)
  }
  
  // Find the inject_line
  
  hardware.ReadLabel();
  hardware.ComplementLabel();

  // If there is no label, abort.
  if (hardware.GetLabelSize() == 0) {
    info.num_errors++;
    return; // (inject fails)
  }

  // If this is just a test CPU, this is as far as we have to go.  Resize,
  // and be done with it.
  if (!host_cpu) {
    hardware.ResizeMemory(new_size);
    return; // (inject doesn't happen)
  }
  
  // Otherwise, inject at the compliment.
  int inject_line = host_cpu->FindFullLabel(hardware.GetLabel()).GetPosition();

  // Cut off injectable code and abort if no compliment is found.
  if (inject_line == -1) {
    hardware.ResizeMemory(new_size);
    return; // (inject fails)
  }

  // Construct the code to be injected.
  cCodeArray inject_code(inject_size);
  for (int i = 0; i < inject_size; i++) {
    inject_code[i] = hardware.GetMemData(new_size + i);
  }
  hardware.ResizeMemory(new_size);
  
  // And inject it!
  host_cpu->InjectCode(&inject_code, inject_line);
  
  // Set the relevent flags.
  flags.SetFlag(CPU_FLAG_INST_INJECT);
}

void cBaseCPU::Inst_TaskGet()
{
  int reg_used = hardware.FindModifiedRegister(REG_CX);

  hardware.Reg_Set(reg_used, hardware.GetInput());
  hardware.DoInput(phenotype);
}

void cBaseCPU::Inst_TaskPut()
{
  int reg_used = hardware.FindModifiedRegister(REG_BX);
  int value = hardware.GetRegister(reg_used);
  hardware.Reg_Set(reg_used, 0);

  // Check for tasks compleated.

  hardware.DoOutput(phenotype, value);
}

void cBaseCPU::Inst_SearchF()
{
  hardware.ReadLabel();
  hardware.ComplementLabel();
  hardware.Reg_Set(REG_BX, hardware.FindLabel(1).GetPosition()
		   - hardware.GetIPPosition());
  hardware.Reg_Set(REG_CX, hardware.GetLabelSize());
}

void cBaseCPU::Inst_SearchB()
{
  hardware.ReadLabel();
  hardware.ComplementLabel();
  hardware.Reg_Set(REG_BX, hardware.GetIPPosition() -
		   hardware.FindLabel(-1).GetPosition());
  hardware.Reg_Set(REG_CX, hardware.GetLabelSize());
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
  int reg_used = hardware.FindModifiedRegister(REG_BX);

  int new_mut_rate = hardware.GetRegister(reg_used);
  if (new_mut_rate < 1) new_mut_rate = 1;
  environment->SetCopyMutRate(new_mut_rate);
}

void cBaseCPU::Inst_ModCopyMut()
{
  int reg_used = hardware.FindModifiedRegister(REG_BX);

  // Don't let copy mutations be non-positive...

  int new_mut_rate = environment->GetCopyMutRate() + hardware.GetRegister(reg_used);
  if (new_mut_rate > 0) {
    environment->SetCopyMutRate(new_mut_rate);
  }
}

// This method notifies the viewer _before_ it executes each instruction.
void cBaseCPU::Notify()
{
  hardware.AdjustIP();
  environment->Notify();

  // Find the number of the instruction to be executed...
  UCHAR inst_id = hardware.GetCurInst();
  tCPUMethod inst_ptr;
 
  // Get a pointer to the corrisponding method...
  if (inst_lib->flags.GetFlag(INST_FLAG_ALT_MODE)) {
    inst_ptr = inst_lib->alt_function[inst_id];
  } else {
    inst_ptr = inst_lib->function[inst_id];
  }

  // And execute it.
  (this->*inst_ptr)();
}


cGenotype * cBaseCPU::LoadCode(char * filename)
{
#ifdef DEBUG
  if (info.cpu_test) {
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

  cCodeArray in_code;
  in_code.Resize(input_file.GetNumLines());  // Setup the code array...

  for (int line_num = 0; line_num < in_code.GetSize(); line_num++) {
    int i;
    
    for (i = 0; i < inst_lib->GetSize(); i++) {
      if(inst_lib->name[i] == input_file.GetLine()) {
	break;
      }
    }

    if (i < inst_lib->GetSize()) {
      in_code[line_num] = i;
    } else {
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

  cGenotype * new_genotype = environment->AddGenotype(&in_code);
  ChangeGenotype(new_genotype);

  // Update stats for the cpu...
  SetParentTrue(TRUE);
  SetCopiedSize(info.active_genotype->GetLength());

  // Initialize the phenotype.
  phenotype.Clear(info.active_genotype->GetLength());

  flags.SetFlag(CPU_FLAG_INJECTED);

  return new_genotype;
}

void cBaseCPU::ChangeGenotype(cGenotype * in_genotype)
{
  // If we only have a test CPU, make this quick...
  if (info.cpu_test) {
    ResetVariables();
    hardware.Memory_CopyData(*(in_genotype->GetCode()));
    info.active_genotype = in_genotype;
    return;
  }

  // If this CPU already had a genotype, handle its removal.
  if (info.active_genotype) {
    environment->Stats_RemoveCreature(info.num_divides, info.age);

    // if this creature was a parasite, let the genotype know.
    if (flags.GetFlag(CPU_FLAG_PARASITE)) info.active_genotype->AddParasite();

    // If we are actually changeing genotypes, adjust the cpu counts.
    if (in_genotype != info.active_genotype) {
      info.active_genotype->RemoveCPU();

      // Adjust the genebank for this genotype.  If it has been removed,
      // make sure to destroy our reference to it.
      if (!environment->AdjustGenotype(info.active_genotype)) {
	info.active_genotype = NULL;
      }
    }
  }

  // Now that we have cleaned up all the statistics to do with the old
  // genotype, reset the CPU fpr the new one....
  ResetVariables();

  // If we are actually adding in a new genotype, set it up.
  if (in_genotype) {
    hardware.Memory_CopyData(*(in_genotype->GetCode()));
    if (in_genotype != info.active_genotype) {
      in_genotype->AddCPU();
      info.active_genotype = in_genotype;
      environment->AdjustGenotype(in_genotype);
    } else {
      in_genotype->SwapCPU();
    }
    environment->Stats_AddCreature(in_genotype->GetID());

    switch (stats.GetDeathMethod()) {
    case DEATH_METHOD_OFF:
      info.max_executed = -1;
      break;
    case DEATH_METHOD_CONST:
      info.max_executed = stats.GetAgeLimit();
      break;
    case DEATH_METHOD_MULTIPLE:
      info.max_executed = stats.GetAgeLimit() * in_genotype->GetLength();
      break;
    }
  }

  // If we are not adding in a new genotype, make the CPU empty.
  else {
    info.active_genotype = NULL;
    hardware.ResetMemory(0);
    environment->Stats_AddCreature(-1);
  }
}

int cBaseCPU::TestProcess()
{
  if (!info.cpu_test) {
    g_debug.Warning("Running TestProcess() on a non-test CPU!");
    return 0;
  }

  // Clear out the CPU for the new run...
  ResetVariables();
  phenotype.Clear(hardware.GetMemorySize());

  int time_allocated = 20 * hardware.GetMemorySize();

  // if (time_allocated > info.max_executed)
  //   time_allocated = info.max_executed;
  int time_used = 0;

  while (time_used++ < time_allocated && hardware.GetMemorySize() &&
	 !info.num_divides) { 
    SingleProcess();
  }

  return time_used;
}

int cBaseCPU::OK()
{
  int result = TRUE;

  // First Check all of the hardware.

  if (!environment->OK()) result = FALSE;
  if (!hardware.OK()) result = FALSE;

  // Some quick info checks...

  if (info.num_divides < 0 || info.num_errors < 0) {
    g_debug.Warning("Execution structure corrupt in CPU");
    result = FALSE;
  }
  
  return result;
}

void cBaseCPU::ActivateChild()
{
  if (!info.cpu_test) {
    g_debug.Warning("Trying to ActivateChild() on a non-test CPU!");
    return;
  }

  // @CAO should give the option to make the child active in this CPU, and
  // run it for a gestation cycle.

  SetParentTrue(repro_data.copy_true);
}

void cBaseCPU::Kill()   // Prepare creature for death (stats & such)
{
  if (info.cpu_test) {
    g_debug.Warning("Trying to Kill() a test cpu");
    return;
  }

  environment->Stats_AddDeath();
}

void cBaseCPU::ResetVariables()
{
  hardware.Reset();
  if (!info.cpu_test) hardware.RandomizeInputs();

  // Info...

  info.copied_size = 0;
  info.executed_size = 0;

  // Info...

  info.total_time_used = 0;
  info.num_divides = 0;
  info.num_errors = 0;
  info.gestation_start = 0;
  info.age = 0;
  info.gestation_time = 0;
  info.fitness = 0.0;

  flags.UnsetFlag(CPU_FLAG_MAL_ACTIVE);
  flags.UnsetFlag(CPU_FLAG_FULL_TRACE);
  flags.UnsetFlag(CPU_FLAG_INJECTED);
  flags.UnsetFlag(CPU_FLAG_PARASITE);
  flags.UnsetFlag(CPU_FLAG_INST_INJECT);
  flags.UnsetFlag(CPU_FLAG_INJECT_HOST);
  flags.UnsetFlag(CPU_FLAG_POINT_MUT);

  // Repro data...

  repro_data.copy_true = FALSE;
}

void cBaseCPU::PrintStatus()
{
#ifdef DEBUG
  static FILE * fp = fopen("trace.dat", "w");

  fprintf(fp, "%d.%d: #%d ", stats.GetUpdate(), stats.GetSubUpdate(),
	  environment->GetID());
  fprintf(fp, "IP[%d] ", hardware.GetIPPosition());
  fprintf(fp, "ax[%d] ", hardware.GetRegister(REG_AX));
  fprintf(fp, "bx[%d] ", hardware.GetRegister(REG_BX));
  fprintf(fp, "cx[%d] ", hardware.GetRegister(REG_CX));
  fprintf(fp, "Mem:%d ", hardware.GetMemorySize());
  fprintf(fp, "NXT:%s\n", inst_lib->name[hardware.GetCurInst()]());
  fflush(fp);
#endif
}

void cBaseCPU::Mutate()  // Mutate a random location in the creature memory.
{
  if (info.cpu_test) {
    g_debug.Warning("Trying to Mutate() a test cpu");
  }

  if (!hardware.GetMemorySize()) {
    return;
  }

  int mut_point = g_random.GetUInt(hardware.GetMemorySize());
  hardware.SetMemory(mut_point, g_random.GetUInt(inst_lib->GetSize()));

  hardware.SetMemFlag(mut_point, INST_FLAG_MUTATED);
  flags.SetFlag(CPU_FLAG_POINT_MUT);
}

int cBaseCPU::Mutate(int mut_point)
{
  if (info.cpu_test) {
    g_debug.Warning("Trying to Mutate(int) a test cpu");
  }

  if (mut_point >= hardware.GetMemorySize()) {
    return 0;
  }

  hardware.SetMemory(mut_point, g_random.GetUInt(inst_lib->GetSize()));
  hardware.SetMemFlag(mut_point, INST_FLAG_MUTATED);
  flags.SetFlag(CPU_FLAG_POINT_MUT);

  return 1;
}

void cBaseCPU::Print(char * filename)
{
  if (!info.cpu_test) {
    g_debug.Warning("Trying to Print() a non-test cpu");
    return;
  }

  int i;
  FILE * fp;
  cCodeArray * code;

  if (!hardware.GetMemorySize()) return;

  // Open the file...

  fp = fopen(filename, "w");
  if (!fp) {
    printf ("ERROR: Unable to open %s\n", filename);
    return;
  }

  // Print the useful info at the top...

  fprintf (fp, "# Filename......: \"%s\"\n", filename);
  fprintf (fp, "# Update Created: %d\n", GetActiveGenotype()->GetUpdateBorn());
  fprintf (fp, "# Merit.........: %.2e\n",
	   phenotype.GetCurMerit().GetDouble());
  fprintf (fp, "# Gestation Time: %d\n", info.gestation_time);
  fprintf (fp, "# Fitness.......: %.2e\n", info.fitness);
  fprintf (fp, "# Errors........: %d\n", info.num_errors);
  fprintf (fp, "# Copied Size...: %d\n", info.copied_size);
  fprintf (fp, "# Executed Size.: %d\n", info.executed_size);
  
  fprintf(fp, "\n");
  fprintf(fp, "# GETS: %2d   PUTS: %2d   ECHOS: %2d\n",
	  phenotype.GetGetCount(), phenotype.GetPutCount(),
	  phenotype.GetTaskCount(TASK_ECHO));
  fprintf(fp, "# NOTS: %2d   NANDS: %2d\n",
	  phenotype.GetTaskCount(TASK_NOT),
	  phenotype.GetTaskCount(TASK_NAND));
  fprintf(fp, "# AND: %2d    ORN: %2d\n",
	  phenotype.GetTaskCount(TASK_AND),
	  phenotype.GetTaskCount(TASK_ORN));
  fprintf(fp, "# ANDN: %2d   OR: %2d\n",
	  phenotype.GetTaskCount(TASK_ANDN),
	  phenotype.GetTaskCount(TASK_OR));
  fprintf(fp, "# NOR: %2d    XOR: %2d   EQU: %2d\n\n",
	  phenotype.GetTaskCount(TASK_NOR),
	  phenotype.GetTaskCount(TASK_XOR),
	  phenotype.GetTaskCount(TASK_EQU));

  // Display the genome

  code = info.active_genotype->GetCode();
  for (i = 0; i < code->GetSize(); i++) {
    fprintf(fp, "%s\n", inst_lib->name[code->operator[](i)]());
  }

  fclose(fp);
}
