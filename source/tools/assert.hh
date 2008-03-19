//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
/* assert.hh ******************************************************************
 Assertion Macros & Functions

 travc@ugcs.caltech.edu
 Time-stamp: <98/06/02 17:07:09 travc>

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
   ASSERT_DEBUGGER     = Exec when assert_debug is failed.
   EXIT_ASSERT_FAILED  = Exit number used for failed assertion

 Notes:
   To use the debug action then the program must declare globally...
       const char * _program_name_;
     And then copy argv[0] to it before the debug action is used.

******************************************************************************/
 
#ifndef	MYASSERT_H
#define	MYASSERT_H

#ifdef assert
#undef assert
#endif


#define ASSERT_DEBUGGER "xterm -geometry 88x72+0+0 -e gdb %s %d &",\
  program_name, getpid()

#define EXIT_ASSERT_FAILED 1


// This define may not be completely portable --  @TCC
#define ASSERT__STRING(expr) #expr


/* Defining the Macros */

#define assert(expr)             DEFINE_ASSERT(abort,  expr, NULL)
#define assert_abort(expr,msg)   DEFINE_ASSERT(abort,  expr, msg)
#define assert_error(expr,msg)   DEFINE_ASSERT(abort,  expr, msg)
#define assert_warning(expr,msg) DEFINE_ASSERT(pause,  expr, msg)
#define assert_comment(expr,msg) DEFINE_ASSERT(none,   expr, msg)
#define assert_debug(expr,msg)   DEFINE_ASSERT(debug,  expr, msg)


#ifdef	NDEBUG 

/* Macro that makes new assertions easy to define => 0 (nothing) */
#define DEFINE_ASSERT(action,expr,msg)   ((void) 0)

class cAssert {
public:
  static void Init(const char * _program_name){ (void)_program_name; }
};

#else /* Not NDEBUG */


/* Macro that makes new assertions easy to define */
#define DEFINE_ASSERT(action,expr,msg) \
  if( !(expr) ){ \
    (cAssert::__assert_print( ASSERT__STRING(expr), __FILE__, __LINE__, \
		     __ASSERT_FUNCTION, msg)); \
    (cAssert::__assert_##action##()); }


class cAssert {
private:
  static const char * program_name;
  
public:
  // Setup (must be called if assert_debug is to work)
  static void Init(const char * _program_name);

  /* Actions */
  static void __assert_abort();
  static void __assert_exit();
  static void __assert_pause();
  static void __assert_none();
  static void __assert_debug();
  
  /* Output funciton... All assertions call this then action */
  static void __assert_print( const char *__assertion,
			    const char *__file,
			    unsigned int __line,
			    const char *__function,
			    const char *msg );

};



/* Version 2.4 and later of GCC define a magical variable `__PRETTY_FUNCTION__'
   which contains the name of the function currently being defined.
   This is broken in G++ before version 2.6.  */
#if (!defined (__GNUC__) || __GNUC__ < 2 || \
     __GNUC_MINOR__ < (defined (__cplusplus) ? 6 : 4))
#define __ASSERT_FUNCTION	((const char *) 0)
#else
#define __ASSERT_FUNCTION	__PRETTY_FUNCTION__
#endif 


#endif /* NDEBUG */

#endif /* MYASSERT_H */

