//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1998 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef INST_HH
#define INST_HH

#include "../defs.hh"
#include "../tools/tools.hh"
#include "../tools/string.hh"

#define INSTRUCTION_DEFAULT   0
#define INSTRUCTION_ERROR   254
#define INSTRUCTION_NONE    255

// For instruction sets which use arguments...
#define NUM_INST_ARGS 2

#define ARG_MOD_NONE_A 0
#define ARG_MOD_NEG    1
#define ARG_MOD_INC    2
#define ARG_MOD_DEC    3
#define ARG_MOD_SHIFTR 4
#define ARG_MOD_BITNEG 5
#define ARG_MOD_SHIFTL 6
#define ARG_MOD_NONE_B 7

#define ARG_TYPE_VALUE_A  0
#define ARG_TYPE_REG      1
#define ARG_TYPE_IP       2
#define ARG_TYPE_STACK    3
#define ARG_TYPE_IO       4
#define ARG_TYPE_MEM      5
#define ARG_TYPE_LABEL    6
#define ARG_TYPE_VALUE_B  7

#define ARG_POS_TYPE    0
#define ARG_POS_MOD     3
#define ARG_POS_POINT   6
#define ARG_POS_PSITE   7


#define REDCODE_OP_VALUE        0
#define REDCODE_OP_DIRECT       1
#define REDCODE_OP_INDIRECT     2
#define REDCODE_OP_DEC_INDIRECT 3

class cBasicInstruction;
class cLongInstruction;
class cInstLib;

class cInstArgument {
private:
  UCHAR type;  // Bits meaning...
               //  0-2: Argument Type 
               //       (000) -or- (111) => Direct Value
               //       (001) => Register      (010) => Instruction Pointer
               //       (011) => Stack         (100) => I/O
               //       (101) => Memory        (110) => Label
               //  3-5: Argument Modification
               //       (000) -or- (111) => None
               //       (001) => Negate        (101) => Bitwise Negate
               //       (010) => Increment     (011) => Decrement
               //       (110) => Shift Left    (100) => Shift Right
               //  6: Is Pointer?
               //  7: Is Parasite? 
  int value;

  static const cInstArgument arg_default;
public:
  cInstArgument(int in_value=0, UCHAR in_type=0)
    { value = in_value; type = in_type; }
  cInstArgument(int value, UCHAR in_type, UCHAR in_mod, UCHAR in_pointer,
		UCHAR in_parasite);
  cInstArgument(const cInstArgument & in_arg) { type = in_arg.type; }

  inline int GetValue()      const { return value; }
  inline UCHAR GetType()     const { return (type >> ARG_POS_TYPE)  & 7; }
  inline UCHAR GetModType()  const { return (type >> ARG_POS_MOD)   & 7; }
  inline UCHAR IsPointer()   const { return (type >> ARG_POS_POINT) & 1; }
  inline UCHAR IsParasite()  const { return (type >> ARG_POS_PSITE) & 1; }

  inline void Set(int in_value, UCHAR in_type)
    { value = in_value; type = in_type; }
  inline void SetValue(int in_value) { value = in_value; }

  // Redcode only needs the first two bits for the type... (@CAO eventually
  // add labels)

  inline UCHAR GetRedcodeType() const { return type & 3; }

  // Idirect Tests...
  inline UCHAR IsValue() const { return GetType() == ARG_TYPE_VALUE_A ||
			                GetType() == ARG_TYPE_VALUE_B; }
  inline UCHAR WriteOK() const { return (!IsValue() && !IsParasite()); }

  cString AsString(const cInstLib * inst_lib) const;
  inline UCHAR AsByte() const { return type; }

  inline void operator=(const cInstArgument & in_arg)
    { type = in_arg.type;  value = in_arg.value; }
  inline int operator==(const cInstArgument & in_arg) const
    { return (type == in_arg.type && value == in_arg.value); }
  inline int operator!=(const cInstArgument & in_arg) const
    { return !operator==(in_arg); }

  // Static Methods
  inline static const cInstArgument & GetArgDefault() { return arg_default; }
};


// This function warns if the incorrect Instruction class is having an
// illeagle method being called.

inline void InstWarn() {
#ifdef DEBUG
  g_debug.Warning("InstWarn()!  Improper instruction method being called!");
#endif  
}


// cInstruction is used throughout the code.  Make sure we typedef it to
// the proper type of instruction which we want to be using!

#ifndef INST_ARGS
typedef cBasicInstruction cInstruction;
#else
typedef cLongInstruction cInstruction;
#endif

class cInstructionInterface {
private:
public:
  cInstructionInterface() { ; }
  virtual ~cInstructionInterface() { ; }

  // Accessors...
  virtual UCHAR GetOp() const = 0;
  virtual void SetOp(UCHAR in_operand) = 0;
  virtual const cInstArgument & GetArg(int id) const = 0;
  virtual void SetArg(int id, UCHAR arg, int value) = 0;
  virtual void SetArg(int id, const cInstArgument & in_arg) = 0;

  /*
  // Operators...
  virtual void operator=(const cBasicInstruction & inst) = 0;
    if (this != &inst) operand = inst.operand;
  }

  inline int operator==(const cBasicInstruction & inst) const
    { return (operand == inst.operand); }
  inline int operator!=(const cBasicInstruction & inst) const
    { return !(operator==(inst)); }
    */

  virtual int OK() const { return TRUE; }
};


// These objects represents an instruction in the instlib.  operands 0-127
// are normal operands.  The others specified so far are:
//    0 : DEFAULT
//  254 : ERROR
//  255 : NONE

// public cInstructionInterface, 
class cBasicInstruction { //: public tIntegrityCheck<cBasicInstruction> {
private:
  UCHAR operand;

  // 'Special' instruction types...
  static const cBasicInstruction inst_error;
  static const cBasicInstruction inst_none;
  static const cBasicInstruction inst_default;

public:
  explicit inline cBasicInstruction(UCHAR in_op=0) { operand = in_op; }
  explicit inline cBasicInstruction(int in_op) { operand = (UCHAR) in_op; }
  inline cBasicInstruction(const cBasicInstruction & _inst);
  inline ~cBasicInstruction() { ; }

  // Accessors...
  inline UCHAR GetOp() const { return operand; }
  inline void SetOp(UCHAR in_operand) { operand = in_operand; }
  inline const cInstArgument & GetArg(int id) const
    { InstWarn(); (void)id; return cInstArgument::GetArgDefault(); }
  inline void SetArg(int id, UCHAR arg, int value)
    { InstWarn(); (void) id; (void) arg; (void) value; }
  inline void SetArg(int id, const cInstArgument & in_arg)
    { InstWarn(); (void) id; (void) in_arg; }

  inline void SetArgValue(int id, int value)
    { InstWarn(); (void) id; (void) value; }

  // Special instrustions...
  static const cBasicInstruction & GetInstDefault() { return inst_default; }
  static const cBasicInstruction & GetInstError()   { return inst_error; }
  static const cBasicInstruction & GetInstNone()    { return inst_none; }

  int OK() const { return TRUE; }

  // Operators...
  inline void operator=(const cBasicInstruction & inst) {
    if (this != &inst) operand = inst.operand;
  }

  inline int operator==(const cBasicInstruction & inst) const
    { return (operand == inst.operand); }
  inline int operator!=(const cBasicInstruction & inst) const
    { return !(operator==(inst)); }
};


class cLongInstruction {
private:
  UCHAR operand;
  cInstArgument args[NUM_INST_ARGS];

  // 'Special' instruction types...
  static const cLongInstruction inst_error;
  static const cLongInstruction inst_none;
  static const cLongInstruction inst_default;
public:
  explicit cLongInstruction(UCHAR _operand=0);
  explicit cLongInstruction(int _operand);
  cLongInstruction(const cLongInstruction & _inst);
  inline ~cLongInstruction() { ; }

  // Accessors...
  inline UCHAR GetOp() const { return operand; }
  inline void SetOp(UCHAR in_op) { operand = in_op; }
  inline const cInstArgument & GetArg(int id) const { return args[id]; }
  inline void SetArg(int id, UCHAR info, int value)
    { args[id].Set(value, info); }
  inline void SetArg(int id, const cInstArgument & in_arg)
    { args[id] = in_arg; }
  inline void SetArgValue(int id, int value) { args[id].SetValue(value); }

  // Special instrustions...
  static const cLongInstruction & GetInstDefault() { return inst_default; }
  static const cLongInstruction & GetInstError()   { return inst_error; }
  static const cLongInstruction & GetInstNone()    { return inst_none; }

  int OK() const { return TRUE; }  

  // Operators...
  void operator=(const cLongInstruction & _inst);

  int operator==(const cLongInstruction & _inst) const;
  inline int operator!=(const cLongInstruction & _inst) const
    { return !(operator==(_inst)); }
};


/////////////
//  Inlines
/////////////

inline cBasicInstruction::cBasicInstruction(const cBasicInstruction & _inst)
{
  *this = _inst;
}

#endif
