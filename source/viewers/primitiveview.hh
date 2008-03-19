//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          // 
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef PRIMITIVEVIEW_HH
#define PRIMITIVEVIEW_HH

#include <iostream>
#include "../defs.hh"
#include "baseview.hh"
#include "../tools/string.hh"
#include "../main/stats.hh"


class cPrimitiveView : public cBaseView {
public:
  cPrimitiveView() {;}
  ~cPrimitiveView() {;}

  inline void DoUpdate() {;}

  inline void NotifyComment(const cString & in_string){
    cerr<<"COMMENT: "<<in_string<<endl; }
  inline void NotifyWarning(const cString & in_string){
    cerr<<"WARNING: "<<in_string<<endl; }
  inline void NotifyError(const cString & in_string){
    cerr<<"ERROR: "<<in_string<<endl; }

  inline void NotifyPause(){ 
    cout<<"NotifyPause"<<endl; }
  inline void NotifyBreakpoint(){ 
    cout<<"NotifyBreakpoint"<<endl; }
  inline void NotifyUpdate(){ }
    //cout<<"Update: "<<cStats::GetUpdate()<<"\t"
	//<<"Generation: "<<cStats::GetAveGeneration()<<endl; }
  inline void NotifyBirth(int p_cell, int d_cell){;}
  inline void NotifyExit(){  cout<<"Exiting"<<endl; }

  inline bool GetEvents(){ return FALSE; }
};

#endif // ifndef PRIMITIVEVIEW

