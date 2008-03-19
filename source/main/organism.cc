//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "../cpu/hardware_method.hh"
#include "organism.hh"

#include "../tools/functions.hh"
#include "../tools/tArray.hh"

#include "config.hh"
#include "inst_set.hh"
#include "inst_util.hh"
#include "genome.hh"
#include "genome_util.hh"
#include "org_message.hh"
#include "../main/inject_genotype.hh"

#include "../cpu/cpu_defs.hh"
#include "../cpu/hardware_base.hh"

#include <iomanip>


using namespace std;


///////////////
//  cOrganism
///////////////

int cOrganism::instance_count(0);


cOrganism::cOrganism(const cGenome & in_genome,
		     const cPopulationInterface & in_interface,
		     const cEnvironment & in_environment)
  : genotype(NULL)
  , phenotype(in_environment)
  , initial_genome(in_genome)
  , mut_info(in_environment.GetMutationLib(), in_genome.GetSize())
  , pop_interface(in_interface)
  , max_executed(-1)
  , lineage_label(-1)
  , lineage(NULL)
    , inbox(0)
    , sent(0)
{
  // Initialization of structures...
  hardware = pop_interface.NewHardware(this);
  cpu_stats.Setup(hardware->GetNumInst());
  instance_count++;
  pop_interface.SetCellID(-1);  // No cell at the moment...

  if (cConfig::GetDeathMethod() > 0) {
    max_executed = cConfig::GetAgeLimit();
    if (cConfig::GetAgeDeviation() > 0.0) {
      max_executed +=
	(int) (g_random.GetRandNormal() * cConfig::GetAgeDeviation());
    }
    if (cConfig::GetDeathMethod() == DEATH_METHOD_MULTIPLE) {
      max_executed *= initial_genome.GetSize();
    }

    // max_executed must be positive or an organism will not die!
    if (max_executed < 1) max_executed = 1;
  }
}


cOrganism::~cOrganism()
{
  pop_interface.RecycleHardware(hardware);
  instance_count--;
}


double cOrganism::GetTestFitness()
{
  return pop_interface.TestFitness();
}
  
void cOrganism::DoInput(const int value, tBuffer<int> & in_buf,
			tBuffer<int> & out_buf)
{
  in_buf.Add(value);
  phenotype.TestInput(in_buf, out_buf);
}

void cOrganism::DoOutput(const int value, tBuffer<int> & in_buf,
			 tBuffer<int> & out_buf)
{
  const tArray<double> & resource_count = pop_interface.GetResources();
  tArray<double> res_change(resource_count.GetSize());

  out_buf.Add(value);
  phenotype.TestOutput(in_buf, out_buf, resource_count, res_change);
  pop_interface.UpdateResources(res_change);
  //if(phenotype.GetToDie() == 1) Die();	//Final Effect Lethal Reaction
}

void cOrganism::SendMessage(cOrgMessage & mess)
{
  if(pop_interface.SendMessage(mess))
    sent.Add(mess);
  else
    {
      //perhaps some kind of message error buffer?
    }
}

bool cOrganism::ReceiveMessage(cOrgMessage & mess)
{
  inbox.Add(mess);
  return true;
}

bool cOrganism::InjectParasite(const cGenome & injected_code)
{
  return pop_interface.InjectParasite(this, injected_code);
}

bool cOrganism::InjectHost(const cCodeLabel & label, const cGenome & injected_code)
{
  return hardware->InjectHost(label, injected_code);
}

void cOrganism::AddParasite(cInjectGenotype * in_genotype)
{
  parasites.push_back(in_genotype);
}

cInjectGenotype & cOrganism::GetParasite(int x)
{
  return *parasites[x];
}

int cOrganism::GetNumParasites()
{
  return parasites.size();
}

void cOrganism::ClearParasites()
{
  parasites.clear();
}

int cOrganism::OK()
{
  if (!hardware->OK()) return false;
  if (!phenotype.OK()) return false;

  return true;
}


double cOrganism::CalcMeritRatio()
{
  const double age = (double) phenotype.GetAge();
  const double merit = phenotype.GetMerit().GetDouble();
  return (merit > 0.0) ? (age / merit ) : age;
}


bool cOrganism::GetTestOnDivide() const { return pop_interface.TestOnDivide();}
bool cOrganism::GetFailImplicit() const { return cConfig::GetFailImplicit(); }

bool cOrganism::GetRevertFatal() const { return cConfig::GetRevertFatal(); }
bool cOrganism::GetRevertNeg()   const { return cConfig::GetRevertNeg(); }
bool cOrganism::GetRevertNeut()  const { return cConfig::GetRevertNeut(); }
bool cOrganism::GetRevertPos()   const { return cConfig::GetRevertPos(); }

bool cOrganism::GetSterilizeFatal() const{return cConfig::GetSterilizeFatal();}
bool cOrganism::GetSterilizeNeg()  const { return cConfig::GetSterilizeNeg(); }
bool cOrganism::GetSterilizeNeut() const { return cConfig::GetSterilizeNeut();}
bool cOrganism::GetSterilizePos()  const { return cConfig::GetSterilizePos(); }


void cOrganism::PrintStatus(ostream & fp)
{
  hardware->PrintStatus(fp);
  phenotype.PrintStatus(fp);
}


bool cOrganism::Divide_CheckViable()
{
  // Make sure required task (if any) has been performed...
  const int required_task = cConfig::GetRequiredTask();
  if (required_task != -1 &&
      phenotype.GetCurTaskCount()[required_task] == 0) {
    Fault(FAULT_LOC_DIVIDE, FAULT_TYPE_ERROR,
	  cStringUtil::Stringf("Lacks required task (%d)",
			       cConfig::GetRequiredTask()));
    return false; //  (divide fails)
  }

  // Make sure the parent is fertile
  if ( phenotype.IsFertile() == false ) {
    Fault(FAULT_LOC_DIVIDE, FAULT_TYPE_ERROR, "Infertile organism");
    return false; //  (divide fails)
  }

  return true;  // Organism has no problem with divide...
}


// This gets called after a successful divide to deal with the child. 
// Returns true if parent lives through this process.

bool cOrganism::ActivateDivide()
{
  // Activate the child!  (Keep Last: may kill this organism!)
  return pop_interface.Divide(this, child_genome);
}


void cOrganism::Fault(int fault_loc, int fault_type, cString fault_desc)
{
  (void) fault_loc;
  (void) fault_type;
  (void) fault_desc;

#ifdef FATAL_ERRORS
  if (fault_type == FAULT_TYPE_ERROR) {
    phenotype.IsFertile() = false;
  }
#endif

#ifdef FATAL_WARNINGS
  if (fault_type == FAULT_TYPE_WARNING) {
    phenotype.IsFertile() = false;
  }
#endif

#ifdef BREAKPOINTS
  phenotype.SetFault(fault_desc);
#endif

  phenotype.IncErrors();
}


//// Save and Load ////
void cOrganism::SaveState(ofstream & fp)
{
  assert(fp.good());

  fp <<"cOrganism"<<endl;

  //// Save If it is alive ////
  if( genotype == NULL ){
    fp <<false<<endl;
  }else{
    fp <<true<<endl;

    //// Save Genotype Genome ////
//    fp << genotype->GetLength() << endl;
    
//      cInstUtil::SaveInternalGenome(fp, hardware->GetInstSet(),
//  				  genotype->GetGenome());

    //// Save Actual Creature Memory & MemFlags ////
//   fp <<hardware->GetMemory().GetSize()<<endl;
//   cInstUtil::PrintGenome(hardware->GetInstSet(), hardware->GetMemory(), fp);
//      fp <<"|"; // marker
//      for( int i=0; i<hardware->GetMemory().GetSize(); ++i ){
//        fp << hardware->GetMemory().GetFlags(i);
//      }
//      fp <<endl;

    //// Save Hardware (Inst_Pointer, Stacks, and the like)
    hardware->SaveState(fp);

    //// Save Phenotype  ////
    phenotype.SaveState(fp);

  } // if there is a genotype here (ie. not dead)
}


void cOrganism::LoadState(ifstream & fp)
{
  hardware->Reset();

  assert(fp.good());

  cString foo;
  fp >>foo;
  assert( foo == "cOrganism" );

  //// Is there a creature there ////
  bool alive_flag = 0;
  fp >>alive_flag;
  if( alive_flag ){

    //// Load Genotype ////
    cGenome in_code =
      cInstUtil::LoadInternalGenome(fp, hardware->GetInstSet());
//    cGenotype * new_genotype = environment->AddGenotype(in_code);
//    ChangeGenotype(new_genotype);

    //// Load Actual Creature Memory & MemFlags ////
//    {	
//      in_code = cInstUtil::LoadInternalGenome(fp, hardware->GetInstSet());
//      hardware->GetMemory() = in_code;
//      CA_FLAG_TYPE flags;
//      char marker;  fp >>marker;  assert( marker == '|' );
//      for( int i=0; i<hardware->GetMemory().GetSize(); ++i ){
//    	  fp.get(flags);
//  	  hardware->SetMemFlags(i, flags);
//      }
//    }

    //// Load Hardware (Inst_Pointer, Stacks, and the like)
    hardware->LoadState(fp);

    //// Load Phenotype  ////
    assert(fp.good());
//      phenotype.Clear(genotype->GetLength());
    phenotype.LoadState(fp);

    //// Adjust Time Slice ////
//    environment->AdjustTimeSlice();

  } // end if not a dead creature
}
