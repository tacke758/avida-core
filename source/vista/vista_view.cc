//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          // 
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "../main/avida.hh"
#include "../main/stats.hh"

#include "vista_view.hh"
#include "vista_main.hh"


// @TCC -- make ipc filenames variables (or at least #defines)
cVistaView::cVistaView() { 
  vista_main = new cVistaMain();
}

cVistaView::~cVistaView(){
  delete vista_main;
  vista_main = NULL;
}


void cVistaView::DoUpdate(){ 
  vista_main->DoUpdate();
}


void cVistaView::NotifyComment(const cString & in_string) {
  vista_main->ShowDialog("Comment", in_string);
}

void cVistaView::NotifyWarning(const cString & in_string) {
  vista_main->ShowDialog("WARNING", in_string);
}

void cVistaView::NotifyError(const cString & in_string) {
  vista_main->ShowDialog("ERROR !!", in_string);
}


void cVistaView::NotifyUpdate(){
  vista_main->UpdateMainWindow();
  vista_main->UpdateStatsWindow();
  vista_main->UpdateMapWindow();
  vista_main->DoUpdate(0);
}

void cVistaView::NotifyBirth(int p_cell, int d_cell){
  if( vista_main->RedrawOnBirth() ){
    //vista_main->UpdateMainWindow();
    vista_main->UpdateStatsWindow();
    vista_main->UpdateMapWindow(p_cell, d_cell);
  }
  vista_main->DoUpdate(0);
}


void cVistaView::NotifyPause(){
  vista_main->UpdatePauseState();
}
  
  
void cVistaView::NotifyExit(){
  cerr<<"VV Exiting"<<endl;
}


void cVistaView::NotifyBreakpoint(){
  cerr<<"VV Breakpoint"<<endl;
}



bool cVistaView::GetEvents(){
  return false;
}
