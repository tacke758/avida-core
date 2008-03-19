//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          // 
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "../tools/ipc.hh"
#include "../main/avida.hh"
#include "../main/stats.hh"
#include "event_ipc.hh"


// @TCC -- make ipc filenames variables (or at least #defines)
cEventIPC::cEventIPC(const cString & _in_file, const cString & _out_file):
 ipc(_in_file,_out_file) {
  // Obliterate the input & output ipc file
  { ofstream tmp("avida_ipc.in"); tmp.close(); }
  { ofstream tmp("avida_ipc.out"); tmp.close(); }
}

cEventIPC::~cEventIPC(){;}

bool cEventIPC::GetEvents(){
  bool rv = false;
  if( ipc.HasInput() ){
    ipc.Recieve();

    cString cur_line = ipc.RBuf().str();
    cerr<<"RECIEVED \""<<cur_line<<"\""<<endl;
    int eh_id = cur_line.PopWord().AsInt();
    cString name = cur_line.PopWord();
    cerr<<"AddEvent "<<eh_id<<" "<<name<<endl;
    if( eh_id != -1 ){
      rv = cAvidaMain::ProcessEvent(name.AsInt(),cur_line, eh_id);
    }else{
      rv = cAvidaMain::ProcessEvent(name,cur_line, eh_id);
    }
    if( rv == false ){
      cString mesg;
      mesg.Set("Failed to add event from cEventIPC\n\"%d %s %s\"",
	       eh_id, name(), cur_line());
      cAvidaMain::NotifyWarning(mesg);
    }
  }
  return rv;
}

