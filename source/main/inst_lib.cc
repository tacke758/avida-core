//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "inst_lib.hh"
#include "../cpu/cpu.hh"
#include "../cpu/head.ii"

//////////////////////
//  cInstLib
//////////////////////

cInstLib::cInstLib(int in_max_size)
{
  num_instructions = 0;
  max_size = in_max_size;
}

cInstLib::cInstLib(cInstLib * in_inst_lib)
{
  int i;

  max_size = in_inst_lib->GetSize();
  num_instructions = max_size;

  for (i = 0; i < num_instructions; i++) {
    name[i] = in_inst_lib->name[i];
    function[i] = in_inst_lib->function[i];
    alt_function[i] = in_inst_lib->alt_function[i];
  }
}

cInstLib::~cInstLib()
{
}

void cInstLib::Add(char * in_name, tCPUMethod in_function)
{
  name[num_instructions] = in_name;
  function[num_instructions] = in_function;
  num_instructions++;
}

void cInstLib::SetAltFunctions(tCPUMethod in_alt_function)
{
  for (int i = 0; i < MAX_INST_SET_SIZE; i++) {
    alt_function[i] = in_alt_function;
  }
}

void cInstLib::ActivateAltFunctions()
{
  int i;
  tCPUMethod temp_function;
  for (i = 0; i < MAX_INST_SET_SIZE; i++) {
    temp_function = function[i];
    function[i] = alt_function[i];
    alt_function[i] = temp_function;
  }

  flags.FlipFlag(INST_FLAG_ALT_MODE);
}


////////////////////////
//  Other functions...
////////////////////////

cInstLib * InitInstructions(const char * inst_filename)
{
  static cInstLib ilib(MAX_INST_SET_SIZE);
  if (!inst_filename || ilib.GetSize()) return &ilib;

  cGenesis iload(inst_filename);

  if (iload.ReadInt("nop-A"))    ilib.Add("nop-A",    &cBaseCPU::Inst_Nop);
  if (iload.ReadInt("nop-B"))    ilib.Add("nop-B",    &cBaseCPU::Inst_Nop);
  if (iload.ReadInt("nop-C"))    ilib.Add("nop-C",    &cBaseCPU::Inst_Nop);
  if (iload.ReadInt("nop-X"))    ilib.Add("nop-X",    &cBaseCPU::Inst_Nop);
  if (iload.ReadInt("if-not-0")) ilib.Add("if-not-0", &cBaseCPU::Inst_IfNot0);
  if (iload.ReadInt("if-n-equ")) ilib.Add("if-n-equ", &cBaseCPU::Inst_IfNEqu);
  if (iload.ReadInt("if-bit-1")) ilib.Add("if-bit-1", &cBaseCPU::Inst_IfBit1);
  if (iload.ReadInt("jump-f"))   ilib.Add("jump-f",   &cBaseCPU::Inst_JumpF);
  if (iload.ReadInt("jump-b"))   ilib.Add("jump-b",   &cBaseCPU::Inst_JumpB);
  if (iload.ReadInt("jump-p"))   ilib.Add("jump-p",   &cBaseCPU::Inst_JumpP);
  if (iload.ReadInt("call"))     ilib.Add("call",     &cBaseCPU::Inst_Call);
  if (iload.ReadInt("return"))   ilib.Add("return",   &cBaseCPU::Inst_Return);
  if (iload.ReadInt("shift-r"))  ilib.Add("shift-r",  &cBaseCPU::Inst_ShiftR);
  if (iload.ReadInt("shift-l"))  ilib.Add("shift-l",  &cBaseCPU::Inst_ShiftL);
  if (iload.ReadInt("bit-1"))    ilib.Add("bit-1",    &cBaseCPU::Inst_Bit1);
  if (iload.ReadInt("set-num"))  ilib.Add("set-num",  &cBaseCPU::Inst_SetNum);
  if (iload.ReadInt("inc"))      ilib.Add("inc",      &cBaseCPU::Inst_Inc);
  if (iload.ReadInt("dec"))      ilib.Add("dec",      &cBaseCPU::Inst_Dec);
  if (iload.ReadInt("zero"))     ilib.Add("zero",     &cBaseCPU::Inst_Zero);
  if (iload.ReadInt("pop"))      ilib.Add("pop",      &cBaseCPU::Inst_Pop);
  if (iload.ReadInt("push"))     ilib.Add("push",     &cBaseCPU::Inst_Push);
  if (iload.ReadInt("add"))      ilib.Add("add",      &cBaseCPU::Inst_Add);
  if (iload.ReadInt("sub"))      ilib.Add("sub",      &cBaseCPU::Inst_Sub);
  if (iload.ReadInt("nand"))     ilib.Add("nand",     &cBaseCPU::Inst_Nand);
  if (iload.ReadInt("nor"))      ilib.Add("nor",      &cBaseCPU::Inst_Nor);
  if (iload.ReadInt("and"))      ilib.Add("and",      &cBaseCPU::Inst_And);
  if (iload.ReadInt("not"))      ilib.Add("not",      &cBaseCPU::Inst_Not);
  if (iload.ReadInt("order"))    ilib.Add("order",    &cBaseCPU::Inst_Order);
  if (iload.ReadInt("xor"))      ilib.Add("xor",      &cBaseCPU::Inst_Xor);
  if (iload.ReadInt("copy"))     ilib.Add("copy",     &cBaseCPU::Inst_Copy);
  if (iload.ReadInt("read"))     ilib.Add("read",   &cBaseCPU::Inst_ReadInst);
  if (iload.ReadInt("write"))    ilib.Add("write",  &cBaseCPU::Inst_WriteInst);
  if (iload.ReadInt("compare"))  ilib.Add("compare",  &cBaseCPU::Inst_Compare);
  if (iload.ReadInt("if-n-cpy")) ilib.Add("if-n-cpy", &cBaseCPU::Inst_IfNCpy);
  if (iload.ReadInt("allocate")) ilib.Add("allocate",&cBaseCPU::Inst_Allocate);
  if (iload.ReadInt("divide"))   ilib.Add("divide",   &cBaseCPU::Inst_Divide);
  if (iload.ReadInt("c-alloc"))  ilib.Add("c-alloc",  &cBaseCPU::Inst_CAlloc);
  if (iload.ReadInt("c-divide")) ilib.Add("c-divide", &cBaseCPU::Inst_CDivide);
  if (iload.ReadInt("inject"))   ilib.Add("inject",   &cBaseCPU::Inst_Inject);
  if (iload.ReadInt("get"))      ilib.Add("get",      &cBaseCPU::Inst_TaskGet);
  if (iload.ReadInt("put"))      ilib.Add("put",      &cBaseCPU::Inst_TaskPut);
  if (iload.ReadInt("search-f")) ilib.Add("search-f", &cBaseCPU::Inst_SearchF);
  if (iload.ReadInt("search-b")) ilib.Add("search-b", &cBaseCPU::Inst_SearchB);
  if (iload.ReadInt("rotate-l")) ilib.Add("rotate-l", &cBaseCPU::Inst_RotateL);
  if (iload.ReadInt("rotate-r")) ilib.Add("rotate-r", &cBaseCPU::Inst_RotateR);
  if (iload.ReadInt("set-cmut"))
    ilib.Add("set-cmut", &cBaseCPU::Inst_SetCopyMut);
  if (iload.ReadInt("mod-cmut"))
    ilib.Add("mod-cmut", &cBaseCPU::Inst_ModCopyMut);

  ilib.SetAltFunctions(&cBaseCPU::Notify);

  return &ilib;
}
