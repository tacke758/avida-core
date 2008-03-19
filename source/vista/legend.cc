//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Color_Chooser.H>

//#include "vista_main.hh"
#include "legend.hh"
#include "grid.hh"

#include <iostream.h>


cLegendData::cLegendData(int _size, int _capacity) : 
 entry(NULL),
 size(_size),
 capacity(_capacity) {
  assert(size > 0);
  if( capacity < size ){
    capacity = size;
  }
  entry = new tArray<sLegendEntry>(capacity);
}


cLegendData::~cLegendData(){
  delete entry;
}


cLegend::cLegend(int _x, int _y, int _w, int _h, const char * label):
 Fl_Widget(_x, _y, _w, _h, label), 
 data(NULL), 
 grid(NULL),
 border(0),
 entry_b(1),
 cell_size(5+2),
 cell_b(1),
 text_offset(-3),
 selected_value(-1) {
  Resize();
}



cLegend::~cLegend() {;}


void cLegend::Resize(){
  if( data != NULL ){
    fl_font(FL_HELVETICA, 14);
    text_h = fl_height();
    if( cell_size >  text_h ) entry_h = cell_size + 2*entry_b;
    else                      entry_h = text_h + 2*entry_b;
    
    cerr<<entry_h*data->GetSize()+2*border<<" "<<data->GetSize()<<endl;
    resize(x(),y(), w(), entry_h*data->GetSize()+2*border);
    if( parent() != NULL ){ parent()->redraw(); }
    redraw();
  }
}


void cLegend::SetLegendData( cLegendData * new_data ){
  assert( new_data != NULL );
  assert( new_data-GetSize() != NULL );
  data = new_data;
  selected_value = -1;
  Resize();
}


void cLegend::SetCellSize(int in){
  cell_size = in+2*cell_b; 
  grid->ReCalc(); 
  Resize(); 
}

void cLegend::SetCellBorder(int in){
  int new_cell_size = cell_size - 2*cell_b; 
  cell_b = in; 
  SetCellSize(new_cell_size);
  grid->ReCalc(); 
  Resize(); 
}


void cLegend::draw(){
  if( data != NULL ){
    const int size = data->GetSize();
    assert( size > 0 );
    fl_font(FL_HELVETICA, 14);
    fl_color(FL_GRAY);
    fl_rectf(x()+border,y()+border,w()-2*border,h()-2*border);
    for( int i=0; i<size; i++ ){
      DrawCell( x() +border,
		y() +border +i*entry_h +entry_h/2 -cell_size/2, i );
      
      // Draw Label, wierd alingment...
      fl_color(FL_BLACK);
      fl_draw( data->GetLabel(i), 
	       x() +border +cell_size +entry_b,
	       y() +border +i*entry_h +entry_h/2 - text_h/2
	       +text_h +text_offset);
    }
  }
}
  

int cLegend::handle(int event){
  int rv = 0;
  static int push_x,push_y;
  switch (event) {
    case FL_PUSH :
      push_x = Fl::event_x() - x();
      push_y = Fl::event_y() - y();
      rv =  1;
      break;

    case FL_RELEASE :

      // Left Click sets the Selected value
      if( Fl::event_button() == 1 ){
	int idx = push_y/entry_h;
	if( data!=NULL  &&  idx < data->GetSize() ){
	  rv =  1;
	  selected_value = idx;
	  grid->redraw();
	  redraw();
	}

	// Right Click on a entry lets you set the color!!!
      }else if( Fl::event_button() == 3 ){
	int idx = push_y/entry_h;
	if( data!=NULL  &&  idx < data->GetSize() ){
	  rv =  1;
	  uchar r = (*data)[idx].color.R();
	  uchar g = (*data)[idx].color.G();
	  uchar b = (*data)[idx].color.B();
	  
	  if (!fl_color_chooser("New color:",r,g,b)) break;
	  (*data)[idx].color.Set(r,g,b);
	  SetCellSize(cell_size-cell_b*2);
	  Fl::wait(0);
	}
      }
      break;
  }
  return rv;
}
