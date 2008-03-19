//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
/* assert.cc ******************************************************************
 Assertion Macros & Functions

 travc@ugcs.caltech.edu
 Time-stamp: <98/06/11 19:26:15 travc>

 If NDEBUG is defined,  all Marcos are become void statements

 All Macros take acion if expr is not met
 All printing is to stderr

 Macros:
   DEFINE_ASSERT( action to take, expression to eval, message to print )

 Default Assertion Macros:
   assert(Boolean expr)                            -  Exits
   assert_error(Boolean expr, const char * msg)    -  Prints msg & Exits
   assert_warning(Boolean expr, const char * msg)  -  Prints msg & Pauses
   assert_comment(Boolean expr, const char * msg)  -  Prints msg 

 Actions:
   abort
   exit
   pause
   debug
   none

 Define Variables:
   ASSERT_DEBUGGER 
   EXIT_ASSERT_FAILED

 Notes:
   To use the debug action then the program must declare globally...
       const char * _program_name_;
     And then copy argv[0] to it before the debug action is used.

******************************************************************************/
 
#include "assert.hh"


#ifdef	NDEBUG 

/* NO ASSERT */

#else /* Not NDEBUG */


#include <stdio.h>
#include "../defs.hh"

#ifdef MSVC_COMPILER
#include <process.h>
#undef ASSERT_DEBUGGER
#endif

#ifdef ASSERT_DEBUGGER
#include <unistd.h>
#include <stdlib.h>
#endif 


/* Need to define static variable _program_name_ */
const char * cAssert::program_name = NULL;


/* Function for the debugger to break at (gdb at least) */

void grab_debugger(){;}



// Setup (program name needed)
void cAssert::Init(const char * _program_name){
  program_name = _program_name;
}


/* Output funciton... All assertions call this then action */

void cAssert::__assert_print( const char *__assertion,
			    const char *__file,
			    unsigned int __line,
			    const char *__function,
			    const char *msg ){
  fprintf(stderr,"%s:%d: ",__file,__line);
  fprintf(stderr,"Assertion Failure\n");
  if( msg!=NULL && msg[0]!='\0' ){
    fprintf(stderr,"\t\"%s\"\n",msg);
  }
  fprintf(stderr,"\tFunction: %s\n",__function);
  fprintf(stderr,"\tCondition: ( %s )\n",__assertion);
  fflush(stderr);
}


/* Actions */

void cAssert::__assert_abort(){
  fprintf(stderr,"\tAborting ... Bye!\n");
  fflush(stderr);
  abort();
}
  
void cAssert::__assert_exit(){
  fprintf(stderr,"\tExit(%d) ... Bye!\n",EXIT_ASSERT_FAILED);
  fflush(stderr);
  exit(EXIT_ASSERT_FAILED);
}
  
void cAssert::__assert_pause(){
  fprintf(stderr,"\tPress Enter to Continue\n");
  fflush(stderr);
  fflush(stdin);
  getc(stdin);
}
  
void cAssert::__assert_none(){;}

void cAssert::__assert_debug(){
#ifdef ASSERT_DEBUGGER
  char buf[2048];
  if( program_name != NULL ){
    sprintf(buf, ASSERT_DEBUGGER);
    fprintf(stderr,"\tRunning: %s\n",buf);
    system(buf);
    while(1) grab_debugger();
  }else{
    fprintf(stderr,"** cAssert::Init(program_name) must be called before debugger **\n");
    __assert_abort();
  }
#else
    fprintf(stderr,"\tNot compiled with ASSERT_DEBUGGER defined\n");
  __assert_abort();
#endif
}
  

#endif /* NDEBUG */


 
