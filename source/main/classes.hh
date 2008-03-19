//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef CLASSES_HH
#define CLASSES_HH

class cCodeArray;
class cGenotype;
class cSpecies;
class cCodeLabel;
class cBaseCPU;
class cStatistics;
class cGenebank;
class cDataProc;
class cPopulation;

class cView;
class cTextWindow;

#define C_NULL       0
#define C_GENOTYPE   1
#define C_LABEL      2

#define C_BASE_CPU   3
#define C_MAIN_CPU   4
#define C_TEST_CPU   5

#define C_SCHEDULE           6
#define C_CONST_SCHEDULE     7
#define C_BLOCK_SCHEDULE     8
#define C_PROB_SCHEDULE      9
#define C_LOG_SCHEDULE_NODE 10
#define C_LOG_SCHEDULE      11
#define C_INT_SCHEDULE      12
#define C_AGE_SCHEDULE      13

#define C_STATISTICS  14
#define C_POPULATION  15
#define C_CODE_ARRAY  16
#define C_SPECIES     17
#define TOTAL_TYPES   18

#endif
