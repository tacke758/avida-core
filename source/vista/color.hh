//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef COLOR_HH
#define COLOR_HH

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>

class cColor {
private:
  //bool is_FL_FREE_COLOR;
  UCHAR r;
  UCHAR g;
  UCHAR b;
public:
  cColor(UCHAR _r=0, UCHAR _g=0, UCHAR _b=0) : //is_FL_FREE_COLOR(false),
   r(_r), g(_g), b(_b) { ; }
  cColor(double _r, double _g, double _b) : //is_FL_FREE_COLOR(false),
    r((UCHAR)(_r*255)), g((UCHAR)(_g*255)), b((UCHAR)(_b*255)) { ; }

  cColor(const cColor & _in) { r= _in.r;  g= _in.g;  b= _in.b;  }

  cColor & operator=(const cColor & _in)
    {  r= _in.r;  g= _in.g;  b= _in.b; return *this; }

  //void SetFL_FREE_COLOR() { is_FL_FREE_COLOR=true; }

  void Set(UCHAR _r, UCHAR _g, UCHAR _b){ 
    //is_FL_FREE_COLOR=false; 
    r=_r; g=_g; b=_b; }
  void Set(double _r, double _g, double _b){
    //is_FL_FREE_COLOR=false; 
    r = (UCHAR)(_r*255);  g = (UCHAR)(_g*255);  b = (UCHAR)(_b*255); }

  void SetActive() { 
    //if( is_FL_FREE_COLOR ) fl_color(FL_FREE_COLOR); else 
    fl_color(r, g, b); }

  UCHAR R() const { return r;}
  UCHAR G() const { return g;}
  UCHAR B() const { return b;}
};

#endif
