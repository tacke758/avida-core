/*
 *  cOrganism.h
 *  Avida
 *
 *  Called "organism.hh" prior to 12/5/05.
 *  Copyright 2005-2006 Michigan State University. All rights reserved.
 *  Copyright 1993-2003 California Institute of Technology.
 *
 */

#ifndef cOrganism_h
#define cOrganism_h

#include <fstream>
#include <map>
#include <utility>
#include <set>
#include <string>
#include <vector>
//#include <boost/config.hpp>
//#include <boost/graph/edge_list.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
//#include <boost/graph/filtered_graph.hpp>
using namespace boost;



#ifndef cCPUMemory_h
#include "cCPUMemory.h"
#endif
#ifndef sCPUStats_h
#include "sCPUStats.h"
#endif
#ifndef cGenome_h
#include "cGenome.h"
#endif
#ifndef cLocalMutations_h
#include "cLocalMutations.h"
#endif
#ifndef cMutationRates_h
#include "cMutationRates.h"
#endif
#ifndef cPhenotype_h
#include "cPhenotype.h"
#endif
#ifndef cOrgInterface_h
#include "cOrgInterface.h"
#endif
#ifndef cOrgSeqMessage_h
#include "cOrgSeqMessage.h"
#endif
#ifndef cOrgSourceMessage_h
#include "cOrgSourceMessage.h"
#endif
#ifndef tArray_h
#include "tArray.h"
#endif
#ifndef tBuffer_h
#include "tBuffer.h"
#endif
#ifndef tList_h
#include "tList.h"
#endif
#ifndef tSmartArray_h
#include "tSmartArray.h"
#endif
#ifndef cSaleItem_h
#include "cSaleItem.h"
#endif

/**
 * The cOrganism class controls the running and manages all the statistics
 * about a creature.  Effectively the chemistry acting on the genome.
 **/

class cAvidaContext;
class cCodeLabel;
class cHardwareBase;
class cGenotype;
class cInjectGenotype;
class cLineage;
class cOrgMessage;
class cOrgSinkMessage;
class cEnvironment;
class cCodeLabel;

// structures used to label vertices and edges 
struct edge_info {
        int edge_label;
		std::string edge_info;
};

struct state_info {
        int state_label;
};


class cOrganism
{
public:

//!Types of the UML types for states and transitions
// define graph -- multisetS - allows for the sorting of out edges by target vertex
typedef adjacency_list<multisetS, vecS, directedS, state_info, edge_info> Graph;
// map the graph's vertex's to their descriptors
typedef graph_traits<Graph>::vertex_descriptor State;
typedef std::map<int, State> NameStateMap;
typedef NameStateMap::iterator nsm_it; 
// A map from the integer representing a transition to the string representing its label
typedef std::map<int, std::string> TransMeaning;
// map the graph's edge's to their descriptors
typedef graph_traits<Graph>::edge_descriptor Transition;
typedef graph_traits<Graph>::out_edge_iterator oei;



	
protected:
  cWorld* m_world;
  cHardwareBase* m_hardware;            // The actual machinary running this organism.
  cGenotype* genotype;                  // Information about organisms with this genome.
  cPhenotype phenotype;                 // Descriptive attributes of organism.
  const cGenome initial_genome;         // Initial genome; can never be changed!
  tArray<cInjectGenotype*> m_parasites; // List of all parasites associated with this organism.
  cMutationRates mut_rates;             // Rate of all possible mutations.
  cLocalMutations mut_info;             // Info about possible mutations;
  cOrgInterface* m_interface;           // Interface back to the population.
  int m_id;								// unique id for each org, is just the number it was born

  // Input and Output with the environment
  int input_pointer;
  tBuffer<int> input_buf;
  tBuffer<int> output_buf;
  tBuffer<int> send_buf;
  tBuffer<int> receive_buf;
  tBuffer<int> received_messages;
  tList<tListNode<cSaleItem> > sold_items;

  // Communication
  int sent_value;         // What number is this org sending?
  bool sent_active;       // Has this org sent a number?
  int test_receive_pos;   // In a test CPU, what message to receive next?

  // Other stats
  cCPUMemory child_genome; // Child genome, while under construction.
  sCPUStats cpu_stats;     // Info for statistics

  int max_executed;      // Max number of instruction executed before death.

  int lineage_label;     // a lineages tag; inherited unchanged in offspring
  cLineage* lineage;    // A lineage descriptor... (different from label)

  tBuffer<cOrgMessage> inbox;
  tBuffer<cOrgMessage> sent;
  
  // UML internal state diagram components
  Graph uml_state_diagram;		// the overall graph of the UML state diagram
  Transition transitions;		// map of transition descriptors to transitions
  NameStateMap states;			// map of the state names 
  TransMeaning transGuardActionInfo; // map of transition integers to the string representing their label
  std::string hil_begin;
  std::string hil;
  std::string hil_end;
  std::map<int, int>  PosToStateLabel;  // a map that relates the number in which the state was inserted
										// to the label the organism assigns it.

  
  class cNetSupport
  {
  public:
    tList<cOrgSinkMessage> pending;
    tSmartArray<cOrgSinkMessage*> received;
    tSmartArray<cOrgSourceMessage> sent;
    tSmartArray<cOrgSeqMessage> seq; 
    int last_seq;
    
    cNetSupport() : last_seq(0) { ; }
    ~cNetSupport();
  };
  cNetSupport* m_net;

#ifdef DEBUG
  bool initialized;      // Has this CPU been initialized yet, w/hardware.
#endif
  bool is_running;       // Does this organism have the CPU?


  cOrganism(); // @not_implemented
  cOrganism(const cOrganism&); // @not_implemented
  cOrganism& operator=(const cOrganism&); // @not_implemented
  
public:
  cOrganism(cWorld* world, cAvidaContext& ctx, const cGenome& in_genome);
  ~cOrganism();

  cHardwareBase& GetHardware() { return *m_hardware; }
  cOrganism* GetNeighbor() { assert(m_interface); return m_interface->GetNeighbor(); }
  int GetNeighborhoodSize() { assert(m_interface); return m_interface->GetNumNeighbors(); }
  void Rotate(int direction) { assert(m_interface); m_interface->Rotate(direction); }
  void DoBreakpoint() { assert(m_interface); m_interface->Breakpoint(); }
  int GetNextInput() { assert(m_interface); return m_interface->GetInputAt(input_pointer); }
  void Die() { assert(m_interface); m_interface->Die(); }
  void Kaboom(int dist) { assert(m_interface); m_interface->Kaboom(dist);}
  int GetCellID() { assert(m_interface); return m_interface->GetCellID(); }
  int GetDebugInfo() { assert(m_interface); return m_interface->Debug(); }
  int GetID() { return m_id; }
  bool GetSentActive() { return sent_active; }
  void SendValue(int value) { sent_active = true; sent_value = value; }
  int RetrieveSentValue() { sent_active = false; return sent_value; }
  int ReceiveValue();
  void SellValue(const int data, const int label, const int sell_price);
  int BuyValue(const int label, const int buy_price);
  tListNode<tListNode<cSaleItem> >* AddSoldItem(tListNode<cSaleItem>* );
  tList<tListNode<cSaleItem> >* GetSoldItems() { return &sold_items; }
  void UpdateMerit(double new_merit) { assert(m_interface); m_interface->UpdateMerit(new_merit); }
  
  // Input & Output Testing
  void DoInput(const int value);
  void DoOutput(cAvidaContext& ctx, const int value);

  // Message stuff
  void SendMessage(cOrgMessage & mess);
  bool ReceiveMessage(cOrgMessage & mess);
  
  // Network Stuff
  void NetGet(cAvidaContext& ctx, int& value, int& seq);
  void NetSend(cAvidaContext& ctx, int value);
  cOrgSinkMessage* NetPop() { return m_net->pending.PopRear(); }
  bool NetReceive(int& value);
  bool NetValidate(cAvidaContext& ctx, int value);
  bool NetRemoteValidate(cAvidaContext& ctx, int value);
  int NetLast() { return m_net->last_seq; }
  void NetReset();
  
  // UML Stuff
//  t_stateMap uml_states;
//  t_transitionMap uml_transitions;
//  t_transitionMap uml_trans_by_state;
//  std::set <int> uml_state_set;
//  std::map <int, std::string> uml_trans_set;
  void ModelCheck(cAvidaContext& ctx);
//  void printXMI(cAvidaContext& ctx);
  void printHIL(cAvidaContext& ctx);
//  std::vector<std::string> trans_info;
//  void InitTransForXMI();
  void InitTransForHIL();
  void InitHILBandE();
  bool AddTrans(int trans, int orig, int dest);
  double NumStates();
  double NumTrans();
  void AssignTransMeaning(int trans);
  Graph& GetGraph();
  State& getStateInPosition (int num);
  bool isTrans(State, State, int);
  int getTransNumber (int pos);
  bool findTrans(int s0_pos, int s1_pos, int t_pos);
  
  // This returns the list of transitions between two states. What I want is to look up one based
  // on its placement in the trans_set, but this has already been solved by HIL printing. Check there after dinner.
 // stl::set<int> getTransBetweenVertices(State, State);
  std::string StringifyAnInt (int x);
  std::string getHil();

  

//  NameStateMap::iterator pos;




  bool InjectParasite(const cGenome& genome);
  bool InjectHost(const cCodeLabel& in_label, const cGenome& genome);
  void AddParasite(cInjectGenotype* cur) { m_parasites.Push(cur); }
  cInjectGenotype& GetParasite(int x) { return *m_parasites[x]; }
  int GetNumParasites() { return m_parasites.GetSize(); }
  void ClearParasites() { m_parasites.Resize(0); }
		      
  int OK();

  double GetTestFitness(cAvidaContext& ctx);
  double CalcMeritRatio();

  cCPUMemory& ChildGenome() { return child_genome; }
  sCPUStats& CPUStats() { return cpu_stats; }

  bool TestCopyMut(cAvidaContext& ctx) const { return MutationRates().TestCopyMut(ctx); }
  bool TestDivideMut(cAvidaContext& ctx) const { return MutationRates().TestDivideMut(ctx); }
  bool TestDivideIns(cAvidaContext& ctx) const { return MutationRates().TestDivideIns(ctx); }
  bool TestDivideDel(cAvidaContext& ctx) const { return MutationRates().TestDivideDel(ctx); }
  bool TestParentMut(cAvidaContext& ctx) const { return MutationRates().TestParentMut(ctx); }
  
  double GetCopyMutProb() const { return MutationRates().GetCopyMutProb(); }
  void SetCopyMutProb(double _p) { return MutationRates().SetCopyMutProb(_p); }
  void SetDivMutProb(double _p) { return MutationRates().SetDivMutProb(_p); }

  double GetInsMutProb() const { return MutationRates().GetInsMutProb(); }
  double GetDelMutProb() const { return MutationRates().GetDelMutProb(); }
  double GetDivMutProb() const { return MutationRates().GetDivMutProb(); }
  double GetParentMutProb() const { return MutationRates().GetParentMutProb();}

  double GetInjectInsProb() const { return MutationRates().GetInjectInsProb(); }
  double GetInjectDelProb() const { return MutationRates().GetInjectDelProb(); }
  double GetInjectMutProb() const { return MutationRates().GetInjectMutProb(); }
  

  bool GetTestOnDivide() const;
  bool GetFailImplicit() const;

  bool GetRevertFatal() const;
  bool GetRevertNeg() const;
  bool GetRevertNeut() const;
  bool GetRevertPos() const;

  bool GetSterilizeFatal() const;
  bool GetSterilizeNeg() const;
  bool GetSterilizeNeut() const;
  bool GetSterilizePos() const;
  double GetNeutralMin() const;
  double GetNeutralMax() const;

  // Access to private variables
  int GetMaxExecuted() const { return max_executed; }

  void SetLineageLabel( int in_label ) { lineage_label = in_label; }
  int GetLineageLabel() const { return lineage_label; }
  
  void SetLineage( cLineage * in_lineage ) { lineage = in_lineage; }
  cLineage * GetLineage() const { return lineage; }

  void SetGenotype(cGenotype * in_genotype) { genotype = in_genotype; }
  cGenotype * GetGenotype() const { return genotype; }

  const cMutationRates& MutationRates() const { return mut_rates; }
  cMutationRates& MutationRates() { return mut_rates; }
  const cLocalMutations& GetLocalMutations() const { return mut_info; }
  cLocalMutations& GetLocalMutations() { return mut_info; }
  
  const cOrgInterface& GetOrgInterface() const { assert(m_interface); return *m_interface; }
  cOrgInterface& GetOrgInterface() { assert(m_interface); return *m_interface; }
  void SetOrgInterface(cOrgInterface* interface);
  
  const cGenome& GetGenome() const { return initial_genome; }
  
  /*
  int GetCurGestation() const;
  */
  const cPhenotype & GetPhenotype() const { return phenotype; }
  cPhenotype & GetPhenotype() { return phenotype; }

  // --------  DEBUG ---------
  void SetRunning(bool in_running) { is_running = in_running; }
  bool GetIsRunning() { return is_running; }


  void PrintStatus(std::ostream& fp, const cString & next_name);
  
  // Divide functions
  bool Divide_CheckViable();
  bool ActivateDivide(cAvidaContext& ctx);
  
  // Other Special Functions
  void Fault(int fault_loc, int fault_type, cString fault_desc="");
};


#ifdef ENABLE_UNIT_TESTS
namespace nOrganism {
  /**
   * Run unit tests
   *
   * @param full Run full test suite; if false, just the fast tests.
   **/
  void UnitTests(bool full = false);
}
#endif  

#endif

