//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
#include <FL/Fl.H>
#include <iostream>

#include "../defs.hh"
#include "../tools/string.hh"
#include "../tools/random.hh"

#include "../main/stats.hh"
#include "../main/config.hh"
#include "../main/avida.hh"
#include "../main/genotype.hh"
#include "../main/genebank.hh"
#include "../cpu/cpu.hh"

#include "cVistaUI.hh"
#include "cMapWindowUI.hh"
#include "map_mode_enum.hh"
#include "cDialogWindow.hh"
#include "grid.hh"
#include "legend.hh"
#include "vista_main.hh"

cVistaMain * vista_main = NULL;


cVistaMain::cVistaMain() : ui(NULL), map_ui(NULL), stats_window_open(false),
			   random(-1),
			   map_value_method(&GetThresholdMapValue),
			   threshold_legend(NULL),
			   birth_legend(NULL),
			   genotype_legend(NULL)
  {
  vista_main = this;
  ui = new cVistaUI;

  //Initial global objects.
  Fl::visual(FL_DOUBLE|FL_INDEX);
  
  ShowMainWindow();
  
  Fl::wait(0.025);
}


cVistaMain::~cVistaMain(){
  // shutdown stuff
  delete ui;
  if( threshold_legend != NULL ){
    delete threshold_legend;
  }
  if( birth_legend != NULL ){
    delete birth_legend;
  }
  if( genotype_legend != NULL ){
    delete genotype_legend;
    delete genotype_last_legend;
  }
}

void cVistaMain::Close(){
  cAvidaMain::StartViewer("primitive");
}


void cVistaMain::DoUpdate(double time){
  Fl::wait(time);
}



void cVistaMain::ShowMainWindow(){ 
  assert(vista_main != NULL);
  assert(ui != NULL);
  ui->show(); 
}

/// Main Window Stuff

void cVistaMain::ExitEvent(){
  cerr<<"Exit Event"<<endl;
  cAvidaMain::ExitProgram(0);
}


void cVistaMain::UpdatePauseState(){
  if( cAvidaMain::IsPaused() ){
    ui->pause_button->set();
    if( map_ui != NULL  && map_ui->map_window->shown() )
      map_ui->pause_button->set();
  }else{
    ui->pause_button->clear();
    if( map_ui != NULL  && map_ui->map_window->shown() )
      map_ui->pause_button->clear();
  }
}


bool cVistaMain::GetPauseState(){
  return cAvidaMain::IsPaused();
}


void cVistaMain::TogglePauseState(){
  cerr<<"Toggle Pause"<<endl;
  if( cAvidaMain::IsStepMode() )  cAvidaMain::ToggleStepMode();
  cAvidaMain::TogglePause();
}

void cVistaMain::Step(){
  ui->pause_button->set();
  if( map_ui != NULL  && map_ui->map_window->shown() )
    map_ui->pause_button->set();
  if( !cAvidaMain::IsStepMode() )  cAvidaMain::ToggleStepMode();
  cAvidaMain::TogglePause();
}


void cVistaMain::UpdateMainWindow(){
  static char buf[32];
  sprintf(buf,"%d",cStats::GetUpdate());
  ui->update_text->value(buf);
  sprintf(buf,"%g",cStats::GetAveGeneration());
  ui->average_generation_text->value(buf);
}

bool cVistaMain::RedrawOnBirth(){ return ui->redraw_on_birth; }


///// Dialog window

void cVistaMain::ShowDialog(const cString & title, const cString & mesg){
  {
    cDialogWindow dwin(title(),mesg());
    dwin.Block();
  }
}


///// Stats Window Stuff
void cVistaMain::UpdateStatsWindow(){ 
  if( ui->stats_window->shown() ){
    static char buf[32];
    // Population Stats
    sprintf(buf,"%d",cStats::GetNumDeaths());
    ui->total_births->value(buf);
    sprintf(buf,"%d",cStats::GetBreedTrue());
    ui->breed_true->value(buf);
    sprintf(buf,"%d",cStats::GetNumParasites());
    ui->parasites->value(buf);
    sprintf(buf,"%f",cStats::GetEnergy());
    ui->energy->value(buf);
    sprintf(buf,"%g",cStats::GetMaxFitness());
    ui->max_fitness->value(buf);
    sprintf(buf,"%g",cStats::GetMaxMerit());
    ui->max_merit->value(buf);
    
    // Dominant Stats
    cGenotype * dom_geno = cAvidaMain::GetBestGenotype();
    sprintf(buf,"%g",dom_geno->GetFitness());
    ui->dom_fitness->value(buf);
    sprintf(buf,"%g",dom_geno->GetMerit());
    ui->dom_merit->value(buf);
    sprintf(buf,"%g",dom_geno->GetGestationTime());
    ui->dom_gestation->value(buf);
    sprintf(buf,"%d",dom_geno->GetLength());
    ui->dom_size->value(buf);
    sprintf(buf,"%g",dom_geno->GetCopiedSize());
    ui->dom_copy_size->value(buf);
    sprintf(buf,"%g",dom_geno->GetExecutedSize());
    ui->dom_exec_size->value(buf);
    sprintf(buf,"%d",dom_geno->GetNumCPUs());
    ui->dom_abundance->value(buf);
    sprintf(buf,"%d", dom_geno->GetThisBirths());
    ui->dom_births->value(buf);
    if (cStats::GetAveMerit() != 0) {
      sprintf(buf,"%g",( ((double) cConfig::GetAveTimeslice()) * 
			 dom_geno->GetFitness() / cStats::GetAveMerit()));
      ui->dom_birth_rate->value(buf);
    } else {
      ui->dom_birth_rate->value("0.0");
    }
    ui->dom_num_threads->value("N/A");
    ui->dom_thread_dist->value("N/A");
    
    // Avearge Stats
    sprintf(buf,"%g",cStats::GetAveFitness());
    ui->ave_fitness->value(buf);
    sprintf(buf,"%g",cStats::GetAveMerit());
    ui->ave_merit->value(buf);
    sprintf(buf,"%g",cStats::GetAveGestation());
    ui->ave_gestation->value(buf);
    sprintf(buf,"%g",cStats::GetAveSize());
    ui->ave_size->value(buf);
    sprintf(buf,"%g",cStats::GetAveCopySize());
    ui->ave_copy_size->value(buf);
    sprintf(buf,"%g",cStats::GetAveExeSize());
    ui->ave_exec_size->value(buf);
    sprintf(buf,"%g",( ((double) cStats::GetNumCreatures()) /
		       ((double) cStats::GetNumGenotypes())));
    ui->ave_abundance->value(buf);
    sprintf(buf,"%g",( ((double) cStats::GetNumDeaths()) /
		       ((double) cStats::GetNumGenotypes())));
    ui->ave_births->value(buf);
    if (cStats::GetAveMerit() != 0) {
      sprintf(buf,"%g",( ((double) cConfig::GetAveTimeslice()) * 
			 cStats::GetAveFitness() / cStats::GetAveMerit()));
      ui->ave_birth_rate->value(buf);
    } else {
      ui->ave_birth_rate->value("0.0");
    }
    sprintf(buf,"%g",cStats::GetAveNumThreads());
    ui->ave_num_threads->value(buf);
    sprintf(buf,"%g",cStats::GetAveThreadDist());
    ui->ave_thread_dist->value(buf);

    ui->stats_window->redraw();
    Fl::wait(0);
  }
}



////// Map Window //////////
void cVistaMain::OpenMapWindow(){
  if( map_ui == NULL ){
    map_ui = new cMapWindowUI(cConfig::GetWorldX(),cConfig::GetWorldY());
  }
  ui->map_window_button->set();
  map_ui->grid->SetOrigin(cConfig::GetWorldX()/2, cConfig::GetWorldY()/2);
  SetupMap();
  map_ui->show();
  Fl::wait(0);
}

void cVistaMain::CloseMapWindow(){
  if( map_ui != NULL ){
    delete map_ui;
    map_ui = NULL;
  }
  ui->map_window_button->clear();
}
    

void cVistaMain::UpdateMapWindow(int p_cell, int d_cell){ 
  if( map_ui != NULL  && map_ui->map_window->shown() ){
    map_ui->grid->SetValue(p_cell, (this->*map_value_method)(p_cell));
    map_ui->grid->SetValue(d_cell, (this->*map_value_method)(d_cell));
    map_ui->grid->redraw();
  }
}


void cVistaMain::UpdateMapWindow(){ 
  if( map_ui != NULL  && map_ui->map_window->shown() ){
    if( map_ui->mode == MAP_MODE_GENOTYPE ){
      UpdateGenotypeLegend();
    }
    for( int i = 0; i<cConfig::GetWorldX()*cConfig::GetWorldY(); i++ ){
      map_ui->grid->SetValue(i, (this->*map_value_method)(i));
    }
    map_ui->grid->redraw();
  }
}

void cVistaMain::SetupMap(int mode){ 
  if( map_ui != NULL  && map_ui->map_window->shown() ){
    switch( mode ){
      case MAP_MODE_THRESHOLD :
	CreateThresholdMap();
	break;
      case MAP_MODE_BIRTH :
	CreateBirthMap();
	break;
      case MAP_MODE_GENOTYPE :
	CreateGenotypeMap();
	break;
      default :
	cAvidaMain::NotifyWarning("Invalid Map Mode!!");
	break;
    }
    map_ui->legend->redraw();
    UpdateMapWindow();
  }
}

void cVistaMain::CreateThresholdMap(){ 
  if( threshold_legend == NULL ){
    threshold_legend = new cLegendData(3);
    threshold_legend->Set(0, 000,000,000, "No Cell");
    threshold_legend->Set(1, 128,000,000, "Under");
    threshold_legend->Set(2, 000,255,000, "Threshold");
  }
  map_value_method = &GetThresholdMapValue;
  map_ui->legend->SetLegendData(threshold_legend);
}
  

void cVistaMain::CreateBirthMap(){ 
  if( birth_legend == NULL ){
    birth_legend = new cLegendData(7);
    birth_legend->Set(0, 000,000,000, "No Cell");
    birth_legend->Set(1, 128,000,000, "0");
    birth_legend->Set(2, 000,100,000, "1");
    birth_legend->Set(3, 000,140,000, "2");
    birth_legend->Set(4, 000,180,000, "3");
    birth_legend->Set(5, 000,220,000, "4");
    birth_legend->Set(6, 000,255,000, "5+");
  }
  map_value_method = &GetBirthMapValue;
  map_ui->legend->SetLegendData(birth_legend);
}


void cVistaMain::CreateGenotypeMap(){ 
  if( genotype_legend == NULL ){
    genotype_legend = new cLegendData(3,
			   cConfig::GetWorldX()*cConfig::GetWorldY()+3);
    genotype_legend->Set(0, 000,000,000, "No Cell");
    genotype_legend->Set(1, 128,000,000, "Not Threshold");
    genotype_legend->Set(2, 255,000,000, "Unkown");
    // The last legend tmp variable for fast updates
    genotype_last_legend = new cLegendData(3,
			   cConfig::GetWorldX()*cConfig::GetWorldY()+3);
    genotype_last_legend->Set(0, 000,000,000, "Foo");
    genotype_last_legend->Set(1, 128,000,000, "Foo");
    genotype_last_legend->Set(2, 255,000,000, "Unkown");
  }
  map_value_method = &GetGenotypeMapValue;
  map_ui->legend->SetLegendData(genotype_legend);
  UpdateGenotypeLegend();
}


void cVistaMain::UpdateGenotypeLegend(){ 
  // Swap the legends
  cLegendData * last_legend = genotype_legend;
  genotype_legend = genotype_last_legend;
  genotype_last_legend = last_legend;
  
  genotype_legend->SetSize(cConfig::GetWorldX()*cConfig::GetWorldY()+3);

  // Copy first three entires (special entries) as is
  (*genotype_legend)[0] = (*last_legend)[0];
  (*genotype_legend)[1] = (*last_legend)[1];
  (*genotype_legend)[2] = (*last_legend)[2];

  // Set random or copy old entries for every other threshold genotype
  int last_selected_value = map_ui->legend->GetSelectedValue();
  int new_selected_value = -1;
  int idx = 3;
  for( cGenotype * cur_genotype = cAvidaMain::GetGenebank()->ResetThread(0);
       cur_genotype != NULL && cur_genotype->GetThreshold();
       cur_genotype = cAvidaMain::GetGenebank()->NextGenotype(0)) {  
    if( cur_genotype->GetThreshold() ){
      // If it has already been given a value
      const int last_value = cur_genotype->GetSymbol();
      if( last_value < last_legend->GetSize() &&
	  last_value > 0 ){
	// Propagate the selected value
	if( last_value == last_selected_value ){
	  new_selected_value = idx;
	}
	// use the color from the old list for idx
	(*genotype_legend)[idx].color = (*last_legend)[last_value].color;
      }else{
	(*genotype_legend)[idx].color.Set( (UCHAR) 000, 
					   random.GetUInt(220)+25, 
					   random.GetUInt(220)+25 );
      }
      (*genotype_legend)[idx].label.Set("%s %d",cur_genotype->GetName()(),
					cur_genotype->GetNumCPUs());
      cur_genotype->SetSymbol(idx);
      idx++;
    }
  }

  genotype_legend->SetSize(idx);
  map_ui->legend->SetLegendData(genotype_legend);
  map_ui->legend->SetSelectedValue(new_selected_value);
  map_ui->legend->redraw();
}


int cVistaMain::GetThresholdMapValue(int i){ 
  static int rv = 0;
  if( cAvidaMain::GetCellGenotype(i) == NULL ){
    rv = 0;
  }else if( !cAvidaMain::GetCellGenotype(i)->GetThreshold() ){
    rv = 1;
  }else{
    rv = 2;
  }
  return rv;
}


int cVistaMain::GetBirthMapValue(int i){ 
  static int rv = 0;
  if( cAvidaMain::GetCellGenotype(i) == NULL ){
    rv = 0;
  }else{
    rv = cAvidaMain::GetCellCPU(i)->GetNumDivides()+1;
  }
  return rv;
}

  
int cVistaMain::GetGenotypeMapValue(int i){ 
  static int rv = 0;
  if( cAvidaMain::GetCellGenotype(i) == NULL ){
    rv = 0;
  }else if( !cAvidaMain::GetCellGenotype(i)->GetThreshold() ){
    rv = 1;
  }else{
    rv = cAvidaMain::GetCellGenotype(i)->GetSymbol();
  }
  return rv;
}

  

