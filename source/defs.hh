//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          //
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef DEFS_HH
#define DEFS_HH

#define DEFAULT_DIR "../work/"

// #define VIEW_PRIMITIVE
// #define VIEW_CURSES
// #define VIEW_NCURSES
// #define VIEW_ANSI

// #define PLATFORM_WINDOWS     // enable to compile for Win32 console
// #define MSVC_COMPILER        // enable to compile with Microsoft VC++

// #define DEBUG
// #define DEBUG_CHECK_OK
// #define DEBUG_MEM_CHECK
// #define TEST

////// Don't change anything below this line. /////

#define AVIDA_VERSION "1.0"


#ifdef VIEW_primitive
#define VIEW_PRIMITIVE
#endif
#ifdef VIEW_curses
#define VIEW_CURSES
#endif
#ifdef VIEW_ncurses
#define VIEW_NCURSES
#endif


#ifdef PLATFORM_WINDOWS
 #ifdef UINT // Some modules will not compile w/out this (e.g. b_struct2)
 #undef UNIT
 #endif

 #ifdef UCHAR
 #undef UCHAR
 #endif

 #include <windows.h>	// Defines, prototypes, etc. for 
#endif

#ifdef MSVC_COMPILER
 #ifdef  _DEBUG         // VC++ debug define
 #define DEBUG
 #endif

 #ifndef _WINDEF_
 #include <string.h>
 #endif
#endif

#define MAX_INT 4294967295
#define MAX_INST_SET_SIZE 64

#ifndef FALSE
 #define FALSE 0
 #define TRUE !FALSE
#endif

#ifndef NULL
 #define NULL 0
#endif

#ifndef _WINDEF_               // Defined by WINDOWS.H
 #define UCHAR unsigned char
 #define UINT  unsigned int
#endif

#define TOOLS_DIR tools
#define CPU_DIR   cpu
#define MAIN_DIR  main
#define VIEW_DIR  viewers

#define MODE_GA       0
#define MODE_TIERRA   1
#define MODE_AVIDA    2
#define MODE_PLAYBACK 3

#define MIN_CREATURE_SIZE 8
#define MAX_CREATURE_SIZE 1024

#define TASK_GET 0
#define TASK_PUT 1
#define TASK_GGP 2

#define TASK_ECHO   0
#define TASK_NAND   1
#define TASK_NOT    2

#define TASK_AND    3
#define TASK_ORN    4

#define TASK_ANDN   5
#define TASK_OR     6

#define TASK_NOR    7
#define TASK_XOR    8
#define TASK_EQU    9

#define NUM_TASKS  10

#define POINT_MUT_BASE  1000000
#define COPY_MUT_BASE   10000
#define DIVIDE_MUT_BASE 100
#define DIVIDE_INS_BASE 100
#define DIVIDE_DEL_BASE 100

#endif
