#ifndef INST_SUPERLIB_HH
#define INST_SUPERLIB_HH


class cInstSuperlibBase {
public:
  virtual const cString &GetName(const unsigned int id) = 0;
  virtual const cString &GetNopName(const unsigned int id) = 0;
  virtual int GetNopMod(const unsigned int id) = 0;
  virtual int GetNopMod(const cInstruction & inst) = 0;
  virtual int GetSize() = 0;
  virtual int GetNumNops() = 0;
  virtual cInstruction GetInst(const cString & in_name) = 0;
  virtual const cInstruction & GetInstDefault() = 0;
  virtual const cInstruction & GetInstError() = 0;
};

#endif
