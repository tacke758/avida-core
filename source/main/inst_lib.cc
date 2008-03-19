//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "inst_lib.hh"
#include "../cpu/cpu.hh"
#include "../cpu/head.ii"

const cString cInstLib::name_unknown("Unknown");

////////////////
//  cInstEntry
////////////////

cInstEntry::cInstEntry()
{
  name = "[none]";
  function = (tCPUMethod) NULL;
  alt_function = (tCPUMethod) NULL;
  redundancy = 1;
  cost = 0;
  ft_cost = 0;
  prob_exe = 1;
  num_args = 0;
}

cInstEntry::cInstEntry(const cInstEntry & _in)
{
  name = _in.name;
  function = _in.function;
  alt_function = _in.alt_function;
  redundancy = _in.redundancy;
  cost = _in.cost;
  ft_cost = _in.ft_cost;
  prob_exe = _in.prob_exe;
  num_args = _in.num_args;
}

cInstEntry::~cInstEntry()
{
}

void cInstEntry::Init(const cString & in_name, tCPUMethod in_function,
		      int in_num_args, int in_redundancy, int in_cost,
		      int in_ft_cost, double in_prob_exe){
  name = in_name;
  function = in_function;
  alt_function = (tCPUMethod) NULL;
  redundancy = in_redundancy;
  cost = in_cost;
  ft_cost = in_ft_cost;
  prob_exe = in_prob_exe;
  num_args = in_num_args;
}

void cInstEntry::operator=(const cInstEntry & _in)
{
  name = _in.name;
  function = _in.function;
  alt_function = _in.alt_function;
  redundancy = _in.redundancy;
  cost = _in.cost;
  ft_cost = _in.ft_cost;
  prob_exe = _in.prob_exe;
  num_args = _in.num_args;
}

//////////////////////
//  cInstLib
//////////////////////

cInstLib::cInstLib()
{
  num_instructions = 0;
  num_nops = 0;
  max_size = INST_BLOCK_SIZE;
  max_nops = NOP_BLOCK_SIZE;
  num_args = 0;
  mode = INST_MODE_BASE;
  type = INSTLIB_TYPE_DEFAULT;

  inst_array = new cInstEntry[INST_BLOCK_SIZE];
  nop_mods = new int[NOP_BLOCK_SIZE];
}

cInstLib::cInstLib(cInstLib * in_inst_lib)
{
  num_instructions = in_inst_lib->GetSize();
  num_nops = in_inst_lib->GetNumNops();
  max_size = num_instructions;
  max_nops = num_nops;
  num_args = in_inst_lib->num_args;
  mode = in_inst_lib->GetMode();

  inst_array = new cInstEntry[max_size];
  nop_mods = new int[num_nops];

  for (int i = 0; i < num_instructions; i++) {
    inst_array[i] = in_inst_lib->inst_array[i];
  }
  for (int j = 0; j < num_nops; j++) {
    nop_mods[j] = in_inst_lib->nop_mods[j];
  }
}

cInstLib::~cInstLib()
{
  delete [] inst_array;
  delete [] nop_mods;
}

cInstLib & cInstLib::operator=(const cInstLib & in_inst_lib)
{
  num_instructions = in_inst_lib.GetSize();
  num_nops = in_inst_lib.GetNumNops();
  max_size = num_instructions;
  max_nops = num_nops;
  num_args = in_inst_lib.num_args;
  mode = in_inst_lib.GetMode();

  inst_array = new cInstEntry[max_size];
  nop_mods = new int[num_nops];

  for (int i = 0; i < num_instructions; i++) {
    inst_array[i] = in_inst_lib.inst_array[i];
  }
  for (int j = 0; j < num_nops; j++) {
    nop_mods[j] = in_inst_lib.nop_mods[j];
  }

  return *this;
}

void cInstLib::IncreaseMaxInst()
{
#ifdef DEBUG
  if (max_size != num_instructions) {
    g_debug.Warning("Running IncreaseMaxInst() with max_size=%d & num_inst=%d",
		    max_size, num_instructions);
  }
#endif

  // Backup the old values....
  cInstEntry * old_insts = inst_array;

  // Setup the new value...
  max_size += INST_BLOCK_SIZE;
  inst_array = new cInstEntry[max_size];

  // Copy the info over...
  for (int i = 0; i < num_instructions; i++) {
    inst_array[i] = old_insts[i];
  }

  // Delete the old values...
  delete [] old_insts;
}

void cInstLib::IncreaseMaxNops()
{
#ifdef DEBUG
  if (max_nops != num_nops) {
    g_debug.Warning("Running IncreaseMaxNops() with max_nops=%d & num_nops=%d",
		    max_nops, num_nops);
  }
#endif

  // Backup the old values....
  int * old_nop_mods = nop_mods;

  // Setup the new value...
  max_nops += NOP_BLOCK_SIZE;
  nop_mods = new int[max_nops];

  // Copy the info over...
  for (int i = 0; i < num_nops; i++) {
    nop_mods[i] = old_nop_mods[i];
  }

  // Delete the old values...
  delete [] old_nop_mods;
}

void cInstLib::Add(char * in_name, tCPUMethod in_function, int in_num_args)
{
  if (num_instructions == max_size)  IncreaseMaxInst();
  
  inst_array[num_instructions].Init(in_name, in_function, in_num_args,
				    1, 0, 0, 1.0);
  // cerr<<"adding inst:"<<num_instructions<<" "<<in_name<<" "<<redundance<<" "<<cost<<" "<<ft_cost<<" "<<prob_exe<<endl;
  num_instructions++;
}

void cInstLib::Add(const cInitFile & file, char * in_name,
		   tCPUMethod in_function, int in_num_args)
{
  cString args = file.ReadString(in_name);
  if( args == "" ) return;

  int redundance  = args.PopWord().AsInt();
  if (redundance == 0) return;
  int    cost     = (args.GetSize() > 0 ) ? args.PopWord().AsInt() : 0;
  int    ft_cost  = (args.GetSize() > 0 ) ? args.PopWord().AsInt() : 0;
  double prob_exe = (args.GetSize() > 0 ) ? args.PopWord().AsDouble() : 1;

  if (num_instructions == max_size)  IncreaseMaxInst();

  inst_array[num_instructions].Init(in_name, in_function, in_num_args,
				    redundance, cost, ft_cost, prob_exe);
  // cerr<<"adding inst:"<<num_instructions<<" "<<in_name<<" "<<redundance<<" "<<cost<<" "<<ft_cost<<" "<<prob_exe<<endl;
  num_instructions++;
}

void cInstLib::AddNop(const cInitFile & file, char * in_name, int reg)
{
  cString args = file.ReadString(in_name);
  if( args == "" ) return;

  int redundance  = args.PopWord().AsInt();
  if (redundance == 0) return;
  int    cost     = (args.GetSize() > 0 ) ? args.PopWord().AsInt() : 0;
  int    ft_cost  = (args.GetSize() > 0 ) ? args.PopWord().AsInt() : 0;
  double prob_exe = (args.GetSize() > 0 ) ? args.PopWord().AsDouble() : 1;

#ifdef DEBUG
  if (num_instructions != num_nops) {
    g_debug.Error("Nops must be at the _beginning_ of an inst_set!");
  }
  if (num_instructions > max_size) {
    g_debug.Error("cInstLib => num_instructions(%d) > max size(%d)!",
		  num_instructions, max_size);
  }
  if (num_nops > max_nops) {
    g_debug.Error("cInstLib => num_nops(%d) > max_nops(%d)!",
		  num_nops, max_nops);
  }
  if (reg >= NUM_REGISTERS) {
    g_debug.Error("cInstLib => Nop's register(%d) > NUM_REG(%d)!",
		  reg, NUM_REGISTERS);
  }
#endif

  if (num_instructions == max_size) IncreaseMaxInst();
  if (num_nops == max_nops) IncreaseMaxNops();

  inst_array[num_instructions].Init(in_name, &cBaseCPU::Inst_Nop,
				    0, redundance, cost, ft_cost, prob_exe);
  nop_mods[num_instructions] = reg;
  num_instructions++;
  num_nops++;
}

void cInstLib::AddLabel(const cInitFile & file, char * in_name)
{
  // This works for the moment...
  AddNop(file, in_name, 0);
  inst_array[num_instructions - 1].SetFunction(&cBaseCPU::InstArg_Label);
}

void cInstLib::SetAltFunctions(tCPUMethod in_alt_function)
{
  for (int i = 0; i < max_size; i++) {
    inst_array[i].SetAltFunction(in_alt_function);
  }
}

// Return the ID of an instruction, or a ERROR if it cannot be found.
UCHAR cInstLib::GetInstID(const cString & in_name) const
{
  int inst;

  for (inst = 0; inst < num_instructions; inst++) {
    if(inst_array[inst].GetName() == in_name) break;
  }

  if (inst < num_instructions) return inst;

#ifdef DEBUG
  assert_debug(FALSE,
     cString::Stringf("Requesting ID for unknown inst [%s] in set size %d",
		      in_name(), num_instructions)());
#endif

  return INSTRUCTION_ERROR;
}


void cInstLib::SetupInstArg(const cString & name, int id,
			    cLongInstruction & in_arg) const
{
  int pos = 0;

  // Determine if we are dealing with a parasite.

  UCHAR comp_parasite = FALSE;
  if (name[pos] == '*') {
    comp_parasite = TRUE;
    pos++;
  }


  // Test the modification type.

  UCHAR mod_type = ARG_MOD_NONE_A;
  if (name.IsSubstring("++", pos)) {
    mod_type = ARG_MOD_INC;
    pos += 2;
  } else if (name.IsSubstring("--", pos)) {
    mod_type = ARG_MOD_DEC;
    pos += 2;
  } else if (name[pos] == '-') {
    mod_type = ARG_MOD_NEG;
    pos++;
  } else if (name[pos] == '~') {
    mod_type = ARG_MOD_BITNEG;
    pos++;
  } else if (name[pos] == '<') {
    mod_type = ARG_MOD_SHIFTL;
    pos++;
  } else if (name[pos] == '>') {
    mod_type = ARG_MOD_SHIFTR;
    pos++;
  }

  // Determine the type of arguments used.

  UCHAR arg_type = ARG_TYPE_VALUE_A;
  UCHAR comp_pointer = FALSE;

  if (name[pos] == '@') {
    comp_pointer = TRUE;
    pos++;
  }

  if (name.GetSize() && !name.IsUpperLetter(pos) && !name.IsNumber(pos)) {
    g_debug.Warning("Unknown arg: [%s]", name());
  }


  // If we're dealing with a component, determine its type.
  if (name.IsSubstring("REG", pos)) {
    arg_type = ARG_TYPE_REG;
    pos += 3;
  } else if (name.IsSubstring("IP", pos)) {
    arg_type = ARG_TYPE_IP;
    pos += 2;
  } else if (name.IsSubstring("STACK", pos)) {
    arg_type = ARG_TYPE_STACK;
    pos += 5;
  } else if (name.IsSubstring("IO", pos)) {
    arg_type = ARG_TYPE_IO;
    pos += 2;
  } else if (name.IsSubstring("MEM", pos)) {
    arg_type = ARG_TYPE_MEM;
    pos += 3;
  } else if (name.IsSubstring("LABEL", pos)) {
    arg_type = ARG_TYPE_LABEL;
    pos += 5;
  } else if (!name.IsNumber(pos) && name.GetSize() > 0){
    g_debug.Warning("Unknown Arg-Component in: [%s]", name());
  }

  int arg_value = 0;
  if ( (name[pos] == ':' && ++pos) || (arg_type == ARG_TYPE_VALUE_A)) {
    arg_value = name.GetWordAt(pos).AsInt();
  }

  // Finally, allow for negative direct arg values.
  if ( (arg_type == ARG_TYPE_VALUE_A) && (mod_type == ARG_MOD_NEG) ) {
    mod_type = ARG_MOD_NONE_A;
    arg_value *= -1;
  }

  cInstArgument arg_info(arg_value, arg_type, mod_type, comp_pointer,
			 comp_parasite);
  in_arg.SetArg(id, arg_info);
}


void cInstLib::SetupInstRed(const cString & name, int id,
			    cLongInstruction & in_arg) const
{
  int pos = 0;

  // Determine if we are dealing with a parasite.

  UCHAR comp_parasite = FALSE;
  if (name[pos] == '*') {
    comp_parasite = TRUE;
    pos++;
  }

  // Test the modification type.

  UCHAR type = REDCODE_OP_VALUE;
  if (name[pos] == '@') {
    type = REDCODE_OP_DIRECT;
    pos++;
  } else if (name[pos] == '$') {
    type = REDCODE_OP_INDIRECT;
    pos++;
  } else if (name[pos] == '>') {
    type = REDCODE_OP_DEC_INDIRECT;
    pos++;
  }

  int is_neg = FALSE;
  if (name[pos] == '-') {
    is_neg = TRUE;
    pos++;
  }

  if (name.GetSize() && !name.IsNumber(pos)) {
    g_debug.Warning("Unknown arg: [%s] (pos=%d)", name(), pos);
  }

  int arg_value = name.GetWordAt(pos).AsInt();

  if (is_neg == TRUE) arg_value *= -1;

  in_arg.SetArg(id, type, arg_value);
}


cInstruction cInstLib::GetInst(const cString & in_name) const
{
  if( in_name == "instruction_none" ){
    return cInstruction(INSTRUCTION_NONE);
  }

#ifndef INST_ARGS
  return cInstruction(GetInstID(in_name));
#else

  UINT id = GetInstID(in_name.GetWord(0));

  cInstruction inst(id);

  if (type == INSTLIB_TYPE_ARGS) {
    SetupInstArg(in_name.GetWord(1), 0, inst);
    SetupInstArg(in_name.GetWord(2), 1, inst);
  } else if (type == INSTLIB_TYPE_REDCODE) {
    SetupInstRed(in_name.GetWord(1), 0, inst);
    SetupInstRed(in_name.GetWord(2), 1, inst);
  }

  return inst;
#endif
}


void cInstLib::PrintCode(const cCodeArray & code, ofstream & fp) const
{
  for (int i = 0; i < code.GetSize(); i++) {
#ifdef INST_ARGS
    fp << GetName(code.Get(i));
    for (int j = 0; j < GetNumArgs(); j++) {
      fp << " " << code.Get(i).GetArg(j).AsString(this);
    }

#else
    fp << GetName(code.Get(i));
#endif

    fp << "\n";
  }
}



cInstLib * cInstLib::InitInstructions(const char * inst_filename)
{
  static cInstLib lib;
  if (!inst_filename || lib.GetSize()) return &lib;

  cInitFile file(inst_filename);
  file.Load();
  file.Compress();

  cString lib_type = file.ReadString("SET_TYPE","BASE_DEFAULT");

  if (lib_type == "BASE_ARG") {
    lib.num_args = NUM_INST_ARGS;
    lib.type = INSTLIB_TYPE_ARGS;
    InitArgsInst(file, lib);
  } else if (lib_type == "BASE_BINARY") {
    lib.num_args = 0;
    lib.type = INSTLIB_TYPE_BINARY;
    InitBinaryInst(file, lib);
  } else if (lib_type == "BASE_REDCODE") {
    lib.num_args = 2;
    lib.type = INSTLIB_TYPE_REDCODE;
    InitRedcodeInst(file, lib);
  } else if (lib_type == "BASE_DEFAULT") {
    lib.num_args = 0;
    lib.type = INSTLIB_TYPE_DEFAULT;
    InitBaseInst(file, lib);
  } else {
    cerr << "Unknown instruction  set type: \"" << lib_type << "...\n";
    exit(0);
  }

  lib.SetAltFunctions(&cBaseCPU::Notify);

  return &lib;
}

void cInstLib::InitBinaryInst(const cInitFile & file, cInstLib & lib)
{
  lib.Add(file, "rep-0", &cBaseCPU::Inst_Bin_Repro);
  lib.Add(file, "rep-1", &cBaseCPU::Inst_Bin_Repro);
}


void cInstLib::InitArgsInst(const cInitFile & file, cInstLib & lib)
{
  lib.AddLabel(file, "label");

  lib.Add(file, "nop",     &cBaseCPU::InstArg_Nop,      0);
  lib.Add(file, "allocate",&cBaseCPU::InstArg_Allocate, 1);
  lib.Add(file, "divide",  &cBaseCPU::InstArg_Divide,   1);
  lib.Add(file, "rotate",  &cBaseCPU::InstArg_Rotate,   1);

  lib.Add(file, "add",     &cBaseCPU::InstArg_Add,   2);
  lib.Add(file, "nand",    &cBaseCPU::InstArg_Nand,  2);
  lib.Add(file, "if-equ",  &cBaseCPU::InstArg_IfEqu, 2);
  lib.Add(file, "if-less", &cBaseCPU::InstArg_IfLess,2);

  lib.Add(file, "copy",    &cBaseCPU::InstArg_Copy,  2);
  lib.Add(file, "swap",    &cBaseCPU::InstArg_Swap,  2);
  lib.Add(file, "shift",   &cBaseCPU::InstArg_Shift, 2);
}


void cInstLib::InitRedcodeInst(const cInitFile & file, cInstLib & lib)
{
  lib.Add(file, "dat", &cBaseCPU::InstRed_Data,         0);
  lib.Add(file, "mov", &cBaseCPU::InstRed_Move,         2);
  lib.Add(file, "add", &cBaseCPU::InstRed_Add,          2);
  lib.Add(file, "sub", &cBaseCPU::InstRed_Sub,          2);
  lib.Add(file, "jmp", &cBaseCPU::InstRed_Jump,         1);
  lib.Add(file, "jmz", &cBaseCPU::InstRed_JumpZero,     2);
  lib.Add(file, "jmn", &cBaseCPU::InstRed_JumpNZero,    2);
  lib.Add(file, "djn", &cBaseCPU::InstRed_DecJumpNZero, 2);
  lib.Add(file, "cmp", &cBaseCPU::InstRed_Compare,      2);
  lib.Add(file, "spl", &cBaseCPU::InstRed_Split,        1);
  lib.Add(file, "slt", &cBaseCPU::InstRed_SkipLess,     2);
  lib.Add(file, "mal", &cBaseCPU::InstRed_Allocate,     1);
  lib.Add(file, "div", &cBaseCPU::InstRed_Divide,       1);
  lib.Add(file, "get", &cBaseCPU::InstRed_Get,          1);
  lib.Add(file, "put", &cBaseCPU::InstRed_Put,          1);
  lib.Add(file, "nand", &cBaseCPU::InstRed_Nand,        2);
}


void cInstLib::InitBaseInst(const cInitFile & file, cInstLib & lib)
{
  // Nops (or other modifying-instructions...)  Note: Nops must be the
  //  first instructions added to the set.
  lib.AddNop(file, "nop-A", REG_AX);
  lib.AddNop(file, "nop-B", REG_BX);
  lib.AddNop(file, "nop-C", REG_CX);

  // Remaining instructions.
  lib.Add(file, "nop-X", &cBaseCPU::Inst_Nop);
  lib.Add(file, "if-equ-0", &cBaseCPU::Inst_If0);
  lib.Add(file, "if-not-0", &cBaseCPU::Inst_IfNot0);
  lib.Add(file, "if-n-equ", &cBaseCPU::Inst_IfNEqu);
  lib.Add(file, "if-equ",   &cBaseCPU::Inst_IfEqu);
  lib.Add(file, "if-grt-0", &cBaseCPU::Inst_IfGr0);
  lib.Add(file, "if-grt",   &cBaseCPU::Inst_IfGr);
  lib.Add(file, "if->=-0",  &cBaseCPU::Inst_IfGrEqu0);
  lib.Add(file, "if->=",    &cBaseCPU::Inst_IfGrEqu);
  lib.Add(file, "if-les-0", &cBaseCPU::Inst_IfLess0);
  lib.Add(file, "if-less",  &cBaseCPU::Inst_IfLess);
  lib.Add(file, "if-<=-0",  &cBaseCPU::Inst_IfLsEqu0);
  lib.Add(file, "if-<=",    &cBaseCPU::Inst_IfLsEqu);
  lib.Add(file, "if-A!=B",  &cBaseCPU::Inst_IfANotEqB);
  lib.Add(file, "if-B!=C",  &cBaseCPU::Inst_IfBNotEqC);
  lib.Add(file, "if-A!=C",  &cBaseCPU::Inst_IfANotEqC);

  lib.Add(file, "if-bit-1", &cBaseCPU::Inst_IfBit1);
  lib.Add(file, "jump-f",   &cBaseCPU::Inst_JumpF);
  lib.Add(file, "jump-b",   &cBaseCPU::Inst_JumpB);
  lib.Add(file, "jump-p",   &cBaseCPU::Inst_JumpP);
  lib.Add(file, "jump-slf", &cBaseCPU::Inst_JumpSelf);
  lib.Add(file, "call",     &cBaseCPU::Inst_Call);
  lib.Add(file, "return",   &cBaseCPU::Inst_Return);

  lib.Add(file, "pop",     &cBaseCPU::Inst_Pop);
  lib.Add(file, "push",    &cBaseCPU::Inst_Push);
  lib.Add(file, "swap-stk",&cBaseCPU::Inst_SwitchStack);
  lib.Add(file, "flip-stk",&cBaseCPU::Inst_FlipStack);
  lib.Add(file, "swap",    &cBaseCPU::Inst_Swap);
  lib.Add(file, "swap-AB", &cBaseCPU::Inst_SwapAB);
  lib.Add(file, "swap-BC", &cBaseCPU::Inst_SwapBC);
  lib.Add(file, "swap-AC", &cBaseCPU::Inst_SwapAC);
  lib.Add(file, "copy-reg",&cBaseCPU::Inst_CopyReg);
  lib.Add(file, "set_A=B", &cBaseCPU::Inst_CopyRegAB);
  lib.Add(file, "set_A=C", &cBaseCPU::Inst_CopyRegAC);
  lib.Add(file, "set_B=A", &cBaseCPU::Inst_CopyRegBA);
  lib.Add(file, "set_B=C", &cBaseCPU::Inst_CopyRegBC);
  lib.Add(file, "set_C=A", &cBaseCPU::Inst_CopyRegCA);
  lib.Add(file, "set_C=B", &cBaseCPU::Inst_CopyRegCB);
  lib.Add(file, "reset",   &cBaseCPU::Inst_Reset);

  lib.Add(file, "pop-A",  &cBaseCPU::Inst_PopA);
  lib.Add(file, "pop-B",  &cBaseCPU::Inst_PopB);
  lib.Add(file, "pop-C",  &cBaseCPU::Inst_PopC);
  lib.Add(file, "push-A", &cBaseCPU::Inst_PushA);
  lib.Add(file, "push-B", &cBaseCPU::Inst_PushB);
  lib.Add(file, "push-C", &cBaseCPU::Inst_PushC);

  lib.Add(file, "shift-r", &cBaseCPU::Inst_ShiftR);
  lib.Add(file, "shift-l", &cBaseCPU::Inst_ShiftL);
  lib.Add(file, "bit-1",   &cBaseCPU::Inst_Bit1);
  lib.Add(file, "set-num", &cBaseCPU::Inst_SetNum);
  lib.Add(file, "inc",     &cBaseCPU::Inst_Inc);
  lib.Add(file, "dec",     &cBaseCPU::Inst_Dec);
  lib.Add(file, "zero",    &cBaseCPU::Inst_Zero);
  lib.Add(file, "neg",     &cBaseCPU::Inst_Neg);
  lib.Add(file, "square",  &cBaseCPU::Inst_Square);
  lib.Add(file, "sqrt",    &cBaseCPU::Inst_Sqrt);
  lib.Add(file, "not",     &cBaseCPU::Inst_Not);

  lib.Add(file, "add",     &cBaseCPU::Inst_Add);
  lib.Add(file, "sub",     &cBaseCPU::Inst_Sub);
  lib.Add(file, "mult",    &cBaseCPU::Inst_Mult);
  lib.Add(file, "div",     &cBaseCPU::Inst_Div);
  lib.Add(file, "mod",     &cBaseCPU::Inst_Mod);
  lib.Add(file, "nand",    &cBaseCPU::Inst_Nand);
  lib.Add(file, "nor",     &cBaseCPU::Inst_Nor);
  lib.Add(file, "and",     &cBaseCPU::Inst_And);
  lib.Add(file, "order",   &cBaseCPU::Inst_Order);
  lib.Add(file, "xor",     &cBaseCPU::Inst_Xor);

  lib.Add(file, "copy",  &cBaseCPU::Inst_Copy);
  lib.Add(file, "read",   &cBaseCPU::Inst_ReadInst);
  lib.Add(file, "write", &cBaseCPU::Inst_WriteInst);
  lib.Add(file, "stk-read", &cBaseCPU::Inst_StackReadInst);
  lib.Add(file, "stk-writ", &cBaseCPU::Inst_StackWriteInst);

  lib.Add(file, "compare",  &cBaseCPU::Inst_Compare);
  lib.Add(file, "if-n-cpy", &cBaseCPU::Inst_IfNCpy);
  lib.Add(file, "allocate", &cBaseCPU::Inst_Allocate);
  lib.Add(file, "divide",   &cBaseCPU::Inst_Divide);
  lib.Add(file, "c-alloc",  &cBaseCPU::Inst_CAlloc);
  lib.Add(file, "c-divide", &cBaseCPU::Inst_CDivide);
  lib.Add(file, "inject",   &cBaseCPU::Inst_Inject);

  lib.Add(file, "get",      &cBaseCPU::Inst_TaskGet);
  lib.Add(file, "stk-get",  &cBaseCPU::Inst_TaskStackGet);
  lib.Add(file, "stk-load", &cBaseCPU::Inst_TaskStackLoad);
  lib.Add(file, "put",      &cBaseCPU::Inst_TaskPut);
  lib.Add(file, "IO",       &cBaseCPU::Inst_TaskIO);
  lib.Add(file, "search-f", &cBaseCPU::Inst_SearchF);
  lib.Add(file, "search-b", &cBaseCPU::Inst_SearchB);
  lib.Add(file, "mem-size", &cBaseCPU::Inst_MemSize);

  lib.Add(file, "rotate-l", &cBaseCPU::Inst_RotateL);
  lib.Add(file, "rotate-r", &cBaseCPU::Inst_RotateR);

  lib.Add(file, "set-cmut",&cBaseCPU::Inst_SetCopyMut);
  lib.Add(file, "mod-cmut",&cBaseCPU::Inst_ModCopyMut);

  // Threading instructions
  lib.Add(file, "fork-th",&cBaseCPU::Inst_ForkThread);
  lib.Add(file, "kill-th",&cBaseCPU::Inst_KillThread);
  lib.Add(file, "id-th",  &cBaseCPU::Inst_ThreadID);

  // Head-based instructions
  lib.Add(file, "h-alloc",  &cBaseCPU::Inst_MaxAlloc);
  lib.Add(file, "h-divide", &cBaseCPU::Inst_HeadDivide);
  lib.Add(file, "h-read",   &cBaseCPU::Inst_HeadRead);
  lib.Add(file, "h-write",  &cBaseCPU::Inst_HeadWrite);
  lib.Add(file, "h-copy",   &cBaseCPU::Inst_HeadCopy);
  lib.Add(file, "h-search", &cBaseCPU::Inst_HeadSearch);  
  lib.Add(file, "h-push",   &cBaseCPU::Inst_HeadPush);  
  lib.Add(file, "h-pop",    &cBaseCPU::Inst_HeadPop);  
  lib.Add(file, "set-head", &cBaseCPU::Inst_SetHead);
  lib.Add(file, "adv-head", &cBaseCPU::Inst_AdvanceHead);
  lib.Add(file, "mov-head", &cBaseCPU::Inst_MoveHead);
  lib.Add(file, "jmp-head", &cBaseCPU::Inst_JumpHead);
  lib.Add(file, "get-head", &cBaseCPU::Inst_GetHead);
  lib.Add(file, "if-label", &cBaseCPU::Inst_IfLabel);
  lib.Add(file, "set-flow", &cBaseCPU::Inst_SetFlow);

  // High-level instructions
  lib.Add(file, "repro",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-A",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-B",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-C",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-D",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-E",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-F",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-G",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-H",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-I",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-J",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-K",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-L",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-M",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-N",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-O",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-P",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-Q",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-R",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-S",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-T",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-U",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-V",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-W",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-X",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-Y",   &cBaseCPU::Inst_Repro);
  lib.Add(file, "repro-Z",   &cBaseCPU::Inst_Repro);

  // Direct Matching Instructions
  lib.Add(file, "dm-jp-f", &cBaseCPU::Inst_DMJumpF);
  lib.Add(file, "dm-jp-b", &cBaseCPU::Inst_DMJumpB);
  lib.Add(file, "dm-call", &cBaseCPU::Inst_DMCall);
  lib.Add(file, "dm-sch-f",&cBaseCPU::Inst_DMSearchF);
  lib.Add(file, "dm-sch-b",&cBaseCPU::Inst_DMSearchB);

  // Relative Addressed Jumps
  lib.Add(file, "re-jp-f", &cBaseCPU::Inst_REJumpF);
  lib.Add(file, "re-jp-b", &cBaseCPU::Inst_REJumpB);

  // Absoulte Addressed Jumps
  lib.Add(file, "abs-jp", &cBaseCPU::Inst_ABSJump);

  // Biologically inspired reproduction
  lib.Add(file, "bc-alloc", &cBaseCPU::Inst_BCAlloc);
  lib.Add(file, "b-copy",   &cBaseCPU::Inst_BCopy);
  lib.Add(file, "b-divide", &cBaseCPU::Inst_BDivide);
  // Bio Error Correction
  lib.Add(file, "b-cpy/2",  &cBaseCPU::Inst_BCopyDiv2);
  lib.Add(file, "b-cpy/3",  &cBaseCPU::Inst_BCopyDiv3);
  lib.Add(file, "b-cpy/4",  &cBaseCPU::Inst_BCopyDiv4);
  lib.Add(file, "b-cpy/5",  &cBaseCPU::Inst_BCopyDiv5);
  lib.Add(file, "b-cpy/6",  &cBaseCPU::Inst_BCopyDiv6);
  lib.Add(file, "b-cpy/7",  &cBaseCPU::Inst_BCopyDiv7);
  lib.Add(file, "b-cpy/8",  &cBaseCPU::Inst_BCopyDiv8);
  lib.Add(file, "b-cpy/9",  &cBaseCPU::Inst_BCopyDiv9);
  lib.Add(file, "b-cpy/10", &cBaseCPU::Inst_BCopyDiv10);
  lib.Add(file, "b-cpy^2",  &cBaseCPU::Inst_BCopyPow2);
  lib.Add(file, "b-ifncpy", &cBaseCPU::Inst_BIfNotCopy);
  lib.Add(file, "b-if-cpy", &cBaseCPU::Inst_BIfCopy);

  // Placebo instructions
  // nop-x (included with nops)
  lib.Add(file, "skip", &cBaseCPU::Inst_Skip);

}
