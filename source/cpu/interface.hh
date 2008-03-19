//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef INTERFACE_HH
#define INTERFACE_HH

#include "../tools/memtrack.hh"

#define CPU_TYPE_TEST    0
#define CPU_TYPE_MAIN    1

class cCPUInterface { 
private:
  int cpu_type;
public:
  cCPUInterface(int in_cpu_type = CPU_TYPE_MAIN);
  ~cCPUInterface();  

  inline int GetCpuType() { return cpu_type; }
};

#endif
