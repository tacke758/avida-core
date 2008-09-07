//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "../defs.hh"
#include "interface.hh"

cCPUInterface::cCPUInterface(int in_cpu_type)
{
  cpu_type = in_cpu_type;
}

cCPUInterface::~cCPUInterface()
{
}
