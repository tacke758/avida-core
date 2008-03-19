//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1998 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "inst.hh"
#include "inst_lib.hh"

// Defs of constant static variables...

const cBasicInstruction cBasicInstruction::inst_default(INSTRUCTION_DEFAULT);
const cBasicInstruction cBasicInstruction::inst_error(INSTRUCTION_ERROR);
const cBasicInstruction cBasicInstruction::inst_none(INSTRUCTION_NONE);

const cLongInstruction cLongInstruction::inst_default(INSTRUCTION_DEFAULT);
const cLongInstruction cLongInstruction::inst_error(INSTRUCTION_ERROR);
const cLongInstruction cLongInstruction::inst_none(INSTRUCTION_NONE);

const cInstArgument cInstArgument::arg_default(0);


//////////////////////
//  cLongInstruction
//////////////////////

cLongInstruction::cLongInstruction(UCHAR _operand)
{
  operand = _operand;
}

cLongInstruction::cLongInstruction(int _operand)
{
  operand = _operand;
}
				     
cLongInstruction::cLongInstruction(const cLongInstruction & _inst)
{
  operand = _inst.operand;
  for (int i = 0; i < NUM_INST_ARGS; i++) {
    args[i] = _inst.args[i];
  }
}

void cLongInstruction::operator=(const cLongInstruction & _inst)
{
  operand = _inst.operand;
  for (int i = 0; i < NUM_INST_ARGS; i++) {
    args[i] = _inst.args[i];
  }
}

int cLongInstruction::operator==(const cLongInstruction & _inst) const
{
  if (operand != _inst.operand) return FALSE;
  for (int i = 0; i < NUM_INST_ARGS; i++)
    if (args[i] != _inst.args[i]) return FALSE;
  return TRUE;
}


//////////////////
// cInstArgument
//////////////////

cInstArgument::cInstArgument(int in_value, UCHAR in_type, UCHAR in_mod,
			     UCHAR in_pointer, UCHAR in_parasite) {
  value = in_value;
  type = 0;
  type |= (in_type      << ARG_POS_TYPE);
  type |= (in_mod       << ARG_POS_MOD);
  type |= (in_pointer   << ARG_POS_POINT);
  type |= (in_parasite  << ARG_POS_PSITE);
}

cString cInstArgument::AsString(const cInstLib * inst_lib) const
{
  cString out_string;

  switch (inst_lib->GetType()) {
  case INSTLIB_TYPE_ARGS:
    if (IsParasite()) out_string += '*';

    if (GetModType() == ARG_MOD_INC) out_string += "++";
    else if (GetModType() == ARG_MOD_DEC) out_string += "--";
    else if (GetModType() == ARG_MOD_NEG) out_string += '-';
    else if (GetModType() == ARG_MOD_BITNEG) out_string += '~';
    else if (GetModType() == ARG_MOD_SHIFTL) out_string += '<';
    else if (GetModType() == ARG_MOD_SHIFTR) out_string += '>';

    if (IsPointer()) out_string += '@';

    if (!IsValue()) {
      if (GetType() == ARG_TYPE_REG)   out_string += "REG:";
      else if (GetType() == ARG_TYPE_IP)    out_string += "IP:";
      else if (GetType() == ARG_TYPE_STACK) out_string += "STACK:";
      else if (GetType() == ARG_TYPE_IO)    out_string += "IO:";
      else if (GetType() == ARG_TYPE_MEM)   out_string += "MEM:";
      else if (GetType() == ARG_TYPE_LABEL) out_string += "LABEL:";
    }
    break;
  case INSTLIB_TYPE_REDCODE:
    if (GetRedcodeType() == REDCODE_OP_DIRECT) out_string += '@';
    else if (GetRedcodeType() == REDCODE_OP_INDIRECT) out_string += '$';
    else if (GetRedcodeType() == REDCODE_OP_DEC_INDIRECT) out_string += ">";
    break;
  default:
    out_string.Set("Error! type=%d", type);
    break;
  }

  out_string.Set("%s%d", out_string(), value);

  return out_string;
}
