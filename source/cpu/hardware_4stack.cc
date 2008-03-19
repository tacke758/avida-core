//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include <iomanip>

#include "hardware_method.hh"
#include "hardware_4stack.hh"

#include "../tools/string_util.hh"

#include "../main/config.hh"
#include "../main/inst_set.hh"
#include "../main/inst_lib.hh"
#include "../main/genome_util.hh"
#include "../main/organism.hh"
#include "../main/phenotype.hh"

#include "test_cpu.hh"
#include <limits.h>

using namespace std;


/////////////////////////
//  cHardware4Stack_Thread
/////////////////////////

cHardware4Stack_Thread::cHardware4Stack_Thread(cHardwareBase * in_hardware, int _id)
  : input_buf(INPUT_BUF_SIZE),
    output_buf(OUTPUT_BUF_SIZE)
{
  Reset(in_hardware, _id);
}

cHardware4Stack_Thread::cHardware4Stack_Thread(const cHardware4Stack_Thread & in_thread, int _id)
  : input_buf(INPUT_BUF_SIZE),
    output_buf(OUTPUT_BUF_SIZE)
{
   id = _id;
   if (id == -1) id = in_thread.id;
   for (int i = 0; i < NUM_REGISTERS; i++) {
     reg[i] = in_thread.reg[i];
   }
   for (int i = 0; i < NUM_HEADS; i++) {
     heads[i] = in_thread.heads[i];
   }
   stack = in_thread.stack;
   input_pointer = in_thread.input_pointer;
}

cHardware4Stack_Thread::~cHardware4Stack_Thread() {}

void cHardware4Stack_Thread::operator=(const cHardware4Stack_Thread & in_thread)
{
  id = in_thread.id;
  for (int i = 0; i < NUM_REGISTERS; i++) {
    reg[i] = in_thread.reg[i];
  }
  for (int i = 0; i < NUM_HEADS; i++) {
    heads[i] = in_thread.heads[i];
  }
  stack = in_thread.stack;
  input_pointer = in_thread.input_pointer;
  input_buf = in_thread.input_buf;
  output_buf = in_thread.output_buf;
}

void cHardware4Stack_Thread::Reset(cHardwareBase * in_hardware, int _id)
{
  id = _id;

  for (int i = 0; i < NUM_REGISTERS; i++) reg[i] = 0;
  for (int i = 0; i < NUM_HEADS; i++) heads[i].Reset(in_hardware);

  stack.Clear();
  cur_stack = 0;
  cur_head = HEAD_IP;
  input_pointer = 0;
  input_buf.Clear();
  output_buf.Clear();
  read_label.Clear();
  next_label.Clear();
}




void cHardware4Stack_Thread::SaveState(ostream & fp){
  assert(fp.good());
  fp << "cHardware4Stack_Thread" << endl;

  // registers
  for( int i=0; i<NUM_REGISTERS; ++i ){
    fp<<reg[i]<<endl;
  }

  // heads (@TCC does not handle parasites!!!)
  for( int i=0; i<NUM_HEADS; ++i ){
    fp<<heads[i].GetPosition()<<endl;
  }

  stack.SaveState(fp);

  fp<<"|"; // marker
  fp<<cur_stack;
  fp<<cur_head;
  fp<<input_pointer;
  fp<<endl;

  // IO buffers
  input_buf.SaveState(fp);
  output_buf.SaveState(fp);

  // Code labels
  read_label.SaveState(fp);
  next_label.SaveState(fp);
}



void cHardware4Stack_Thread::LoadState(istream & fp){
  assert(fp.good());
  cString foo;
  fp >> foo;
  assert( foo == "cHardware4Stack_Thread");

  // registers
  for( int i=0; i<NUM_REGISTERS; ++i ){
    fp>>reg[i];
  }

  // heads (@TCC does not handle parasites!!!)
  for( int i=0; i<NUM_HEADS; ++i ){
    int pos;
    fp>>pos;
    heads[i].AbsSet(pos);
  }

  // stack
  stack.LoadState(fp);

  char marker; fp>>marker; assert( marker == '|' );
  /* YIKES!  data loss below: */
  char the_cur_stack = cur_stack;
  char the_cur_head = cur_head;
  char the_input_pointer = input_pointer;
  fp.get(the_cur_stack);
  fp.get(the_cur_head);
  fp.get(the_input_pointer);

  // IO buffers
  input_buf.LoadState(fp);
  output_buf.LoadState(fp);

  // Code labels
  read_label.LoadState(fp);
  next_label.LoadState(fp);
}


///////////////
//  cInstLib4Stack
///////////////

class cInstLib4Stack : public cInstLibBase {
  const size_t m_nopmods_array_size;
  const size_t m_function_array_size;
  cString *m_nopmod_names;
  cString *m_function_names;
  const int *m_nopmods;
  cHardware4Stack::tHardware4StackMethod *m_functions;
  static const cInstruction inst_error;
  static const cInstruction inst_default;
public:
  cInstLib4Stack(
    size_t nopmod_array_size,
    size_t function_array_size,
    cString *nopmod_names,
    cString *function_names,
    const int *nopmods,
    cHardware4Stack::tHardware4StackMethod *functions
  ):m_nopmods_array_size(nopmod_array_size),
    m_function_array_size(function_array_size),
    m_nopmod_names(nopmod_names),
    m_function_names(function_names),
    m_nopmods(nopmods),
    m_functions(functions)
  {}
  cHardware4Stack::tHardware4StackMethod *GetFunctions(void){ return m_functions; } 
  const cString &GetName(const unsigned int id) {
    assert(id < m_function_array_size);
    return m_function_names[id];
  }
  const cString &GetNopName(const unsigned int id) {
    assert(id < m_nopmods_array_size);
    return m_nopmod_names[id];
  }
  int GetNopMod(const unsigned int id){
    assert(id < m_nopmods_array_size);
    return m_nopmods[id];
  }
  int GetNopMod(const cInstruction & inst){
    return GetNopMod(inst.GetOp());
  }
  int GetSize(){ return m_function_array_size; }
  int GetNumNops(){ return m_nopmods_array_size; }
  cInstruction GetInst(const cString & in_name){
    for (unsigned int i = 0; i < m_function_array_size; i++) {
      if (m_function_names[i] == in_name) return cInstruction(i);
    }
    return cInstLib4Stack::GetInstError();
  }
  const cInstruction & GetInstDefault(){ return inst_default; }
  const cInstruction & GetInstError(){ return inst_error; }
};


///////////////
//  cHardware4Stack
///////////////

const cInstruction cInstLib4Stack::inst_error(255);
const cInstruction cInstLib4Stack::inst_default(0);
cInstLibBase *cHardware4Stack::GetInstLib(){ return s_inst_slib; }

cInstLib4Stack *cHardware4Stack::s_inst_slib = cHardware4Stack::initInstLib();
cInstLib4Stack *cHardware4Stack::initInstLib(void){
  struct cNOPEntry4Stack {
    cNOPEntry4Stack(const cString &name, int nop_mod):name(name), nop_mod(nop_mod){}
    cString name;
    int nop_mod;
  };
  static const cNOPEntry4Stack s_n_array[] = {
    cNOPEntry4Stack("nop-A", REG_AX),
    cNOPEntry4Stack("nop-B", REG_BX),
    cNOPEntry4Stack("nop-C", REG_CX)
  };

  struct cInstEntry4Stack {
    cInstEntry4Stack(const cString &name, tHardware4StackMethod function):name(name), function(function){}
    cString name;
    tHardware4StackMethod function;
  };
  static const cInstEntry4Stack s_f_array[] = {
    cInstEntry4Stack("nop-A",     &cHardware4Stack::Inst_Nop),
    cInstEntry4Stack("nop-B",     &cHardware4Stack::Inst_Nop),
    cInstEntry4Stack("nop-C",     &cHardware4Stack::Inst_Nop),
    cInstEntry4Stack("nop-X",     &cHardware4Stack::Inst_Nop),
    cInstEntry4Stack("if-equ-0",  &cHardware4Stack::Inst_If0),
    cInstEntry4Stack("if-not-0",  &cHardware4Stack::Inst_IfNot0),
    cInstEntry4Stack("if-n-equ",  &cHardware4Stack::Inst_IfNEqu),
    cInstEntry4Stack("if-equ",    &cHardware4Stack::Inst_IfEqu),
    cInstEntry4Stack("if-grt-0",  &cHardware4Stack::Inst_IfGr0),
    cInstEntry4Stack("if-grt",    &cHardware4Stack::Inst_IfGr),
    cInstEntry4Stack("if->=-0",   &cHardware4Stack::Inst_IfGrEqu0),
    cInstEntry4Stack("if->=",     &cHardware4Stack::Inst_IfGrEqu),
    cInstEntry4Stack("if-les-0",  &cHardware4Stack::Inst_IfLess0),
    cInstEntry4Stack("if-less",   &cHardware4Stack::Inst_IfLess),
    cInstEntry4Stack("if-<=-0",   &cHardware4Stack::Inst_IfLsEqu0),
    cInstEntry4Stack("if-<=",     &cHardware4Stack::Inst_IfLsEqu),
    cInstEntry4Stack("if-A!=B",   &cHardware4Stack::Inst_IfANotEqB),
    cInstEntry4Stack("if-B!=C",   &cHardware4Stack::Inst_IfBNotEqC),
    cInstEntry4Stack("if-A!=C",   &cHardware4Stack::Inst_IfANotEqC),
    
    cInstEntry4Stack("if-bit-1",  &cHardware4Stack::Inst_IfBit1),
    cInstEntry4Stack("jump-f",    &cHardware4Stack::Inst_JumpF),
    cInstEntry4Stack("jump-b",    &cHardware4Stack::Inst_JumpB),
    cInstEntry4Stack("jump-p",    &cHardware4Stack::Inst_JumpP),
    cInstEntry4Stack("jump-slf",  &cHardware4Stack::Inst_JumpSelf),
    cInstEntry4Stack("call",      &cHardware4Stack::Inst_Call),
    cInstEntry4Stack("return",    &cHardware4Stack::Inst_Return),

    cInstEntry4Stack("pop",       &cHardware4Stack::Inst_Pop),
    cInstEntry4Stack("push",      &cHardware4Stack::Inst_Push),
    cInstEntry4Stack("swap-stk",  &cHardware4Stack::Inst_SwitchStack),
    cInstEntry4Stack("flip-stk",  &cHardware4Stack::Inst_FlipStack),
    cInstEntry4Stack("swap",      &cHardware4Stack::Inst_Swap),
    cInstEntry4Stack("swap-AB",   &cHardware4Stack::Inst_SwapAB),
    cInstEntry4Stack("swap-BC",   &cHardware4Stack::Inst_SwapBC),
    cInstEntry4Stack("swap-AC",   &cHardware4Stack::Inst_SwapAC),
    cInstEntry4Stack("copy-reg",  &cHardware4Stack::Inst_CopyReg),
    cInstEntry4Stack("set_A=B",   &cHardware4Stack::Inst_CopyRegAB),
    cInstEntry4Stack("set_A=C",   &cHardware4Stack::Inst_CopyRegAC),
    cInstEntry4Stack("set_B=A",   &cHardware4Stack::Inst_CopyRegBA),
    cInstEntry4Stack("set_B=C",   &cHardware4Stack::Inst_CopyRegBC),
    cInstEntry4Stack("set_C=A",   &cHardware4Stack::Inst_CopyRegCA),
    cInstEntry4Stack("set_C=B",   &cHardware4Stack::Inst_CopyRegCB),
    cInstEntry4Stack("reset",     &cHardware4Stack::Inst_Reset),

    cInstEntry4Stack("pop-A",     &cHardware4Stack::Inst_PopA),
    cInstEntry4Stack("pop-B",     &cHardware4Stack::Inst_PopB),
    cInstEntry4Stack("pop-C",     &cHardware4Stack::Inst_PopC),
    cInstEntry4Stack("push-A",    &cHardware4Stack::Inst_PushA),
    cInstEntry4Stack("push-B",    &cHardware4Stack::Inst_PushB),
    cInstEntry4Stack("push-C",    &cHardware4Stack::Inst_PushC),

    cInstEntry4Stack("shift-r",   &cHardware4Stack::Inst_ShiftR),
    cInstEntry4Stack("shift-l",   &cHardware4Stack::Inst_ShiftL),
    cInstEntry4Stack("bit-1",     &cHardware4Stack::Inst_Bit1),
    cInstEntry4Stack("set-num",   &cHardware4Stack::Inst_SetNum),
    cInstEntry4Stack("inc",       &cHardware4Stack::Inst_Inc),
    cInstEntry4Stack("dec",       &cHardware4Stack::Inst_Dec),
    cInstEntry4Stack("zero",      &cHardware4Stack::Inst_Zero),
    cInstEntry4Stack("neg",       &cHardware4Stack::Inst_Neg),
    cInstEntry4Stack("square",    &cHardware4Stack::Inst_Square),
    cInstEntry4Stack("sqrt",      &cHardware4Stack::Inst_Sqrt),
    cInstEntry4Stack("not",       &cHardware4Stack::Inst_Not),
    cInstEntry4Stack("minus-18",  &cHardware4Stack::Inst_Minus18),
    
    cInstEntry4Stack("add",       &cHardware4Stack::Inst_Add),
    cInstEntry4Stack("sub",       &cHardware4Stack::Inst_Sub),
    cInstEntry4Stack("mult",      &cHardware4Stack::Inst_Mult),
    cInstEntry4Stack("div",       &cHardware4Stack::Inst_Div),
    cInstEntry4Stack("mod",       &cHardware4Stack::Inst_Mod),
    cInstEntry4Stack("nand",      &cHardware4Stack::Inst_Nand),
    cInstEntry4Stack("nor",       &cHardware4Stack::Inst_Nor),
    cInstEntry4Stack("and",       &cHardware4Stack::Inst_And),
    cInstEntry4Stack("order",     &cHardware4Stack::Inst_Order),
    cInstEntry4Stack("xor",       &cHardware4Stack::Inst_Xor),
    
    cInstEntry4Stack("copy",      &cHardware4Stack::Inst_Copy),
    cInstEntry4Stack("read",      &cHardware4Stack::Inst_ReadInst),
    cInstEntry4Stack("write",     &cHardware4Stack::Inst_WriteInst),
    cInstEntry4Stack("stk-read",  &cHardware4Stack::Inst_StackReadInst),
    cInstEntry4Stack("stk-writ",  &cHardware4Stack::Inst_StackWriteInst),
    
    cInstEntry4Stack("compare",   &cHardware4Stack::Inst_Compare),
    cInstEntry4Stack("if-n-cpy",  &cHardware4Stack::Inst_IfNCpy),
    cInstEntry4Stack("allocate",  &cHardware4Stack::Inst_Allocate),
    cInstEntry4Stack("divide",    &cHardware4Stack::Inst_Divide),
    cInstEntry4Stack("c-alloc",   &cHardware4Stack::Inst_CAlloc),
    cInstEntry4Stack("c-divide",  &cHardware4Stack::Inst_CDivide),
    cInstEntry4Stack("inject",    &cHardware4Stack::Inst_Inject),
    cInstEntry4Stack("inject-r",  &cHardware4Stack::Inst_InjectRand),
    cInstEntry4Stack("get",       &cHardware4Stack::Inst_TaskGet),
    cInstEntry4Stack("stk-get",   &cHardware4Stack::Inst_TaskStackGet),
    cInstEntry4Stack("stk-load",  &cHardware4Stack::Inst_TaskStackLoad),
    cInstEntry4Stack("put",       &cHardware4Stack::Inst_TaskPut),
    cInstEntry4Stack("IO",        &cHardware4Stack::Inst_TaskIO),
    cInstEntry4Stack("search-f",  &cHardware4Stack::Inst_SearchF),
    cInstEntry4Stack("search-b",  &cHardware4Stack::Inst_SearchB),
    cInstEntry4Stack("mem-size",  &cHardware4Stack::Inst_MemSize),

    cInstEntry4Stack("rotate-l",  &cHardware4Stack::Inst_RotateL),
    cInstEntry4Stack("rotate-r",  &cHardware4Stack::Inst_RotateR),

    cInstEntry4Stack("set-cmut",  &cHardware4Stack::Inst_SetCopyMut),
    cInstEntry4Stack("mod-cmut",  &cHardware4Stack::Inst_ModCopyMut),

    // Threading instructions
    cInstEntry4Stack("fork-th",   &cHardware4Stack::Inst_ForkThread),
    cInstEntry4Stack("kill-th",   &cHardware4Stack::Inst_KillThread),
    cInstEntry4Stack("id-th",     &cHardware4Stack::Inst_ThreadID),

    // Head-based instructions
    cInstEntry4Stack("h-alloc",   &cHardware4Stack::Inst_MaxAlloc),
    cInstEntry4Stack("h-divide",  &cHardware4Stack::Inst_HeadDivide),
    cInstEntry4Stack("h-read",    &cHardware4Stack::Inst_HeadRead),
    cInstEntry4Stack("h-write",   &cHardware4Stack::Inst_HeadWrite),
    cInstEntry4Stack("h-copy",    &cHardware4Stack::Inst_HeadCopy),
    cInstEntry4Stack("h-search",  &cHardware4Stack::Inst_HeadSearch),
    cInstEntry4Stack("h-push",    &cHardware4Stack::Inst_HeadPush),
    cInstEntry4Stack("h-pop",     &cHardware4Stack::Inst_HeadPop),
    cInstEntry4Stack("set-head",  &cHardware4Stack::Inst_SetHead),
    cInstEntry4Stack("adv-head",  &cHardware4Stack::Inst_AdvanceHead),
    cInstEntry4Stack("mov-head",  &cHardware4Stack::Inst_MoveHead),
    cInstEntry4Stack("jmp-head",  &cHardware4Stack::Inst_JumpHead),
    cInstEntry4Stack("get-head",  &cHardware4Stack::Inst_GetHead),
    cInstEntry4Stack("if-label",  &cHardware4Stack::Inst_IfLabel),
    cInstEntry4Stack("set-flow",  &cHardware4Stack::Inst_SetFlow),

    cInstEntry4Stack("h-copy2",    &cHardware4Stack::Inst_HeadCopy2),
    cInstEntry4Stack("h-copy3",    &cHardware4Stack::Inst_HeadCopy3),
    cInstEntry4Stack("h-copy4",    &cHardware4Stack::Inst_HeadCopy4),
    cInstEntry4Stack("h-copy5",    &cHardware4Stack::Inst_HeadCopy5),
    cInstEntry4Stack("h-copy6",    &cHardware4Stack::Inst_HeadCopy6),
    cInstEntry4Stack("h-copy7",    &cHardware4Stack::Inst_HeadCopy7),
    cInstEntry4Stack("h-copy8",    &cHardware4Stack::Inst_HeadCopy8),
    cInstEntry4Stack("h-copy9",    &cHardware4Stack::Inst_HeadCopy9),
    cInstEntry4Stack("h-copy10",   &cHardware4Stack::Inst_HeadCopy10),

    cInstEntry4Stack("h-divide1",      &cHardware4Stack::Inst_HeadDivide1),
    cInstEntry4Stack("h-divide2",      &cHardware4Stack::Inst_HeadDivide2),
    cInstEntry4Stack("h-divide3",      &cHardware4Stack::Inst_HeadDivide3),
    cInstEntry4Stack("h-divide4",      &cHardware4Stack::Inst_HeadDivide4),
    cInstEntry4Stack("h-divide5",      &cHardware4Stack::Inst_HeadDivide5),
    cInstEntry4Stack("h-divide6",      &cHardware4Stack::Inst_HeadDivide6),
    cInstEntry4Stack("h-divide7",      &cHardware4Stack::Inst_HeadDivide7),
    cInstEntry4Stack("h-divide8",      &cHardware4Stack::Inst_HeadDivide8),
    cInstEntry4Stack("h-divide9",      &cHardware4Stack::Inst_HeadDivide9),
    cInstEntry4Stack("h-divide10",     &cHardware4Stack::Inst_HeadDivide10),
    cInstEntry4Stack("h-divide16",     &cHardware4Stack::Inst_HeadDivide16),
    cInstEntry4Stack("h-divide32",     &cHardware4Stack::Inst_HeadDivide32),
    cInstEntry4Stack("h-divide50",     &cHardware4Stack::Inst_HeadDivide50),
    cInstEntry4Stack("h-divide100",    &cHardware4Stack::Inst_HeadDivide100),
    cInstEntry4Stack("h-divide500",    &cHardware4Stack::Inst_HeadDivide500),
    cInstEntry4Stack("h-divide1000",   &cHardware4Stack::Inst_HeadDivide1000),
    cInstEntry4Stack("h-divide5000",   &cHardware4Stack::Inst_HeadDivide5000),
    cInstEntry4Stack("h-divide10000",  &cHardware4Stack::Inst_HeadDivide10000),
    cInstEntry4Stack("h-divide50000",  &cHardware4Stack::Inst_HeadDivide50000),
    cInstEntry4Stack("h-divide0.5",    &cHardware4Stack::Inst_HeadDivide0_5),
    cInstEntry4Stack("h-divide0.1",    &cHardware4Stack::Inst_HeadDivide0_1),
    cInstEntry4Stack("h-divide0.05",   &cHardware4Stack::Inst_HeadDivide0_05),
    cInstEntry4Stack("h-divide0.01",   &cHardware4Stack::Inst_HeadDivide0_01),
    cInstEntry4Stack("h-divide0.001",  &cHardware4Stack::Inst_HeadDivide0_001),

    // High-level instructions
    cInstEntry4Stack("repro",      &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-A",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-B",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-C",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-D",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-E",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-F",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-G",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-H",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-I",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-J",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-K",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-L",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-M",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-N",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-O",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-P",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-Q",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-R",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-S",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-T",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-U",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-V",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-W",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-X",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-Y",    &cHardware4Stack::Inst_Repro),
    cInstEntry4Stack("repro-Z",    &cHardware4Stack::Inst_Repro),

    // Placebo instructions
    // nop-x (included with nops)
    cInstEntry4Stack("skip",      &cHardware4Stack::Inst_Skip)
  };

  const int n_size = sizeof(s_n_array)/sizeof(cNOPEntry4Stack);
  static cString n_names[n_size];
  static int nop_mods[n_size];
  for (int i = 0; i < n_size; i++){
    n_names[i] = s_n_array[i].name;
    nop_mods[i] = s_n_array[i].nop_mod;
  }

  const int f_size = sizeof(s_f_array)/sizeof(cInstEntry4Stack);
  static cString f_names[f_size];
  static tHardware4StackMethod functions[f_size];
  for (int i = 0; i < f_size; i++){
    f_names[i] = s_f_array[i].name;
    functions[i] = s_f_array[i].function;
  }

  cInstLib4Stack *inst_lib = new cInstLib4Stack(
    n_size,
    f_size,
    n_names,
    f_names,
    nop_mods,
    functions
  );

  cout <<
  "<cHardware4Stack::initInstLib> test of ick" <<endl<<
  " --- GetSize(): " << inst_lib->GetSize() <<endl<<
  " --- GetNumNops(): " << inst_lib->GetNumNops() <<endl<<
  " --- GetName(last): " <<
  inst_lib->GetName(inst_lib->GetSize() - 1) <<endl<<
  endl;

  return inst_lib;
}

cHardware4Stack::cHardware4Stack(cOrganism * in_organism, cInstSet * in_inst_set)
  : cHardwareBase(in_organism, in_inst_set)
{
  /* FIXME:  reorganize storage of m_functions.  -- kgn */
  m_functions = s_inst_slib->GetFunctions();
  /**/
  memory = in_organism->GetGenome();  // Initialize memory...
  Reset();                            // Setup the rest of the hardware...
}


cHardware4Stack::~cHardware4Stack()
{
}


void cHardware4Stack::Recycle(cOrganism * new_organism, cInstSet * in_inst_set)
{
  cHardwareBase::Recycle(new_organism, in_inst_set);
  memory = new_organism->GetGenome();
  Reset();
}


void cHardware4Stack::Reset()
{
  global_stack.Clear();
  thread_time_used = 0;

  // We want to reset to have a single thread.
  threads.Resize(1);

  // Reset that single thread.
  threads[0].Reset(this, 0);
  thread_id_chart = 1; // Mark only the first thread as taken...
  cur_thread = 0;

  mal_active = false;

#ifdef INSTRUCTION_COSTS
  // instruction cost arrays
  const int num_inst_cost = GetNumInst();
  inst_cost.Resize(num_inst_cost);
  inst_ft_cost.Resize(num_inst_cost);

  for (int i = 0; i < num_inst_cost; i++) {
    inst_cost[i] = GetInstSet().GetCost(cInstruction(i));
    inst_ft_cost[i] = GetInstSet().GetFTCost(cInstruction(i));
  }
#endif

}


// This function processes the very next command in the genome, and is made
// to be as optimized as possible.  This is the heart of avida.

void cHardware4Stack::SingleProcess(ostream * trace_fp)
{
  organism->GetPhenotype().IncTimeUsed();
  if (GetNumThreads() > 1) thread_time_used++;

  // If we have threads turned on and we executed each thread in a single
  // timestep, adjust the number of instructions executed accordingly.
  const int num_inst_exec = (cConfig::GetThreadSlicingMethod() == 1) ?
    GetNumThreads() : 1;
  
  for (int i = 0; i < num_inst_exec; i++) {
    // Setup the hardware for the next instruction to be executed.
    NextThread();
    advance_ip = true;
    IP().Adjust();

#ifdef BREAKPOINTS
    if (IP().FlagBreakpoint() == true) {
      organism->DoBreakpoint();
    }
#endif
    
    // Print the status of this CPU at each step...
    if (trace_fp != NULL) organism->PrintStatus(*trace_fp);
    
    // Find the instruction to be executed
    const cInstruction & cur_inst = IP().GetInst();

    // Test if costs have been paid and it is okay to execute this now...
    const bool exec = SingleProcess_PayCosts(cur_inst);

    // Now execute the instruction...
    if (exec == true) {
      SingleProcess_ExecuteInst(cur_inst);

      // Some instruction (such as jump) may turn advance_ip off.  Ususally
      // we now want to move to the next instruction in the memory.
      if (advance_ip == true) IP().Advance();
    } // if exec
    
  } // Previous was executed once for each thread...

  // Kill creatures who have reached their max num of instructions executed
  const int max_executed = organism->GetMaxExecuted();
  if ( max_executed > 0 &&
       organism->GetPhenotype().GetTimeUsed() >= max_executed ) {
    organism->Die();
  }
}


// This method will test to see if all costs have been paid associated
// with executing an instruction and only return true when that instruction
// should proceed.
bool cHardware4Stack::SingleProcess_PayCosts(const cInstruction & cur_inst)
{
#ifdef INSTRUCTION_COSTS
  assert(cur_inst.GetOp() < inst_cost.GetSize());

  // If first time cost hasn't been paid off...
  if ( inst_ft_cost[cur_inst.GetOp()] > 0 ) {
    inst_ft_cost[cur_inst.GetOp()]--;       // dec cost
    return false;
  }
    
  // Next, look at the per use cost
  if ( GetInstSet().GetCost(cur_inst) > 0 ) {
    if ( inst_cost[cur_inst.GetOp()] > 1 ){  // if isn't paid off (>1)
      inst_cost[cur_inst.GetOp()]--;         // dec cost
      return false;
    } else {                                 // else, reset cost array
      inst_cost[cur_inst.GetOp()] = GetInstSet().GetCost(cur_inst);
    }
  }
    
  // Prob of exec
  if ( GetInstSet().GetProbFail(cur_inst) > 0.0 ){
    return !( g_random.P(GetInstSet().GetProbFail(cur_inst)) );
  }
#endif
  return true;
}

// This method will handle the actuall execution of an instruction
// within single process, once that function has been finalized.
bool cHardware4Stack::SingleProcess_ExecuteInst(const cInstruction & cur_inst) 
{
  // Get a pointer to the corrisponding method...
  /* XXX start -- kgn */
  //tHardwareMethod inst_ptr = GetInstSet().GetFunction(cur_inst);
  int inst_idx = GetInstSet().GetLibFunctionIndex(cur_inst);
  /* XXX end */
      
  // Mark the instruction as executed
  IP().FlagExecuted() = true;
	
#ifdef EXECUTION_ERRORS
  // If there is an execution error, execute a random instruction.
  if (organism->TestExeErr()) inst_ptr = GetInstSet().GetRandFunction();
#endif
	
#ifdef INSTRUCTION_COUNT
  // instruction execution count incremeneted
  organism->GetPhenotype().IncCurInstCount(cur_inst.GetOp());
#endif
	
  // And execute it.
  /* XXX start -- kgn */
  //const bool exec_success = (this->*inst_ptr)();
  const bool exec_success
  //= (this->*(s_inst_slib->GetFunctions()[inst_idx]))();
  = (this->*(m_functions[inst_idx]))();
  /* XXX end */
	
#ifdef INSTRUCTION_COUNT
  // decremenet if the instruction was not executed successfully
  if (exec_success == false) {
    organism->GetPhenotype().DecCurInstCount(cur_inst.GetOp());
  }
#endif	

  return exec_success;
}



void cHardware4Stack::LoadGenome(const cGenome & new_genome)
{
  GetMemory() = new_genome;
}


bool cHardware4Stack::OK()
{
  bool result = true;

  if (!memory.OK()) result = false;

  for (int i = 0; i < GetNumThreads(); i++) {
    assert (threads[i].input_pointer < IO_SIZE);

    if (threads[i].stack.OK() == false) result = false;
    if (threads[i].next_label.OK() == false) result = false;
  }

  return result;
}

void cHardware4Stack::PrintStatus(ostream & fp)
{
  fp << organism->GetPhenotype().GetTimeUsed() << " "
     << "IP:" << IP().GetPosition() << "    "

     << "AX:" << Register(REG_AX) << " "
     << setbase(16) << "[0x" << Register(REG_AX) << "]  " << setbase(10)

     << "BX:" << Register(REG_BX) << " "
     << setbase(16) << "[0x" << Register(REG_BX) << "]  " << setbase(10)

     << "CX:" << Register(REG_CX) << " "
     << setbase(16) << "[0x" << Register(REG_CX) << "]" << setbase(10)

     << endl;

  fp << "  R-Head:" << GetHead(HEAD_READ).GetPosition() << " "
     << "W-Head:" << GetHead(HEAD_WRITE).GetPosition()  << " "
     << "F-Head:" << GetHead(HEAD_FLOW).GetPosition()   << "  "
     << "RL:" << GetReadLabel().AsString() << "   "
     << "NEXT: >> " << inst_set->GetName(IP().GetInst())() << " <<"
     << endl;

  fp << "  Mem (" << GetMemory().GetSize() << "):"
		  << "  " << GetMemory().AsString()
		  << endl;
  fp.flush();
}


void cHardware4Stack::DoInput(const int value)
{
  organism->DoInput(value, GetInputBuffer(), GetOutputBuffer());
}

void cHardware4Stack::DoOutput(const int value)
{
  organism->DoOutput(value, GetInputBuffer(), GetOutputBuffer());
}




/////////////////////////////////////////////////////////////////////////
// Method: cHardware4Stack::FindLabel(direction)
//
// Search in 'direction' (+ or - 1) from the instruction pointer for the
// compliment of the label in 'next_label' and return a pointer to the
// results.  If direction is 0, search from the beginning of the genome.
//
/////////////////////////////////////////////////////////////////////////

cCPUHead cHardware4Stack::FindLabel(int direction)
{
  cCPUHead & inst_ptr = IP();

  // Start up a search head at the position of the instruction pointer.
  cCPUHead search_head(inst_ptr);
  cCodeLabel & search_label = GetLabel();

  // Make sure the label is of size > 0.

  if (search_label.GetSize() == 0) {
    return inst_ptr;
  }

  // Call special functions depending on if jump is forwards or backwards.
  int found_pos = 0;
  if( direction < 0 ) {
    found_pos = FindLabel_Backward(search_label, inst_ptr.GetMemory(),
			   inst_ptr.GetPosition() - search_label.GetSize());
  }

  // Jump forward.
  else if (direction > 0) {
    found_pos = FindLabel_Forward(search_label, inst_ptr.GetMemory(),
			   inst_ptr.GetPosition());
  }

  // Jump forward from the very beginning.
  else {
    found_pos = FindLabel_Forward(search_label, inst_ptr.GetMemory(), 0);
  }
  
  // Return the last line of the found label, if it was found.
  if (found_pos >= 0) search_head.Set(found_pos - 1);

  // Return the found position (still at start point if not found).
  return search_head;
}


// Search forwards for search_label from _after_ position pos in the
// memory.  Return the first line _after_ the the found label.  It is okay
// to find search label's match inside another label.

int cHardware4Stack::FindLabel_Forward(const cCodeLabel & search_label,
				 const cGenome & search_genome, int pos)
{
  assert (pos < search_genome.GetSize() && pos >= 0);

  int search_start = pos;
  int label_size = search_label.GetSize();
  bool found_label = false;

  // Move off the template we are on.
  pos += label_size;

  // Search until we find the complement or exit the memory.
  while (pos < search_genome.GetSize()) {

    // If we are within a label, rewind to the beginning of it and see if
    // it has the proper sub-label that we're looking for.

    if (inst_set->IsNop(search_genome[pos])) {
      // Find the start and end of the label we're in the middle of.

      int start_pos = pos;
      int end_pos = pos + 1;
      while (start_pos > search_start &&
	     inst_set->IsNop( search_genome[start_pos - 1] )) {
	start_pos--;
      }
      while (end_pos < search_genome.GetSize() &&
	     inst_set->IsNop( search_genome[end_pos] )) {
	end_pos++;
      }
      int test_size = end_pos - start_pos;

      // See if this label has the proper sub-label within it.
      int max_offset = test_size - label_size + 1;
      int offset = start_pos;
      for (offset = start_pos; offset < start_pos + max_offset; offset++) {

	// Test the number of matches for this offset.
	int matches;
	for (matches = 0; matches < label_size; matches++) {
	  if (search_label[matches] !=
	      inst_set->GetNopMod( search_genome[offset + matches] )) {
	    break;
	  }
	}

	// If we have found it, break out of this loop!
	if (matches == label_size) {
	  found_label = true;
	  break;
	}
      }

      // If we've found the complement label, set the position to the end of
      // the label we found it in, and break out.

      if (found_label == true) {
	// pos = end_pos;
	pos = label_size + offset;
	break;
      }

      // We haven't found it; jump pos to just after the current label being
      // checked.
      pos = end_pos;
    }

    // Jump up a block to the next possible point to find a label,
    pos += label_size;
  }

  // If the label was not found return a -1.
  if (found_label == false) pos = -1;

  return pos;
}

// Search backwards for search_label from _before_ position pos in the
// memory.  Return the first line _after_ the the found label.  It is okay
// to find search label's match inside another label.

int cHardware4Stack::FindLabel_Backward(const cCodeLabel & search_label,
				  const cGenome & search_genome, int pos)
{
  assert (pos < search_genome.GetSize());

  int search_start = pos;
  int label_size = search_label.GetSize();
  bool found_label = false;

  // Move off the template we are on.
  pos -= label_size;

  // Search until we find the complement or exit the memory.
  while (pos >= 0) {
    // If we are within a label, rewind to the beginning of it and see if
    // it has the proper sub-label that we're looking for.

    if (inst_set->IsNop( search_genome[pos] )) {
      // Find the start and end of the label we're in the middle of.

      int start_pos = pos;
      int end_pos = pos + 1;
      while (start_pos > 0 && inst_set->IsNop(search_genome[start_pos - 1])) {
	start_pos--;
      }
      while (end_pos < search_start &&
	     inst_set->IsNop(search_genome[end_pos])) {
	end_pos++;
      }
      int test_size = end_pos - start_pos;

      // See if this label has the proper sub-label within it.
      int max_offset = test_size - label_size + 1;
      for (int offset = start_pos; offset < start_pos + max_offset; offset++) {

	// Test the number of matches for this offset.
	int matches;
	for (matches = 0; matches < label_size; matches++) {
	  if (search_label[matches] !=
	      inst_set->GetNopMod(search_genome[offset + matches])) {
	    break;
	  }
	}

	// If we have found it, break out of this loop!
	if (matches == label_size) {
	  found_label = true;
	  break;
	}
      }

      // If we've found the complement label, set the position to the end of
      // the label we found it in, and break out.

      if (found_label == true) {
	pos = end_pos;
	break;
      }

      // We haven't found it; jump pos to just before the current label
      // being checked.
      pos = start_pos - 1;
    }

    // Jump up a block to the next possible point to find a label,
    pos -= label_size;
  }

  // If the label was not found return a -1.
  if (found_label == false) pos = -1;

  return pos;
}

// Search for 'in_label' anywhere in the hardware.
cCPUHead cHardware4Stack::FindLabel(const cCodeLabel & in_label, int direction)
{
  assert (in_label.GetSize() > 0);

  // IDEALY:
  // Keep making jumps (in the proper direction) equal to the label
  // length.  If we are inside of a label, check its size, and see if
  // any of the sub-labels match properly.
  // FOR NOW:
  // Get something which works, no matter how inefficient!!!

  cCPUHead temp_head(this);

  while (temp_head.InMemory()) {
    // IDEALY: Analyze the label we are in; see if the one we are looking
    // for could be a sub-label of it.  Skip past it if not.

    int i;
    for (i = 0; i < in_label.GetSize(); i++) {
      if (!inst_set->IsNop(temp_head.GetInst()) ||
	  in_label[i] != inst_set->GetNopMod(temp_head.GetInst())) {
	break;
      }
    }
    if (i == GetLabel().GetSize()) {
      temp_head.AbsJump(i - 1);
      return temp_head;
    }

    temp_head.AbsJump(direction);     // IDEALY: MAKE LARGER JUMPS
  }

  temp_head.AbsSet(-1);
  return temp_head;
}

// @CAO: direction is not currently used; should be used to indicate the
// direction which the heads[HEAD_IP] should progress through a creature.
cCPUHead cHardware4Stack::FindFullLabel(const cCodeLabel & in_label)
{
  // cout << "Running FindFullLabel with " << in_label.AsString() <<
  // endl;

  assert(in_label.GetSize() > 0); // Trying to find label of 0 size!

  cCPUHead temp_head(this);

  while (temp_head.InMemory()) {
    // If we are not in a label, jump to the next checkpoint...
    if (inst_set->IsNop(temp_head.GetInst())) {
      temp_head.AbsJump(in_label.GetSize());
      continue;
    }

    // Otherwise, rewind to the begining of this label...

    while (!(temp_head.AtFront()) && inst_set->IsNop(temp_head.GetInst(-1)))
      temp_head.AbsJump(-1);

    // Calculate the size of the label being checked, and make sure they
    // are equal.

    int checked_size = 0;
    while (inst_set->IsNop(temp_head.GetInst(checked_size))) {
      checked_size++;
    }
    if (checked_size != in_label.GetSize()) {
      temp_head.AbsJump(checked_size + 1);
      continue;
    }

    // cout << "Testing label at line " << temp_head.GetPosition() <<
    // endl;

    // ...and do the comparison...

    int j;
    bool label_match = true;
    for (j = 0; j < in_label.GetSize(); j++) {
      if (!inst_set->IsNop(temp_head.GetInst(j)) ||
	  in_label[j] != inst_set->GetNopMod(temp_head.GetInst(j))) {
	temp_head.AbsJump(in_label.GetSize() + 1);
	label_match = false;
	break;
      }
    }

    if (label_match) {
      // If we have found the label, return the position after it.
      temp_head.AbsJump(j - 1);
      return temp_head;
    }

    // We have not found the label... increment i.

    temp_head.AbsJump(in_label.GetSize() + 1);
  }

  // The label does not exist in this creature.

  temp_head.AbsSet(-1);
  return temp_head;
}


int cHardware4Stack::Inject(const cCodeLabel & in_label, const cGenome & injection)
{
  // Make sure the genome will be below max size after injection.

  const int new_size = injection.GetSize() + GetMemory().GetSize();
  if (new_size > MAX_CREATURE_SIZE) return 1; // (inject fails)

  const int inject_line = FindFullLabel(in_label).GetPosition();

  // Abort if no compliment is found.
  if (inject_line == -1) return 2; // (inject fails)

  // Inject the code!
  InjectCode(injection, inject_line+1);

  return 0; // (inject succeeds!)
}

int cHardware4Stack::InjectThread(const cCodeLabel & in_label, const cGenome & injection)
{
  // Make sure the genome will be below max size after injection.

  const int new_size = injection.GetSize() + GetMemory().GetSize();
  if (new_size > MAX_CREATURE_SIZE) return 1; // (inject fails)

  const int inject_line = FindFullLabel(in_label).GetPosition();

  // Abort if no compliment is found.
  if (inject_line == -1) return 2; // (inject fails)

  // Inject the code!
  InjectCodeThread(injection, inject_line+1);

  return 0; // (inject succeeds!)
}

void cHardware4Stack::InjectCode(const cGenome & inject_code, const int line_num)
{
  assert(line_num >= 0);
  assert(line_num <= memory.GetSize());
  assert(memory.GetSize() + inject_code.GetSize() < MAX_CREATURE_SIZE);

  // Inject the new code.
  const int inject_size = inject_code.GetSize();
  memory.Insert(line_num, inject_code);
  
  // Set instruction flags on the injected code
  for (int i = line_num; i < line_num + inject_size; i++) {
    memory.FlagInjected(i) = true;
  }
  organism->GetPhenotype().IsModified() = true;

  // Adjust all of the heads to take into account the new mem size.

  for (int i=0; i < NUM_HEADS; i++) {    
    if (!GetHead(i).TestParasite() &&
	GetHead(i).GetPosition() > line_num)
      GetHead(i).Jump(inject_size);
  }
}

void cHardware4Stack::InjectCodeThread(const cGenome & inject_code, const int line_num)
{
  assert(line_num >= 0);
  assert(line_num <= memory.GetSize());
  assert(memory.GetSize() + inject_code.GetSize() < MAX_CREATURE_SIZE);
  
  if(ForkThread())
    {
      // Inject the new code.
      const int inject_size = inject_code.GetSize();
      memory.Insert(line_num, inject_code);
      
      // Set instruction flags on the injected code
      for (int i = line_num; i < line_num + inject_size; i++) {
	memory.FlagInjected(i) = true;
      }
      organism->GetPhenotype().IsModified() = true;
      organism->GetPhenotype().IsMultiThread() = true;
      
      // Adjust all of the heads to take into account the new mem size.
      
      int currthread = GetCurThread();
      SetThread(0);
      for (int i=0; i<GetNumThreads()-2; i++)
	{
	  for (int j=0; j < NUM_HEADS; j++) 
	    {    
	      if (!GetHead(i).TestParasite() && GetHead(i).GetPosition() > line_num)
		GetHead(i).Jump(inject_size);
	    }
	  NextThread();
	}
      SetThread(currthread);
          
    }
  else
    {
      //Some kind of error message should go here...but what?
    }

}

void cHardware4Stack::Mutate(int mut_point)
{
  // Test if trying to mutate outside of genome...
  assert(mut_point >= 0 && mut_point < GetMemory().GetSize());

  GetMemory()[mut_point] = GetRandomInst();
  GetMemory().FlagMutated(mut_point) = true;
  GetMemory().FlagPointMut(mut_point) = true;
  //organism->GetPhenotype().IsMutated() = true;
  organism->CPUStats().mut_stats.point_mut_count++;
}

int cHardware4Stack::PointMutate(const double mut_rate)
{
  const int num_muts =
    g_random.GetRandBinomial(GetMemory().GetSize(), mut_rate);

  for (int i = 0; i < num_muts; i++) {
    const int pos = g_random.GetUInt(GetMemory().GetSize());
    Mutate(pos);
  }

  return num_muts;
}


// Trigger mutations of a specific type.  Outside triggers cannot specify
// a head since hardware types are not known.

bool cHardware4Stack::TriggerMutations(int trigger)
{
  // Only update triggers should happen from the outside!
  assert(trigger == MUTATION_TRIGGER_UPDATE);

  // Assume instruction pointer is the intended target (if one is even
  // needed!

  return TriggerMutations(trigger, IP());
}

bool cHardware4Stack::TriggerMutations(int trigger, cCPUHead & cur_head)
{
  // Collect information about mutations from the organism.
  cLocalMutations & mut_info = organism->GetLocalMutations();
  const tList<cMutation> & mut_list =
    mut_info.GetMutationLib().GetMutationList(trigger);

  // If we have no mutations for this trigger, stop here.
  if (mut_list.GetSize() == 0) return false;
  bool has_mutation = false;

  // Determine what memory this mutation will be affecting.
  cCPUMemory & target_mem = (trigger == MUTATION_TRIGGER_DIVIDE) 
    ? organism->ChildGenome() : GetMemory();

  // Loop through all mutations associated with this trigger and test them.
  tConstListIterator<cMutation> mut_it(mut_list);

  while (mut_it.Next() != NULL) {
    const cMutation * cur_mut = mut_it.Get();
    const int mut_id = cur_mut->GetID();
    const int scope = cur_mut->GetScope();
    const double rate = mut_info.GetRate(mut_id);
    switch (scope) {
    case MUTATION_SCOPE_GENOME:
      if (TriggerMutations_ScopeGenome(cur_mut, target_mem, cur_head, rate)) {
	has_mutation = true;
	mut_info.IncCount(mut_id);
      }
      break;
    case MUTATION_SCOPE_LOCAL:
    case MUTATION_SCOPE_PROP:
      if (TriggerMutations_ScopeLocal(cur_mut, target_mem, cur_head, rate)) {
	has_mutation = true;
	mut_info.IncCount(mut_id);
      }
      break;
    case MUTATION_SCOPE_GLOBAL:
    case MUTATION_SCOPE_SPREAD:
      int num_muts =
	TriggerMutations_ScopeGlobal(cur_mut, target_mem, cur_head, rate);
      if (num_muts > 0) {
	has_mutation = true;
	mut_info.IncCount(mut_id, num_muts);
      }
      break;
    }
  }

  return has_mutation;
}

bool cHardware4Stack::TriggerMutations_ScopeGenome(const cMutation * cur_mut,
          cCPUMemory & target_memory, cCPUHead & cur_head, const double rate)
{
  // The rate we have stored indicates the probability that a single
  // mutation will occur anywhere in the genome.
  
  if (g_random.P(rate) == true) {
    // We must create a temporary head and use it to randomly determine the
    // position in the genome to be mutated.
    cCPUHead tmp_head(cur_head);
    tmp_head.AbsSet(g_random.GetUInt(target_memory.GetSize()));
    TriggerMutations_Body(cur_mut->GetType(), target_memory, tmp_head);
    return true;
  }
  return false;
}

bool cHardware4Stack::TriggerMutations_ScopeLocal(const cMutation * cur_mut,
          cCPUMemory & target_memory, cCPUHead & cur_head, const double rate)
{
  // The rate we have stored is the probability for a mutation at this single
  // position in the genome.

  if (g_random.P(rate) == true) {
    TriggerMutations_Body(cur_mut->GetType(), target_memory, cur_head);
    return true;
  }
  return false;
}

int cHardware4Stack::TriggerMutations_ScopeGlobal(const cMutation * cur_mut,
          cCPUMemory & target_memory, cCPUHead & cur_head, const double rate)
{
  // The probability we have stored is per-site, so we can pull a random
  // number from a binomial distribution to determine the number of mutations
  // that should occur.

  const int num_mut =
    g_random.GetRandBinomial(target_memory.GetSize(), rate);

  if (num_mut > 0) {
    for (int i = 0; i < num_mut; i++) {
      cCPUHead tmp_head(cur_head);
      tmp_head.AbsSet(g_random.GetUInt(target_memory.GetSize()));
      TriggerMutations_Body(cur_mut->GetType(), target_memory, tmp_head);
    }
  }

  return num_mut;
}

void cHardware4Stack::TriggerMutations_Body(int type, cCPUMemory & target_memory,
					 cCPUHead & cur_head)
{
  const int pos = cur_head.GetPosition();

  switch (type) {
  case MUTATION_TYPE_POINT:
    target_memory[pos] = GetRandomInst();
    target_memory.FlagMutated(pos) = true;
    break;
  case MUTATION_TYPE_INSERT:
  case MUTATION_TYPE_DELETE:
  case MUTATION_TYPE_HEAD_INC:
  case MUTATION_TYPE_HEAD_DEC:
  case MUTATION_TYPE_TEMP:
  case MUTATION_TYPE_KILL:
  default:
    cout << "Error: Mutation type not implemented!" << endl;
    break;
  };
}

void cHardware4Stack::ReadInst(const int in_inst)
{
  if (inst_set->IsNop( cInstruction(in_inst) )) {
    GetReadLabel().AddNop(in_inst);
  } else {
    GetReadLabel().Clear();
  }
}


void cHardware4Stack::AdjustHeads()
{
  for (int i = 0; i < GetNumThreads(); i++) {
    for (int j = 0; j < NUM_HEADS; j++) {
      threads[i].heads[j].Adjust();
    }
  }
}



// This function looks at the current position in the info of a creature,
// and sets the next_label to be the sequence of nops which follows.  The
// instruction pointer is left on the last line of the label found.

void cHardware4Stack::ReadLabel(int max_size)
{
  int count = 0;
  cCPUHead * inst_ptr = &( IP() );

  GetLabel().Clear();

  while (inst_set->IsNop(inst_ptr->GetNextInst()) &&
	 (count < max_size)) {
    count++;
    inst_ptr->Advance();
    GetLabel().AddNop(inst_set->GetNopMod(inst_ptr->GetInst()));

    // If this is the first line of the template, mark it executed.
    if (GetLabel().GetSize() <=	cConfig::GetMaxLabelExeSize()) {
      inst_ptr->FlagExecuted() = true;
    }
  }
}


bool cHardware4Stack::ForkThread()
{
  const int num_threads = GetNumThreads();
  if (num_threads == cConfig::GetMaxCPUThreads()) return false;

  // Make room for the new thread.
  threads.Resize(num_threads + 1);

  // Initialize the new thread to the same values as the current one.
  threads[num_threads] = threads[cur_thread];

  // Find the first free bit in thread_id_chart to determine the new
  // thread id.
  int new_id = 0;
  while ( (thread_id_chart >> new_id) & 1 == 1) new_id++;
  threads[num_threads].SetID(new_id);
  thread_id_chart |= (1 << new_id);

  return true;
}


int cHardware4Stack::TestParasite() const
{
  return IP().TestParasite();
}


bool cHardware4Stack::KillThread()
{
  // Make sure that there is always at least one thread...
  if (GetNumThreads() == 1) return false;

  // Note the current thread and set the current back one.
  const int kill_thread = cur_thread;
  PrevThread();
  
  // Turn off this bit in the thread_id_chart...
  thread_id_chart ^= 1 << threads[kill_thread].GetID();

  // Copy the last thread into the kill position
  const int last_thread = GetNumThreads() - 1;
  if (last_thread != kill_thread) {
    threads[kill_thread] = threads[last_thread];
  }

  // Kill the thread!
  threads.Resize(GetNumThreads() - 1);

  if (cur_thread > kill_thread) cur_thread--;

  return true;
}


void cHardware4Stack::SaveState(ostream & fp)
{
  // note, memory & child_memory handled by cpu (@CAO Not any more!)
  assert(fp.good());

  fp<<"cHardware4Stack"<<endl;

  // global_stack (in inverse order so load can just push)
  global_stack.SaveState(fp);

  fp << thread_time_used  << endl;
  fp << GetNumThreads()   << endl;
  fp << cur_thread        << endl;

  // Threads
  for( int i = 0; i < GetNumThreads(); i++ ) {
    threads[i].SaveState(fp);
  }
}


void cHardware4Stack::LoadState(istream & fp)
{
  // note, memory & child_memory handled by cpu (@CAO Not any more!)
  assert(fp.good());

  cString foo;
  fp>>foo;
  assert( foo == "cHardware4Stack" );

  // global_stack
  global_stack.LoadState(fp);

  int num_threads;
  fp >> thread_time_used;
  fp >> num_threads;
  fp >> cur_thread;

  // Threads
  for( int i = 0; i < num_threads; i++ ){
    threads[i].LoadState(fp);
  }
}


////////////////////////////
//  Instruction Helpers...
////////////////////////////

inline int cHardware4Stack::FindModifiedRegister(int default_register)
{
  assert(default_register < NUM_REGISTERS);  // Reg ID too high.

  if (GetInstSet().IsNop(IP().GetNextInst())) {
    IP().Advance();
    default_register = GetInstSet().GetNopMod(IP().GetInst());
    IP().FlagExecuted() = true;
  }
  return default_register;
}


inline int cHardware4Stack::FindModifiedHead(int default_head)
{
  assert(default_head < NUM_HEADS); // Head ID too high.

  if (GetInstSet().IsNop(IP().GetNextInst())) {
    IP().Advance();
    default_head = GetInstSet().GetNopMod(IP().GetInst());
    IP().FlagExecuted() = true;
  }
  return default_head;
}


inline int cHardware4Stack::FindComplementRegister(int base_reg)
{
  const int comp_reg = base_reg + 1;
  return (comp_reg  == NUM_REGISTERS) ? 0 : comp_reg;
}


inline void cHardware4Stack::Fault(int fault_loc, int fault_type, cString fault_desc)
{
  organism->Fault(fault_loc, fault_type, fault_desc);
}


bool cHardware4Stack::Allocate_Necro(const int new_size)
{
  GetMemory().ResizeOld(new_size);
  return true;
}

bool cHardware4Stack::Allocate_Random(const int old_size, const int new_size)
{
  GetMemory().Resize(new_size);

  for (int i = old_size; i < new_size; i++) {
    GetMemory()[i] = GetInstSet().GetRandomInst();
  }
  return true;
}

bool cHardware4Stack::Allocate_Default(const int new_size)
{
  GetMemory().Resize(new_size);

  // New space already defaults to default instruction...

  return true;
}

bool cHardware4Stack::Allocate_Main(const int allocated_size)
{
  // must do divide before second allocate & must allocate positive amount...
  if (cConfig::GetRequireAllocate() && mal_active == true) {
    Fault(FAULT_LOC_ALLOC, FAULT_TYPE_ERROR, "Allocate already active");
    return false;
  }
  if (allocated_size < 1) {
    Fault(FAULT_LOC_ALLOC, FAULT_TYPE_ERROR,
	  cStringUtil::Stringf("Allocate of %d too small", allocated_size));
    return false;
  }

  const int old_size = GetMemory().GetSize();
  const int new_size = old_size + allocated_size;

  // Make sure that the new size is in range.
  if (new_size > MAX_CREATURE_SIZE  ||  new_size < MIN_CREATURE_SIZE) {
    Fault(FAULT_LOC_ALLOC, FAULT_TYPE_ERROR,
	  cStringUtil::Stringf("Invalid post-allocate size (%d)",
			       new_size));
    return false;
  }

  const int max_alloc_size = (int) (old_size * cConfig::GetChildSizeRange());
  if (allocated_size > max_alloc_size) {
    Fault(FAULT_LOC_ALLOC, FAULT_TYPE_ERROR,
	  cStringUtil::Stringf("Allocate too large (%d > %d)",
			       allocated_size, max_alloc_size));
    return false;
  }

  const int max_old_size =
    (int) (allocated_size * cConfig::GetChildSizeRange());
  if (old_size > max_old_size) {
    Fault(FAULT_LOC_ALLOC, FAULT_TYPE_ERROR,
	  cStringUtil::Stringf("Allocate too small (%d > %d)",
			       old_size, max_old_size));
    return false;
  }

  switch (cConfig::GetAllocMethod()) {
  case ALLOC_METHOD_NECRO:
    // Only break if this succeeds -- otherwise just do random.
    if (Allocate_Necro(new_size) == true) break;
  case ALLOC_METHOD_RANDOM:
    Allocate_Random(old_size, new_size);
    break;
  case ALLOC_METHOD_DEFAULT:
    Allocate_Default(new_size);
    break;
  }

  mal_active = true;

  return true;
}


bool cHardware4Stack::Divide_CheckViable(const int child_size,
				      const int parent_size)
{
  // Make sure the organism is okay with dividing now...
  if (organism->Divide_CheckViable() == false) return false; // (divide fails)

  // If required, make sure an allocate has occured.
  if (cConfig::GetRequireAllocate() && mal_active == false) {
    Fault(FAULT_LOC_DIVIDE, FAULT_TYPE_ERROR, "Must allocate before divide");
    return false; //  (divide fails)
  }

  // Make sure that neither parent nor child will be below the minimum size.

  const int genome_size = organism->GetGenome().GetSize();
  const double size_range = cConfig::GetChildSizeRange();
  const int min_size = Max(MIN_CREATURE_SIZE, (int) (genome_size/size_range));
  const int max_size = Min(MAX_CREATURE_SIZE, (int) (genome_size*size_range));
  
  if (child_size < min_size || child_size > max_size) {
    Fault(FAULT_LOC_DIVIDE, FAULT_TYPE_ERROR,
	  cStringUtil::Stringf("Invalid offspring length (%d)", child_size));
    return false; // (divide fails)
  }
  if (parent_size < min_size || parent_size > max_size) {
    Fault(FAULT_LOC_DIVIDE, FAULT_TYPE_ERROR,
	  cStringUtil::Stringf("Invalid post-divide length (%d)",parent_size));
    return false; // (divide fails)
  }

  // Count the number of lines executed in the parent, and make sure the
  // specified fraction has been reached.

  int executed_size = 0;
  for (int i = 0; i < parent_size; i++) {
    if (GetMemory().FlagExecuted(i)) executed_size++;
  }

  const int min_exe_lines = (int) (parent_size * cConfig::GetMinExeLines());
  if (executed_size < min_exe_lines) {
    Fault(FAULT_LOC_DIVIDE, FAULT_TYPE_ERROR,
	  cStringUtil::Stringf("Too few executed lines (%d < %d)",
			       executed_size, min_exe_lines));
    return false; // (divide fails)
  }
	
  // Count the number of lines which were copied into the child, and make
  // sure the specified fraction has been reached.

  int copied_size = 0;
  for (int i = parent_size; i < parent_size + child_size; i++) {
    if (GetMemory().FlagCopied(i)) copied_size++;
  }

  const int min_copied =  (int) (child_size * cConfig::GetMinCopiedLines());
  if (copied_size < min_copied) {
    Fault(FAULT_LOC_DIVIDE, FAULT_TYPE_ERROR,
	  cStringUtil::Stringf("Too few copied commands (%d < %d)",
			       copied_size, min_copied));
    return false; // (divide fails)
  }

  // Save the information we collected here...
  organism->GetPhenotype().SetLinesExecuted(executed_size);
  organism->GetPhenotype().SetLinesCopied(copied_size);

  return true; // (divide succeeds!)
}

void cHardware4Stack::Divide_DoMutations(double mut_multiplier)
{
  sCPUStats & cpu_stats = organism->CPUStats();
  cCPUMemory & child_genome = organism->ChildGenome();
  
  organism->GetPhenotype().SetDivType(mut_multiplier);

  // Divide Mutations
  if (organism->TestDivideMut()) {
    const UINT mut_line = g_random.GetUInt(child_genome.GetSize());
    child_genome[mut_line] = GetRandomInst();
    cpu_stats.mut_stats.divide_mut_count++;
  }

  // Divide Insertions
  if (organism->TestDivideIns() && child_genome.GetSize() < MAX_CREATURE_SIZE){
    const UINT mut_line = g_random.GetUInt(child_genome.GetSize() + 1);
    child_genome.Insert(mut_line, GetRandomInst());
    cpu_stats.mut_stats.divide_insert_mut_count++;
  }

  // Divide Deletions
  if (organism->TestDivideDel() && child_genome.GetSize() > MIN_CREATURE_SIZE){
    const UINT mut_line = g_random.GetUInt(child_genome.GetSize());
    // if( child_genome.FlagCopied(mut_line) == true) copied_size_change--;
    child_genome.Remove(mut_line);
    cpu_stats.mut_stats.divide_delete_mut_count++;
  }

  // Divide Mutations (per site)
  if(organism->GetDivMutProb() > 0){
    int num_mut = g_random.GetRandBinomial(child_genome.GetSize(), 
				   	   organism->GetDivMutProb() / mut_multiplier);
    // If we have lines to mutate...
    if( num_mut > 0 ){
      for (int i = 0; i < num_mut; i++) {
	int site = g_random.GetUInt(child_genome.GetSize());
	child_genome[site]=GetRandomInst();
	cpu_stats.mut_stats.div_mut_count++;
      }
    }
  }


  // Insert Mutations (per site)
  if(organism->GetInsMutProb() > 0){
    int num_mut = g_random.GetRandBinomial(child_genome.GetSize(),
					   organism->GetInsMutProb());
    // If would make creature to big, insert up to MAX_CREATURE_SIZE
    if( num_mut + child_genome.GetSize() > MAX_CREATURE_SIZE ){
      num_mut = MAX_CREATURE_SIZE - child_genome.GetSize();
    }
    // If we have lines to insert...
    if( num_mut > 0 ){
      // Build a list of the sites where mutations occured
      static int mut_sites[MAX_CREATURE_SIZE];
      for (int i = 0; i < num_mut; i++) {
	mut_sites[i] = g_random.GetUInt(child_genome.GetSize() + 1);
      }
      // Sort the list
      qsort( (void*)mut_sites, num_mut, sizeof(int), &IntCompareFunction );
      // Actually do the mutations (in reverse sort order)
      for(int i = num_mut-1; i >= 0; i--) {
	child_genome.Insert(mut_sites[i], GetRandomInst());
	cpu_stats.mut_stats.insert_mut_count++;
      }
    }
  }


  // Delete Mutations (per site)
  if( organism->GetDelMutProb() > 0 ){
    int num_mut = g_random.GetRandBinomial(child_genome.GetSize(),
					   organism->GetDelMutProb());
    // If would make creature too small, delete down to MIN_CREATURE_SIZE
    if (child_genome.GetSize() - num_mut < MIN_CREATURE_SIZE) {
      num_mut = child_genome.GetSize() - MIN_CREATURE_SIZE;
    }

    // If we have lines to delete...
    for (int i = 0; i < num_mut; i++) {
      int site = g_random.GetUInt(child_genome.GetSize());
      // if (child_genome.FlagCopied(site) == true) copied_size_change--;
      child_genome.Remove(site);
      cpu_stats.mut_stats.delete_mut_count++;
    }
  }

  // Mutations in the parent's genome
  if (organism->GetParentMutProb() > 0) {
    for (int i = 0; i < GetMemory().GetSize(); i++) {
      if (organism->TestParentMut()) {
	GetMemory()[i] = GetRandomInst();
	cpu_stats.mut_stats.parent_mut_line_count++;
      }
    }
  }


  // Count up mutated lines
  for(int i = 0; i < GetMemory().GetSize(); i++){
    if (GetMemory().FlagPointMut(i) == true) {
      cpu_stats.mut_stats.point_mut_line_count++;
    }
  }
  for(int i = 0; i < child_genome.GetSize(); i++){
    if( child_genome.FlagCopyMut(i) == true) {
      cpu_stats.mut_stats.copy_mut_line_count++;
    }
  }
}


// test whether the offspring creature contains an advantageous mutation.
void cHardware4Stack::Divide_TestFitnessMeasures()
{
  cPhenotype & phenotype = organism->GetPhenotype();
  phenotype.CopyTrue() = ( organism->ChildGenome() == organism->GetGenome() );
  phenotype.ChildFertile() = true;

  // Only continue if we're supposed to do a fitness test on divide...
  if (organism->GetTestOnDivide() == false) return;

  // If this was a perfect copy, then we don't need to worry about any other
  // tests...  Theoretically, we need to worry about the parent changing,
  // but as long as the child is always compared to the original genotype,
  // this won't be an issue.
  if (phenotype.CopyTrue() == true) return;

  const double parent_fitness = organism->GetTestFitness();
  const double neut_min = parent_fitness * FITNESS_NEUTRAL_MIN;
  const double neut_max = parent_fitness * FITNESS_NEUTRAL_MAX;
  
  cCPUTestInfo test_info;
  test_info.UseRandomInputs();
  cTestCPU::TestGenome(test_info, organism->ChildGenome());
  const double child_fitness = test_info.GetGenotypeFitness();
  
  bool revert = false;
  bool sterilize = false;
  
  // If implicit mutations are turned off, make sure this won't spawn one.
  if (organism->GetFailImplicit() == true) {
    if (test_info.GetMaxDepth() > 0) sterilize = true;
  }
  
  if (child_fitness == 0.0) {
    // Fatal mutation... test for reversion.
    if (g_random.P(organism->GetRevertFatal())) revert = true;
    if (g_random.P(organism->GetSterilizeFatal())) sterilize = true;
  } else if (child_fitness < neut_min) {
    if (g_random.P(organism->GetRevertNeg())) revert = true;
    if (g_random.P(organism->GetSterilizeNeg())) sterilize = true;
  } else if (child_fitness <= neut_max) {
    if (g_random.P(organism->GetRevertNeut())) revert = true;
    if (g_random.P(organism->GetSterilizeNeut())) sterilize = true;
  } else {
    if (g_random.P(organism->GetRevertPos())) revert = true;
    if (g_random.P(organism->GetSterilizePos())) sterilize = true;
  }
  
  // Ideally, we won't have reversions and sterilizations turned on at the
  // same time, but if we do, give revert the priority.
  if (revert == true) {
    organism->ChildGenome() = organism->GetGenome();
  }

  if (sterilize == true) {
    organism->GetPhenotype().ChildFertile() = false;
  }
}


bool cHardware4Stack::Divide_Main(const int div_point, const int extra_lines, double mut_multiplier)
{
  const int child_size = GetMemory().GetSize() - div_point - extra_lines;

  // Make sure this divide will produce a viable offspring.
  const bool viable = Divide_CheckViable(child_size, div_point);
  if (viable == false) return false;

  // Since the divide will now succeed, set up the information to be sent
  // to the new organism
  cGenome & child_genome = organism->ChildGenome();
  child_genome = cGenomeUtil::Crop(memory, div_point, div_point+child_size);

  // Cut off everything in this memory past the divide point.
  GetMemory().Resize(div_point);

  // Handle Divide Mutations...
  Divide_DoMutations(mut_multiplier);

  // Many tests will require us to run the offspring through a test CPU;
  // this is, for example, to see if mutations need to be reverted or if
  // lineages need to be updated.
  Divide_TestFitnessMeasures();

#ifdef INSTRUCTION_COSTS
  // reset first time instruction costs
  for (int i = 0; i < inst_ft_cost.GetSize(); i++) {
    inst_ft_cost[i] = GetInstSet().GetFTCost(cInstruction(i));
  }
#endif

  mal_active = false;
  if (cConfig::GetDivideMethod() == DIVIDE_METHOD_SPLIT) {
    advance_ip = false;
  }

  // Activate the child, and do more work if the parent lives through the
  // birth.
  bool parent_alive = organism->ActivateDivide();
  if (parent_alive) {
    if (cConfig::GetDivideMethod() == DIVIDE_METHOD_SPLIT) Reset();
  }

  return true;
}


//////////////////////////
// And the instructions...
//////////////////////////

bool cHardware4Stack::Inst_If0()          // Execute next if ?bx? ==0.
{
  const int reg_used = FindModifiedRegister(REG_BX);
  if (Register(reg_used) != 0)  IP().Advance();
  return true; 
}

bool cHardware4Stack::Inst_IfNot0()       // Execute next if ?bx? != 0.
{ 
  const int reg_used = FindModifiedRegister(REG_BX);
  if (Register(reg_used) == 0)  IP().Advance();
  return true;
}

bool cHardware4Stack::Inst_IfEqu()      // Execute next if bx == ?cx?
{
  const int reg_used = FindModifiedRegister(REG_BX);
  const int reg_used2 = FindComplementRegister(reg_used);
  if (Register(reg_used) != Register(reg_used2))  IP().Advance();
  return true;
}

bool cHardware4Stack::Inst_IfNEqu()     // Execute next if bx != ?cx?
{
  const int reg_used = FindModifiedRegister(REG_BX);
  const int reg_used2 = FindComplementRegister(reg_used);
  if (Register(reg_used) == Register(reg_used2))  IP().Advance();
  return true;
}

bool cHardware4Stack::Inst_IfGr0()       // Execute next if ?bx? ! < 0.
{
  const int reg_used = FindModifiedRegister(REG_BX);
  if (Register(reg_used) <= 0)  IP().Advance();
  return true;
}

bool cHardware4Stack::Inst_IfGr()       // Execute next if bx > ?cx?
{
  const int reg_used = FindModifiedRegister(REG_BX);
  const int reg_used2 = FindComplementRegister(reg_used);
  if (Register(reg_used) <= Register(reg_used2))  IP().Advance();
  return true;
}

bool cHardware4Stack::Inst_IfGrEqu0()       // Execute next if ?bx? != 0.
{
  const int reg_used = FindModifiedRegister(REG_BX);
  if (Register(reg_used) < 0)  IP().Advance();
  return true;
}

bool cHardware4Stack::Inst_IfGrEqu()       // Execute next if bx > ?cx?
{
  const int reg_used = FindModifiedRegister(REG_BX);
  const int reg_used2 = FindComplementRegister(reg_used);
  if (Register(reg_used) < Register(reg_used2)) IP().Advance();
  return true;
}

bool cHardware4Stack::Inst_IfLess0()       // Execute next if ?bx? != 0.
{
  const int reg_used = FindModifiedRegister(REG_BX);
  if (Register(reg_used) >= 0)  IP().Advance();
  return true;
}

bool cHardware4Stack::Inst_IfLess()       // Execute next if ?bx? < ?cx?
{
  const int reg_used = FindModifiedRegister(REG_BX);
  const int reg_used2 = FindComplementRegister(reg_used);
  if (Register(reg_used) >=  Register(reg_used2))  IP().Advance();
  return true;
}

bool cHardware4Stack::Inst_IfLsEqu0()       // Execute next if ?bx? != 0.
{
  const int reg_used = FindModifiedRegister(REG_BX);
  if (Register(reg_used) > 0) IP().Advance();
  return true;
}

bool cHardware4Stack::Inst_IfLsEqu()       // Execute next if bx > ?cx?
{
  const int reg_used = FindModifiedRegister(REG_BX);
  const int reg_used2 = FindComplementRegister(reg_used);
  if (Register(reg_used) >  Register(reg_used2))  IP().Advance();
  return true;
}

bool cHardware4Stack::Inst_IfBit1()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  if ((Register(reg_used) & 1) == 0)  IP().Advance();
  return true;
}

bool cHardware4Stack::Inst_IfANotEqB()     // Execute next if AX != BX
{
  if (Register(REG_AX) == Register(REG_BX) )  IP().Advance();
  return true;
}

bool cHardware4Stack::Inst_IfBNotEqC()     // Execute next if BX != CX
{
  if (Register(REG_BX) == Register(REG_CX) )  IP().Advance();
  return true;
}

bool cHardware4Stack::Inst_IfANotEqC()     // Execute next if AX != BX
{
  if (Register(REG_AX) == Register(REG_CX) )  IP().Advance();
  return true;
}

bool cHardware4Stack::Inst_JumpF()
{
  ReadLabel();
  GetLabel().Rotate(1);

  // If there is no label, jump BX steps.
  if (GetLabel().GetSize() == 0) {
    GetActiveHead().Jump(Register(REG_BX));
    return true;
  }

  // Otherwise, try to jump to the complement label.
  const cCPUHead jump_location(FindLabel(1));
  if ( jump_location.GetPosition() != -1 ) {
    GetActiveHead().Set(jump_location);
    return true;
  }

  // If complement label was not found; record an error.
  organism->Fault(FAULT_LOC_JUMP, FAULT_TYPE_ERROR,
		  "jump-f: No complement label");
  return false;
}


bool cHardware4Stack::Inst_JumpB()
{
  ReadLabel();
  GetLabel().Rotate(1);

  // If there is no label, jump BX steps.
  if (GetLabel().GetSize() == 0) {
    GetActiveHead().Jump(-Register(REG_BX));
    return true;
  }

  // otherwise jump to the complement label.
  const cCPUHead jump_location(FindLabel(-1));
  if ( jump_location.GetPosition() != -1 ) {
    GetActiveHead().Set(jump_location);
    return true;
  }

  // If complement label was not found; record an error.
  organism->Fault(FAULT_LOC_JUMP, FAULT_TYPE_ERROR,
		  "jump-b: No complement label");
  return false;
}

bool cHardware4Stack::Inst_JumpP()
{
  cOrganism * other_organism = organism->GetNeighbor();

  // Make sure the other organism was found and that its hardware is of the
  // same type, or else we won't be able to be parasitic on it.
  if (other_organism == NULL ||
      other_organism->GetHardware().GetType() != GetType()) {
    // Without another organism, its hard to determine if we're dealing
    // with a parasite.  For the moment, we'll assume it is and move on.
    // @CAO Do better!
    organism->GetPhenotype().IsParasite() = true;
    return true;
  }

  // Otherwise, grab the hardware from the neighbor, and use it!
  cHardware4Stack & other_hardware = (cHardware4Stack &) other_organism->GetHardware();

  ReadLabel();
  GetLabel().Rotate(1);

  // If there is no label, jump to line BX in creature.
  if (GetLabel().GetSize() == 0) {
    const int new_pos = Register(REG_BX);
    IP().Set(new_pos, &other_hardware);
    organism->GetPhenotype().IsParasite() = true;
    return true;
  }

  // otherwise jump to the complement label.
  const cCPUHead jump_location(other_hardware.FindFullLabel(GetLabel()));
  if (jump_location.GetPosition() != -1) {
    IP().Set(jump_location);
    organism->GetPhenotype().IsParasite() = true;
    return true;
  }

  // If complement label was not found; record a warning (since the
  // actual neighbors are not under the organisms control, this is not
  // a full-scale error).
  organism->Fault(FAULT_LOC_JUMP, FAULT_TYPE_WARNING,
		  "jump-p: No complement label");
  return false;
}

bool cHardware4Stack::Inst_JumpSelf()
{
  ReadLabel();
  GetLabel().Rotate(1);

  // If there is no label, jump to line BX in creature.
  if (GetLabel().GetSize() == 0) {
    IP().Set(Register(REG_BX), this);
    return true;
  }

  // otherwise jump to the complement label.
  const cCPUHead jump_location( FindFullLabel(GetLabel()) );
  if ( jump_location.GetPosition() != -1 ) {
    IP().Set(jump_location);
    return true;
  }

  // If complement label was not found; record an error.
  organism->Fault(FAULT_LOC_JUMP, FAULT_TYPE_ERROR,
		  "jump-slf: no complement label");
  return false;
}

bool cHardware4Stack::Inst_Call()
{
  // Put the starting location onto the stack
  const int location = IP().GetPosition();
  StackPush(location);

  // Jump to the compliment label (or by the ammount in the bx register)
  ReadLabel();
  GetLabel().Rotate(1);

  if (GetLabel().GetSize() == 0) {
    IP().Jump(Register(REG_BX));
    return true;
  }

  const cCPUHead jump_location(FindLabel(1));
  if (jump_location.GetPosition() != -1) {
    IP().Set(jump_location);
    return true;
  }

  // If complement label was not found; record an error.
  organism->Fault(FAULT_LOC_JUMP, FAULT_TYPE_ERROR,
		  "call: no complement label");
  return false;
}

bool cHardware4Stack::Inst_Return()
{
  IP().Set(StackPop());
  return true;
}

bool cHardware4Stack::Inst_Pop()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  Register(reg_used) = StackPop();
  return true;
}

bool cHardware4Stack::Inst_Push()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  StackPush(Register(reg_used));
  return true;
}

bool cHardware4Stack::Inst_HeadPop()
{
  const int head_used = FindModifiedHead(HEAD_IP);
  GetHead(head_used).Set(StackPop(), this);
  return true;
}

bool cHardware4Stack::Inst_HeadPush()
{
  const int head_used = FindModifiedHead(HEAD_IP);
  StackPush(GetHead(head_used).GetPosition());
  if (head_used == HEAD_IP) {
    GetHead(head_used).Set(GetHead(HEAD_FLOW));
    advance_ip = false;
  }
  return true;
}


bool cHardware4Stack::Inst_PopA() { Register(REG_AX) = StackPop(); return true;}
bool cHardware4Stack::Inst_PopB() { Register(REG_BX) = StackPop(); return true;}
bool cHardware4Stack::Inst_PopC() { Register(REG_CX) = StackPop(); return true;}

bool cHardware4Stack::Inst_PushA() { StackPush(Register(REG_AX)); return true;}
bool cHardware4Stack::Inst_PushB() { StackPush(Register(REG_AX)); return true;}
bool cHardware4Stack::Inst_PushC() { StackPush(Register(REG_AX)); return true;}

bool cHardware4Stack::Inst_SwitchStack() { SwitchStack(); return true;}
bool cHardware4Stack::Inst_FlipStack()   { StackFlip(); return true;}

bool cHardware4Stack::Inst_Swap()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  const int other_reg = FindComplementRegister(reg_used);
  Swap(Register(reg_used), Register(other_reg));
  return true;
}

bool cHardware4Stack::Inst_SwapAB() { Swap(Register(REG_AX), Register(REG_BX)); return true; }
bool cHardware4Stack::Inst_SwapBC() { Swap(Register(REG_BX), Register(REG_CX)); return true; }
bool cHardware4Stack::Inst_SwapAC() { Swap(Register(REG_AX), Register(REG_CX)); return true; }

bool cHardware4Stack::Inst_CopyReg()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  const int other_reg = FindComplementRegister(reg_used);
  Register(other_reg) = Register(reg_used);
  return true;
}

bool cHardware4Stack::Inst_CopyRegAB() { Register(REG_AX) = Register(REG_BX);   return true;
}
bool cHardware4Stack::Inst_CopyRegAC() { Register(REG_AX) = Register(REG_CX);   return true;
}
bool cHardware4Stack::Inst_CopyRegBA() { Register(REG_BX) = Register(REG_AX);   return true;
}
bool cHardware4Stack::Inst_CopyRegBC() { Register(REG_BX) = Register(REG_CX);   return true;
}
bool cHardware4Stack::Inst_CopyRegCA() { Register(REG_CX) = Register(REG_AX);   return true;
}
bool cHardware4Stack::Inst_CopyRegCB() { Register(REG_CX) = Register(REG_BX);   return true;
}

bool cHardware4Stack::Inst_Reset()
{
  Register(REG_AX) = 0;
  Register(REG_BX) = 0;
  Register(REG_CX) = 0;
  StackClear();
  return true;
}

bool cHardware4Stack::Inst_ShiftR()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  Register(reg_used) >>= 1;
  return true;
}

bool cHardware4Stack::Inst_ShiftL()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  Register(reg_used) <<= 1;
  return true;
}

bool cHardware4Stack::Inst_Bit1()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  Register(reg_used) |=  1;
  return true;
}

bool cHardware4Stack::Inst_SetNum()
{
  ReadLabel();
  Register(REG_BX) = GetLabel().AsInt();
  return true;
}

bool cHardware4Stack::Inst_Inc()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  Register(reg_used) += 1;
  return true;
}

bool cHardware4Stack::Inst_Dec()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  Register(reg_used) -= 1;
  return true;
}

bool cHardware4Stack::Inst_Zero()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  Register(reg_used) = 0;
  return true;
}

bool cHardware4Stack::Inst_Neg()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  Register(reg_used) = 0-Register(reg_used);
  return true;
}

bool cHardware4Stack::Inst_Square()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  Register(reg_used) = Register(reg_used) * Register(reg_used);
  return true;
}

bool cHardware4Stack::Inst_Sqrt()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  const int value = Register(reg_used);
  if (value > 1) Register(reg_used) = (int) sqrt((double) value);
  else if (value < 0) {
    Fault(FAULT_LOC_MATH, FAULT_TYPE_ERROR, "sqrt: value is negative");
    return false;
  }
  return true;
}

bool cHardware4Stack::Inst_Log()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  const int value = Register(reg_used);
  if (value >= 1) Register(reg_used) = (int) log((double) value);
  else if (value < 0) {
    Fault(FAULT_LOC_MATH, FAULT_TYPE_ERROR, "log: value is negative");
    return false;
  }
  return true;
}

bool cHardware4Stack::Inst_Log10()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  const int value = Register(reg_used);
  if (value >= 1) Register(reg_used) = (int) log10((double) value);
  else if (value < 0) {
    Fault(FAULT_LOC_MATH, FAULT_TYPE_ERROR, "log10: value is negative");
    return false;
  }
  return true;
}

bool cHardware4Stack::Inst_Minus18()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  Register(reg_used) -= 18;
  return true;
}

bool cHardware4Stack::Inst_Add()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  Register(reg_used) = Register(REG_BX) + Register(REG_CX);
  return true;
}

bool cHardware4Stack::Inst_Sub()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  Register(reg_used) = Register(REG_BX) - Register(REG_CX);
  return true;
}

bool cHardware4Stack::Inst_Mult()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  Register(reg_used) = Register(REG_BX) * Register(REG_CX);
  return true;
}

bool cHardware4Stack::Inst_Div()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  if (Register(REG_CX) != 0) {
    if (0-INT_MAX > Register(REG_BX) && Register(REG_CX) == -1)
      Fault(FAULT_LOC_MATH, FAULT_TYPE_ERROR, "div: Float exception");
    else
      Register(reg_used) = Register(REG_BX) / Register(REG_CX);
  } else {
    Fault(FAULT_LOC_MATH, FAULT_TYPE_ERROR, "div: dividing by 0");
    return false;
  }
  return true;
}

bool cHardware4Stack::Inst_Mod()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  if (Register(REG_CX) != 0) {
    Register(reg_used) = Register(REG_BX) % Register(REG_CX);
  } else {
    Fault(FAULT_LOC_MATH, FAULT_TYPE_ERROR, "mod: modding by 0");
  return false;
  }
  return true;
}


bool cHardware4Stack::Inst_Nand()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  Register(reg_used) = ~(Register(REG_BX) & Register(REG_CX));
  return true;
}

bool cHardware4Stack::Inst_Nor()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  Register(reg_used) = ~(Register(REG_BX) | Register(REG_CX));
  return true;
}

bool cHardware4Stack::Inst_And()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  Register(reg_used) = (Register(REG_BX) & Register(REG_CX));
  return true;
}

bool cHardware4Stack::Inst_Not()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  Register(reg_used) = ~(Register(reg_used));
  return true;
}

bool cHardware4Stack::Inst_Order()
{
  if (Register(REG_BX) > Register(REG_CX)) {
    Swap(Register(REG_BX), Register(REG_CX));
  }
  return true;
}

bool cHardware4Stack::Inst_Xor()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  Register(reg_used) = Register(REG_BX) ^ Register(REG_CX);
  return true;
}

bool cHardware4Stack::Inst_Copy()
{
  const cCPUHead from(this, Register(REG_BX));
  cCPUHead to(this, Register(REG_AX) + Register(REG_BX));
  sCPUStats & cpu_stats = organism->CPUStats();

  if (organism->TestCopyMut()) {
    to.SetInst(GetRandomInst());
    to.FlagMutated() = true;  // Mark this instruction as mutated...
    to.FlagCopyMut() = true;  // Mark this instruction as copy mut...
    //organism->GetPhenotype().IsMutated() = true;
    cpu_stats.mut_stats.copy_mut_count++;
  } else {
    to.SetInst(from.GetInst());
    to.FlagMutated() = false;  // UnMark
    to.FlagCopyMut() = false;  // UnMark
  }

  to.FlagCopied() = true;  // Set the copied flag.
  cpu_stats.mut_stats.copies_exec++;
  return true;
}

bool cHardware4Stack::Inst_ReadInst()
{
  const int reg_used = FindModifiedRegister(REG_CX);
  const cCPUHead from(this, Register(REG_BX));

  // Dis-allowing mutations on read, for the moment (write only...)
  // @CAO This allows perfect error-correction...
  Register(reg_used) = from.GetInst().GetOp();
  return true;
}

bool cHardware4Stack::Inst_WriteInst()
{
  cCPUHead to(this, Register(REG_AX) + Register(REG_BX));
  const int reg_used = FindModifiedRegister(REG_CX);
  const int value = Mod(Register(reg_used), GetNumInst());
  sCPUStats & cpu_stats = organism->CPUStats();

  // Change value on a mutation...
  if (organism->TestCopyMut()) {
    to.SetInst(GetRandomInst());
    to.FlagMutated() = true;      // Mark this instruction as mutated...
    to.FlagCopyMut() = true;      // Mark this instruction as copy mut...
    //organism->GetPhenotype().IsMutated() = true;
    cpu_stats.mut_stats.copy_mut_count++;
  } else {
    to.SetInst(cInstruction(value));
    to.FlagMutated() = false;     // UnMark
    to.FlagCopyMut() = false;     // UnMark
  }

  to.FlagCopied() = true;  // Set the copied flag.
  cpu_stats.mut_stats.copies_exec++;
  return true;
}

bool cHardware4Stack::Inst_StackReadInst()
{
  const int reg_used = FindModifiedRegister(REG_CX);
  cCPUHead from(this, Register(reg_used));
  StackPush(from.GetInst().GetOp());
  return true;
}

bool cHardware4Stack::Inst_StackWriteInst()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  cCPUHead to(this, Register(REG_AX) + Register(reg_used));
  const int value = Mod(StackPop(), GetNumInst());
  sCPUStats & cpu_stats = organism->CPUStats();

  // Change value on a mutation...
  if (organism->TestCopyMut()) {
    to.SetInst(GetRandomInst());
    to.FlagMutated() = true;      // Mark this instruction as mutated...
    to.FlagCopyMut() = true;      // Mark this instruction as copy mut...
    //organism->GetPhenotype().IsMutated() = true;
    cpu_stats.mut_stats.copy_mut_count++;
  } else {
    to.SetInst(cInstruction(value));
    to.FlagMutated() = false;     // UnMark
    to.FlagCopyMut() = false;     // UnMark
  }

  to.FlagCopied() = true;  // Set the copied flag.
  cpu_stats.mut_stats.copies_exec++;
  return true;
}

bool cHardware4Stack::Inst_Compare()
{
  const int reg_used = FindModifiedRegister(REG_CX);
  cCPUHead from(this, Register(REG_BX));
  cCPUHead to(this, Register(REG_AX) + Register(REG_BX));

  // Compare is dangerous -- it can cause mutations!
  if (organism->TestCopyMut()) {
    to.SetInst(GetRandomInst());
    to.FlagMutated() = true;      // Mark this instruction as mutated...
    to.FlagCopyMut() = true;      // Mark this instruction as copy mut...
    //organism->GetPhenotype().IsMutated() = true;
  }

  Register(reg_used) = from.GetInst().GetOp() - to.GetInst().GetOp();

  return true;
}

bool cHardware4Stack::Inst_IfNCpy()
{
  const cCPUHead from(this, Register(REG_BX));
  const cCPUHead to(this, Register(REG_AX) + Register(REG_BX));

  // Allow for errors in this test...
  if (organism->TestCopyMut()) {
    if (from.GetInst() != to.GetInst()) IP().Advance();
  } else {
    if (from.GetInst() == to.GetInst()) IP().Advance();
  }
  return true;
}

bool cHardware4Stack::Inst_Allocate()   // Allocate bx more space...
{
  const int size = GetMemory().GetSize();
  if( Allocate_Main(Register(REG_BX)) ) {
    Register(REG_AX) = size;
    return true;
  } else return false;
}

bool cHardware4Stack::Inst_Divide()  
{ 
  return Divide_Main(Register(REG_AX));    
}

bool cHardware4Stack::Inst_CDivide() 
{ 
  return Divide_Main(GetMemory().GetSize() / 2);   
}

bool cHardware4Stack::Inst_CAlloc()  
{ 
  return Allocate_Main(GetMemory().GetSize());   
}

bool cHardware4Stack::Inst_MaxAlloc()   // Allocate maximal more
{
  const int cur_size = GetMemory().GetSize();
  const int alloc_size = Min((int) (cConfig::GetChildSizeRange() * cur_size),
			     MAX_CREATURE_SIZE - cur_size);
  if( Allocate_Main(alloc_size) ) {
    Register(REG_AX) = cur_size;
    return true;
  } else return false;
}


bool cHardware4Stack::Inst_Repro()
{
  // Setup child
  cCPUMemory & child_genome = organism->ChildGenome();
  child_genome = GetMemory();
  organism->GetPhenotype().SetLinesCopied(GetMemory().GetSize());

  int lines_executed = 0;
  for ( int i = 0; i < GetMemory().GetSize(); i++ ) {
    if ( GetMemory().FlagExecuted(i) == true ) lines_executed++;
  }
  organism->GetPhenotype().SetLinesExecuted(lines_executed);

  // Perform Copy Mutations...
  if (organism->GetCopyMutProb() > 0) { // Skip this if no mutations....
    for (int i = 0; i < GetMemory().GetSize(); i++) {
      if (organism->TestCopyMut()) {
	child_genome[i]=GetRandomInst();
	//organism->GetPhenotype().IsMutated() = true;
      }
    }
  }
  Divide_DoMutations();

  // Many tests will require us to run the offspring through a test CPU;
  // this is, for example, to see if mutations need to be reverted or if
  // lineages need to be updated.
  Divide_TestFitnessMeasures();

#ifdef INSTRUCTION_COSTS
  // reset first time instruction costs
  for (int i = 0; i < inst_ft_cost.GetSize(); i++) {
    inst_ft_cost[i] = GetInstSet().GetFTCost(cInstruction(i));
  }
#endif

  if (cConfig::GetDivideMethod() == DIVIDE_METHOD_SPLIT) advance_ip = false;

  organism->ActivateDivide();

  return true;
}

// The inject instruction can be used instead of a divide command, paired
// with an allocate.  Note that for an inject to work, one needs to have a
// broad range for sizes allowed to be allocated.
//
// This command will cut out from read-head to write-head.
// It will then look at the template that follows the command and inject it
// into the complement template found in a neighboring organism.

bool cHardware4Stack::Inst_Inject()
{
  AdjustHeads();
  const int start_pos = GetHead(HEAD_READ).GetPosition();
  const int end_pos = GetHead(HEAD_WRITE).GetPosition();
  const int inject_size = end_pos - start_pos;

  // Make sure the creature will still be above the minimum size,
  if (inject_size <= 0) {
    Fault(FAULT_LOC_INJECT, FAULT_TYPE_ERROR, "inject: no code to inject");
    return false; // (inject fails)
  }
  if (start_pos < MIN_CREATURE_SIZE) {
    Fault(FAULT_LOC_INJECT, FAULT_TYPE_ERROR, "inject: new size too small");
    return false; // (inject fails)
  }

  // Since its legal to cut out the injected piece, do so.
  cGenome inject_code( cGenomeUtil::Crop(GetMemory(), start_pos, end_pos) );
  GetMemory().Remove(start_pos, inject_size);

  // If we don't have a host, stop here.
  cOrganism * host_organism = organism->GetNeighbor();
  if (host_organism == NULL) return false;

  // Scan for the label to match...
  ReadLabel();

  // If there is no label, abort.
  if (GetLabel().GetSize() == 0) {
    Fault(FAULT_LOC_INJECT, FAULT_TYPE_ERROR, "inject: label required");
    return false; // (inject fails)
  }

  // Search for the label in the host...
  GetLabel().Rotate(1);

  const int inject_signal =
    host_organism->GetHardware().Inject(GetLabel(), inject_code);
  if (inject_signal == 1) {
    Fault(FAULT_LOC_INJECT, FAULT_TYPE_WARNING, "inject: host too large.");
    return false; // Inject failed.
  }
  if (inject_signal == 2) {
    Fault(FAULT_LOC_INJECT, FAULT_TYPE_WARNING, "inject: target not in host.");
    return false; // Inject failed.
  }

  // Set the relevent flags.
  organism->GetPhenotype().IsModifier() = true;

  return true;
}


bool cHardware4Stack::Inst_InjectRand()
{
  // Rotate to a random facing and then run the normal inject instruction
  const int num_neighbors = organism->GetNeighborhoodSize();
  organism->Rotate(g_random.GetUInt(num_neighbors));
  Inst_Inject();
  return true;
}

// The inject instruction can be used instead of a divide command, paired
// with an allocate.  Note that for an inject to work, one needs to have a
// broad range for sizes allowed to be allocated.
//
// This command will cut out from read-head to write-head.
// It will then look at the template that follows the command and inject it
// into the complement template found in a neighboring organism.

bool cHardware4Stack::Inst_InjectThread()
{
  AdjustHeads();
  const int start_pos = GetHead(HEAD_READ).GetPosition();
  const int end_pos = GetHead(HEAD_WRITE).GetPosition();
  const int inject_size = end_pos - start_pos;

  // Make sure the creature will still be above the minimum size,
  if (inject_size <= 0) {
    Fault(FAULT_LOC_INJECT, FAULT_TYPE_ERROR, "inject: no code to inject");
    return false; // (inject fails)
  }
  if (start_pos < MIN_CREATURE_SIZE) {
    Fault(FAULT_LOC_INJECT, FAULT_TYPE_ERROR, "inject: new size too small");
    return false; // (inject fails)
  }

  // Since its legal to cut out the injected piece, do so.
  cGenome inject_code( cGenomeUtil::Crop(GetMemory(), start_pos, end_pos) );
  GetMemory().Remove(start_pos, inject_size);

  // If we don't have a host, stop here.
  cOrganism * host_organism = organism->GetNeighbor();
  if (host_organism == NULL) return false;

  // Scan for the label to match...
  ReadLabel();

  // If there is no label, abort.
  if (GetLabel().GetSize() == 0) {
    Fault(FAULT_LOC_INJECT, FAULT_TYPE_ERROR, "inject: label required");
    return false; // (inject fails)
  }

  // Search for the label in the host...
  GetLabel().Rotate(1);

  const int inject_signal =
    host_organism->GetHardware().InjectThread(GetLabel(), inject_code);
  if (inject_signal == 1) {
    Fault(FAULT_LOC_INJECT, FAULT_TYPE_WARNING, "inject: host too large.");
    return false; // Inject failed.
  }
  if (inject_signal == 2) {
    Fault(FAULT_LOC_INJECT, FAULT_TYPE_WARNING, "inject: target not in host.");
    return false; // Inject failed.
  }

  // Set the relevent flags.
  organism->GetPhenotype().IsModifier() = true;

  return true;
}

bool cHardware4Stack::Inst_TaskGet()
{
  const int reg_used = FindModifiedRegister(REG_CX);
  const int value = organism->GetInput();
  Register(reg_used) = value;
  DoInput(value);
  return true;
}

bool cHardware4Stack::Inst_TaskStackGet()
{
  const int value = organism->GetInput();
  StackPush(value);
  DoInput(value);
  return true;
}

bool cHardware4Stack::Inst_TaskStackLoad()
{
  for (int i = 0; i < IO_SIZE; i++) StackPush( organism->GetInput() );
  return true;
}

bool cHardware4Stack::Inst_TaskPut()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  const int value = Register(reg_used);
  Register(reg_used) = 0;
  DoOutput(value);
  return true;
}

bool cHardware4Stack::Inst_TaskIO()
{
  const int reg_used = FindModifiedRegister(REG_BX);

  // Do the "put" component
  const int value_out = Register(reg_used);
  DoOutput(value_out);  // Check for tasks compleated.

  // Do the "get" component
  const int value_in = organism->GetInput();
  Register(reg_used) = value_in;
  DoInput(value_in);
  return true;
}

bool cHardware4Stack::Inst_SearchF()
{
  ReadLabel();
  GetLabel().Rotate(1);
  const int search_size = FindLabel(1).GetPosition() - IP().GetPosition();
  Register(REG_BX) = search_size;
  Register(REG_CX) = GetLabel().GetSize();
  return true;
}

bool cHardware4Stack::Inst_SearchB()
{
  ReadLabel();
  GetLabel().Rotate(1);
  const int search_size = IP().GetPosition() - FindLabel(-1).GetPosition();
  Register(REG_BX) = search_size;
  Register(REG_CX) = GetLabel().GetSize();
  return true;
}

bool cHardware4Stack::Inst_MemSize()
{
  Register(FindModifiedRegister(REG_BX)) = GetMemory().GetSize();
  return true;
}


bool cHardware4Stack::Inst_RotateL()
{
  const int num_neighbors = organism->GetNeighborhoodSize();

   // If this organism has no neighbors, ignore rotate.
  if (num_neighbors == 0) return false;

  ReadLabel();

  // Always rotate at least once.
  organism->Rotate(-1);

  // If there is no label, then the one rotation was all we want.
  if (!GetLabel().GetSize()) return true;

  // Rotate until a complement label is found (or all have been checked).
  GetLabel().Rotate(1);
  for (int i = 1; i < num_neighbors; i++) {
    cOrganism * neighbor = organism->GetNeighbor();

    // Assuming we have a neighbor and it is of the same hardware type,
    // search for the label in it.
    if (neighbor != NULL &&
	neighbor->GetHardware().GetType() == GetType()) {

      // If this facing has the full label, stop here.
      cHardware4Stack & cur_hardware = (cHardware4Stack &) neighbor->GetHardware();
      if (cur_hardware.FindFullLabel( GetLabel() ).InMemory()) return true;
    }

    // Otherwise keep rotating...
    organism->Rotate(-1);
  }
  return true;
}

bool cHardware4Stack::Inst_RotateR()
{
  const int num_neighbors = organism->GetNeighborhoodSize();

   // If this organism has no neighbors, ignore rotate.
  if (num_neighbors == 0) return false;

  ReadLabel();

  // Always rotate at least once.
  organism->Rotate(-1);

  // If there is no label, then the one rotation was all we want.
  if (!GetLabel().GetSize()) return true;

  // Rotate until a complement label is found (or all have been checked).
  GetLabel().Rotate(1);
  for (int i = 1; i < num_neighbors; i++) {
    cOrganism * neighbor = organism->GetNeighbor();

    // Assuming we have a neighbor and it is of the same hardware type,
    // search for the label in it.
    if (neighbor != NULL &&
	neighbor->GetHardware().GetType() == GetType()) {

      // If this facing has the full label, stop here.
      cHardware4Stack & cur_hardware = (cHardware4Stack &) neighbor->GetHardware();
      if (cur_hardware.FindFullLabel( GetLabel() ).InMemory()) return true;
    }

    // Otherwise keep rotating...
    organism->Rotate(1);
  }
  return true;
}

bool cHardware4Stack::Inst_SetCopyMut()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  const int new_mut_rate = Max( Register(reg_used), 1 );
  organism->SetCopyMutProb(((double) new_mut_rate) / 10000.0);
  return true;
}

bool cHardware4Stack::Inst_ModCopyMut()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  const double new_mut_rate = organism->GetCopyMutProb() +
    ((double) Register(reg_used)) / 10000.0;
  if (new_mut_rate > 0.0) organism->SetCopyMutProb(new_mut_rate);
  return true;
}


// Multi-threading.

bool cHardware4Stack::Inst_ForkThread()
{
  IP().Advance();
  if (!ForkThread()) Fault(FAULT_LOC_THREAD_FORK, FAULT_TYPE_FORK_TH);
  return true;
}

bool cHardware4Stack::Inst_KillThread()
{
  if (!KillThread()) Fault(FAULT_LOC_THREAD_KILL, FAULT_TYPE_KILL_TH);
  else advance_ip = false;
  return true;
}

bool cHardware4Stack::Inst_ThreadID()
{
  const int reg_used = FindModifiedRegister(REG_BX);
  Register(reg_used) = GetCurThreadID();
  return true;
}


// Head-based instructions

bool cHardware4Stack::Inst_SetHead()
{
  const int head_used = FindModifiedHead(HEAD_IP);
  SetActiveHead(head_used);
  return true;
}

bool cHardware4Stack::Inst_AdvanceHead()
{
  const int head_used = FindModifiedHead(HEAD_WRITE);
  GetHead(head_used).Advance();
  return true;
}
 
bool cHardware4Stack::Inst_MoveHead()
{
  const int head_used = FindModifiedHead(HEAD_IP);
  GetHead(head_used).Set(GetHead(HEAD_FLOW));
  if (head_used == HEAD_IP) advance_ip = false;
  return true;
}

bool cHardware4Stack::Inst_JumpHead()
{
  const int head_used = FindModifiedHead(HEAD_IP);
  GetHead(head_used).Jump( Register(REG_CX) );
  return true;
}

bool cHardware4Stack::Inst_GetHead()
{
  const int head_used = FindModifiedHead(HEAD_IP);
  Register(REG_CX) = GetHead(head_used).GetPosition();
  return true;
}

bool cHardware4Stack::Inst_IfLabel()
{
  ReadLabel();
  GetLabel().Rotate(1);
  if (GetLabel() != GetReadLabel())  IP().Advance();
  return true;
}

bool cHardware4Stack::Inst_HeadDivideMut(double mut_multiplier)
{
  AdjustHeads();
  const int divide_pos = GetHead(HEAD_READ).GetPosition();
  int child_end =  GetHead(HEAD_WRITE).GetPosition();
  if (child_end == 0) child_end = GetMemory().GetSize();
  const int extra_lines = GetMemory().GetSize() - child_end;
  bool ret_val = Divide_Main(divide_pos, extra_lines, mut_multiplier);
  // Re-adjust heads.
  AdjustHeads();
  return ret_val; 
}

bool cHardware4Stack::Inst_HeadDivide()
{
  return Inst_HeadDivideMut(1);
}

bool cHardware4Stack::Inst_HeadDivideSex()  
{ 
  organism->GetPhenotype().SetDivideSex(true);
  return Inst_HeadDivide(); 
}

bool cHardware4Stack::Inst_HeadDivideAsex()  
{ 
  organism->GetPhenotype().SetDivideSex(false);
  return Inst_HeadDivide(); 
}

bool cHardware4Stack::Inst_HeadDivide1()  { return Inst_HeadDivideMut(1); }
bool cHardware4Stack::Inst_HeadDivide2()  { return Inst_HeadDivideMut(2); }
bool cHardware4Stack::Inst_HeadDivide3()  { return Inst_HeadDivideMut(3); }
bool cHardware4Stack::Inst_HeadDivide4()  { return Inst_HeadDivideMut(4); }
bool cHardware4Stack::Inst_HeadDivide5()  { return Inst_HeadDivideMut(5); }
bool cHardware4Stack::Inst_HeadDivide6()  { return Inst_HeadDivideMut(6); }
bool cHardware4Stack::Inst_HeadDivide7()  { return Inst_HeadDivideMut(7); }
bool cHardware4Stack::Inst_HeadDivide8()  { return Inst_HeadDivideMut(8); }
bool cHardware4Stack::Inst_HeadDivide9()  { return Inst_HeadDivideMut(9); }
bool cHardware4Stack::Inst_HeadDivide10()  { return Inst_HeadDivideMut(10); }
bool cHardware4Stack::Inst_HeadDivide16()  { return Inst_HeadDivideMut(16); }
bool cHardware4Stack::Inst_HeadDivide32()  { return Inst_HeadDivideMut(32); }
bool cHardware4Stack::Inst_HeadDivide50()  { return Inst_HeadDivideMut(50); }
bool cHardware4Stack::Inst_HeadDivide100()  { return Inst_HeadDivideMut(100); }
bool cHardware4Stack::Inst_HeadDivide500()  { return Inst_HeadDivideMut(500); }
bool cHardware4Stack::Inst_HeadDivide1000()  { return Inst_HeadDivideMut(1000); }
bool cHardware4Stack::Inst_HeadDivide5000()  { return Inst_HeadDivideMut(5000); }
bool cHardware4Stack::Inst_HeadDivide10000()  { return Inst_HeadDivideMut(10000); }
bool cHardware4Stack::Inst_HeadDivide50000()  { return Inst_HeadDivideMut(50000); }
bool cHardware4Stack::Inst_HeadDivide0_5()  { return Inst_HeadDivideMut(0.5); }
bool cHardware4Stack::Inst_HeadDivide0_1()  { return Inst_HeadDivideMut(0.1); }
bool cHardware4Stack::Inst_HeadDivide0_05()  { return Inst_HeadDivideMut(0.05); }
bool cHardware4Stack::Inst_HeadDivide0_01()  { return Inst_HeadDivideMut(0.01); }
bool cHardware4Stack::Inst_HeadDivide0_001()  { return Inst_HeadDivideMut(0.001); }

bool cHardware4Stack::Inst_HeadRead()
{
  const int head_id = FindModifiedHead(HEAD_READ);
  GetHead(head_id).Adjust();
  sCPUStats & cpu_stats = organism->CPUStats();

  // Mutations only occur on the read, for the moment.
  int read_inst = 0;
  if (organism->TestCopyMut()) {
    read_inst = GetRandomInst().GetOp();
    cpu_stats.mut_stats.copy_mut_count++;  // @CAO, hope this is good!
  } else {
    read_inst = GetHead(head_id).GetInst().GetOp();
  }
  Register(REG_BX) = read_inst;
  ReadInst(read_inst);

  cpu_stats.mut_stats.copies_exec++;  // @CAO, this too..
  GetHead(head_id).Advance();
  return true;
}

bool cHardware4Stack::Inst_HeadWrite()
{
  const int head_id = FindModifiedHead(HEAD_WRITE);
  cCPUHead & active_head = GetHead(head_id);

  active_head.Adjust();

  int value = Register(REG_BX);
  if (value < 0 || value >= GetNumInst()) value = 0;

  active_head.SetInst(cInstruction(value));
  active_head.FlagCopied() = true;

  // Advance the head after write...
  active_head++;
  return true;
}

bool cHardware4Stack::Inst_HeadCopy()
{
  // For the moment, this cannot be nop-modified.
  cCPUHead & read_head = GetHead(HEAD_READ);
  cCPUHead & write_head = GetHead(HEAD_WRITE);
  sCPUStats & cpu_stats = organism->CPUStats();

  read_head.Adjust();
  write_head.Adjust();

  // TriggerMutations(MUTATION_TRIGGER_READ, read_head);
  
  // Do mutations.
  cInstruction read_inst = read_head.GetInst();
  if (organism->TestCopyMut()) {
    read_inst = GetRandomInst();
    cpu_stats.mut_stats.copy_mut_count++; 
    write_head.FlagMutated() = true;
    write_head.FlagCopyMut() = true;
    //organism->GetPhenotype().IsMutated() = true;
  }
  ReadInst(read_inst.GetOp());

  cpu_stats.mut_stats.copies_exec++;

  write_head.SetInst(read_inst);
  write_head.FlagCopied() = true;  // Set the copied flag...

  // TriggerMutations(MUTATION_TRIGGER_WRITE, write_head);

  read_head.Advance();
  write_head.Advance();
  return true;
}

bool cHardware4Stack::HeadCopy_ErrorCorrect(double reduction)
{
  // For the moment, this cannot be nop-modified.
  cCPUHead & read_head = GetHead(HEAD_READ);
  cCPUHead & write_head = GetHead(HEAD_WRITE);
  sCPUStats & cpu_stats = organism->CPUStats();

  read_head.Adjust();
  write_head.Adjust();

  // Do mutations.
  cInstruction read_inst = read_head.GetInst();
  if ( g_random.P(organism->GetCopyMutProb() / reduction) ) {
    read_inst = GetRandomInst();
    cpu_stats.mut_stats.copy_mut_count++; 
    write_head.FlagMutated() = true;
    write_head.FlagCopyMut() = true;
    //organism->GetPhenotype().IsMutated() = true;
  }
  ReadInst(read_inst.GetOp());

  cpu_stats.mut_stats.copies_exec++;

  write_head.SetInst(read_inst);
  write_head.FlagCopied() = true;  // Set the copied flag...

  read_head.Advance();
  write_head.Advance();
  return true;
}

bool cHardware4Stack::Inst_HeadCopy2()  { return HeadCopy_ErrorCorrect(2); }
bool cHardware4Stack::Inst_HeadCopy3()  { return HeadCopy_ErrorCorrect(3); }
bool cHardware4Stack::Inst_HeadCopy4()  { return HeadCopy_ErrorCorrect(4); }
bool cHardware4Stack::Inst_HeadCopy5()  { return HeadCopy_ErrorCorrect(5); }
bool cHardware4Stack::Inst_HeadCopy6()  { return HeadCopy_ErrorCorrect(6); }
bool cHardware4Stack::Inst_HeadCopy7()  { return HeadCopy_ErrorCorrect(7); }
bool cHardware4Stack::Inst_HeadCopy8()  { return HeadCopy_ErrorCorrect(8); }
bool cHardware4Stack::Inst_HeadCopy9()  { return HeadCopy_ErrorCorrect(9); }
bool cHardware4Stack::Inst_HeadCopy10() { return HeadCopy_ErrorCorrect(10); }

bool cHardware4Stack::Inst_HeadSearch()
{
  ReadLabel();
  GetLabel().Rotate(1);
  cCPUHead found_pos = FindLabel(0);
  const int search_size = found_pos.GetPosition() - IP().GetPosition();
  Register(REG_BX) = search_size;
  Register(REG_CX) = GetLabel().GetSize();
  GetHead(HEAD_FLOW).Set(found_pos);
  GetHead(HEAD_FLOW).Advance();
  return true; 
}

bool cHardware4Stack::Inst_SetFlow()
{
  const int reg_used = FindModifiedRegister(REG_CX);
  GetHead(HEAD_FLOW).Set(Register(reg_used), this);
  return true; 
}

// Direct Matching Templates

bool cHardware4Stack::Inst_DMJumpF()
{
  ReadLabel();

  // If there is no label, jump BX steps.
  if (GetLabel().GetSize() == 0) {
    IP().Jump(Register(REG_BX));
    return true;
  }

  // Otherwise, jump to the label.
  cCPUHead jump_location(FindLabel(1));
  if (jump_location.GetPosition() != -1) {
    IP().Set(jump_location);
    return true;
  }

  // If complement label was not found; record an error.
  organism->Fault(FAULT_LOC_JUMP, FAULT_TYPE_ERROR,
		  "dm-jump-f: no complement label");
  return false;
}


//// Placebo insts ////
bool cHardware4Stack::Inst_Skip()
{
  IP().Advance();
  return true;
}


