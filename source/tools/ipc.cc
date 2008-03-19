//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
#include <unistd.h>
#include <time.h>
#include <iostream>
#include "ipc.hh"

////// cIPC Methods... //////

cIPC::cIPC() : rbuf(NULL), sbuf(NULL) { ; }

bool cIPC::ClearRBuf(){
  if( rbuf != NULL ){
    delete rbuf->str();
    delete rbuf;
    rbuf = NULL;
  }
  return true;
}
  
bool cIPC::ClearSBuf(){
  if( sbuf != NULL ){
    delete sbuf->str();
    delete sbuf;
    sbuf = NULL;
  }
  return true;
}

//// RecieveBuffer Access ////
strstream & cIPC::RBuf(){
  if( rbuf == NULL ){
    rbuf = new strstream;
  }
  return *rbuf;
}

//// SendBuffer Access for Composition ////
ostrstream & cIPC::SBuf(){ 
  if( sbuf == NULL ){
    sbuf = new ostrstream();
  }
  return *sbuf; 
}



////// cFileIPC /////

//// Constructor & Destructor ////

cFileIPC::cFileIPC(const cString & _rfilename, const cString & _sfilename) :
 rfilename(_rfilename), sfilename(_sfilename) {
  // make sure the read file exists by opening it temporarially with append
  ofstream tmpstream(rfilename, ios::app); tmpstream.close();
  rfs.open(rfilename);
  sfs.open(sfilename,ios::app);
}


cFileIPC::~cFileIPC(){ 
  rfs.close();
  sfs.close();
  ClearRBuf();
  ClearSBuf();
}


//// Internal Methods ////

bool cFileIPC::AckSendRecieve(const int usleep_time){ 
  bool rval = false;
  if( Recieve(usleep_time) ){
    cString rec_mesg(rbuf->str());
    //if( strcmp(rec_mesg,RECIEVE_MESG) == 0 ){
    if( rec_mesg = RECIEVE_MESG ){
      rval=true;
    }
  }
  return rval;
}


bool cFileIPC::PartRecieve(int & send_mesg_got){ 
  bool rval = false;
  const int send_mesg_size = sizeof(SEND_MESG)-2;
  char c;

  // Ensure rbuf exists
  if( rbuf == NULL ){
    rbuf = new strstream;
  }

  while( rfs.good() && !rfs.eof() && rval == false && rfs.get(c) ){
    // If the character isn't part of the SEND_MESG, add to buffer
    if( c != SEND_MESG[send_mesg_got] ){
      // If we thought we were working on a SEND_MESG, add those to buffer
      if( send_mesg_got > 0 ){
	for( int i=0; i<send_mesg_got; ++i ){
	  rbuf->put(SEND_MESG[i]);
	}
	send_mesg_got = 0;
      }
	rbuf->put(c);
    }else{ 
      // If we are getting partial SEND_MESG still
      if( send_mesg_got < send_mesg_size ){
	send_mesg_got++;
      }else{
	// We completed SEND_MESG
	rbuf->put('\0');
	rval = true;
      }
    }
  } // while( rfs.good() && !rfs.eof() && rval == false && rfs.get(c) )
  return rval;
}


//// Test Methods ////
bool cFileIPC::HasInput(){
  bool rval = true;
  if( rfs.peek() < 0 ){
    rval = false;
    rfs.clear();
  }
  return rval;
}


//// Send Methods ////

bool cFileIPC::Send(){ 
  // Non-Blocking 
  assert( sbuf != NULL );
  sfs<<sbuf->rdbuf()<<SEND_MESG<<flush; 
  ClearSBuf();
  return true; 
}


bool cFileIPC::Send(const cString & mesg){ 
  ClearSBuf();
  sfs<<mesg()<<SEND_MESG<<flush; 
  return true;
}


//// Recieve Methods ////

bool cFileIPC::Recieve(const int usleep_time, const int timeout_sec){
  // Blocks until SEND_MESG is recieved || timeout
  // if timeout_sec < 0, no timeout
  bool rval = false;
  bool timeout = false;
  int begin_time = 0;
  int send_mesg_got = 0;

  // Clear the rbuf
  ClearRBuf();
  rbuf = new strstream;

  // Set begin time
  if( timeout_sec > 0 ){
    begin_time = time(NULL);
  }

  while( rval == false  &&  timeout == false ){
    if( (rval = PartRecieve(send_mesg_got)) == false ){
      rfs.clear();
    }
    usleep(usleep_time);
    if( timeout_sec > 0 ){
      timeout = ( time(NULL) > begin_time+timeout_sec );
    }
  } 
  return rval;
}


bool cFileIPC::AckRecieve(const int usleep_time, const int timeout_sec){
  bool rval = false;
  if( Recieve(usleep_time, timeout_sec) ){
    rval = Send(RECIEVE_MESG);
  }
  return rval;
}


void cFileIPC::SeekEnd(){
  sfs.seekp(0,ios::end);
  rfs.seekg(0,ios::end);
}


void cFileIPC::Sync(){
  // Should be matched with a Sync on the other end...
  Send(SYNC_MESG);
  bool got_sync_mesg = false;
  while( !got_sync_mesg ){
    Recieve();
    cString foo(RBuf().str());
    cerr<<"SR "<<foo()<<endl;
    if( foo == SYNC_MESG ){
      got_sync_mesg = true;
    }
  }
}
