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

using namespace std;


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
  
  // UML
  // initialize the hil string values
  InitHILBandE();		
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

//	cout << "Model check begin " << endl;
	printHIL(ctx);
//	cout << "Model check end " << endl;

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


/*
void cOrganism::printXMI(cAvidaContext& ctx)
{

// UML pretty print part....
  std::set<int>::iterator it_st_start;
  std::set<int>::iterator it_st_end;
  std::set<int>::iterator i;

  
  it_st_start = uml_state_set.begin();
  it_st_end = uml_state_set.end();
  int count;

  // print state XMI information
  cout << "<UML:StateMachine xmi.id=\"XDE-BACD03B9-FA04-4C7F-B464-748CB92A47B2\"";
  cout << "name=\"StateMachine1\" isSpecification=\"false\">" << endl;
  cout << "<UML:StateMachine.top>" << endl;  
  cout << "<UML:CompositeState xmi.id=\"XDE-3C5902C0-E8A6-4E0C-88D5-4ACB7E0EDF15\"";
  cout << "isConcurrent=\"false\" name=\"TOP\" isSpecification=\"false\">" << endl;
  cout << "<UML:CompositeState.subvertex>" << endl;

  for(i = it_st_start; i!=it_st_end; ++i){
    if (i == it_st_start) { 
		  // initial state 
//		cout << "<UML:Pseudostate xmi.id=\"" << *i << "\" kind=\"initial\" outgoing=\"\"";
		cout << "<UML:Pseudostate xmi.id=\"" << *i << "\" kind=\"initial\" ";
		cout << "name=\"\" isSpecification=\"false\"/>" << endl;

	} else {
//		cout << "<UML:CompositeState xmi.id=\"" << *i << "\" isConcurrent=\"false\" outgoing=\"\" name=\"";
		cout << "<UML:CompositeState xmi.id=\"" << *i << "\" isConcurrent=\"false\" name=\"";
		cout << *i << "\" isSpecification=\"false\"/>" << endl;
	}
  }
  cout << "</UML:CompositeState.subvertex>" << endl;
  cout << "</UML:CompositeState>" << endl;
  cout << "</UML:StateMachine.top>" << endl;


  count = 0;
  cout << "<UML:StateMachine.transitions>" << endl;
  for (t_transitionMap::iterator it = uml_transitions.begin(); it != uml_transitions.end(); ++it) {
	 cout << "<UML:Transition xmi.id=\"" << (*it).first << "\" source=\"" << (*it).second.first;
	 cout << "\" target=\"" <<  (*it).second.second << "\" name=\"" <<  (*it).first;
	 cout << "\" isSpecification=\"false\">" << endl;
	 
	 if (trans_info.size() > count) {
		 cout << trans_info[count] << endl;
	 }
	 cout << "</UML:Transition>" << endl;

	 count ++;
  }
  
  cout << "</UML:StateMachine.transitions>" << endl;
  cout << "</UML:StateMachine>" << endl;

// end of UML pretty printing... 

}


void cOrganism::InitTransForXMI()
{
	//  0 - init state to state 1
	trans_info.push_back("");
	// 1 - state 1 to state 2 - getOperationalState()
	trans_info.push_back("<UML:Transition.trigger> <UML:Event> <UML:ModelElement.namespace> <UML:Namespace> <UML:Namespace.ownedElement> <UML:CallEvent xmi.id=\"XDE-7126ED39-5D5D-4160-924B-303514B17EAB\" operation=\"XDE-1266DA8A-61C0-43B4-A77C-200F54A6585D\" name=\"getOperationalState\" isSpecification=\"false\"/> </UML:Namespace.ownedElement></UML:Namespace> </UML:ModelElement.namespace> </UML:Event> </UML:Transition.trigger> ");
	// 2 - state 2 to state 1 - opState = True
	trans_info.push_back("<UML:Transition.effect> <UML:UninterpretedAction xmi.id=\"XDE-D9BCD8D1-7FC4-4B14-9E76-D3A642799013\" isAsynchronous=\"false\" name=\"\" isSpecification=\"false\"> <UML:Action.script> <UML:ActionExpression language=\"\" body=\"operationalState:=1;^ComputingComponent.ccTRUE\"/>` </UML:Action.script> </UML:UninterpretedAction> </UML:Transition.effect>");
	// 3 - state 2 to state 1 - opState = False
	trans_info.push_back ("<UML:Transition.effect> <UML:UninterpretedAction xmi.id=\"XDE-9F00136E-D61D-4BB0-B7D6-1E795238FD1E\" isAsynchronous=\"false\" name=\"\" isSpecification=\"false\"> <UML:Action.script> <UML:ActionExpression language=\"\" body=\"operationalState:=0;^ComputingComponent.ccFALSE\"/> </UML:Action.script> </UML:UninterpretedAction> </UML:Transition.effect>");
	// 4 - state 1 to state 3 - getBrightnessValue()
	trans_info.push_back ("<UML:Transition.trigger> <UML:Event> <UML:ModelElement.namespace> <UML:Namespace> <UML:Namespace.ownedElement> <UML:CallEvent xmi.id=\"XDE-A28463C5-2F9F-457C-B6F3-241526CA4791\" operation=\"XDE-E84A5762-CA92-4E03-A237-FE5AE2C99D9A\" name=\"getBrightnessValue\" isSpecification=\"false\"/> </UML:Namespace.ownedElement> </UML:Namespace> </UML:ModelElement.namespace> </UML:Event> </UML:Transition.trigger>"); 
	// 5 - state 3 to state 4 - Environment.getBrightnessValue();
	trans_info.push_back ("<UML:Transition.effect> <UML:UninterpretedAction xmi.id=\"XDE-6C3D3042-5C7A-4746-8A90-BEDB86FD2FF4\" isAsynchronous=\"false\" name=\"\" isSpecification=\"false\"> <UML:Action.script> <UML:ActionExpression language=\"\" body=\"^Environment.getBrightnessValue\"/> </UML:Action.script> </UML:UninterpretedAction> </UML:Transition.effect>" ) ;
	// 6 - state 4 to state 5 - setBrightnessValue();
	trans_info.push_back ("<UML:Transition.trigger> <UML:Event> <UML:ModelElement.namespace> <UML:Namespace> <UML:Namespace.ownedElement> <UML:CallEvent xmi.id=\"XDE-79243838-9C4E-4908-9637-9F9583043BE4\" operation=\"XDE-C8BD0DBA-E427-41A0-95F4-98FAA920ACA9\" name=\"setBrightnessValue\" isSpecification=\"false\"/> </UML:Namespace.ownedElement> </UML:Namespace>  </UML:ModelElement.namespace> </UML:Event>  </UML:Transition.trigger>");
	// 7 - state 5 to state 6 - value < min
	trans_info.push_back ("<UML:Transition.guard> <UML:Guard> <UML:Guard.expression> <UML:BooleanExpression body=\"brightnessValue&lt;0\" language=\"\"/> </UML:Guard.expression> </UML:Guard> </UML:Transition.guard> <UML:Transition.effect> <UML:UninterpretedAction xmi.id=\"XDE-0B7A10EB-A9FC-4DE8-BBF1-AF1C9A970E7F\" isAsynchronous=\"false\" name=\"\" isSpecification=\"false\"> <UML:Action.script> <UML:ActionExpression language=\"\" body=\"correctedBrightnessValue:=0\"/> </UML:Action.script> </UML:UninterpretedAction> </UML:Transition.effect>");
	// 8 - state 5 to state 6 - value > max
	trans_info.push_back ("<UML:Transition.guard> <UML:Guard> <UML:Guard.expression> <UML:BooleanExpression body=\"brightnessValue&gt;1000\" language=\"\"/> </UML:Guard.expression> </UML:Guard> </UML:Transition.guard> <UML:Transition.effect> <UML:UninterpretedAction xmi.id=\"XDE-7D6DDE48-7568-4043-B00A-87EFBE1A6CB3\" isAsynchronous=\"false\" name=\"\" isSpecification=\"false\"> <UML:Action.script> <UML:ActionExpression language=\"\" body=\"correctedBrightnessValue:=1000\"/> </UML:Action.script> </UML:UninterpretedAction> </UML:Transition.effect>");
	// 9 - state 5 to state 6 - value > min && value < max
	trans_info.push_back ("<UML:Transition.guard> <UML:Guard> <UML:Guard.expression> <UML:BooleanExpression body=\"brightnessValue &gt;=0 &amp; brightnessValue&lt;=1000\" language=\"\"/> </UML:Guard.expression> </UML:Guard> </UML:Transition.guard> <UML:Transition.effect> <UML:UninterpretedAction xmi.id=\"XDE-8E3B2DF6-D63B-4A70-9CD3-FF0DE13EEDAD\" isAsynchronous=\"false\" name=\"\" isSpecification=\"false\"> <UML:Action.script> <UML:ActionExpression language=\"\" body=\"correctedBrightnessValue:=brightnessValue\"/> </UML:Action.script> </UML:UninterpretedAction> </UML:Transition.effect>");
	// 10 - state 6 to state 1 - set Computing Component brightness value
	trans_info.push_back("<UML:Transition.effect> <UML:UninterpretedAction xmi.id=\"XDE-101E5C46-12EA-4169-9DC9-D3661EE9836B\" isAsynchronous=\"false\" name=\"\" isSpecification=\"false\"> <UML:Action.script> <UML:ActionExpression language=\"\" body=\"^ComputingComponent.setBrightnessValue(brightnessValue)\"/> </UML:Action.script> </UML:UninterpretedAction> </UML:Transition.effect>");
	
}
*/
void cOrganism::InitTransForHIL()
{

	// assign transition values to map elements
	std::map <int, std::string>::iterator it;
	int count = 0;

	for (it = transGuardActionInfo.begin(); it!=transGuardActionInfo.end(); ++it) { 
	switch (count){
		case 0:
			(*it).second = "";
			break;
		case 1:
			(*it).second = "getOperationalState";
			break;
		case 2:
			(*it).second = "/operationalState:=1^ComputingComponent.ccTRUE";
			break;	
		case 3: 
			(*it).second = "/operationalState:=0^ComputingComponent.ccFALSE";
			break;
		case 4: 
			(*it).second = "getBrightnessValue";
			break;
		case 5:
			(*it).second = "^Environment.getBrightnessValue";
			break;
		case 6:
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
			break;
		default:
			(*it).second = "--too many trans--";
	}
		count++;
	}	
}	
	
	
	
/*	if (transGuardActionInfo.size() >= 6) {
	it = uml_trans_set.begin();
	(*it).second = "";
	++it;
	(*it).second = "^Environment.getBrightnessValue";
	++it;
	(*it).second = "/operationalState:=1^ComputingComponent.ccTRUE";
	++it;
	(*it).second = "/operationalState:=0^ComputingComponent.ccFALSE";
	++it;
	(*it).second = "getOperationalState";
	++it;
	(*it).second = "getBrightnessValue";
	++it;
	(*it).second = "^ComputingComponent.setBrightnessValue(brightnessValue)";
	++it;
	(*it).second = "[brightnessValue<0]/correctedBrightnessValue:=0";
	++it;
	(*it).second = "[brightnessValue >=0 & brightnessValue<=1000]/correctedBrightnessValue:=brightnessValue";
	++it;
	(*it).second = "[brightnessValue>1000]/correctedBrightnessValue:=1000";
	++it;
	(*it).second = "setBrightnessValue(brightnessValue)";
	}*/



//void cOrganism::AssignTransMeaning (int trans)
//{ 
	

//	return;
//}


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
				//cout << "added edge labeled " << trans << endl;
				// add trans to table...
				transGuardActionInfo.insert(std::make_pair(trans, ""));
			}
		}
	return true;
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


void cOrganism::printHIL(cAvidaContext& ctx)
{
	Graph::vertex_iterator i, iend;
	
	oei e1, e2;
	int trans_label;
	int dest_state;
	std::string temp;
	int tempint;
	
	InitTransForHIL();

	hil = "";
	// loop through all states
	// print initial information for the init state.
	//cout << "Num states: " << NumStates() << endl;
	if (NumStates() > 0) {
		//cout << "Initial \"\" " << uml_state_diagram[0].state_label << endl;
		hil += "Initial \"\" s"; 
		temp = StringifyAnInt(uml_state_diagram[0].state_label);
		hil += temp;
		hil += " ; \n"; 

	}
	
	for (tie(i, iend) = vertices(uml_state_diagram); i != iend; ++i) {
//		cout << "State " << uml_state_diagram[*i].state_label << " { " << endl;
		hil += "State s";
		temp = StringifyAnInt(uml_state_diagram[*i].state_label);
		hil += temp;
		hil += " { \n"; 

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

std::string cOrganism::getHil()
{
//	std::string temp = hil_begin + hil + hil_end;
//	std::string temp = hil;

//	cout << "PRINT HIL" << endl;
//	cout << temp << endl;
//	cout << "END PRINT HIL" <<endl;
	return (hil_begin + hil + hil_end);
}

void cOrganism::InitHILBandE()
{
/*
	hil_begin = "Formalize as promela ;\n \
	Model XDEModel{\n \
	Class BrightnessSensor {\n \
		InstanceVar int brightnessValue ;\n \
		InstanceVar int correctedBrightnessValue ;\n \
        InstanceVar bool operationalState ;\n \
        Signal getBrightnessValue( ) ;\n \
        Signal getOperationalState( ) ;\n \
        Signal setBrightnessValue(int ) ;\n ";
	
	hil_end = "}\n\
	Class ComputingComponent {\n \
        InstanceVar bool automaticMode ;\n \
        InstanceVar int brightnessValue ;\n \
        InstanceVar int computedDimmerValue ;\n \
        InstanceVar int desiredBrightnessValue ;\n \
        InstanceVar bool initialized ;\n \
        InstanceVar bool manualMode ;\n \
        InstanceVar bool motionDetected ;\n \
        Signal ccAck( ) ;\n \
        Signal ccFALSE( ) ;\n \
        Signal ccTRUE( ) ;\n \
        Signal setBrightnessValue(int ) ;\n \
        Signal setDesiredBrightnessValue(int ) ;\n \
        Initial  \"\" Init ;\n \
        State Init  {\n \
                Transition \"\" to Initialize ;\n \
        }\n \
        CompositeState Initialize {\n \
                Transition \"ccFALSE\" to PowerOff ;\n \
        Initial  \"\" InitializationStart ;\n \
        State BSensRec  {\n \
                Transition \"^MotionSensor.getOperationalState\" to MSensReq ;\n \
        }\n \
        State BSensReq  {\n \
                Transition \"ccTRUE\" to BSensRec ;\n \
        }\n \
        State DimmerRec  {\n \
                Transition \"^BrightnessSensor.getOperationalState\" to BSensReq ;\n \
        }\n \
        State DimmerReq  {\n \
                Transition \"ccTRUE\" to DimmerRec ;\n \
        }\n \
        State InitializationStart  {\n \
                Transition \"^Dimmer.getOperationalState\" to DimmerReq ;\n \
        }\n \
        State MSensReq  {\n \
                Transition \"ccTRUE/initialized:=1\" to NormalBehavior ;\n \
        }\n \
        }\n \
        CompositeState NormalBehavior {\n \
        Initial  \"\" NBInit ;\n \
        CompositeState Automatic {\n \
        Initial  \"/automaticMode:=1^UserInterface.setDisplayMode(1)\" AIdle ;\n \
        State AIdle  {\n \
                Transition \"ccAck^MotionSensor.isRoomOccupied\" to State1 ;\n \
        }\n \
        State ActualValueRequested  {\n \
                Transition \"setBrightnessValue(brightnessValue)\" to BrightnessValueReceived ;\n \
        }\n \
        State BrightnessValueReceived  {\n \
                Transition \"/computedDimmerValue:=desiredBrightnessValue-brightnessValue\" to CorrectionValueCalculated ;\n \
        }\n \
        State CorrectionValueCalculated  {\n \
                Transition \"^Dimmer.changeDimmerValue(computedDimmerValue)\" to CorrectionValueSumbitted ;\n \
        }\n \
        State CorrectionValueSumbitted  {\n \
                Transition \"ccAck/automaticMode:=0; motionDetected:=0\" to NBInit ;\n \
        }\n \
        State DesiredValueReceived  {\n \
                Transition \"^BrightnessSensor.getBrightnessValue\" to ActualValueRequested ;\n \
        }\n \
        State DesiredValueRequested  {\n \
                Transition \"setDesiredBrightnessValue(desiredBrightnessValue)\" to DesiredValueReceived ;\n \
        }\n \
        State MotionValueReceived  {\n \
                Transition \"[motionDetected=1]^UserInterface.getDesiredBrightnessValue\" to DesiredValueRequested ;\n \
                Transition \"[motionDetected=0]\" to NBInit ;\n \
        }\n \
        State State1  {\n \
                Transition \"ccFALSE/motionDetected:=0\" to MotionValueReceived ;\n \
                Transition \"ccTRUE/motionDetected:=1\" to MotionValueReceived ;\n \
        }\n \
        }\n \
        State EnterAuto  {\n \
                Transition \"\" to OpStatusAutoSet ;\n \
        }\n \
        State EnterManual  {\n \
                Transition \"\" to OpStatusManualSet ;\n \
        }\n \
        CompositeState Manual {\n \
        Initial  \"/manualMode:=1^UserInterface.setDisplayMode(2)\" MIdle ;\n \
        State DimmerValueSet  {\n \
                Transition \"ccAck\" to Done ;\n \
        }\n \
        State Done  {\n \
                Transition \"/manualMode:=0\" to NBInit ;\n \
        }\n \
        State MIdle  {\n \
                Transition \"ccAck^Dimmer.changeDimmerValue(1000)\" to DimmerValueSet ;\n \
        }\n \
        }\n \
        State ManualModeCheck  {\n \
                Transition \"ccTRUE\" to EnterManual ;\n \
                Transition \"ccFALSE\" to EnterAuto ;\n \
        }\n \
        State NBInit  {\n \
                Transition \"^UserInterface.isManualMode\" to ManualModeCheck ;\n \
        }\n \
        State OpStatusAutoSet  {\n \
                Transition \"\" to Automatic ;\n \
        }\n \
        State OpStatusManualSet  {\n \
                Transition \"\" to Manual ;\n \
        }\n \
        }\n \
        State PowerOff  {\n \
        }\n \
}\n \
Class Dimmer {\n \
        InstanceVar int deltaDimmerValue ;\n \
        InstanceVar int dimmerValue ;\n \
        InstanceVar bool operationalState ;\n \
        InstanceVar int tempDimmerValue ;\n \
        Signal changeDimmerValue(int ) ;\n \
        Signal dimmerAck( ) ;\n \
        Signal getOperationalState( ) ;\n \
        Initial  \"\" Idle ;\n \
        State DetermineOS  {\n \
                Transition \"/operationalState:=1^ComputingComponent.ccTRUE\" to Idle ;\n \
                Transition \"/operationalState:=0^ComputingComponent.ccFALSE\" to Idle ;\n \
        }\n \
        State DimmerValueSet  {\n \
                Transition \"dimmerAck^ComputingComponent.ccAck\" to Idle ;\n \
        }\n \
        State Idle  {\n \
                Transition \"getOperationalState\" to DetermineOS ;\n \
                Transition \"changeDimmerValue(deltaDimmerValue)/tempDimmerValue:=dimmerValue+deltaDimmerValue\" to TempValueReceived ;\n \
        }\n \
        State TempValueReceived  {\n \
                Transition \"[tempDimmerValue>1000]/dimmerValue:=1000\" to ValueProcessed ;\n \
                Transition \"[tempDimmerValue>=0 & tempDimmerValue<=1000]/dimmerValue:=tempDimmerValue\" to ValueProcessed ;\n \
                Transition \"[tempDimmerValue<0]/dimmerValue:=0\" to ValueProcessed ;\n \
        }\n \
        State ValueProcessed  {\n \
                Transition \"^Environment.setDimmerValue(dimmerValue)\" to DimmerValueSet ;\n \
        }\n \
}\n \
Class Environment {\n \
        InstanceVar int brightnessValue ;\n \
        InstanceVar int dimmerValue ;\n \
        InstanceVar bool motionValue ;\n \
        InstanceVar int totalBrightnessValue ;\n \
        Signal getBrightnessValue( ) ;\n \
        Signal isRoomOccupied( ) ;\n \
        Signal setDimmerValue(int ) ;\n \
        Initial  \"\" Idle ;\n \
        State BrightnessValueRequested  {\n \
                Transition \"/brightnessValue:=1100\" to BrightnessValueSelected ;\n \
                Transition \"/brightnessValue:=850\" to BrightnessValueSelected ;\n \
                Transition \"/brightnessValue:=0\" to BrightnessValueSelected ;\n \
                Transition \"/brightnessValue:=300\" to BrightnessValueSelected ;\n \
        }\n \
        State BrightnessValueSelected  {\n \
                Transition \"/totalBrightnessValue:=brightnessValue+dimmerValue\" to TotalValueComputed ;\n \
        }\n \
        State DimmerValueSet  {\n \
                Transition \"/totalBrightnessValue:=brightnessValue+dimmerValue^Dimmer.dimmerAck\" to Idle ;\n \
        }\n \
        State Idle  {\n \
                Transition \"setDimmerValue(dimmerValue)\" to DimmerValueSet ;\n \
                Transition \"isRoomOccupied\" to MotionValueRequested ;\n \
                Transition \"getBrightnessValue\" to BrightnessValueRequested ;\n \
        }\n \
        State MotionValueRequested  {\n \
                Transition \"/motionValue:=1^MotionSensor.msTRUE\" to MotionValueSelected ;\n \
                Transition \"/motionValue:=0^MotionSensor.msFALSE\" to MotionValueSelected ;\n \
        }\n \
        State MotionValueSelected  {\n \
                Transition \"\" to Idle ;\n \
        }\n \
        State TotalValueComputed  {\n \
                Transition \"^BrightnessSensor.setBrightnessValue(totalBrightnessValue)\" to Idle ;\n \
        }\n \
}\n \
Class MotionSensor {\n \
        InstanceVar bool motionValue ;\n \
        InstanceVar bool operationalState ;\n \
        Signal getOperationalState( ) ;\n \
        Signal isRoomOccupied( ) ;\n \
        Signal msFALSE( ) ;\n \
        Signal msTRUE( ) ;\n \
        Initial  \"\" Idle ;\n \
        State DetermineOS  {\n \
                Transition \"/operationalState:=1^ComputingComponent.ccTRUE\" to Idle ;\n \
                Transition \"/operationalState:=0^ComputingComponent.ccFALSE\" to Idle ;\n \
        }\n \
        State Idle  {\n \
                Transition \"getOperationalState\" to DetermineOS ;\n \
                Transition \"isRoomOccupied^Environment.isRoomOccupied\" to MotionValueRequested ;\n \
        }\n \
        State MotionValueRequested  {\n \
                Transition \"msFALSE^ComputingComponent.ccFALSE\" to Idle ;\n \
                Transition \"msTRUE^ComputingComponent.ccTRUE\" to Idle ;\n \
        }\n \
}\n \
Class UserInterface {\n \
        InstanceVar int desiredBrightnessValue ;\n \
        InstanceVar short displayMode ;\n \
        InstanceVar bool manualMode ;\n \
        Signal getDesiredBrightnessValue( ) ;\n \
        Signal isManualMode( ) ;\n \
        Signal setDisplayMode(short ) ;\n \
        Initial  \"\" DesiredValueRequested ;\n \
        State DesiredValueRequested  {\n \
                Transition \"/desiredBrightnessValue:=1000\" to Idle ;\n \
                Transition \"/desiredBrightnessValue:=500\" to Idle ;\n \
                Transition \"/desiredBrightnessValue:=0\" to Idle ;\n \
        }\n \
        State Idle  {\n \
                Transition \"isManualMode\" to State2 ;\n \
                Transition \"setDisplayMode(displayMode)\" to OperationalStatusSet ;\n \
                Transition \"getDesiredBrightnessValue\" to State1 ;\n \
        }\n \
        State OperationalStatusSet  {\n \
                Transition \"^ComputingComponent.ccAck\" to Idle ;\n \
        }\n \
        State State1  {\n \
                Transition \"^ComputingComponent.setDesiredBrightnessValue(desiredBrightnessValue)\" to Idle ;\n \
        }\n \
        State State2  {\n \
                Transition \"/manualMode:=1^ComputingComponent.ccTRUE\" to Idle ;\n \
                Transition \"/manualMode:=0^ComputingComponent.ccFALSE\" to Idle ;\n \
        }\n \
}\n \
\n \
}\n \ "; 
*/
	return;
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
