//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>

#include "legend.hh"
#include "grid.hh"

#include <iostream.h>

cGrid::cGrid(int _x, int _y, int _w, int _h, const char * label):
 Fl_Widget(_x, _y, _w, _h, label), 
 grid(NULL), 
 legend(NULL), 
 rows(0),
 cols(0),
 gx0(0),
 gy0(0),
 border(3),
 selected_cell(-1),
 selected_value(-1) {;}

cGrid::~cGrid() {
  if( grid != NULL )
    delete grid;
}

void cGrid::SetupSize(int _cols, int _rows, cLegend * _legend){
  assert(_cols > 0  &&  _rows > 0);
  cols = _cols;
  rows = _rows;
  legend = _legend;
  legend->Setup(this);
  selected_cell = -1;
  selected_value = -1;
  gx0 = 0;
  gy0 = 0;

  //if( update_grid != NULL )
    //delete update_grid;
  //update_grid = new tArray<bool>(cols*rows);

  if( grid != NULL )
    delete grid;
  grid = new tArray<int>(cols*rows);

  ReCalc();
}


void cGrid::ReCalc(){
  // CellSize from legend.... if there is one
  if( legend != NULL ){
    cell_size = legend->GetCellSize();
  }else{
    cerr<<"NO LEGEND AND GRID RECALC!"<<endl;
    cell_size = 1;
  }

  // World Center
  wc_x = (double)w() / 2.0;
  wc_y = (double)h() / 2.0;
  // Grid to World
  // Scale
  gwm_x = (double)cell_size;
  gwm_y = (double)cell_size;
  // Offset
  gwb_x = -(gwm_x*gx0) + wc_x - gwm_x*cols/2;
  gwb_y = -(gwm_y*gy0) + wc_y - gwm_y*rows/2;

  // World to Grid
  // Scale
  wgm_x = 1.0/(double)cell_size;
  wgm_y = 1.0/(double)cell_size;
  // Offset handled in WtoG functions

  // Corners...
  worldgrid_x0 = (int)(gwm_x*gx0 + gwb_x);
  worldgrid_y0 = (int)(gwm_y*gy0 + gwb_y);
  worldgrid_xmax = (int)(gwm_x*(gx0+cols) + gwb_x); 
  worldgrid_ymax = (int)(gwm_y*(gy0+rows) + gwb_y); 

  // Clipping Area
  if( worldgrid_x0 < border ) clip_x0 = border; 
  else                        clip_x0 = worldgrid_x0;
  if( worldgrid_y0 < border ) clip_y0 = border; 
  else                        clip_y0 = worldgrid_y0;
  if( worldgrid_xmax > w()-border*2 ) clip_xmax = w()-border; 
  else                                clip_xmax = worldgrid_xmax;
  if( worldgrid_ymax > h()-border*2 ) clip_ymax = h()-border; 
  else                                clip_ymax = worldgrid_ymax;

  redraw();
}


void cGrid::draw(){
  if( grid != NULL && legend != NULL && legend->GetSize() > 0 ){
    assert(legend != NULL );
    
    // Grey out the whole area
    //fl_color(FL_BLACK);
    //fl_rectf(x(), y(), w(), h());
    fl_color(FL_GRAY);
    fl_rectf(x()+border, y()+border, w()-border*2, h()-border*2);

    // Clipping area is actual grid
    fl_clip(x()+clip_x0, y()+clip_y0, clip_xmax-clip_x0, clip_ymax-clip_y0); 

    // Frame for the actual grid (in case we are zoomed out far)
    fl_color(FL_BLACK);
    fl_rectf(x()+worldgrid_x0-1, y()+worldgrid_y0-1, 
	     worldgrid_xmax-worldgrid_x0+2, worldgrid_ymax-worldgrid_y0+2);

    // Actually color the gird
    static int value, idx, i, j, wx, wy;
    const int size = rows*cols;
    for( idx=0; idx<size; ++idx ){
      i = idx % cols;
      j = idx / cols;
  
      wx = (int)GtoWX(i);
      wy = (int)GtoWY(j);
      
      value = (*grid)[idx];
      
      // Actually draw
      legend->DrawCell(x()+wx, y()+wy, value);
      // if the cell is over the edge, draw on other edge
      if( wx+cell_size > worldgrid_xmax ){
	legend->DrawCell(x()+wx +worldgrid_x0-worldgrid_xmax, 
			 y()+wy,  
			 value);
      }
      if( wy+cell_size > worldgrid_ymax ){
	legend->DrawCell(x()+wx, 
			 y()+wy +worldgrid_y0-worldgrid_ymax, 
			 value );
	// Handle the lower left corner cell if need be
	if( wx+cell_size > worldgrid_xmax ){
	  legend->DrawCell(x()+wx +worldgrid_x0-worldgrid_xmax, 
			   y()+wy +worldgrid_y0-worldgrid_ymax, 
			   value );
	}
	
      }
    }

    // Draw and indicator over the selected cell if there is one
    if( selected_cell >=0 && selected_cell < size ){
      i = selected_cell % cols;
      j = selected_cell / cols;
      wx = (int)GtoWX(i);
      wy = (int)GtoWY(j);

      DrawSelectedBox(x()+wx, y()+wy ); 
      if( wx+cell_size > worldgrid_xmax ){
	DrawSelectedBox(x()+wx +worldgrid_x0-worldgrid_xmax, y()+wy); 
      }
      if( wy+cell_size > worldgrid_ymax ){
	DrawSelectedBox(x()+wx, y()+wy +worldgrid_y0-worldgrid_ymax);
	// Handle the lower left corner cell if need be
	if( wx+cell_size > worldgrid_xmax ){
	  DrawSelectedBox( x()+wx +worldgrid_x0-worldgrid_xmax, 
			   y()+wy +worldgrid_y0-worldgrid_ymax);
	}
      }
    }
    
    fl_pop_clip();
  }
}


void cGrid::DrawSelectedBox(int xpos, int ypos){
  fl_color(FL_YELLOW);
  fl_line(xpos, ypos, xpos+cell_size-1, ypos+cell_size-1);
  fl_line(xpos, ypos+cell_size-1, xpos+cell_size-1, ypos);
  fl_rect(xpos, ypos, cell_size, cell_size);
}


const char * cGrid::GetOriginString(){
  sprintf(buf,"%.2f,%.2f",gx0,gy0);
  return buf;
}


int cGrid::handle(int event){
  int rv = 0;
  static int push_x,push_y;
  switch (event) {
    case FL_PUSH : {
      push_x = Fl::event_x() - x();
      push_y = Fl::event_y() - y();
      rv =  1;
      break; }
    
      
    case FL_RELEASE : {
      rv =  1;
      // Click selects
      // Calculate click position....
      double gx = WtoGX(push_x);
      double gy = WtoGY(push_y);
      if( gx < 0 || gy < 0 ){
	cerr<<"Out of bounds click"<<endl;
      }else{
	cerr<<"click "<<(int)gx<<","<<(int)gy<<endl;
	  // If it is selected, Unselect, else Selcet
	if( selected_cell == (int)gy*cols + (int)gx ){
	  selected_cell = -1;
	  legend->SelectValue(-1);
	}else{
	  selected_cell = (int)gy*cols + (int)gx;
	  legend->SelectValue((*grid)[selected_cell]);
	}
	redraw();
      }
      break; 
    }

    case FL_FOCUS : {
      rv = 1;
      break;
    }
    case FL_UNFOCUS : {
      // @TCC -- UNFOCUS... Is this the right response to give it away?
      rv = 1;
      break;
    }
    case FL_KEYBOARD : {
      switch ( Fl::event_key() ){
	case FL_Left :
	  rv = 1;
	  gx0 -= 1;
	  if( gx0 < 0 ) gx0 += cols;
	  ReCalc();
	  break;
	case FL_Right :
	  rv = 1;
	  gx0 += 1;
	  if( gx0 >= rows ) gx0 -= cols;
	  ReCalc();
	  break;
	case FL_Up :
	  rv = 1;
	  gy0 -= 1;
	  if( gy0 < 0 ) gy0 += rows;
	  ReCalc();
	  break;
	case FL_Down :
	  rv = 1;
	  gy0 += 1;
	  if( gy0 >= rows ) gy0 -= rows;
	  ReCalc();
	  break;
      }
    }
  }

  return rv;
}
