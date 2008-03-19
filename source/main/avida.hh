//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef AVIDA_HH
#define AVIDA_HH

#include "../defs.hh"
#include "../tools/string.hh"
#include "../tools/assert.hh"

// cAvidaMain is a static class to handle the main() level of an avida run
// Globals and the like should actually be part of this class.

class cEventManager;
class cEventList;
class cEventIPC;
class cPopulation;
class cBaseView;
class cView;

class cGenebank;
class cGenotype;
class cBaseCPU;


class cAvidaMain {
private:
  static cEventManager * event_manager;
  static cEventList * event_list;
  static cEventIPC * event_ipc;

  static cPopulation * population;


  // Event Hander IDs
  static int main_eh_id;
  static int population_eh_id;

public:
  static cView * viewer;  // @TCC old Viewer
private:
  static cBaseView * base_viewer;

  // Global state varaibles and flags
  static bool step_flag;
  static bool paused_flag;
  static bool done_flag;

  // Internal Functions...
  static void SetupViewer(const int viewer_type);
  static void CollectData(cGenebank & genebank); // Should not be here @TCC

public:
  // Setup
  static void Setup();
  static void ReadEventListFile(const cString & filename = "event_list" );

  // State Changes
  static void ToggleStepMode(){ step_flag = !step_flag; }
  static void TogglePause(){ paused_flag=!paused_flag; }
  static inline void SetDoneFlag(bool in=TRUE){ done_flag = in; }
  // State Access
  static bool IsStepMode(){ return step_flag; }
  static bool IsPaused(){ return paused_flag; }
  static inline bool GetDoneFlag(){ return done_flag; }

  // Primary Functions
  static void Run();
  static void Analyze();
  static void GetEvents();
  static void ProcessPause();
  static void ProcessPopulationUpdate();

  // Adding Events
  static bool AddEventFileFormat(const cString & event_line);
  static bool ProcessEvent(const cString & name, const cString & args,
			   int event_handler_id = -1);
  static bool ProcessEvent(int event_enum, const cString & args,
			   int event_handler_id = -1);

  // Notify Functions
  static void NotifyComment(const cString & in_string);
  static void NotifyWarning(const cString & in_string);
  static void NotifyError(const cString & in_string);
  static void NotifyUpdate();
  static void NotifyBirth(int p_cell, int d_cell);
  static void NotifyBreakpoint();

  // Misc Functions
  static void SyncEventList();
  static void ExitProgram(int ignore);

  static int GetMainEventHandlerID(){ return main_eh_id; }
  static int GetPopulationEventHandlerID(){ return population_eh_id; }

  // Viewers
  static void StartViewer(const cString & viewer_name);


  // Functions that shouldn't have to be here.... @TCC
  static cGenotype * GetBestGenotype();
  // Stuff For Viewer to Access
  static cGenebank * GetGenebank();
  static cGenotype * GetCellGenotype(int cpu_id);
  static const cBaseCPU * GetCellCPU(int cpu_id);

};

#endif
