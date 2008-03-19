//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef LEGEND_HH
#define LEGEND_HH

#include "FL/Fl.H"
#include <FL/Enumerations.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Image.H>

#include "../tools/tArray.hh"

#include "../tools/string.hh"

#include "color.hh"

#include "grid.hh"



struct sLegendEntry {
  cColor color;
  cString label;

  sLegendEntry() {;}

  sLegendEntry(sLegendEntry & in){
    color = in.color;
    label = in.label;
  }

  ~sLegendEntry() {;}
};



class cLegendData {
private:
  tArray<sLegendEntry> * entry;
  int size;
  int capacity;
public:
  cLegendData(int size, int capacity=-1);
  ~cLegendData();

  void SetSize(int in){ assert(size <= capacity); size=in; }
  int GetSize(){ return size; }

  void Set(int idx, int R, int G, int B, const cString & label=""){ 
    SetLabel(idx,label);
    SetColor(idx,R,G,B);
  }

  void SetLabel(int idx, const cString & label=""){ 
    assert(idx >= 0  &&  idx < size);  
    (*entry)[idx].label = label; 
  }

  void SetColor(int idx, cColor & in){
    assert(idx >= 0  &&  idx < size);  
    (*entry)[idx].color = in; 
  }

  void SetColor(int idx, int R, int G, int B){
    assert(idx >= 0  &&  idx < size);  
    (*entry)[idx].color.Set((UCHAR)R,G,B); 
  }

  const char * GetLabel(int idx){ 
    assert(idx >= 0  &&  idx < size);  
    return (*entry)[idx].label(); }

  cColor & GetColor(int idx){ 
    assert(idx >= 0  &&  idx < size);  
    return (*entry)[idx].color; }

  sLegendEntry & operator[] (int idx){ 
    assert(entry != NULL);
    assert(idx >= 0);
    assert(idx < size);
    return (*entry)[idx];
  }
};



class cLegend : public Fl_Widget {
private:
  cLegendData * data;

  cGrid * grid;

  int border;

  int entry_h;
  int entry_b;

  int cell_size;
  int cell_b;

  int text_h;
  int text_offset;

  int selected_value;
  
private:

protected:
  void draw();
  int handle(int _event);// { return 0; }
  void Resize();

public:
  cLegend(int _x, int _y, int _w, int _h, const char * label=0);
  ~cLegend();

  void Setup(cGrid * _grid){ grid = _grid; }

  int GetCellSize(){ return cell_size; }

  void SetCellSize(int in); 
  void SetCellBorder(int in); 
  void SetBorder(int in){ border=in; Resize(); }

  int GetSelectedValue(){ return selected_value; }
  void SetSelectedValue(int in){ selected_value=in; }

  void SelectValue(int in){ selected_value=in; redraw(); }

  int GetSize(){ return (data==NULL) ? 0 : data->GetSize(); }


  void DrawCell(int xpos, int ypos, int value){
    assert(data != NULL);
    assert(value >= 0);  
    // over size uses last entry
    if( value >= data->GetSize() ) value = data->GetSize() -1; 

    //assert( (*entry)[value].image != NULL );
    //(*entry)[value].image->draw( xpos+cell_b, ypos+cell_b,
    //cell_size -2*cell_b, cell_size -2*cell_b );
    (*data)[value].color.SetActive();
    fl_rectf(xpos+cell_b, ypos+cell_b, 
	     cell_size -2*cell_b, cell_size -2*cell_b );
    
    // If selected....
    if( value == selected_value ){
      int r = (*data)[value].color.R()+128%256;
      int g = (*data)[value].color.G()+128%256;
      int b = (*data)[value].color.B()+128%256;
      fl_color((UCHAR)r,g,b);
      fl_line(xpos, ypos, xpos+cell_size, ypos+cell_size);
      fl_line(xpos, ypos+cell_size, xpos+cell_size, ypos);
      fl_rect(xpos+cell_b, ypos+cell_b, 
	      cell_size -2*cell_b, cell_size -2*cell_b );
    }
  }


  void SetLegendData( cLegendData * new_entry_array );
    
};

#endif
