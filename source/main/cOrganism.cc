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

/* 
  // create in-memory representation of model
  AddTrans(0,0,1);
  AddTrans(1,1,2);
  AddTrans(2,2,3);
  AddTrans(3,3,1);
*/

  // initialize the iterators to point to the first element
  triggers.push_back("ta");
  triggers.push_back("tb");
  triggers.push_back("tc");
  triggers.push_back("td");
  guards.push_back("ga");
  guards.push_back("gb");
  guards.push_back("gc");
  guards.push_back("gd");
  actions.push_back("aa");
  actions.push_back("ab");
  actions.push_back("ac");
  actions.push_back("ad");
  
  trigger_index = 0;
  action_index = 0;
  guard_index = 0;
  trans_label_index = 0;
  orig_state_index = 0;
  dest_state_index = 0;
//  total_states = 0;
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
void cOrganism::modelCheck(cAvidaContext& ctx)
{

	printXMI();
	

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
  
	m_world->GetStats().addState(states.size());
	m_world->GetStats().addTrans(transitions.size());
	m_world->GetStats().addTransLabel(transition_labels.size());
  
}

void cOrganism::seedModel() {
	std::string data, line; 
	std::ifstream infile;
	infile.open("instinctModel");
	assert(infile.is_open());
	
	while (getline (infile, line))
	{
		data.append(line);
		line.erase();
	}
	
	//read from file; load into string/strstream, and return it.
	
	//return data;
	return;

}

bool cOrganism::findTransLabel(transition_label t) { 
	for(std::vector<transition_label>::iterator i=transition_labels.begin(); i!=transition_labels.end(); ++i){
		if ((i->trigger == t.trigger) && (i->guard == t.guard) && (i->action == t.action)) {
//		if (i->trigger == t.trigger) {
			return true;
		}
	}
	return false;
}

bool cOrganism::findTrans(int orig, int dest)
{
	for(std::vector<transition>::iterator i=transitions.begin(); i!=transitions.end(); ++i){
		if((i->orig_state == orig) && (i->dest_state == dest)) {
			return true;
		}
	}
	
	return false;

}

bool cOrganism::findTrans(int orig, int dest, std::string label) 
{
	std::string t_lab;
	for(std::vector<transition>::iterator i=transitions.begin(); i!=transitions.end(); ++i){
		if((i->orig_state == orig) && (i->dest_state == dest)) {
			t_lab = (i->trans.trigger + i->trans.guard + i->trans.action);
			if (t_lab == label) {
				return true;
			}
		}
	}
	
	return false;
}

bool cOrganism::findTrans(transition t) 
{
	for(std::vector<transition>::iterator i=transitions.begin(); i!=transitions.end(); ++i){
		if((i->orig_state == t.orig_state) && (i->dest_state == t.dest_state) && 
			(i->trans.trigger == t.trans.trigger) && (i->trans.guard == t.trans.guard) && 
			(i->trans.action == t.trans.action)) {
				return true;
		}
	}
	
	return false;
}


template <typename T>
bool cOrganism::absoluteMoveIndex (T x, int &index, int amount )
{
/*	if (x.size() == 0) {
		return false;
	}
	
	if (amount > 0) { 
//		index += (amount % x.size()); // this provides relative jumping
		index = (amount % x.size());

		// index is greater than vector
		if (index >= x.size()) { 
			index -= x.size();
		} else if(index < 0) { 
			index += x.size();
		}
	}	
	*/
	index = 0;
	return relativeMoveIndex(x, index, amount);
}

template <typename T>
bool cOrganism::relativeMoveIndex (T x, int &index, int amount )
{
	if (x.size() == 0) {
		return false;
	}
	
	if (amount > 0) { 
//		index += (amount % x.size()); // this provides relative jumping
		index += (amount % x.size());

		// index is greater than vector
		if (index >= x.size()) { 
			index -= x.size();
		} else if(index < 0) { 
			index += x.size();
		}
	}	
		
	return true;
}


bool cOrganism::absoluteJumpTrigger(int jump_amount)
{
	return absoluteMoveIndex(triggers, trigger_index, jump_amount);
}

bool cOrganism::absoluteJumpGuard(int jump_amount)
{
	return absoluteMoveIndex(guards, guard_index, jump_amount);	
}

bool cOrganism::absoluteJumpAction(int jump_amount)
{
	return absoluteMoveIndex(actions, action_index, jump_amount);
}

bool cOrganism::absoluteJumpTransitionLabel(int jump_amount)
{
	return absoluteMoveIndex(transition_labels, trans_label_index, jump_amount);
}

bool cOrganism::absoluteJumpOriginState(int jump_amount) 
{
	return absoluteMoveIndex(states, orig_state_index, jump_amount);
}

bool cOrganism::absoluteJumpDestinationState(int jump_amount) 
{
	return absoluteMoveIndex(states, dest_state_index, jump_amount);
}

bool cOrganism::relativeJumpTrigger(int jump_amount)
{
	return relativeMoveIndex(triggers, trigger_index, jump_amount);
}

bool cOrganism::relativeJumpGuard(int jump_amount)
{
	return relativeMoveIndex(guards, guard_index, jump_amount);	
}

bool cOrganism::relativeJumpAction(int jump_amount)
{
	return relativeMoveIndex(actions, action_index, jump_amount);
}

bool cOrganism::relativeJumpTransitionLabel(int jump_amount)
{
	return relativeMoveIndex(transition_labels, trans_label_index, jump_amount);
}

bool cOrganism::relativeJumpOriginState(int jump_amount) 
{
	return relativeMoveIndex(states, orig_state_index, jump_amount);
}

bool cOrganism::relativeJumpDestinationState(int jump_amount) 
{
	return relativeMoveIndex(states, dest_state_index, jump_amount);
}

std::string cOrganism::getTrigger()
{
	if (triggers.size() == 0) {
		return "";
	} else {
		return triggers[trigger_index];
	}
}

std::string cOrganism::getGuard()
{
	if (guards.size() == 0) {
		return "";
	} else {
		return guards[guard_index];
	}
}

std::string cOrganism::getAction()
{
	if (actions.size() == 0) {
		return "";
	} else {
		return actions[action_index];
	}
}

int cOrganism::getOrigState()
{
	return orig_state_index;
}
 
int cOrganism::getDestState()
{
	return dest_state_index;
}

transition_label cOrganism::getTransLabel()
{
	return transition_labels[trans_label_index];
}

// State manipulation
bool cOrganism::addState()
{
	if (states.size() < 11) {
		states.push_back(states.size());
		dest_state_index = states.size() - 1;
	} else {
		 return false;
	}
	
	return true;
}

bool cOrganism::addTransitionLabel()
{

	transition_label t;
	t.trigger = getTrigger();
	t.guard = getGuard();
	t.action = getAction();
	
	// no dupes
	if (findTransLabel(t)){
		return false;
	 }
	
	transition_labels.push_back(t);
	
	// Move the transition label index to the most recently created
	trans_label_index = transition_labels.size() - 1;
	
	return true;
}


bool cOrganism::addTransition()
{
	if ((states.size() == 0) || (transition_labels.size() == 0)) {

		return false;
	} 

	transition t;
	t.orig_state = getOrigState();
	t.dest_state = getDestState();
	t.trans = getTransLabel();
	
	// no dupes
    if (findTrans(t)) {
		return false;
	}

	transitions.push_back(t);
		
	return true;

}


bool cOrganism::addTransitionTotal()
{
	if ((states.size() == 0)) {

		return false;
	} 

	transition t;
	t.orig_state = getOrigState();
	t.dest_state = getDestState();
	
	transition_label tl;
	tl.trigger = getTrigger();
	tl.guard = getGuard();
	tl.action = getAction();
	t.trans = tl;
	
	
	// no dupes
    if (findTrans(t)) {
		return false;
	}

	transitions.push_back(t);
		
	return true;

}


int cOrganism::numStates()
{
	return states.size();
}

int cOrganism::numTrans()
{
	return transitions.size();
}

// print the label. Change - signs to _
std::string cOrganism::StringifyAnInt(int x) { 

	std::ostringstream o;

	if (x < 0) {
		x = abs(x);
		o << "_";
	} 
	
	o << x;
	return o.str();
}

std::string cOrganism::getXMI()
{
	return (xmi_begin + xmi + xmi_end);
}

void cOrganism::printXMI()
{
	std::string temp, temp1, temp2;

	int s_count = 0;
	int t_count = 0;
	xmi = "";

	if (numStates() > 0) {
		temp = StringifyAnInt(s_count);
		xmi += "<UML:Pseudostate xmi.id=\"s" + temp + "\" kind=\"initial\" outgoing=\"\" name=\"s";
		xmi += temp + "\" isSpecification=\"false\"/>\n";
		++s_count;
	}
	
	
	for (; s_count < numStates(); ++s_count) {
		temp = "s" + StringifyAnInt(s_count);
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



	for (t_count = 0; t_count < numTrans(); ++t_count) { 
		// info determined from the trans itself....
		temp = "t" + StringifyAnInt(t_count);
		temp1 = "s" + StringifyAnInt(transitions[t_count].orig_state);
		temp2 = "s" + StringifyAnInt(transitions[t_count].dest_state);
		temp = temp + temp1 + temp2;

		xmi+= "<UML:Transition xmi.id=\"" + temp + "\"";
		xmi+= " source=\"" + temp1 + "\"";
		xmi += " target=\"" + temp2 + "\" name=\"\" isSpecification=\"false\">\n";

		// Get guard, trigger, and action
		temp = transitions[t_count].trans.trigger;
		temp1 = transitions[t_count].trans.guard;
		temp2 = transitions[t_count].trans.action;

		// print trigger, if any
		if (temp != "") {
			xmi+= "<UML:Transition.trigger> <UML:Event> <UML:ModelElement.namespace> <UML:Namespace> ";
			xmi+= "<UML:Namespace.ownedElement> <UML:CallEvent xmi.id=\"\"  operation=\"\" ";
			xmi+= "name=\"" + temp + "isSpecification=\"false\"/> "; 
			xmi+= "</UML:Namespace.ownedElement> </UML:Namespace> </UML:ModelElement.namespace> ";
			xmi+= "</UML:Event>  </UML:Transition.trigger> ";
		}
		
		// print guard, if any
		// Note: for guard to work, '<' => '&lt'
		if (temp1 != ""){
			xmi+= "<UML:Transition.guard> <UML:Guard> <UML:Guard.expression> ";
			xmi+= "<UML:BooleanExpression body=\"" + temp1 + "\" language=\"\"/> ";
			xmi+= "</UML:Guard.expression> </UML:Guard> </UML:Transition.guard> ";
		}
		
		// print action, if any
		if (temp2 != "") { 
			xmi+= "<UML:Transition.effect> <UML:UninterpretedAction xmi.id=\"\" ";
			xmi+= "isAsynchronous=\"false\" name=\"\" isSpecification=\"false\"> ";
			xmi+= "<UML:Action.script> <UML:ActionExpression language=\"\" body=\" "; 
			xmi+= temp2 + "\"/> </UML:Action.script> </UML:UninterpretedAction> </UML:Transition.effect> ";		
		}
		
		xmi += "</UML:Transition>\n";

	
	}

	return;
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
