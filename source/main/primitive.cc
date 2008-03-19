//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "../main/avida.hh"

#include "../main/config.hh"
#include "../cpu/hardware_method.hh"
#include "../main/environment.hh"
#include "../main/population.hh"


using namespace std;


int main(int argc, char * argv[])
{
  cEnvironment environment;
  cPopulationInterface test_interface;
  SetupAvida( argc, argv, environment, test_interface );

  if (cConfig::GetAnalyzeMode() == true) {
    cAvidaDriver_Base::main_driver = new cAvidaDriver_Analyze();
  }
  else {
    cAvidaDriver_Base::main_driver = new cAvidaDriver_Population(environment);
  }

  cAvidaDriver_Base::main_driver->Run();

  // Exit Nicely
  ExitAvida(0);
}




