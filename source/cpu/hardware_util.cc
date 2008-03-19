//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "../tools/string.hh"
#include "../tools/file.hh"

#include "../main/inst_set.hh"
#include "../main/inst_util.hh"
#include "../main/config.hh"

#include "hardware_method.hh"
#include "hardware_util.hh"

#include "hardware_base.hh"
#include "hardware_cpu.hh"
#include "hardware_4stack.hh"

using namespace std;


#ifdef USE_INST_SET_CODE
void cHardwareUtil::LoadInstSet(cString filename, cInstSet & inst_set, 
				tDictionary<int> & nop_dict,
				tDictionary<int> & inst_dict
){
#else /* USE_INST_SET_CODE */
void cHardwareUtil::LoadInstSet(cString & filename, cInstSet & inst_set, 
				tDictionary<int> & nop_dict,
				tDictionary<tHardwareMethod> & inst_dict
){
#endif /* USE_INST_SET_CODE */
  // If there is no filename, use the default for the appropriate hardware.
  cString default_filename = "unknown";
  if (cConfig::GetHardwareType() == HARDWARE_TYPE_CPU_ORIGINAL) {
    default_filename = cHardwareCPU::GetDefaultInstFilename();
  }
  else if (cConfig::GetHardwareType() == HARDWARE_TYPE_CPU_4STACK) {
    default_filename = cHardware4Stack::GetDefaultInstFilename();
  }

  if (filename == "") {
    cerr << "Warning: No instruction set specified; using default '"
	 << filename << "'." << endl;
    filename = default_filename;
  }

  cInitFile file(filename);

  // If we could not open the instruction set what to do?
  if (file.IsOpen() == false) {

    // If this is the default filename, write the file and try again.
    if (filename == default_filename) {
      if (cConfig::GetHardwareType() == HARDWARE_TYPE_CPU_ORIGINAL) {
	cHardwareCPU::WriteDefaultInstSet();
      }
      else if (cConfig::GetHardwareType() == HARDWARE_TYPE_CPU_4STACK) {
	cHardware4Stack::WriteDefaultInstSet();
      }      
    }

    // If this is not the default filename, give and error and stop.
    else {
      cerr << "Error: Could not open instruction set '" << filename
	   << "'.  Halting." << endl;
      exit(1);
    }
  }

  file.Load();
  file.Compress();

  for (int line_id = 0; line_id < file.GetNumLines(); line_id++) {
    cString cur_line = file.GetLine(line_id);
    cString inst_name = cur_line.PopWord();
    int redundancy = cur_line.PopWord().AsInt();
    int cost = cur_line.PopWord().AsInt();
    int ft_cost = cur_line.PopWord().AsInt();
    double prob_fail = cur_line.PopWord().AsDouble();

    // If this instruction has 0 redundancy, we don't want it!
    if (redundancy < 0) continue;
    if (redundancy > 256) {
      cerr << "Error: Max redundancy is 256.  Resetting redundancy of \""
	   << inst_name << "\" from " << redundancy << " to 256." << endl;
      redundancy = 256;
    }
    
    // Otherwise, this instruction will be in the set.
    // First, determine if it is a nop...
#ifdef USE_INST_SET_CODE
    /* XXX start -- kgn */
    int nop_mod = -1;
    if(nop_dict.Find(inst_name, nop_mod) == true) {
      inst_set.AddNop2(nop_mod, redundancy, ft_cost, cost, prob_fail);
      continue;
    }
    /* XXX end */
#else /* USE_INST_SET_CODE */
    int nop_mod = -1;
    /* XXX start -- kgn */
    int nop_mod_2 = -1;
    /* XXX end */
    if (nop_dict.Find(inst_name, nop_mod) == true) {
      inst_set.AddNop(inst_name, &cHardwareBase::Inst_Nop, nop_mod,
		      redundancy, ft_cost, cost, prob_fail);
      continue;
    }
#endif /* USE_INST_SET_CODE */

    // Otherwise, it had better be in the main dictionary...
#ifdef USE_INST_SET_CODE
    /* XXX start -- kgn */
    int fun_id = -1;
    if(inst_dict.Find(inst_name, fun_id) == true){
      inst_set.Add2(fun_id, redundancy, ft_cost, cost, prob_fail);
      continue;
    }
    /* XXX end */
#else /* USE_INST_SET_CODE */
    tHardwareMethod cpu_method = NULL;
    /* XXX start -- kgn */
    int fun_id = -1;
    /* XXX end */
    if (inst_dict.Find(inst_name, cpu_method) == true) {
      inst_set.Add(inst_name, cpu_method, redundancy, ft_cost, cost, prob_fail);
      continue;
    }
#endif /* USE_INST_SET_CODE */

    // Oh oh!  Didn't find an instruction!
    cerr << endl
	 << "Error: Could not find instruction '" << inst_name << "'" << endl
	 << "       (Best match = '"
	 << inst_dict.NearMatch(inst_name) << "')" << endl;
    exit(1);
  }

  cerr << "Loaded Instruction Library \"" << filename
       << "\" with " << inst_set.GetSize() << " instructions." << endl;
}

void cHardwareUtil::LoadInstSet_CPUOriginal(const cString & filename, cInstSet & inst_set)
{
  // Nops (or other modifying-instructions...)  Note: Nops must be the
  //  first instructions added to the set.
#ifndef USE_INST_SET_CODE
  tDictionary< int > nop_dict;
  nop_dict.Add("nop-A", REG_AX);
  nop_dict.Add("nop-B", REG_BX);
  nop_dict.Add("nop-C", REG_CX);
#endif /* !USE_INST_SET_CODE */

  // Build a dictionary of instructions and their corresponding methods...
#ifndef USE_INST_SET_CODE
  tDictionary< tHardwareMethod > inst_dict;

  // Remaining instructions.
  inst_dict.Add("nop-X",    (tHardwareMethod) &cHardwareCPU::Inst_Nop);
  inst_dict.Add("if-equ-0", (tHardwareMethod) &cHardwareCPU::Inst_If0);
  inst_dict.Add("if-not-0", (tHardwareMethod) &cHardwareCPU::Inst_IfNot0);
  inst_dict.Add("if-n-equ", (tHardwareMethod) &cHardwareCPU::Inst_IfNEqu);
  inst_dict.Add("if-equ",   (tHardwareMethod) &cHardwareCPU::Inst_IfEqu);
  inst_dict.Add("if-grt-0", (tHardwareMethod) &cHardwareCPU::Inst_IfGr0);
  inst_dict.Add("if-grt",   (tHardwareMethod) &cHardwareCPU::Inst_IfGr);
  inst_dict.Add("if->=-0",  (tHardwareMethod) &cHardwareCPU::Inst_IfGrEqu0);
  inst_dict.Add("if->=",    (tHardwareMethod) &cHardwareCPU::Inst_IfGrEqu);
  inst_dict.Add("if-les-0", (tHardwareMethod) &cHardwareCPU::Inst_IfLess0);
  inst_dict.Add("if-less",  (tHardwareMethod) &cHardwareCPU::Inst_IfLess);
  inst_dict.Add("if-<=-0",  (tHardwareMethod) &cHardwareCPU::Inst_IfLsEqu0);
  inst_dict.Add("if-<=",    (tHardwareMethod) &cHardwareCPU::Inst_IfLsEqu);
  inst_dict.Add("if-A!=B",  (tHardwareMethod) &cHardwareCPU::Inst_IfANotEqB);
  inst_dict.Add("if-B!=C",  (tHardwareMethod) &cHardwareCPU::Inst_IfBNotEqC);
  inst_dict.Add("if-A!=C",  (tHardwareMethod) &cHardwareCPU::Inst_IfANotEqC);

  inst_dict.Add("if-bit-1", (tHardwareMethod) &cHardwareCPU::Inst_IfBit1);
  inst_dict.Add("jump-f",   (tHardwareMethod) &cHardwareCPU::Inst_JumpF);
  inst_dict.Add("jump-b",   (tHardwareMethod) &cHardwareCPU::Inst_JumpB);
  inst_dict.Add("jump-p",   (tHardwareMethod) &cHardwareCPU::Inst_JumpP);
  inst_dict.Add("jump-slf", (tHardwareMethod) &cHardwareCPU::Inst_JumpSelf);
  inst_dict.Add("call",     (tHardwareMethod) &cHardwareCPU::Inst_Call);
  inst_dict.Add("return",   (tHardwareMethod) &cHardwareCPU::Inst_Return);

  inst_dict.Add("pop",     (tHardwareMethod) &cHardwareCPU::Inst_Pop);
  inst_dict.Add("push",    (tHardwareMethod) &cHardwareCPU::Inst_Push);
  inst_dict.Add("swap-stk",(tHardwareMethod) &cHardwareCPU::Inst_SwitchStack);
  inst_dict.Add("flip-stk",(tHardwareMethod) &cHardwareCPU::Inst_FlipStack);
  inst_dict.Add("swap",    (tHardwareMethod) &cHardwareCPU::Inst_Swap);
  inst_dict.Add("swap-AB", (tHardwareMethod) &cHardwareCPU::Inst_SwapAB);
  inst_dict.Add("swap-BC", (tHardwareMethod) &cHardwareCPU::Inst_SwapBC);
  inst_dict.Add("swap-AC", (tHardwareMethod) &cHardwareCPU::Inst_SwapAC);
  inst_dict.Add("copy-reg",(tHardwareMethod) &cHardwareCPU::Inst_CopyReg);
  inst_dict.Add("set_A=B", (tHardwareMethod) &cHardwareCPU::Inst_CopyRegAB);
  inst_dict.Add("set_A=C", (tHardwareMethod) &cHardwareCPU::Inst_CopyRegAC);
  inst_dict.Add("set_B=A", (tHardwareMethod) &cHardwareCPU::Inst_CopyRegBA);
  inst_dict.Add("set_B=C", (tHardwareMethod) &cHardwareCPU::Inst_CopyRegBC);
  inst_dict.Add("set_C=A", (tHardwareMethod) &cHardwareCPU::Inst_CopyRegCA);
  inst_dict.Add("set_C=B", (tHardwareMethod) &cHardwareCPU::Inst_CopyRegCB);
  inst_dict.Add("reset",   (tHardwareMethod) &cHardwareCPU::Inst_Reset);

  inst_dict.Add("pop-A",  (tHardwareMethod) &cHardwareCPU::Inst_PopA);
  inst_dict.Add("pop-B",  (tHardwareMethod) &cHardwareCPU::Inst_PopB);
  inst_dict.Add("pop-C",  (tHardwareMethod) &cHardwareCPU::Inst_PopC);
  inst_dict.Add("push-A", (tHardwareMethod) &cHardwareCPU::Inst_PushA);
  inst_dict.Add("push-B", (tHardwareMethod) &cHardwareCPU::Inst_PushB);
  inst_dict.Add("push-C", (tHardwareMethod) &cHardwareCPU::Inst_PushC);

  inst_dict.Add("shift-r", (tHardwareMethod) &cHardwareCPU::Inst_ShiftR);
  inst_dict.Add("shift-l", (tHardwareMethod) &cHardwareCPU::Inst_ShiftL);
  inst_dict.Add("bit-1",   (tHardwareMethod) &cHardwareCPU::Inst_Bit1);
  inst_dict.Add("set-num", (tHardwareMethod) &cHardwareCPU::Inst_SetNum);
  inst_dict.Add("inc",     (tHardwareMethod) &cHardwareCPU::Inst_Inc);
  inst_dict.Add("dec",     (tHardwareMethod) &cHardwareCPU::Inst_Dec);
  inst_dict.Add("zero",    (tHardwareMethod) &cHardwareCPU::Inst_Zero);
  inst_dict.Add("neg",     (tHardwareMethod) &cHardwareCPU::Inst_Neg);
  inst_dict.Add("square",  (tHardwareMethod) &cHardwareCPU::Inst_Square);
  inst_dict.Add("sqrt",    (tHardwareMethod) &cHardwareCPU::Inst_Sqrt);
  inst_dict.Add("not",     (tHardwareMethod) &cHardwareCPU::Inst_Not);
  inst_dict.Add("log",     (tHardwareMethod) &cHardwareCPU::Inst_Log);
  inst_dict.Add("log10",     (tHardwareMethod) &cHardwareCPU::Inst_Log10);
  inst_dict.Add("minus-17", (tHardwareMethod) &cHardwareCPU::Inst_Minus17);

  inst_dict.Add("add",     (tHardwareMethod) &cHardwareCPU::Inst_Add);
  inst_dict.Add("sub",     (tHardwareMethod) &cHardwareCPU::Inst_Sub);
  inst_dict.Add("mult",    (tHardwareMethod) &cHardwareCPU::Inst_Mult);
  inst_dict.Add("div",     (tHardwareMethod) &cHardwareCPU::Inst_Div);
  inst_dict.Add("mod",     (tHardwareMethod) &cHardwareCPU::Inst_Mod);
  inst_dict.Add("nand",    (tHardwareMethod) &cHardwareCPU::Inst_Nand);
  inst_dict.Add("nor",     (tHardwareMethod) &cHardwareCPU::Inst_Nor);
  inst_dict.Add("and",     (tHardwareMethod) &cHardwareCPU::Inst_And);
  inst_dict.Add("order",   (tHardwareMethod) &cHardwareCPU::Inst_Order);
  inst_dict.Add("xor",     (tHardwareMethod) &cHardwareCPU::Inst_Xor);

  inst_dict.Add("copy",    (tHardwareMethod)&cHardwareCPU::Inst_Copy);
  inst_dict.Add("read",    (tHardwareMethod)&cHardwareCPU::Inst_ReadInst);
  inst_dict.Add("write",   (tHardwareMethod)&cHardwareCPU::Inst_WriteInst);
  inst_dict.Add("stk-read",(tHardwareMethod)&cHardwareCPU::Inst_StackReadInst);
  inst_dict.Add("stk-writ",(tHardwareMethod)&cHardwareCPU::Inst_StackWriteInst);

  inst_dict.Add("compare",  (tHardwareMethod) &cHardwareCPU::Inst_Compare);
  inst_dict.Add("if-n-cpy", (tHardwareMethod) &cHardwareCPU::Inst_IfNCpy);
  inst_dict.Add("allocate", (tHardwareMethod) &cHardwareCPU::Inst_Allocate);
  inst_dict.Add("divide",   (tHardwareMethod) &cHardwareCPU::Inst_Divide);
  inst_dict.Add("c-alloc",  (tHardwareMethod) &cHardwareCPU::Inst_CAlloc);
  inst_dict.Add("c-divide", (tHardwareMethod) &cHardwareCPU::Inst_CDivide);
  inst_dict.Add("inject",   (tHardwareMethod) &cHardwareCPU::Inst_Inject);
  inst_dict.Add("inject-r", (tHardwareMethod) &cHardwareCPU::Inst_InjectRand);
  inst_dict.Add("inject-th", (tHardwareMethod) &cHardwareCPU::Inst_InjectThread);

  inst_dict.Add("get",     (tHardwareMethod)&cHardwareCPU::Inst_TaskGet);
  inst_dict.Add("stk-get", (tHardwareMethod)&cHardwareCPU::Inst_TaskStackGet);
  inst_dict.Add("stk-load",(tHardwareMethod)&cHardwareCPU::Inst_TaskStackLoad);
  inst_dict.Add("put",     (tHardwareMethod)&cHardwareCPU::Inst_TaskPut);
  inst_dict.Add("IO",      (tHardwareMethod)&cHardwareCPU::Inst_TaskIO);
  inst_dict.Add("search-f",(tHardwareMethod)&cHardwareCPU::Inst_SearchF);
  inst_dict.Add("search-b",(tHardwareMethod)&cHardwareCPU::Inst_SearchB);
  inst_dict.Add("mem-size",(tHardwareMethod)&cHardwareCPU::Inst_MemSize);

  inst_dict.Add("rotate-l", (tHardwareMethod) &cHardwareCPU::Inst_RotateL);
  inst_dict.Add("rotate-r", (tHardwareMethod) &cHardwareCPU::Inst_RotateR);

  inst_dict.Add("set-cmut", (tHardwareMethod) &cHardwareCPU::Inst_SetCopyMut);
  inst_dict.Add("mod-cmut", (tHardwareMethod) &cHardwareCPU::Inst_ModCopyMut);

  // Threading instructions
  inst_dict.Add("fork-th", (tHardwareMethod) &cHardwareCPU::Inst_ForkThread);
  inst_dict.Add("kill-th", (tHardwareMethod) &cHardwareCPU::Inst_KillThread);
  inst_dict.Add("id-th",   (tHardwareMethod) &cHardwareCPU::Inst_ThreadID);
  
  // Head-based instructions
  inst_dict.Add("h-alloc",  (tHardwareMethod) &cHardwareCPU::Inst_MaxAlloc);
  inst_dict.Add("h-divide", (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide);
  inst_dict.Add("h-read",   (tHardwareMethod) &cHardwareCPU::Inst_HeadRead);
  inst_dict.Add("h-write",  (tHardwareMethod) &cHardwareCPU::Inst_HeadWrite);
  inst_dict.Add("h-copy",   (tHardwareMethod) &cHardwareCPU::Inst_HeadCopy);
  inst_dict.Add("h-search", (tHardwareMethod) &cHardwareCPU::Inst_HeadSearch);
  inst_dict.Add("h-push",   (tHardwareMethod) &cHardwareCPU::Inst_HeadPush);  
  inst_dict.Add("h-pop",    (tHardwareMethod) &cHardwareCPU::Inst_HeadPop);  
  inst_dict.Add("set-head", (tHardwareMethod) &cHardwareCPU::Inst_SetHead);
  inst_dict.Add("adv-head", (tHardwareMethod) &cHardwareCPU::Inst_AdvanceHead);
  inst_dict.Add("mov-head", (tHardwareMethod) &cHardwareCPU::Inst_MoveHead);
  inst_dict.Add("jmp-head", (tHardwareMethod) &cHardwareCPU::Inst_JumpHead);
  inst_dict.Add("get-head", (tHardwareMethod) &cHardwareCPU::Inst_GetHead);
  inst_dict.Add("if-label", (tHardwareMethod) &cHardwareCPU::Inst_IfLabel);
  inst_dict.Add("set-flow", (tHardwareMethod) &cHardwareCPU::Inst_SetFlow);

  inst_dict.Add("h-copy2",   (tHardwareMethod) &cHardwareCPU::Inst_HeadCopy2);
  inst_dict.Add("h-copy3",   (tHardwareMethod) &cHardwareCPU::Inst_HeadCopy3);
  inst_dict.Add("h-copy4",   (tHardwareMethod) &cHardwareCPU::Inst_HeadCopy4);
  inst_dict.Add("h-copy5",   (tHardwareMethod) &cHardwareCPU::Inst_HeadCopy5);
  inst_dict.Add("h-copy6",   (tHardwareMethod) &cHardwareCPU::Inst_HeadCopy6);
  inst_dict.Add("h-copy7",   (tHardwareMethod) &cHardwareCPU::Inst_HeadCopy7);
  inst_dict.Add("h-copy8",   (tHardwareMethod) &cHardwareCPU::Inst_HeadCopy8);
  inst_dict.Add("h-copy9",   (tHardwareMethod) &cHardwareCPU::Inst_HeadCopy9);
  inst_dict.Add("h-copy10",  (tHardwareMethod) &cHardwareCPU::Inst_HeadCopy10);


  inst_dict.Add("divide-sex",     (tHardwareMethod) &cHardwareCPU::Inst_HeadDivideSex);
  inst_dict.Add("divide-asex",    (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide1);

  inst_dict.Add("h-divide1",     (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide1);
  inst_dict.Add("h-divide2",     (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide2);
  inst_dict.Add("h-divide3",     (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide3);
  inst_dict.Add("h-divide4",     (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide4);
  inst_dict.Add("h-divide5",     (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide5);
  inst_dict.Add("h-divide6",     (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide6);
  inst_dict.Add("h-divide7",     (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide7);
  inst_dict.Add("h-divide8",     (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide8);
  inst_dict.Add("h-divide9",     (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide9);
  inst_dict.Add("h-divide10",    (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide10);
  inst_dict.Add("h-divide16",    (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide16);
  inst_dict.Add("h-divide32",    (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide32);
  inst_dict.Add("h-divide50",    (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide50);
  inst_dict.Add("h-divide100",   (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide100);
  inst_dict.Add("h-divide500",   (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide500);
  inst_dict.Add("h-divide1000",  (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide1000);
  inst_dict.Add("h-divide5000",  (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide5000);
  inst_dict.Add("h-divide10000",  (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide10000);
  inst_dict.Add("h-divide50000",  (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide50000);
  inst_dict.Add("h-divide0.5",   (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide0_5);
  inst_dict.Add("h-divide0.1",   (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide0_1);
  inst_dict.Add("h-divide0.05",  (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide0_05);
  inst_dict.Add("h-divide0.01",  (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide0_01);
  inst_dict.Add("h-divide0.001", (tHardwareMethod) &cHardwareCPU::Inst_HeadDivide0_001);

  // High-level instructions
  inst_dict.Add("repro",     (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-A",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-B",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-C",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-D",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-E",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-F",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-G",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-H",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-I",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-J",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-K",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-L",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-M",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-N",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-O",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-P",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-Q",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-R",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-S",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-T",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-U",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-V",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-W",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-X",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-Y",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);
  inst_dict.Add("repro-Z",   (tHardwareMethod) &cHardwareCPU::Inst_Repro);

  // Placebo instructions
  // nop-x (included with nops)
  inst_dict.Add("skip", (tHardwareMethod) &cHardwareCPU::Inst_Skip);
#endif /* !USE_INST_SET_CODE */
  
#ifdef USE_INST_SET_CODE
  /* XXX start -- kgn */
  tDictionary<int> nop_dict;
  for(int i=0; i<inst_set.GetInstLib()->GetNumNops(); i++)
    nop_dict.Add(inst_set.GetInstLib()->GetNopName(i), i);

  tDictionary<int> inst_dict;
  for(int i=0; i<inst_set.GetInstLib()->GetSize(); i++)
    inst_dict.Add(inst_set.GetInstLib()->GetName(i), i);
  /* XXX end */
#endif /* USE_INST_SET_CODE */

  // And load it on up!
  LoadInstSet(filename, inst_set, nop_dict, inst_dict);
}

void cHardwareUtil::LoadInstSet_CPU4Stack(const cString & filename, cInstSet & inst_set)
{
  // Nops (or other modifying-instructions...)  Note: Nops must be the
  //  first instructions added to the set.
#ifndef USE_INST_SET_CODE
  tDictionary< int > nop_dict;
  nop_dict.Add("Nop-A", REG_AX);
  nop_dict.Add("Nop-B", REG_BX);
  nop_dict.Add("Nop-C", REG_CX);
  nop_dict.Add("Nop-D", REG_DX);
#endif /* !USE_INST_SET_CODE */

  // Build a dictionary of instructions and their corresponding methods...
#ifndef USE_INST_SET_CODE
  tDictionary< tHardwareMethod > inst_dict;
  
  // Remaining instructions.
  //1
  //inst_dict.Add("nop-A",     &cHardware4Stack::Inst_Nop), 
  //2
  //inst_dict.Add("nop-B",     &cHardware4Stack::Inst_Nop), 
  //3
  //inst_dict.Add("nop-C",     &cHardware4Stack::Inst_Nop),   
  //4 - not implemented yet...
  //inst_dict.Add("nop-D",     &cHardware4Stack::Inst_Nop), 
  //5
  inst_dict.Add("Nop-X", (tHardwareMethod) &cHardware4Stack::Inst_Nop);
  //6 
  inst_dict.Add("Val-Shift-R", (tHardwareMethod) &cHardware4Stack::Inst_ShiftR);
  //7
  inst_dict.Add("Val-Shift-L", (tHardwareMethod) &cHardware4Stack::Inst_ShiftL);
  //8
  inst_dict.Add("Val-Nand",  (tHardwareMethod) &cHardware4Stack::Inst_Nand);
  //9
  inst_dict.Add("Val-Add",  (tHardwareMethod) &cHardware4Stack::Inst_Add);
  //10
  inst_dict.Add("Val-Sub",  (tHardwareMethod)  &cHardware4Stack::Inst_Sub);
  //11
  inst_dict.Add("Val-Mult",  (tHardwareMethod) &cHardware4Stack::Inst_Mult);
  //12
  inst_dict.Add("Val-Div",   (tHardwareMethod) &cHardware4Stack::Inst_Div);
  //13
  inst_dict.Add("SetMemory", (tHardwareMethod) &cHardware4Stack::Inst_MaxAlloc);
  //14
  inst_dict.Add("Divide", (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide);
  //15
  inst_dict.Add("Inst-Read", (tHardwareMethod) &cHardware4Stack::Inst_HeadRead);
  //16
  inst_dict.Add("Inst-Write", (tHardwareMethod) &cHardware4Stack::Inst_HeadWrite);
  //keeping this one for the transition period
  inst_dict.Add("Inst-Copy", (tHardwareMethod) &cHardware4Stack::Inst_HeadCopy);
  //17
  inst_dict.Add("If-Equal", (tHardwareMethod) &cHardware4Stack::Inst_IfEqu);
  //18
  inst_dict.Add("If-Not-Equal", (tHardwareMethod) &cHardware4Stack::Inst_IfNEqu);
  //19
  inst_dict.Add("If-Less", (tHardwareMethod) &cHardware4Stack::Inst_IfLess);
  //20
  inst_dict.Add("If-Greater", (tHardwareMethod) &cHardware4Stack::Inst_IfGr);
  //21
  inst_dict.Add("Head-Push",  (tHardwareMethod) &cHardware4Stack::Inst_HeadPush);
  //22
  inst_dict.Add("Head-Pop",  (tHardwareMethod) &cHardware4Stack::Inst_HeadPop);
  //23
  inst_dict.Add("Head-Move", (tHardwareMethod) &cHardware4Stack::Inst_MoveHead);
  //24
  inst_dict.Add("Search", (tHardwareMethod) &cHardware4Stack::Inst_HeadSearch);
  //25
  inst_dict.Add("Push-Next", (tHardwareMethod) &cHardware4Stack::Inst_PushA);
  //26
  inst_dict.Add("Push-Prev", (tHardwareMethod) &cHardware4Stack::Inst_PushB);
  //27
  inst_dict.Add("Push-Comp", (tHardwareMethod) &cHardware4Stack::Inst_PushC);
  //28 - Not implemented yet...
  //inst_dict.Add("Val-Delete", &cHardware4Stack::Inst_ValDelete
  //29
  inst_dict.Add("Val-Copy", (tHardwareMethod) &cHardware4Stack::Inst_CopyReg);
  //30
  inst_dict.Add("ThreadFork", (tHardwareMethod) &cHardware4Stack::Inst_ForkThread);
  //31
  inst_dict.Add("if-label", (tHardwareMethod) &cHardware4Stack::Inst_IfLabel);
  //32
  inst_dict.Add("Val-Inc", (tHardwareMethod) &cHardware4Stack::Inst_Inc);
  //33
  inst_dict.Add("Val-Dec", (tHardwareMethod) &cHardware4Stack::Inst_Dec);
  //34
  inst_dict.Add("Val-Mod",  (tHardwareMethod) &cHardware4Stack::Inst_Mod);
  //35
  inst_dict.Add("ThreadKill", (tHardwareMethod) &cHardware4Stack::Inst_KillThread);/*
  // Remaining instructions.
  inst_dict.Add("nop-X",    (tHardwareMethod) &cHardware4Stack::Inst_Nop);
  inst_dict.Add("if-equ-0", (tHardwareMethod) &cHardware4Stack::Inst_If0);
  inst_dict.Add("if-not-0", (tHardwareMethod) &cHardware4Stack::Inst_IfNot0);
  inst_dict.Add("if-n-equ", (tHardwareMethod) &cHardware4Stack::Inst_IfNEqu);
  inst_dict.Add("if-equ",   (tHardwareMethod) &cHardware4Stack::Inst_IfEqu);
  inst_dict.Add("if-grt-0", (tHardwareMethod) &cHardware4Stack::Inst_IfGr0);
  inst_dict.Add("if-grt",   (tHardwareMethod) &cHardware4Stack::Inst_IfGr);
  inst_dict.Add("if->=-0",  (tHardwareMethod) &cHardware4Stack::Inst_IfGrEqu0);
  inst_dict.Add("if->=",    (tHardwareMethod) &cHardware4Stack::Inst_IfGrEqu);
  inst_dict.Add("if-les-0", (tHardwareMethod) &cHardware4Stack::Inst_IfLess0);
  inst_dict.Add("if-less",  (tHardwareMethod) &cHardware4Stack::Inst_IfLess);
  inst_dict.Add("if-<=-0",  (tHardwareMethod) &cHardware4Stack::Inst_IfLsEqu0);
  inst_dict.Add("if-<=",    (tHardwareMethod) &cHardware4Stack::Inst_IfLsEqu);
  inst_dict.Add("if-A!=B",  (tHardwareMethod) &cHardware4Stack::Inst_IfANotEqB);
  inst_dict.Add("if-B!=C",  (tHardwareMethod) &cHardware4Stack::Inst_IfBNotEqC);
  inst_dict.Add("if-A!=C",  (tHardwareMethod) &cHardware4Stack::Inst_IfANotEqC);

  inst_dict.Add("if-bit-1", (tHardwareMethod) &cHardware4Stack::Inst_IfBit1);
  inst_dict.Add("jump-f",   (tHardwareMethod) &cHardware4Stack::Inst_JumpF);
  inst_dict.Add("jump-b",   (tHardwareMethod) &cHardware4Stack::Inst_JumpB);
  inst_dict.Add("jump-p",   (tHardwareMethod) &cHardware4Stack::Inst_JumpP);
  inst_dict.Add("jump-slf", (tHardwareMethod) &cHardware4Stack::Inst_JumpSelf);
  inst_dict.Add("call",     (tHardwareMethod) &cHardware4Stack::Inst_Call);
  inst_dict.Add("return",   (tHardwareMethod) &cHardware4Stack::Inst_Return);

  inst_dict.Add("pop",     (tHardwareMethod) &cHardware4Stack::Inst_Pop);
  inst_dict.Add("push",    (tHardwareMethod) &cHardware4Stack::Inst_Push);
  inst_dict.Add("swap-stk",(tHardwareMethod) &cHardware4Stack::Inst_SwitchStack);
  inst_dict.Add("flip-stk",(tHardwareMethod) &cHardware4Stack::Inst_FlipStack);
  inst_dict.Add("swap",    (tHardwareMethod) &cHardware4Stack::Inst_Swap);
  inst_dict.Add("swap-AB", (tHardwareMethod) &cHardware4Stack::Inst_SwapAB);
  inst_dict.Add("swap-BC", (tHardwareMethod) &cHardware4Stack::Inst_SwapBC);
  inst_dict.Add("swap-AC", (tHardwareMethod) &cHardware4Stack::Inst_SwapAC);
  inst_dict.Add("copy-reg",(tHardwareMethod) &cHardware4Stack::Inst_CopyReg);
  inst_dict.Add("set_A=B", (tHardwareMethod) &cHardware4Stack::Inst_CopyRegAB);
  inst_dict.Add("set_A=C", (tHardwareMethod) &cHardware4Stack::Inst_CopyRegAC);
  inst_dict.Add("set_B=A", (tHardwareMethod) &cHardware4Stack::Inst_CopyRegBA);
  inst_dict.Add("set_B=C", (tHardwareMethod) &cHardware4Stack::Inst_CopyRegBC);
  inst_dict.Add("set_C=A", (tHardwareMethod) &cHardware4Stack::Inst_CopyRegCA);
  inst_dict.Add("set_C=B", (tHardwareMethod) &cHardware4Stack::Inst_CopyRegCB);
  inst_dict.Add("reset",   (tHardwareMethod) &cHardware4Stack::Inst_Reset);

  inst_dict.Add("pop-A",  (tHardwareMethod) &cHardware4Stack::Inst_PopA);
  inst_dict.Add("pop-B",  (tHardwareMethod) &cHardware4Stack::Inst_PopB);
  inst_dict.Add("pop-C",  (tHardwareMethod) &cHardware4Stack::Inst_PopC);
  // inst_dict.Add("push-A", (tHardwareMethod) &cHardware4Stack::Inst_PushA);
  inst_dict.Add("push-B", (tHardwareMethod) &cHardware4Stack::Inst_PushB);
  inst_dict.Add("push-C", (tHardwareMethod) &cHardware4Stack::Inst_PushC);

  inst_dict.Add("shift-r", (tHardwareMethod) &cHardware4Stack::Inst_ShiftR);
  inst_dict.Add("shift-l", (tHardwareMethod) &cHardware4Stack::Inst_ShiftL);
  inst_dict.Add("bit-1",   (tHardwareMethod) &cHardware4Stack::Inst_Bit1);
  inst_dict.Add("set-num", (tHardwareMethod) &cHardware4Stack::Inst_SetNum);
  inst_dict.Add("inc",     (tHardwareMethod) &cHardware4Stack::Inst_Inc);
  inst_dict.Add("dec",     (tHardwareMethod) &cHardware4Stack::Inst_Dec);
  inst_dict.Add("zero",    (tHardwareMethod) &cHardware4Stack::Inst_Zero);
  inst_dict.Add("neg",     (tHardwareMethod) &cHardware4Stack::Inst_Neg);
  inst_dict.Add("square",  (tHardwareMethod) &cHardware4Stack::Inst_Square);
  inst_dict.Add("sqrt",    (tHardwareMethod) &cHardware4Stack::Inst_Sqrt);
  inst_dict.Add("not",     (tHardwareMethod) &cHardware4Stack::Inst_Not);
  inst_dict.Add("log",     (tHardwareMethod) &cHardware4Stack::Inst_Log);
  inst_dict.Add("log10",   (tHardwareMethod) &cHardware4Stack::Inst_Log10);
  inst_dict.Add("minus-18",(tHardwareMethod) &cHardware4Stack::Inst_Minus18);

  inst_dict.Add("add",     (tHardwareMethod) &cHardware4Stack::Inst_Add);
  inst_dict.Add("sub",     (tHardwareMethod) &cHardware4Stack::Inst_Sub);
  inst_dict.Add("mult",    (tHardwareMethod) &cHardware4Stack::Inst_Mult);
  inst_dict.Add("div",     (tHardwareMethod) &cHardware4Stack::Inst_Div);
  inst_dict.Add("mod",     (tHardwareMethod) &cHardware4Stack::Inst_Mod);
  inst_dict.Add("nand",    (tHardwareMethod) &cHardware4Stack::Inst_Nand);
  inst_dict.Add("nor",     (tHardwareMethod) &cHardware4Stack::Inst_Nor);
  inst_dict.Add("and",     (tHardwareMethod) &cHardware4Stack::Inst_And);
  inst_dict.Add("order",   (tHardwareMethod) &cHardware4Stack::Inst_Order);
  inst_dict.Add("xor",     (tHardwareMethod) &cHardware4Stack::Inst_Xor);

  inst_dict.Add("copy",    (tHardwareMethod)&cHardware4Stack::Inst_Copy);
  inst_dict.Add("read",    (tHardwareMethod)&cHardware4Stack::Inst_ReadInst);
  inst_dict.Add("write",   (tHardwareMethod)&cHardware4Stack::Inst_WriteInst);
  inst_dict.Add("stk-read",(tHardwareMethod)&cHardware4Stack::Inst_StackReadInst);
  inst_dict.Add("stk-writ",(tHardwareMethod)&cHardware4Stack::Inst_StackWriteInst);

  inst_dict.Add("compare",  (tHardwareMethod) &cHardware4Stack::Inst_Compare);
  inst_dict.Add("if-n-cpy", (tHardwareMethod) &cHardware4Stack::Inst_IfNCpy);
  inst_dict.Add("allocate", (tHardwareMethod) &cHardware4Stack::Inst_Allocate);
  inst_dict.Add("divide",   (tHardwareMethod) &cHardware4Stack::Inst_Divide);
  inst_dict.Add("c-alloc",  (tHardwareMethod) &cHardware4Stack::Inst_CAlloc);
  inst_dict.Add("c-divide", (tHardwareMethod) &cHardware4Stack::Inst_CDivide);
  inst_dict.Add("inject",   (tHardwareMethod) &cHardware4Stack::Inst_Inject);
  inst_dict.Add("inject-r", (tHardwareMethod) &cHardware4Stack::Inst_InjectRand);
  inst_dict.Add("inject-th", (tHardwareMethod) &cHardware4Stack::Inst_InjectThread);

  inst_dict.Add("get",     (tHardwareMethod)&cHardware4Stack::Inst_TaskGet);
  inst_dict.Add("stk-get", (tHardwareMethod)&cHardware4Stack::Inst_TaskStackGet);
  inst_dict.Add("stk-load",(tHardwareMethod)&cHardware4Stack::Inst_TaskStackLoad);
  inst_dict.Add("put",     (tHardwareMethod)&cHardware4Stack::Inst_TaskPut);
  inst_dict.Add("IO",      (tHardwareMethod)&cHardware4Stack::Inst_TaskIO);
  inst_dict.Add("search-f",(tHardwareMethod)&cHardware4Stack::Inst_SearchF);
  inst_dict.Add("search-b",(tHardwareMethod)&cHardware4Stack::Inst_SearchB);
  inst_dict.Add("mem-size",(tHardwareMethod)&cHardware4Stack::Inst_MemSize);

  inst_dict.Add("rotate-l", (tHardwareMethod) &cHardware4Stack::Inst_RotateL);
  inst_dict.Add("rotate-r", (tHardwareMethod) &cHardware4Stack::Inst_RotateR);

  inst_dict.Add("set-cmut", (tHardwareMethod) &cHardware4Stack::Inst_SetCopyMut);
  inst_dict.Add("mod-cmut", (tHardwareMethod) &cHardware4Stack::Inst_ModCopyMut);

  // Threading instructions
  inst_dict.Add("fork-th", (tHardwareMethod) &cHardware4Stack::Inst_ForkThread);
  inst_dict.Add("kill-th", (tHardwareMethod) &cHardware4Stack::Inst_KillThread);
  inst_dict.Add("id-th",   (tHardwareMethod) &cHardware4Stack::Inst_ThreadID);
  
  // Head-based instructions
  inst_dict.Add("h-alloc",  (tHardwareMethod) &cHardware4Stack::Inst_MaxAlloc);
  inst_dict.Add("h-divide", (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide);
  inst_dict.Add("h-read",   (tHardwareMethod) &cHardware4Stack::Inst_HeadRead);
  inst_dict.Add("h-write",  (tHardwareMethod) &cHardware4Stack::Inst_HeadWrite);
  inst_dict.Add("h-copy",   (tHardwareMethod) &cHardware4Stack::Inst_HeadCopy);
  inst_dict.Add("h-search", (tHardwareMethod) &cHardware4Stack::Inst_HeadSearch);
  inst_dict.Add("h-push",   (tHardwareMethod) &cHardware4Stack::Inst_HeadPush);  
  inst_dict.Add("h-pop",    (tHardwareMethod) &cHardware4Stack::Inst_HeadPop);  
  inst_dict.Add("set-head", (tHardwareMethod) &cHardware4Stack::Inst_SetHead);
  inst_dict.Add("adv-head", (tHardwareMethod) &cHardware4Stack::Inst_AdvanceHead);
  inst_dict.Add("mov-head", (tHardwareMethod) &cHardware4Stack::Inst_MoveHead);
  inst_dict.Add("jmp-head", (tHardwareMethod) &cHardware4Stack::Inst_JumpHead);
  inst_dict.Add("get-head", (tHardwareMethod) &cHardware4Stack::Inst_GetHead);
  inst_dict.Add("if-label", (tHardwareMethod) &cHardware4Stack::Inst_IfLabel);
  inst_dict.Add("set-flow", (tHardwareMethod) &cHardware4Stack::Inst_SetFlow);

  inst_dict.Add("h-copy2",   (tHardwareMethod) &cHardware4Stack::Inst_HeadCopy2);
  inst_dict.Add("h-copy3",   (tHardwareMethod) &cHardware4Stack::Inst_HeadCopy3);
  inst_dict.Add("h-copy4",   (tHardwareMethod) &cHardware4Stack::Inst_HeadCopy4);
  inst_dict.Add("h-copy5",   (tHardwareMethod) &cHardware4Stack::Inst_HeadCopy5);
  inst_dict.Add("h-copy6",   (tHardwareMethod) &cHardware4Stack::Inst_HeadCopy6);
  inst_dict.Add("h-copy7",   (tHardwareMethod) &cHardware4Stack::Inst_HeadCopy7);
  inst_dict.Add("h-copy8",   (tHardwareMethod) &cHardware4Stack::Inst_HeadCopy8);
  inst_dict.Add("h-copy9",   (tHardwareMethod) &cHardware4Stack::Inst_HeadCopy9);
  inst_dict.Add("h-copy10",  (tHardwareMethod) &cHardware4Stack::Inst_HeadCopy10);


  inst_dict.Add("divide-sex",     (tHardwareMethod) &cHardware4Stack::Inst_HeadDivideSex);
  inst_dict.Add("divide-asex",    (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide1);

  inst_dict.Add("h-divide1",     (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide1);
  inst_dict.Add("h-divide2",     (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide2);
  inst_dict.Add("h-divide3",     (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide3);
  inst_dict.Add("h-divide4",     (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide4);
  inst_dict.Add("h-divide5",     (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide5);
  inst_dict.Add("h-divide6",     (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide6);
  inst_dict.Add("h-divide7",     (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide7);
  inst_dict.Add("h-divide8",     (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide8);
  inst_dict.Add("h-divide9",     (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide9);
  inst_dict.Add("h-divide10",    (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide10);
  inst_dict.Add("h-divide16",    (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide16);
  inst_dict.Add("h-divide32",    (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide32);
  inst_dict.Add("h-divide50",    (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide50);
  inst_dict.Add("h-divide100",   (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide100);
  inst_dict.Add("h-divide500",   (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide500);
  inst_dict.Add("h-divide1000",  (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide1000);
  inst_dict.Add("h-divide5000",  (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide5000);
  inst_dict.Add("h-divide10000",  (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide10000);
  inst_dict.Add("h-divide50000",  (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide50000);
  inst_dict.Add("h-divide0.5",   (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide0_5);
  inst_dict.Add("h-divide0.1",   (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide0_1);
  inst_dict.Add("h-divide0.05",  (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide0_05);
  inst_dict.Add("h-divide0.01",  (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide0_01);
  inst_dict.Add("h-divide0.001", (tHardwareMethod) &cHardware4Stack::Inst_HeadDivide0_001);

  // High-level instructions
  inst_dict.Add("repro",     (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-A",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-B",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-C",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-D",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-E",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-F",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-G",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-H",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-I",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-J",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-K",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-L",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-M",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-N",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-O",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-P",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-Q",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-R",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-S",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-T",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-U",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-V",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-W",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-X",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-Y",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);
  inst_dict.Add("repro-Z",   (tHardwareMethod) &cHardware4Stack::Inst_Repro);

  // Placebo instructions
  // nop-x (included with nops)
  inst_dict.Add("skip", (tHardwareMethod) &cHardware4Stack::Inst_Skip);
										*/
  cout << "BLEE! Instruction Library in util has " << inst_dict.GetSize()
       << " instructions + " << nop_dict.GetSize() << " nops." << endl;

#endif /* !USE_INST_SET_CODE */
  
#ifdef USE_INST_SET_CODE
  /* XXX start -- kgn */
  tDictionary<int> nop_dict;
  for(int i=0; i<inst_set.GetInstLib()->GetNumNops(); i++)
    nop_dict.Add(inst_set.GetInstLib()->GetNopName(i), i);

  tDictionary<int> inst_dict;
  for(int i=0; i<inst_set.GetInstLib()->GetSize(); i++)
    inst_dict.Add(inst_set.GetInstLib()->GetName(i), i);
  /* XXX end */
#endif /* USE_INST_SET_CODE */

  cout << "Instruction Library in util has " << inst_dict.GetSize()
       << " instructions and " << nop_dict.GetSize() <<  " nops." << endl;


  // And load it on up!
  LoadInstSet(filename, inst_set, nop_dict, inst_dict);
}

cInstSet & cHardwareUtil::DefaultInstSet(const cString & inst_filename)
{
  static cInstSet inst_set;

  // If we don't have an instruction set yet, and we've been given a filename,
  // set it up.
  if (inst_filename != "" && inst_set.GetSize() == 0) {
    if (cConfig::GetHardwareType() == HARDWARE_TYPE_CPU_ORIGINAL) {
#ifdef USE_INST_SET_CODE
      inst_set.SetInstLib(cHardwareCPU::GetInstLib());
#endif /* USE_INST_SET_CODE */
      LoadInstSet_CPUOriginal(inst_filename, inst_set);
    }
    else if (cConfig::GetHardwareType() == HARDWARE_TYPE_CPU_4STACK) {
#ifdef USE_INST_SET_CODE
      inst_set.SetInstLib(cHardware4Stack::GetInstLib());
#endif /* USE_INST_SET_CODE */
      LoadInstSet_CPU4Stack(inst_filename, inst_set);
    }
  }

  return inst_set;
}

