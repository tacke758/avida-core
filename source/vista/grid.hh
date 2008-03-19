//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef GRID_HH
#define GRID_HH

#include <stdio.h>
#include <FL/Enumerations.H>
#include <FL/Fl_Widget.H>

#include "../tools/tArray.hh"

#include "../tools/string.hh"

#include "color.hh"

class cLegend;


class cGrid : public Fl_Widget {
private:
  tArray<int> * grid;
  cLegend * legend;

  int rows;
  int cols;

  double gx0;
  double gy0;

  int border;

  int cell_size;

  // Selection
  int selected_cell;
  int selected_value;

  
  // Cached stuff to avoid extra computations
  // Scale and offset (y=mx+b)
  double wgm_x;
  double gwm_x;
  double wgb_x;
  double gwb_x;
  double wgm_y;
  double gwm_y;
  double wgb_y;
  double gwb_y;
  // Center offset
  double wc_x;
  double wc_y;
  // Cell size stuff
  int inner_cell_size;
  // Corners...
  int worldgrid_x0;
  int worldgrid_y0;
  int worldgrid_xmax;
  int worldgrid_ymax;
  // Clipping area
  int clip_x0;
  int clip_y0;
  int clip_xmax;
  int clip_ymax;

  // For GetCenterString
  char buf[32];

private:
  double WtoGX(double in){ 
    if( in < clip_x0 ){ 
      in = -1;
    }else if( in >= clip_xmax ){
      in = -1;
    }else{
      in = (wgm_x*(in-worldgrid_x0)) + gx0;
      if( in >= cols ){
	in -= cols;
      }
    }
    return in;
  }

  double WtoGY(double in){ 
    if( in < clip_y0 ){
      in = -1;
    }else if( in >= clip_ymax ){
      in = -1;
    }else{
      in = (wgm_y*(in-worldgrid_y0)) + gy0;
      if( in >= rows ){
	in -= rows;
      }
    }
    return in;
  }

  double GtoWX(double in){
    if(       in < gx0 ){ 
      in += cols;
    }else if( in >= gx0 + cols ){
      in -= cols;
    }
    return (gwm_x*in) + gwb_x; 
  }

  double GtoWY(double in){
    if(       in < gy0 ){ 
      in += rows;
    }else if( in >= gy0 + rows ){
      in -= rows;
    }
    return (gwm_y*in) + gwb_y; 
  }


protected:
  void draw();
  int handle(int _event);

  void DrawSelectedBox(int xpos, int ypos);

public:
  cGrid(int _x, int _y, int _w, int _h, const char * label=0);
  ~cGrid();
  void SetupSize(int x_cells, int y_cells, cLegend * legend);

  void ReCalc();

  int GetCols(){ return cols; }
  int GetRows(){ return rows; }

  const char * GetOriginString();
  double GetX0(){ return gx0; }
  double GetY0(){ return gy0; }


  void SetValue(int x, int y, int v) { SetValue(x+y*cols, v); }
  void SetValue(int i, int v) { assert( grid!=NULL ); (*grid)[i]=v; } 

  void SetBorder(int in){ border=in; ReCalc(); }

  void SetX0(double in){ SetOrigin(in, gy0); }
  void SetY0(double in){ SetOrigin(gx0, in); }
  void SetOrigin(double _gx0, double _gy0){ gx0=_gx0; gy0=_gy0; ReCalc(); }
};

#endif
