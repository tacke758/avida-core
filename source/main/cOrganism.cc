/*
 *  cOrganism.cc
 *  Avida
 *
 *  Called "organism.cc" prior to 12/5/05.
 *  Copyright 2005-2006 Michigan State University. All rights reserved.
 *  Copyright 1993-2003 California Institute of Technology.
 *
 */

#include "cOrganism.h"

#include "cAvidaContext.h"
#include "nHardware.h"
#include "cEnvironment.h"
#include "functions.h"
#include "cGenome.h"
#include "cGenomeUtil.h"
#include "cGenotype.h"
#include "cHardwareBase.h"
#include "cHardwareManager.h"
#include "cInjectGenotype.h"
#include "cInstSet.h"
#include "cInstUtil.h"
#include "cOrgMessage.h"
#include "cOrgSinkMessage.h"
#include "cStringUtil.h"
#include "cTaskContext.h"
#include "cTools.h"
#include "cWorld.h"
#include "cStats.h"



#include <iomanip>
#include <fstream>

using namespace std;

std::string loadFile(const char* filename) {
	std::string data, line; // or maybe stringstream? (strstream?)
	std::ifstream infile;
	infile.open(filename);
	assert(infile.is_open());
	
	while (getline (infile, line))
	{
		data.append(line);
		line.erase();
	}
	
	//read from file; load into string/strstream, and return it.
	return data;
}

std::string cOrganism::xmi_begin = loadFile("xmi_begin");
std::string cOrganism::xmi_end = loadFile("xmi_end");


cOrganism::cOrganism(cWorld* world, cAvidaContext& ctx, const cGenome& in_genome)
  : m_world(world)
  , genotype(NULL)
  , phenotype(world)
  , initial_genome(in_genome)
  , mut_info(world->GetEnvironment().GetMutationLib(), in_genome.GetSize())
  , m_interface(NULL)
  , input_pointer(0)
  , input_buf(INPUT_BUF_SIZE)
  , output_buf(OUTPUT_BUF_SIZE)
  , send_buf(SEND_BUF_SIZE)
  , receive_buf(RECEIVE_BUF_SIZE)
  , sent_value(0)
  , sent_active(false)
  , test_receive_pos(0)
  , max_executed(-1)
  , lineage_label(-1)
  , lineage(NULL)
  , inbox(0)
  , sent(0)
  , m_net(NULL)
  , received_messages(RECEIVED_MESSAGES_SIZE)
  , is_running(false)
{
  // Initialization of structures...
  m_hardware = m_world->GetHardwareManager().Create(this);
  cpu_stats.Setup();

  if (m_world->GetConfig().DEATH_METHOD.Get() > DEATH_METHOD_OFF) {
    max_executed = m_world->GetConfig().AGE_LIMIT.Get();
    if (m_world->GetConfig().AGE_DEVIATION.Get() > 0.0) {
      max_executed += (int) (ctx.GetRandom().GetRandNormal() * m_world->GetConfig().AGE_DEVIATION.Get());
    }
    if (m_world->GetConfig().DEATH_METHOD.Get() == DEATH_METHOD_MULTIPLE) {
      max_executed *= initial_genome.GetSize();
    }

    // max_executed must be positive or an organism will not die!
    if (max_executed < 1) max_executed = 1;
  }
  
  if (m_world->GetConfig().NET_ENABLED.Get()) m_net = new cNetSupport();
  m_id = m_world->GetStats().GetTotCreatures();
  
	
}


cOrganism::~cOrganism()
{
  assert(is_running == false);
  delete m_hardware;
  delete m_interface;
  if (m_net != NULL) delete m_net;
}

cOrganism::cNetSupport::~cNetSupport()
{
  while (pending.GetSize()) delete pending.Pop();
  for (int i = 0; i < received.GetSize(); i++) delete received[i];
}

void cOrganism::SetOrgInterface(cOrgInterface* interface)
{
  delete m_interface;
  m_interface = interface;
}


double cOrganism::GetTestFitness(cAvidaContext& ctx)
{
  assert(m_interface);
  return genotype->GetTestFitness(ctx);
}
  
int cOrganism::ReceiveValue()
{
  assert(m_interface);
  const int out_value = m_interface->ReceiveValue();
  receive_buf.Add(out_value);
  return out_value;
}

void cOrganism::SellValue(const int data, const int label, const int sell_price)
{
	if (sold_items.GetSize() < 10)
	{
		assert (m_interface);
		m_interface->SellValue(data, label, sell_price, m_id);
		m_world->GetStats().AddMarketItemSold();
	}
}

int cOrganism::BuyValue(const int label, const int buy_price)
{
	assert (m_interface);
	const int receive_value = m_interface->BuyValue(label, buy_price);
	if (receive_value != 0)
	{
		// put this value in storage place for recieved values
		received_messages.Add(receive_value);
		// update loss of buy_price to merit
		double cur_merit = GetPhenotype().GetMerit().GetDouble();
		cur_merit -= buy_price;
		UpdateMerit(cur_merit);
		m_world->GetStats().AddMarketItemBought();
	}
	return receive_value;
}

tListNode<tListNode<cSaleItem> >* cOrganism::AddSoldItem(tListNode<cSaleItem>* sold_node)
{
	tListNode<tListNode<cSaleItem> >* node_pt = sold_items.PushRear(sold_node);
	return node_pt;
}

void cOrganism::DoInput(const int value)
{
  input_buf.Add(value);
  phenotype.TestInput(input_buf, output_buf);
}

void cOrganism::DoOutput(cAvidaContext& ctx, const int value)
{
  assert(m_interface);
  const tArray<double> & resource_count = m_interface->GetResources();

  tList<tBuffer<int> > other_input_list;
  tList<tBuffer<int> > other_output_list;

  // If tasks require us to consider neighbor inputs, collect them...
  if (m_world->GetEnvironment().GetTaskLib().UseNeighborInput() == true) {
    const int num_neighbors = m_interface->GetNumNeighbors();
    for (int i = 0; i < num_neighbors; i++) {
      m_interface->Rotate();
      cOrganism * cur_neighbor = m_interface->GetNeighbor();
      if (cur_neighbor == NULL) continue;

      other_input_list.Push( &(cur_neighbor->input_buf) );
    }
  }

  // If tasks require us to consider neighbor outputs, collect them...
  if (m_world->GetEnvironment().GetTaskLib().UseNeighborOutput() == true) {
    const int num_neighbors = m_interface->GetNumNeighbors();
    for (int i = 0; i < num_neighbors; i++) {
      m_interface->Rotate();
      cOrganism * cur_neighbor = m_interface->GetNeighbor();
      if (cur_neighbor == NULL) continue;

      other_output_list.Push( &(cur_neighbor->output_buf) );
    }
  }
  
  bool net_valid = false;
  if (m_net) net_valid = NetValidate(ctx, value);

  // Do the testing of tasks performed...
  output_buf.Add(value);
  tArray<double> res_change(resource_count.GetSize());
  tArray<int> insts_triggered;

  tBuffer<int>* received_messages_point = &received_messages;
  if (!m_world->GetConfig().SAVE_RECEIVED.Get())
	  received_messages_point = NULL;
  cTaskContext taskctx(input_buf, output_buf, other_input_list, other_output_list, net_valid, 0, received_messages_point);
  phenotype.TestOutput(ctx, taskctx, send_buf, receive_buf, resource_count, res_change, insts_triggered);
  m_interface->UpdateResources(res_change);

  for (int i = 0; i < insts_triggered.GetSize(); i++) {
    const int cur_inst = insts_triggered[i];
    m_hardware->ProcessBonusInst(ctx, cInstruction(cur_inst) );
  }
}

void cOrganism::SendMessage(cOrgMessage & mess)
{
  assert(m_interface);
  if(m_interface->SendMessage(mess))
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


void cOrganism::NetGet(cAvidaContext& ctx, int& value, int& seq)
{
  assert(m_net);
  seq = m_net->seq.GetSize();
  m_net->seq.Resize(seq + 1);
  value = ctx.GetRandom().GetUInt(1 << 16);
  m_net->seq[seq].SetValue(value);
}

void cOrganism::NetSend(cAvidaContext& ctx, int value)
{
  assert(m_net);
  int index = -1;
  
  // Search for previously sent value
  for (int i = m_net->sent.GetSize() - 1; i >= 0; i--) {
    if (m_net->sent[i].GetValue() == value) {
      index = i;
      m_net->sent[i].SetSent();
      break;
    }
  }
  
  // If not found, add new message
  if (index == -1) {
    index = m_net->sent.GetSize();
    m_net->sent.Resize(index + 1);
    m_net->sent[index] = cOrgSourceMessage(value);
  }
  
  // Test if this message will be dropped
  const double drop_prob = m_world->GetConfig().NET_DROP_PROB.Get();
  if (drop_prob > 0.0 && ctx.GetRandom().P(drop_prob)) {
    m_net->sent[index].SetDropped();
    return;
  }
  
  // Test if this message will be corrupted
  int actual_value = value;
  const double mut_prob = m_world->GetConfig().NET_MUT_PROB.Get();
  if (mut_prob > 0.0 && ctx.GetRandom().P(mut_prob)) {
    switch (m_world->GetConfig().NET_MUT_TYPE.Get())
    {
      case 0: // Flip a single random bit
        actual_value ^= 1 << ctx.GetRandom().GetUInt(31);
        m_net->sent[index].SetCorrupted();
        break;
      case 1: // Flip the last bit
        actual_value ^= 1;
        m_net->sent[index].SetCorrupted();
        break;
      default:
        // invalid selection, no action
        break;
    }
  }
  
  assert(m_interface);
  cOrgSinkMessage* msg = new cOrgSinkMessage(m_interface->GetCellID(), value, actual_value);
  m_net->pending.Push(msg);
}

bool cOrganism::NetReceive(int& value)
{
  assert(m_net && m_interface);
  cOrgSinkMessage* msg = m_interface->NetReceive();
  if (msg == NULL) {
    value = 0;
    return false;
  }
  
  m_net->received.Push(msg);
  value = msg->GetActualValue();
  return true;
}

bool cOrganism::NetValidate(cAvidaContext& ctx, int value)
{
  assert(m_net);
  
  if (0xFFFF0000 & value) return false;
  
  for (int i = 0; i < m_net->received.GetSize(); i++) {
    cOrgSinkMessage* msg = m_net->received[i];
    if (!msg->GetValidated() && (msg->GetOriginalValue() & 0xFFFF) == value) {
      msg->SetValidated();
      assert(m_interface);
      return m_interface->NetRemoteValidate(ctx, msg);
    }
  }
    
  return false;
}

bool cOrganism::NetRemoteValidate(cAvidaContext& ctx, int value)
{
  assert(m_net);

  bool found = false;
  for (int i = m_net->last_seq; i < m_net->seq.GetSize(); i++) {
    cOrgSeqMessage& msg = m_net->seq[i];
    if (msg.GetValue() == value && !msg.GetReceived()) {
      m_net->seq[i].SetReceived();
      found = true;
      break;
    }
  }
  if (!found) return false;

  int completed = 0;
  while (m_net->last_seq < m_net->seq.GetSize() && m_net->seq[m_net->last_seq].GetReceived()) {
    completed++;
    m_net->last_seq++;
  }
  
  if (completed) {
    assert(m_interface);
    const tArray<double>& resource_count = m_interface->GetResources();
    
    tList<tBuffer<int> > other_input_list;
    tList<tBuffer<int> > other_output_list;
    
    // If tasks require us to consider neighbor inputs, collect them...
    if (m_world->GetEnvironment().GetTaskLib().UseNeighborInput() == true) {
      const int num_neighbors = m_interface->GetNumNeighbors();
      for (int i = 0; i < num_neighbors; i++) {
        m_interface->Rotate();
        cOrganism * cur_neighbor = m_interface->GetNeighbor();
        if (cur_neighbor == NULL) continue;
        
        other_input_list.Push( &(cur_neighbor->input_buf) );
      }
    }
    
    // If tasks require us to consider neighbor outputs, collect them...
    if (m_world->GetEnvironment().GetTaskLib().UseNeighborOutput() == true) {
      const int num_neighbors = m_interface->GetNumNeighbors();
      for (int i = 0; i < num_neighbors; i++) {
        m_interface->Rotate();
        cOrganism * cur_neighbor = m_interface->GetNeighbor();
        if (cur_neighbor == NULL) continue;
        
        other_output_list.Push( &(cur_neighbor->output_buf) );
      }
    }
        
    // Do the testing of tasks performed...
    output_buf.Add(value);
    tArray<double> res_change(resource_count.GetSize());
    tArray<int> insts_triggered;
    cTaskContext taskctx(input_buf, output_buf, other_input_list, other_output_list, false, completed);
    phenotype.TestOutput(ctx, taskctx, send_buf, receive_buf, resource_count, res_change, insts_triggered);
    m_interface->UpdateResources(res_change);
    
    for (int i = 0; i < insts_triggered.GetSize(); i++) {
      const int cur_inst = insts_triggered[i];
      m_hardware->ProcessBonusInst(ctx, cInstruction(cur_inst) );
    }
  }
  
  return true;
}

void cOrganism::NetReset()
{
  if (m_net) {
    while (m_net->pending.GetSize()) delete m_net->pending.Pop();
    for (int i = 0; i < m_net->received.GetSize(); i++) delete m_net->received[i];
    m_net->received.Resize(0);
    m_net->sent.Resize(0);
    m_net->seq.Resize(0);
  }
}

/// UML Functions /// 
/// This function is a copy of DoOutput /// 
void cOrganism::ModelCheck(cAvidaContext& ctx)
{

	// CHECK OUT WHETHER THE IDEAL WORKS....
	printXMI(ctx);
	//printIdealXMI(ctx);
	

  assert(m_interface);
  const tArray<double> & resource_count = m_interface->GetResources();

  tList<tBuffer<int> > other_input_list;
  tList<tBuffer<int> > other_output_list;

  // If tasks require us to consider neighbor inputs, collect them...
  if (m_world->GetEnvironment().GetTaskLib().UseNeighborInput() == true) {
    const int num_neighbors = m_interface->GetNumNeighbors();
    for (int i = 0; i < num_neighbors; i++) {
      m_interface->Rotate();
      cOrganism * cur_neighbor = m_interface->GetNeighbor();
      if (cur_neighbor == NULL) continue;

      other_input_list.Push( &(cur_neighbor->input_buf) );
    }
  }

  // If tasks require us to consider neighbor outputs, collect them...
  if (m_world->GetEnvironment().GetTaskLib().UseNeighborOutput() == true) {
    const int num_neighbors = m_interface->GetNumNeighbors();
    for (int i = 0; i < num_neighbors; i++) {
      m_interface->Rotate();
      cOrganism * cur_neighbor = m_interface->GetNeighbor();
      if (cur_neighbor == NULL) continue;

      other_output_list.Push( &(cur_neighbor->output_buf) );
    }
  }
 
  
//  bool net_valid = false;
//  if (m_net) net_valid = NetValidate(ctx, value);

  // Do the testing of tasks performed...
 // output_buf.Add(value);
  tArray<double> res_change(resource_count.GetSize());
  tArray<int> insts_triggered;

  tBuffer<int>* received_messages_point = &received_messages;
  if (!m_world->GetConfig().SAVE_RECEIVED.Get())
	  received_messages_point = NULL; 
// change task context eventually	  
  cTaskContext taskctx(input_buf, output_buf, other_input_list, other_output_list, false, 0, received_messages_point, this);
  phenotype.TestOutput(ctx, taskctx, send_buf, receive_buf, resource_count, res_change, insts_triggered);
  m_interface->UpdateResources(res_change);

  for (int i = 0; i < insts_triggered.GetSize(); i++) {
    const int cur_inst = insts_triggered[i];
    m_hardware->ProcessBonusInst(ctx, cInstruction(cur_inst) );
  }
  
  m_world->GetStats().UpdateModelStats(uml_state_diagram);
  
}


// This sets the meaning of the transitions. The only question is whether some of the operation, 
// and action ids that have been changed to "" are needed.

void cOrganism::InitTransForXMI()
{

	// assign transition values to map elements
	std::map <int, std::string>::iterator it;
	std::string temp;
	int count = 0;

	for (it = transGuardActionInfo.begin(); it!=transGuardActionInfo.end(); ++it) { 
	switch (count){
		case 0:
			(*it).second = "";
			break;
		case 1:
			temp = "";
			temp += "<UML:Transition.effect> <UML:UninterpretedAction xmi.id=\"XDE-176F1237-1448-4226-A095-075FABD68B33\"";
			temp += " isAsynchronous=\"false\" name=\"\" isSpecification=\"false\">";
			temp += "<UML:Action.script> <UML:ActionExpression language=\"\" "; 
			temp += "body=\"^TempSensor.getTempData()\"/>  </UML:Action.script> ";
			temp += "</UML:UninterpretedAction> </UML:Transition.effect> \n";
			(*it).second = temp;
			break;
		case 2:
			temp = "";
			temp += "<UML:Transition.trigger> <UML:Event> <UML:ModelElement.namespace> ";
            temp += "<UML:Namespace> <UML:Namespace.ownedElement> ";
			temp += "<UML:CallEvent xmi.id=\"XDE-4C4256DD-D7D7-4687-AA73-761334859279\" " ;
			temp += " operation=\"XDE-9517D6BA-8666-4A82-AFEA-62D60FE37B07\" name=\"setTempData\" ";
			temp += " isSpecification=\"false\"/> </UML:Namespace.ownedElement> </UML:Namespace> ";
			temp += " </UML:ModelElement.namespace> </UML:Event> </UML:Transition.trigger>\n";
			(*it).second = temp;
			break;
		case 3: 
			(*it).second = "";
			break;
		case 4: 
			temp = "";
			temp += "<UML:Transition.effect>  <UML:UninterpretedAction xmi.id=\"XDE-8280CF2B-DA14-4989-AC7F-D83012DE3234\"";
			temp += " isAsynchronous=\"false\" name=\"\" isSpecification=\"false\"> ";
			temp += "<UML:Action.script> <UML:ActionExpression language=\"\" ";
			temp += " body=\"^TempSensor.getOpState()\"/>  </UML:Action.script> ";
			temp += " </UML:UninterpretedAction> </UML:Transition.effect>\n";
			(*it).second =temp;
			break;
		case 5:
			temp = "";
			temp += "<UML:Transition.trigger> <UML:Event> <UML:ModelElement.namespace>";
			temp += "<UML:Namespace> <UML:Namespace.ownedElement> ";
			temp += "<UML:CallEvent xmi.id=\"XDE-C2891D3C-A49E-4DF0-BD95-A291630F4E4B\" ";
			temp += " operation=\"XDE-4437EBF1-9C42-4EB4-B7CF-415697B567CD\" name=\"setTempOpState\"";
			temp += " isSpecification=\"false\"/> </UML:Namespace.ownedElement> </UML:Namespace>";
			temp += " </UML:ModelElement.namespace> </UML:Event>  </UML:Transition.trigger>\n";
			(*it).second = temp;
			break;	
/*		case 6:
			 (*it).second = "setBrightnessValue(brightnessValue)";
			break;
		case 7:
			(*it).second = "[brightnessValue<0]/correctedBrightnessValue:=0";
			break;
		case 8:
			(*it).second = "[brightnessValue >=0 & brightnessValue<=1000]/correctedBrightnessValue:=brightnessValue";
			break;
		case 9:
			(*it).second = "[brightnessValue>1000]/correctedBrightnessValue:=1000";
			break;
		case 10:
			(*it).second = "^ComputingComponent.setBrightnessValue(brightnessValue)";
			break;*/
		default:
			(*it).second = " ";
	}
		count++;
	}	
}	
	
	


bool cOrganism::AddTrans(int trans, int orig, int dest) 
{
	NameStateMap::iterator pos1, pos2;
	bool inserted1, inserted2, inserted3;
	bool exists = 0;
	State u, v;


	// create or find origin state
	tie(pos1, inserted1) = states.insert(std::make_pair(orig, State()));
        if (inserted1) {
                u = add_vertex(uml_state_diagram);
                //cout << "added state named " << orig << endl;
                uml_state_diagram[u].state_label = orig;
                pos1->second = u;
				
				// add to the map between position and label.... 
				PosToStateLabel[states.size()] = orig;
				
        } else  {
                u = pos1->second;
                //cout << "found state named " << orig << endl;
        }

	// create or find dest state
	tie(pos2, inserted2) = states.insert(std::make_pair(dest, State()));
        if (inserted2) {
                v = add_vertex(uml_state_diagram);
                //cout << "added state named " << dest << endl;
                uml_state_diagram[v].state_label = dest;
                pos2->second = v;
				
				// add to map betweek positions and label.....
				PosToStateLabel[states.size()] = orig;

				
        } else  {
                v = pos2->second;
                //cout << "found state named " << dest << endl;
        }
		// call isTrans...	
		exists = isTrans(u, v, trans);
		if (exists == 0) {
			tie(transitions, inserted3) = add_edge(u, v, uml_state_diagram);
			//cout << "Adding transition " << u << " " << v << " " << trans << endl;
			if (inserted3) {
				uml_state_diagram[transitions].edge_label = trans;
				//uml_state_diagram[transitions].edge_info = "";
				uml_state_diagram[transitions].start_state = orig;
				uml_state_diagram[transitions].end_state = dest;
				
				
				//cout << "added edge labeled " << trans << endl;
				// add trans to table...
				transGuardActionInfo.insert(std::make_pair(trans, ""));
			}
		}
	return true;
}


// May eventually want to consider removing the states attached to a transition, if there are not
// any other transitions that point to it...
// Also, currently, this is not handling the potential that this is the only
// transition with a certain integer label and thus it should be removed from the mapping
// of labels to strings...
void cOrganism::deleteTrans(int pos) 
{

	Graph::edge_iterator e, eend;
	int count = 0;
	int s_start_lab, s_end_lab; //, trans_lab;
//	Transition t;

	for (tie(e, eend) = edges(uml_state_diagram); e != eend; ++e) { 
		if (count == pos) {
//			remove_edge(e, uml_state_diagram);
			//t = uml_state_diagram[*e];
			s_start_lab = uml_state_diagram[*e].start_state;
			s_end_lab = uml_state_diagram[*e].end_state;
			//trans_lab = uml_state_diagram[*e].edge_label;
			remove_edge(*e, uml_state_diagram);
			break;
		}

		count ++;
	}
	
	return;
}



// similar to getTransBetweenVertices... 
// optimization candidate perhaps using edge_range as per DBK
bool cOrganism::isTrans(State u, State v, int trans_name)
{
	oei e1, e2;
	bool exists = 0;

	// check to see if another trans has the same start, dest, and label 
	// if so, do not create trans. Else, create trans.
	for (tie(e1, e2) = out_edges(u, uml_state_diagram); e1 != e2; ++e1) {
		if ((target(*e1, uml_state_diagram) == v) && (uml_state_diagram[*e1].edge_label == trans_name)) {
			 exists = 1;
			 break;
		} 
	}
	
	return exists;
}

int cOrganism::getTransNumber (int pos) 
{
	int count = 0;
	int trans_name = -1;
	std::map <int, std::string>::iterator it;
	
	for (it = transGuardActionInfo.begin(); it!=transGuardActionInfo.end(); ++it) { 
		if (count == pos){
			trans_name = (*it).first;
			break;
		}
		count++;
	}
	
	return trans_name;

}

bool cOrganism::findTrans(int s0_pos, int s1_pos, int t_pos)
{
	int found_entry = 0;
	int trans_name;
	
	trans_name = getTransNumber (t_pos); 
	
	if (trans_name == -1) {
		return found_entry;
	}
	
	if ((NumStates() <= s0_pos) || (NumStates() <= s1_pos)) {
		return found_entry;
	}
	
	cOrganism::State& s0 = getStateInPosition(s0_pos);
	cOrganism::State& s1 = getStateInPosition(s1_pos);

	if (isTrans(s0, s1, trans_name)) { 
		found_entry = 1;
	} 
	
	return found_entry;

}


void cOrganism::printIdealXMI(cAvidaContext& ctx) 
{
	std::string temp;
	temp = "0";
	xmi += "<UML:Pseudostate xmi.id=\"s" + temp + "\" kind=\"initial\" outgoing=\"\" name=\"\" isSpecification=\"false\"/>\n";
	
	for (int j = 1; j < 5; ++j) {
		temp = "s" + StringifyAnInt(j);
		xmi+="<UML:CompositeState xmi.id=\"";
		xmi+=temp;
		xmi+= "\" isConcurrent=\"false\" name=\""; 
		xmi+= temp; 
		xmi+= "\" isSpecification=\"false\"/>\n";
		}
		
		// end the set of states....
		xmi+= "</UML:CompositeState.subvertex>\n";
		xmi+= "</UML:CompositeState>\n";
		xmi+= "</UML:StateMachine.top>\n";
		
		// start the set of transitions...
		xmi+="<UML:StateMachine.transitions>\n";


	// trans 0 
	
		temp = "t0";

		xmi+= "<UML:Transition xmi.id=\"" + temp + "\"";
		temp = "s0";
		xmi+= " source=\"" + temp + "\"";
		temp = "s1";
		xmi += " target=\"" + temp + "\" name=\"\" isSpecification=\"false\">\n";
		//xmi += temp;
		xmi += "</UML:Transition>\n";

	
	// trans 1
	
		temp = "t1";

		xmi+= "<UML:Transition xmi.id=\"" + temp + "\"";
		temp = "s1";
		xmi+= " source=\"" + temp + "\"";
		temp = "s2";
		xmi += " target=\"" + temp + "\" name=\"\" isSpecification=\"false\">\n";
		temp = "";
		temp += "<UML:Transition.effect> <UML:UninterpretedAction xmi.id=\"XDE-176F1237-1448-4226-A095-075FABD68B33\"";
		temp += " isAsynchronous=\"false\" name=\"\" isSpecification=\"false\">";
		temp += "<UML:Action.script> <UML:ActionExpression language=\"\" "; 
		temp += "body=\"^TempSensor.getTempData()\"/>  </UML:Action.script> ";
		temp += "</UML:UninterpretedAction> </UML:Transition.effect> \n";
		xmi += temp;
		xmi += "</UML:Transition>\n";
	
	// trans 2
		temp = "t2";

		xmi+= "<UML:Transition xmi.id=\"" + temp + "\"";
		temp = "s2";
		xmi+= " source=\"" + temp + "\"";
		temp = "s3";
		xmi += " target=\"" + temp + "\" name=\"\" isSpecification=\"false\">\n";

		temp = "";
		temp += "<UML:Transition.trigger> <UML:Event> <UML:ModelElement.namespace> ";
		temp += "<UML:Namespace> <UML:Namespace.ownedElement> ";
		temp += "<UML:CallEvent xmi.id=\"XDE-4C4256DD-D7D7-4687-AA73-761334859279\" " ;
		temp += " operation=\"XDE-9517D6BA-8666-4A82-AFEA-62D60FE37B07\" name=\"setTempData\" ";
		temp += " isSpecification=\"false\"/> </UML:Namespace.ownedElement> </UML:Namespace> ";
		temp += " </UML:ModelElement.namespace> </UML:Event> </UML:Transition.trigger>\n";
		xmi += temp;
		xmi += "</UML:Transition>\n";	
	
	// trans 3
		temp = "t3";

		xmi+= "<UML:Transition xmi.id=\"" + temp + "\"";
		temp = "s3";
		xmi+= " source=\"" + temp + "\"";
		temp = "s1";
		xmi += " target=\"" + temp + "\" name=\"\" isSpecification=\"false\">\n";

		temp = "";
		xmi += temp;
		xmi += "</UML:Transition>\n";
	
	// trans 4
		temp = "t4";

		xmi+= "<UML:Transition xmi.id=\"" + temp + "\"";
		temp = "s1";
		xmi+= " source=\"" + temp + "\"";
		temp = "s4";
		xmi += " target=\"" + temp + "\" name=\"\" isSpecification=\"false\">\n";

		temp = "";
		temp += "<UML:Transition.effect>  <UML:UninterpretedAction xmi.id=\"XDE-8280CF2B-DA14-4989-AC7F-D83012DE3234\"";
		temp += " isAsynchronous=\"false\" name=\"\" isSpecification=\"false\"> ";
		temp += "<UML:Action.script> <UML:ActionExpression language=\"\" ";
		temp += " body=\"^TempSensor.getOpState()\"/>  </UML:Action.script> ";
		temp += " </UML:UninterpretedAction> </UML:Transition.effect>\n";
			
		xmi += temp;
		xmi += "</UML:Transition>\n";
	
	// trans 5
		temp = "t5";

		xmi+= "<UML:Transition xmi.id=\"" + temp + "\"";
		temp = "s4";
		xmi+= " source=\"" + temp + "\"";
		temp = "s1";
		xmi += " target=\"" + temp + "\" name=\"\" isSpecification=\"false\">\n";

		temp = "";
		temp += "<UML:Transition.trigger> <UML:Event> <UML:ModelElement.namespace>";
		temp += "<UML:Namespace> <UML:Namespace.ownedElement> ";
		temp += "<UML:CallEvent xmi.id=\"XDE-C2891D3C-A49E-4DF0-BD95-A291630F4E4B\" ";
		temp += " operation=\"XDE-4437EBF1-9C42-4EB4-B7CF-415697B567CD\" name=\"setTempOpState\"";
		temp += " isSpecification=\"false\"/> </UML:Namespace.ownedElement> </UML:Namespace>";
		temp += " </UML:ModelElement.namespace> </UML:Event>  </UML:Transition.trigger>\n";
					
		xmi += temp;
		xmi += "</UML:Transition>\n";

//	for (tie(e, eend) = edges(uml_state_diagram); e != eend; ++e) { 
/*	for (int k = 0; k < 6; ++k) {

		// info determined from the trans itself....
		//trans_label = uml_state_diagram[*e].edge_label;
		
		temp = "t" + StringifyAnInt(k);

		xmi+= "<UML:Transition xmi.id=\"" + temp + "\"";
		temp = "s" + StringifyAnInt(uml_state_diagram[*e].start_state);
		xmi+= " source=\"" + temp + "\"";
		temp = "s" + StringifyAnInt(uml_state_diagram[*e].end_state);
		xmi += " target=\"" + temp + "\" name=\"\" isSpecification=\"false\">\n";
		
		
		temp = transGuardActionInfo[trans_label];
		xmi += temp;
		//temp = (transGuardActionInfo[uml_state_diagram[e*].edge_label])->second;

		xmi += "</UML:Transition>\n";
	}*/

	//cout << "begin XMI" << endl;
	//cout << xmi << endl;
	//cout << "end XMI" << endl;


}

void cOrganism::printXMI(cAvidaContext& ctx)
{

	Graph::vertex_iterator i, iend;
	Graph::edge_iterator e, eend;
	
	//oei e1, e2;
	int trans_label;
	int dest_state;
	std::string temp;
	int tempint;
	
	InitTransForXMI();

	xmi = "";
	// loop through all states
	// print initial information for the init state.
	//cout << "Num states: " << NumStates() << endl;

	tie(i, iend) = vertices(uml_state_diagram);
	
	if (NumStates() > 0) {
		//cout << "Initial \"\" " << uml_state_diagram[0].state_label << endl;
	//	hil += "Initial \"\" s"; 
		temp = StringifyAnInt(uml_state_diagram[0].state_label);
	//	hil += temp;
	//	hil += " ; \n"; */
		xmi += "<UML:Pseudostate xmi.id=\"s" + temp + "\" kind=\"initial\" outgoing=\"\" name=\"\" isSpecification=\"false\"/>\n";
		++i;
	}
	
	
	for (; i != iend; ++i) {
//		cout << "State " << uml_state_diagram[*i].state_label << " { " << endl;
/*		hil += "State s";
		temp = StringifyAnInt(uml_state_diagram[*i].state_label);
		hil += temp;
		hil += " { \n"; 
*/
		temp = "s" + StringifyAnInt(uml_state_diagram[*i].state_label);
		xmi+="<UML:CompositeState xmi.id=\"";
		xmi+=temp;
		//xmi+= uml_state_diagram[*i].state_label;
		xmi+= "\" isConcurrent=\"false\" name=\""; 
		xmi+= temp; 
		xmi+= "\" isSpecification=\"false\"/>\n";
		}
		
		// end the set of states....
		xmi+= "</UML:CompositeState.subvertex>\n";
		xmi+= "</UML:CompositeState>\n";
		xmi+= "</UML:StateMachine.top>\n";
		
		// start the set of transitions...
		xmi+="<UML:StateMachine.transitions>\n";



	for (tie(e, eend) = edges(uml_state_diagram); e != eend; ++e) { 
		// info determined from the trans itself....
		trans_label = uml_state_diagram[*e].edge_label;
		temp = "t" + StringifyAnInt(uml_state_diagram[*e].edge_label);
		
		// if I manage to set edge_info, I could then use that to print...
		// currently the start state and end state are already encoded. :)
		
		//xmi+=temp;
//		s = target(uml_state_diagram[e*], uml_state_diagram);
		
//		temp += "s" + StringifyAnInt(target(e, uml_state_diagram));

		xmi+= "<UML:Transition xmi.id=\"" + temp + "\"";
		temp = "s" + StringifyAnInt(uml_state_diagram[*e].start_state);
		xmi+= " source=\"" + temp + "\"";
		temp = "s" + StringifyAnInt(uml_state_diagram[*e].end_state);
		xmi += " target=\"" + temp + "\" name=\"\" isSpecification=\"false\">\n";
		
		
		temp = transGuardActionInfo[trans_label];
		xmi += temp;
		//temp = (transGuardActionInfo[uml_state_diagram[e*].edge_label])->second;

		xmi += "</UML:Transition>\n";

	
	}

/*
		// print each transition...
		for (tie(e1, e2) = out_edges((*i), uml_state_diagram); e1 != e2; ++e1) {
			trans_label = uml_state_diagram[*e1].edge_label;
			dest_state = target(*e1, uml_state_diagram);
			hil += "Transition \"" +  transGuardActionInfo[trans_label];
//			temp = StringifyAnInt(tempint);
			hil += "\" to s";
			temp = StringifyAnInt( uml_state_diagram[dest_state].state_label);
			hil += temp + " ; \n";
		}
		hil +=  "} \n"; 
	}
	//cout << "HIL : " << hil << endl;
	*/


	//cout << "XMI : " << xmi << endl;
}




std::string cOrganism::StringifyAnInt(int x) { 
	std::ostringstream o;
	o << x;
	return o.str();
}

// return the number of states in a state diagram
double cOrganism::NumStates() 
{
	return (double) num_vertices(uml_state_diagram);
}

double cOrganism::NumTrans()
{
	return (double) num_edges(uml_state_diagram);
}

std::string cOrganism::getXMI()
{
	std::string temp = xmi_begin + xmi + xmi_end;
//	cout << "PRINT XMI" << endl;
//	cout << temp << endl;
//	cout << "END PRINT XMI" << endl;
//	std::string temp = hil;

	cout << "PRINT XMI" << endl;
	cout << temp << endl;
	cout << "END PRINT XMI" <<endl;
	return (xmi_begin + xmi + xmi_end);
}

// This function takes in an integer and returns the state in that position within the states map. Recall that this
// map relates integers (the number assigned to a state) to a state in the graph.
// ... Potential candidate for optimization as per DBK ... 

cOrganism::State& cOrganism::getStateInPosition (int num)
{
	/*std::pair<vertex_iterator, vertex_iterator>
vertices(const adjacency_list& g)*/
//	Graph::vertex_iterator vi, vi_end;
//	tie(vi,vi_end) = vertices(uml_state_diagram);
//	vi+=num;
	//graph_traits<Graph>::vertex_descriptor b = *vi;
//	return *vi;

	int count = 0;
	// This code uses a value ordering on the states (lowest number = position 0)
	nsm_it i;
	for (i=states.begin(); i!=states.end(); ++i)
	{
		if (count == num) {
			break;
		}
		count++;
	}
	return i->second;
	

/*
	int x = PosToStateLabel[num];
	return (states[x]);
*/

	
}


// if you ask for something greater than the number of states then you get the highest numbered state.
int cOrganism::getStateLabelInPosition (int num)
{
	/*std::pair<vertex_iterator, vertex_iterator>
vertices(const adjacency_list& g)*/
//	Graph::vertex_iterator vi, vi_end;
//	tie(vi,vi_end) = vertices(uml_state_diagram);
//	vi+=num;
	//graph_traits<Graph>::vertex_descriptor b = *vi;
//	return *vi;

	int count = 0;
	// This code uses a value ordering on the states (lowest number = position 0)
	nsm_it i;
	for (i=states.begin(); i!=states.end(); ++i)
	{
		if (count == num) {
			break;
		}
		count++;
	}
	return i->first;
	

/*
	int x = PosToStateLabel[num];
	return (states[x]);
*/	
}


// if you ask for something greater than the number of trans -- you get the highest numbered one...
int cOrganism::getTransLabelInPosition (int num)
{
	int count = 0;
	tm_it i;
	for (i=transGuardActionInfo.begin(); i!=transGuardActionInfo.end(); ++i)
	{
		if (count == num) {
			break;
		}
		count++;
	}
	int test = i->first;

	return i->first;
	
}

cOrganism::Graph& cOrganism::GetGraph()
{
	return uml_state_diagram;
}


bool cOrganism::InjectParasite(const cGenome& injected_code)
{
  assert(m_interface);
  return m_interface->InjectParasite(this, injected_code);
}

bool cOrganism::InjectHost(const cCodeLabel& label, const cGenome& injected_code)
{
  return m_hardware->InjectHost(label, injected_code);
}

int cOrganism::OK()
{
  if (m_hardware->OK() && phenotype.OK()) return true;
  return false;
}


double cOrganism::CalcMeritRatio()
{
  const double age = (double) phenotype.GetAge();
  const double merit = phenotype.GetMerit().GetDouble();
  return (merit > 0.0) ? (age / merit ) : age;
}


bool cOrganism::GetTestOnDivide() const { return m_interface->TestOnDivide();}
bool cOrganism::GetFailImplicit() const { return m_world->GetConfig().FAIL_IMPLICIT.Get(); }

bool cOrganism::GetRevertFatal() const { return m_world->GetConfig().REVERT_FATAL.Get(); }
bool cOrganism::GetRevertNeg()   const { return m_world->GetConfig().REVERT_DETRIMENTAL.Get(); }
bool cOrganism::GetRevertNeut()  const { return m_world->GetConfig().REVERT_NEUTRAL.Get(); }
bool cOrganism::GetRevertPos()   const { return m_world->GetConfig().REVERT_BENEFICIAL.Get(); }

bool cOrganism::GetSterilizeFatal() const{return m_world->GetConfig().STERILIZE_FATAL.Get();}
bool cOrganism::GetSterilizeNeg()  const { return m_world->GetConfig().STERILIZE_DETRIMENTAL.Get(); }
bool cOrganism::GetSterilizeNeut() const { return m_world->GetConfig().STERILIZE_NEUTRAL.Get();}
bool cOrganism::GetSterilizePos()  const { return m_world->GetConfig().STERILIZE_BENEFICIAL.Get(); }
double cOrganism::GetNeutralMin() const { return m_world->GetConfig().NEUTRAL_MIN.Get();}
double cOrganism::GetNeutralMax() const { return m_world->GetConfig().NEUTRAL_MAX.Get();}


void cOrganism::PrintStatus(ostream& fp, const cString & next_name)
{
  fp << "---------------------------" << endl;
  m_hardware->PrintStatus(fp);
  phenotype.PrintStatus(fp);
  fp << "---------------------------" << endl;
  fp << "ABOUT TO EXECUTE: " << next_name << endl;
}


bool cOrganism::Divide_CheckViable()
{
  // Make sure required task (if any) has been performed...
  const int required_task = m_world->GetConfig().REQUIRED_TASK.Get();
  const int immunity_task = m_world->GetConfig().IMMUNITY_TASK.Get();

  if (required_task != -1 && phenotype.GetCurTaskCount()[required_task] == 0) { 
    if (immunity_task==-1 || phenotype.GetCurTaskCount()[immunity_task] == 0) {
      Fault(FAULT_LOC_DIVIDE, FAULT_TYPE_ERROR,
            cStringUtil::Stringf("Lacks required task (%d)", required_task));
      return false; //  (divide fails)
    } 
  }

  const int required_reaction = m_world->GetConfig().REQUIRED_REACTION.Get();
  if (required_reaction != -1 && phenotype.GetCurTaskCount()[required_reaction] == 0) {
    Fault(FAULT_LOC_DIVIDE, FAULT_TYPE_ERROR,
          cStringUtil::Stringf("Lacks required reaction (%d)", required_reaction));
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

bool cOrganism::ActivateDivide(cAvidaContext& ctx)
{
  assert(m_interface);
  // Activate the child!  (Keep Last: may kill this organism!)
  return m_interface->Divide(ctx, this, child_genome);
}


void cOrganism::Fault(int fault_loc, int fault_type, cString fault_desc)
{
  (void) fault_loc;
  (void) fault_type;
  (void) fault_desc;

#if FATAL_ERRORS
  if (fault_type == FAULT_TYPE_ERROR) {
    phenotype.IsFertile() = false;
  }
#endif

#if FATAL_WARNINGS
  if (fault_type == FAULT_TYPE_WARNING) {
    phenotype.IsFertile() = false;
  }
#endif

#if BREAKPOINTS
  phenotype.SetFault(fault_desc);
#endif

  phenotype.IncErrors();
}
