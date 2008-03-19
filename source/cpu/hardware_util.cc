//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "hardware_method.hh"
#include "hardware_util.hh"

#include "../tools/string.hh"
#include "../tools/file.hh"

#include "../main/inst_lib.hh"
#include "../main/inst_util.hh"

#include "hardware_base.hh"
#include "hardware_cpu.hh"


using namespace std;


void cHardwareUtil::LoadInstLib(const cString & filename, cInstLib & lib, 
				tDictionary<int> & nop_dict,
				tDictionary<tHardwareMethod> & inst_dict,
                                /* XXX start -- kgn */
				tDictionary<int> & nop_dict_2,
				tDictionary<int> & inst_dict_2)
                                /* XXX end */
{
  cInitFile file(filename);
  file.Load();
  file.Compress();

  while (file.GetNumLines() > 0) {
    cString cur_line = file.RemoveLine();
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
    int nop_mod = -1;
    /* XXX start -- kgn */
    int nop_mod_2 = -1;
    /* XXX end */
    if (nop_dict.Find(inst_name, nop_mod) == true) {
      lib.AddNop(inst_name, &cHardwareBase::Inst_Nop, nop_mod,
		 redundancy, ft_cost, cost, prob_fail);
      /* XXX start -- kgn */
      if(nop_dict_2.Find(inst_name, nop_mod_2) == true) {
        lib.AddNop2(nop_mod_2, redundancy, ft_cost, cost, prob_fail);
      } else { /* dicts mismatch! die horribly. */ exit(1); }
      /* XXX end */
      continue;
    }

    // Otherwise, it had better be in the main dictionary...
    tHardwareMethod cpu_method = NULL;
    /* XXX start -- kgn */
    int fun_id = -1;
    /* XXX end */
    if (inst_dict.Find(inst_name, cpu_method) == true) {
      lib.Add(inst_name, cpu_method, redundancy, ft_cost, cost, prob_fail);
      /* XXX start -- kgn */
      if(inst_dict_2.Find(inst_name, fun_id) == true){
        lib.Add2(fun_id, redundancy, ft_cost, cost, prob_fail);
      } else { /* dicts mismatch! die horribly. */ exit(1); }
      /* XXX end */
      continue;
    }

    // Oh oh!  Didn't find an instruction!
    cerr << endl
	 << "Error: Could not find instruction '" << inst_name << "'" << endl
	 << "       (Best match = '"
	 << inst_dict.NearMatch(inst_name) << "')" << endl;
    exit(1);
  }

  cerr << "Loaded Instruction Library \"" << filename
       << "\" with " << lib.GetSize() << " instructions." << endl;
}

void cHardwareUtil::LoadInstLibCPU(const cString & filename, cInstLib & lib)
{
  // Nops (or other modifying-instructions...)  Note: Nops must be the
  //  first instructions added to the set.
  tDictionary< int > nop_dict;
  nop_dict.Add("nop-A", REG_AX);
  nop_dict.Add("nop-B", REG_BX);
  nop_dict.Add("nop-C", REG_CX);

  // Build a dictionary of instructions and their corresponding methods...
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
  
  /* XXX start -- kgn */
  tDictionary<int> nop_dict_2;
  for(int i=0; i<lib.GetInstSuperlib()->GetNumNops(); i++)
    nop_dict_2.Add(lib.GetInstSuperlib()->GetNopName(i), i);

  tDictionary<int> inst_dict_2;
  for(int i=0; i<lib.GetInstSuperlib()->GetSize(); i++)
    inst_dict_2.Add(lib.GetInstSuperlib()->GetName(i), i);
  /* XXX end */

  // And load it on up!
  LoadInstLib(filename, lib, nop_dict, inst_dict, nop_dict_2, inst_dict_2);
}

cInstLib & cHardwareUtil::DefaultInstLib(const cString & inst_filename)
{
  static cInstLib lib;
  if (inst_filename == "" || lib.GetSize()) return lib;

  LoadInstLibCPU(inst_filename, lib);

  return lib;
}

