/* memtrack.cc ****************************************************************
 Memory tracking functions

 travc@ugcs.caltech.edu
 Time-stamp: <98/06/02 16:51:50 travc>

******************************************************************************/
 
#include "memtrack.hh"

#ifdef DEBUG_INTEGRITY_CHECK

int tIntegrityCheck<cCodeArray>::total_count(0);
int tIntegrityCheck<cCodeArray>::new_count(0);
int tIntegrityCheck<cCodeArray>::del_count(0);
int tIntegrityCheck<cCodeArray>::prime(0);
cList tIntegrityCheck<cCodeArray>::object_table[INTEGRITY_HASH_SIZE];

int tIntegrityCheck<cBasicInstruction>::total_count(0);
int tIntegrityCheck<cBasicInstruction>::new_count(0);
int tIntegrityCheck<cBasicInstruction>::del_count(0);
int tIntegrityCheck<cBasicInstruction>::prime(0);
cList tIntegrityCheck<cBasicInstruction>::object_table[INTEGRITY_HASH_SIZE];

int tIntegrityCheck<cIntegratedScheduleNode>::total_count(0);
int tIntegrityCheck<cIntegratedScheduleNode>::new_count(0);
int tIntegrityCheck<cIntegratedScheduleNode>::del_count(0);
int tIntegrityCheck<cIntegratedScheduleNode>::prime(0);
cList tIntegrityCheck<cIntegratedScheduleNode>::object_table[INTEGRITY_HASH_SIZE];

#endif  // DEBUG_INTEGRITY_CHECK
