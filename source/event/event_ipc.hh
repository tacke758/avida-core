//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          // 
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef EVENT_IPC_HH
#define EVENT_IPC_HH

#include <stdlib.h>
#include "../defs.hh"
#include "../tools/string.hh"
#include "../tools/ipc.hh"

// Declaring so we can store a pointer


class cEventIPC {
private:
  cFileIPC ipc;

public:
  cEventIPC(const cString & _in_file, const cString & _out_file);
  ~cEventIPC();

  // Interface
  bool GetEvents();

  cIPC & GetIPC(){ return ipc; }
};

#endif // #ifndef EVENT_IPC_HH
