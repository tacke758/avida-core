//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          // 
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////


#ifndef CPU_DEFS_HH
#define CPU_DEFS_HH

#define TEST_CPU_GENERATIONS 3  // Default generations tested for viability.

#define MAX_TEST_DEPTH       10  // Maximum number of generations tested

// The following defines setup the structure of the CPU.  Changing them
// (especially the number of registers) could have effects in other parts
// of the code!

#define MAX_NOPS 4
#define MAX_LABEL_SIZE 10 
#define REG_AX 0
#define REG_BX 1
#define REG_CX 2
#define NUM_REGISTERS 3

#define HEAD_IP     0
#define HEAD_READ   1
#define HEAD_WRITE  2
#define HEAD_FLOW   3
#define NUM_HEADS   4


#define STACK_SIZE 10
#define IO_SIZE 3

#define GEN_RESOURCE   0
#define NUM_RESOURCES  1

#define FAULT_TYPE_WARNING 0
#define FAULT_TYPE_ERROR   1

#define FAULT_TYPE_FORK_TH 0
#define FAULT_TYPE_KILL_TH 0

class cInstLib;		// defined in instlib.hh

// The following flags are attached to the CPU of each creature
// There are a maximum of 32 flags per CPU currently.

//  #define CPU_FLAG_MAL_ACTIVE  0  // Is the creature currently gestating?
//  #define CPU_FLAG_FULL_TRACE  1  // Allows specific creatures to be traced
//  #define CPU_FLAG_PARENT_TRUE 2  // Is genotype the same as parent?
//  #define CPU_FLAG_INJECTED    3  // Was this creature injected into the soup?
//  #define CPU_FLAG_PARASITE    4  // Has this CPU acted as a parasite?
//  #define CPU_FLAG_INST_INJECT 5  // Has CPU executed an inject instruction?
//  #define CPU_FLAG_MODIFIED    6  // Has CPU been modified by another CPU?
//  #define CPU_FLAG_POINT_MUT   7  // Has the memroy been point-muted?
//  #define CPU_FLAG_BIO_MAL_ACTIVE  8  // Is child code array allocated?
//  #define CPU_FLAG_ADVANCE_IP  16 // Do we advance IP after exec'ing this inst?
//  #define CPU_FLAG_SEARCH_APPROX_SIZE   31  // Did a search ~= size?

#endif
