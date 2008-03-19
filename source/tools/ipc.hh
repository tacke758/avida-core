//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef IPC_HH
#define IPC_HH

#include <iostream>
#include <strstream>
#include <fstream>
#include <iomanip>

#include "../tools/assert.hh"
#include "../tools/string.hh"


// Messages for handshakes and acknoledgements
#define SYNC_MESG "#% SYNC %#\n"
#define SEND_MESG "#% SEND %#\n"
#define RECIEVE_MESG "#% RECIEVE %#\n"


class cIPC {
protected:

  strstream * rbuf;
  ostrstream * sbuf;

public:
  cIPC();
  virtual ~cIPC() {;}
  
  virtual bool ClearRBuf();
  virtual bool ClearSBuf();

  virtual bool HasInput() = 0;  // Is there something waiting for Recieve?

  virtual bool Send() = 0;
  virtual bool Send(const cString & mesg) = 0;
  virtual bool AckSend() = 0;
  virtual bool AckSend(const cString & mesg) = 0;

  virtual bool Recieve(const int usleep_time=250, 
		       const int timeout_sec=(5*60)) = 0;
  virtual bool AckRecieve(const int usleep_time=250, 
		  	  const int timeout_sec=(5*60)) = 0;

  virtual void SeekEnd() = 0;
  virtual void Sync() = 0;

  // Buffer Access
  virtual strstream & RBuf();
  virtual ostrstream & SBuf();
};


class cFileIPC : public cIPC {
protected:
  
  cString rfilename;
  cString sfilename;

  ifstream rfs;
  ofstream sfs;

protected:
  bool AckSendRecieve(const int usleep_time=0);

public:
  cFileIPC(const cString & _rfilename, const cString & _sfilename);
  ~cFileIPC();

  bool HasInput();
  
  bool Send();
  bool Send(const cString & mesg);
  bool AckSend(){ Send(); return AckSendRecieve(); }
  bool AckSend(const cString & mesg){ Send(mesg); return AckSendRecieve(); }

  bool PartRecieve(int & send_mesg_got);
  bool Recieve(const int usleep_time=250, const int timeout_sec=(5*60));
  bool AckRecieve(const int usleep_time=250, const int timeout_sec=(5*60));

  void SeekEnd();
  void Sync();

};


#endif // ifndef IPC_HH
