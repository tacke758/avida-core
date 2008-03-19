//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          // 
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "baseview.hh"
#include "externalview.hh"
#include "../main/avida.hh"
#include "../main/stats.hh"


cExternalView::cExternalView() : 
  event_handler_id(-1) { 
}

cExternalView::~cExternalView(){;}

void cExternalView::NotifyComment(const cString & in_string) {
  cerr<<"cExternalView::Comment: "<<in_string()<<endl;
}

void cExternalView::NotifyWarning(const cString & in_string) {
  cerr<<"cExternalView::Warning: "<<in_string()<<endl;
}

void cExternalView::NotifyError(const cString & in_string) {
  cerr<<"cExternalView::ERROR: "<<in_string()<<endl;
}


void cExternalView::NotifyUpdate(){
  cerr<<"cExternalView::Notify Update"<<endl;
}


void cExternalView::NotifyPause(){
  cerr<<"cExternalView::Notify Pause"<<endl;
}
  
  
void cExternalView::NotifyExit(){
  cerr<<"cExternalView::Notify Exit"<<endl;
}


bool cExternalView::GetEvents(){
  bool rv = false;
  return rv;
}


