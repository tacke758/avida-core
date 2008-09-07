//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "cpu.hh"
#include "head.hh"
#include "head.ii"
#include "cpu.ii"

cCPUHead::cCPUHead() {
  main_cpu = NULL;
  cur_cpu = NULL;
  position = 0;
}

cCPUHead::cCPUHead(cBaseCPU * in_cpu, int in_pos) {
  main_cpu = in_cpu;
  cur_cpu  = in_cpu;

  position = in_pos;
  if (in_pos) Adjust();
}

cCPUHead::cCPUHead(const cCPUHead & in_cpu_head) {
  main_cpu = in_cpu_head.main_cpu;
  cur_cpu  = in_cpu_head.cur_cpu;
  position = in_cpu_head.position;
}
