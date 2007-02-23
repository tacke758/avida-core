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
  
  triggers_it = triggers.begin();
  guards_it = guards.begin();
  actions_it = actions.begin();

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


// This sets the meaning of the transitions to those of the Multi-Sensor Example.

void cOrganism::initTransForMSXMI()
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
		default:
			(*it).second = " ";
	}
		count++;
	}	
}	
	

void cOrganism::initTransForBSXMI()
{

	// This particular assignment of transitions is designed to evolve the ability of Avida
	// to develop a function that checks for operational state...
	
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
			temp += "<UML:Transition.trigger> <UML:Event> <UML:ModelElement.namespace> <UML:Namespace> ";
			temp += "<UML:Namespace.ownedElement> <UML:CallEvent xmi.id=\"XDE-A28463C5-2F9F-457C-B6F3-241526CA4791\" ";
			temp += " operation=\"XDE-E84A5762-CA92-4E03-A237-FE5AE2C99D9A\" name=\"getBrightnessValue\" ";
			temp += "isSpecification=\"false\"/> </UML:Namespace.ownedElement> </UML:Namespace> ";
			temp += " </UML:ModelElement.namespace> </UML:Event> </UML:Transition.trigger> ";
			(*it).second =temp;
			break;
		case 2:
			temp = "";
			temp += "<UML:Transition.effect> <UML:UninterpretedAction xmi.id=\"XDE-6C3D3042-5C7A-4746-8A90-BEDB86FD2FF4\" ";
			temp += "isAsynchronous=\"false\" name=\"\" isSpecification=\"false\"> <UML:Action.script> ";
			temp += " <UML:ActionExpression language=\"\" body=\"^Environment.getBrightnessValue\"/> ";
			temp += "</UML:Action.script> </UML:UninterpretedAction> </UML:Transition.effect> ";
			(*it).second = temp;
			break;	
		case 3:
			temp = "";
			temp += "<UML:Transition.trigger> <UML:Event> <UML:ModelElement.namespace> <UML:Namespace> ";
			temp += " <UML:Namespace.ownedElement> <UML:CallEvent xmi.id=\"XDE-79243838-9C4E-4908-9637-9F9583043BE4\" ";
			temp += " operation=\"XDE-C8BD0DBA-E427-41A0-95F4-98FAA920ACA9\" name=\"setBrightnessValue\" ";
			temp += "isSpecification=\"false\"/> </UML:Namespace.ownedElement> </UML:Namespace> ";
			temp += " </UML:ModelElement.namespace> </UML:Event> </UML:Transition.trigger> ";
			(*it).second = temp;
			break;
		case 4:
			temp = "";
			temp += "<UML:Transition.guard> <UML:Guard> <UML:Guard.expression> ";
			temp += "<UML:BooleanExpression body=\"brightnessValue &gt;=0 &amp; brightnessValue&lt;=1000\" language=\"\"/>";
			temp += " </UML:Guard.expression> </UML:Guard> </UML:Transition.guard> <UML:Transition.effect> ";
			temp += " <UML:UninterpretedAction xmi.id=\"XDE-8E3B2DF6-D63B-4A70-9CD3-FF0DE13EEDAD\" ";
			temp += " isAsynchronous=\"false\" name=\"\" isSpecification=\"false\"> <UML:Action.script> ";
			temp += " <UML:ActionExpression language=\"\" body=\"correctedBrightnessValue:=brightnessValue\"/> ";
			temp += " </UML:Action.script> </UML:UninterpretedAction> </UML:Transition.effect> ";
			(*it).second = temp;
			break;
		case 5:
			temp = "";
			temp += "<UML:Transition.guard> <UML:Guard> <UML:Guard.expression> ";
			temp += "<UML:BooleanExpression body=\"brightnessValue&lt;0\" language=\"\"/> ";
			temp += " </UML:Guard.expression> </UML:Guard> </UML:Transition.guard>  <UML:Transition.effect> ";
			temp += " <UML:UninterpretedAction xmi.id=\"XDE-0B7A10EB-A9FC-4DE8-BBF1-AF1C9A970E7F\" ";
			temp += " isAsynchronous=\"false\" name=\"\" isSpecification=\"false\">  <UML:Action.script> ";
			temp += " <UML:ActionExpression language=\"\" body=\"correctedBrightnessValue:=0\"/> ";
			temp += " </UML:Action.script> </UML:UninterpretedAction> </UML:Transition.effect> ";
			(*it).second = temp;
			break;
		case 6:
			temp = "";
			temp += "<UML:Transition.guard> <UML:Guard> <UML:Guard.expression> ";
			temp += "<UML:BooleanExpression body=\"brightnessValue&gt;1000\" language=\"\"/> ";
			temp += " </UML:Guard.expression> </UML:Guard> </UML:Transition.guard>  <UML:Transition.effect> ";
			temp += " <UML:UninterpretedAction xmi.id=\"XDE-7D6DDE48-7568-4043-B00A-87EFBE1A6CB3\" ";
			temp += " isAsynchronous=\"false\" name=\"\" isSpecification=\"false\">  <UML:Action.script> ";
			temp += " <UML:ActionExpression language=\"\" body=\"correctedBrightnessValue:=1000\"/> ";
			temp += " </UML:Action.script> </UML:UninterpretedAction> </UML:Transition.effect> ";
			(*it).second = temp;
			break;			
		case 7:
			temp = "";
			temp += "<UML:Transition.effect> <UML:UninterpretedAction xmi.id=\"XDE-101E5C46-12EA-4169-9DC9-D3661EE9836B\" ";
			temp += "isAsynchronous=\"false\" name="" isSpecification=\"false\"> <UML:Action.script> ";
			temp += " <UML:ActionExpression language=\"\" body=\"^ComputingComponent.setBrightnessValue(brightnessValue)\"/> ";
			temp += " </UML:Action.script> </UML:UninterpretedAction> </UML:Transition.effect> ";
			(*it).second = temp;
			break;	
		case 8:
			temp = "";
			temp += "<UML:Transition.trigger> <UML:Event> <UML:ModelElement.namespace> <UML:Namespace> ";
			temp += "<UML:Namespace.ownedElement> <UML:CallEvent xmi.id=\"XDE-7126ED39-5D5D-4160-924B-303514B17EAB\" ";
			temp += " operation=\"XDE-1266DA8A-61C0-43B4-A77C-200F54A6585D\" name=\"getOperationalState\"";
			temp += " isSpecification=\"false\"/> </UML:Namespace.ownedElement> </UML:Namespace> ";
			temp += " </UML:ModelElement.namespace> </UML:Event> </UML:Transition.trigger>";
			(*it).second = temp;
			break;
		case 9:
			temp = "";
			temp += "<UML:Transition.effect> <UML:UninterpretedAction xmi.id=\"XDE-D9BCD8D1-7FC4-4B14-9E76-D3A642799013\"";
			temp += " isAsynchronous=\"false\" name=\"\" isSpecification=\"false\"> <UML:Action.script> ";
			temp += "<UML:ActionExpression language=\"\" body=\"operationalState:=1;^ComputingComponent.ccTRUE\"/> ";
			temp += "</UML:Action.script> </UML:UninterpretedAction> </UML:Transition.effect>";
			(*it).second = temp;
			break;
		case 10: 
			temp = "";
			temp += "<UML:Transition.effect> <UML:UninterpretedAction xmi.id=\"XDE-9F00136E-D61D-4BB0-B7D6-1E795238FD1E\"";
			temp += " isAsynchronous=\"false\" name=\"\" isSpecification=\"false\"> <UML:Action.script> ";
			temp += " <UML:ActionExpression language=\"\" body=\"operationalState:=0;^ComputingComponent.ccFALSE\"/> ";
			temp += "</UML:Action.script>  </UML:UninterpretedAction> </UML:Transition.effect> ";
			(*it).second = "";
			break;
	
		default:
			(*it).second = " ";
	}
		count++;
	}

/*	// This particular set of commented out transitions is designed to test the ability of 
	// Avida to evolve the min/max functions that check value. 
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
			temp += "<UML:Transition.trigger> <UML:Event> <UML:ModelElement.namespace> <UML:Namespace> ";
			temp += "<UML:Namespace.ownedElement> <UML:CallEvent xmi.id=\"XDE-7126ED39-5D5D-4160-924B-303514B17EAB\" ";
			temp += " operation=\"XDE-1266DA8A-61C0-43B4-A77C-200F54A6585D\" name=\"getOperationalState\"";
			temp += " isSpecification=\"false\"/> </UML:Namespace.ownedElement> </UML:Namespace> ";
			temp += " </UML:ModelElement.namespace> </UML:Event> </UML:Transition.trigger>";
			(*it).second = temp;
			break;
		case 2:
			temp = "";
			temp += "<UML:Transition.effect> <UML:UninterpretedAction xmi.id=\"XDE-D9BCD8D1-7FC4-4B14-9E76-D3A642799013\"";
			temp += " isAsynchronous=\"false\" name=\"\" isSpecification=\"false\"> <UML:Action.script> ";
			temp += "<UML:ActionExpression language=\"\" body=\"operationalState:=1;^ComputingComponent.ccTRUE\"/> ";
			temp += "</UML:Action.script> </UML:UninterpretedAction> </UML:Transition.effect>";
			(*it).second = temp;
			break;
		case 3: 
			temp = "";
			temp += "<UML:Transition.effect> <UML:UninterpretedAction xmi.id=\"XDE-9F00136E-D61D-4BB0-B7D6-1E795238FD1E\"";
			temp += " isAsynchronous=\"false\" name=\"\" isSpecification=\"false\"> <UML:Action.script> ";
			temp += " <UML:ActionExpression language=\"\" body=\"operationalState:=0;^ComputingComponent.ccFALSE\"/> ";
			temp += "</UML:Action.script>  </UML:UninterpretedAction> </UML:Transition.effect> ";
			(*it).second = "";
			break;
		case 4: 
			temp = "";
			temp += "<UML:Transition.trigger> <UML:Event> <UML:ModelElement.namespace> <UML:Namespace> ";
			temp += "<UML:Namespace.ownedElement> <UML:CallEvent xmi.id=\"XDE-A28463C5-2F9F-457C-B6F3-241526CA4791\" ";
			temp += " operation=\"XDE-E84A5762-CA92-4E03-A237-FE5AE2C99D9A\" name=\"getBrightnessValue\" ";
			temp += "isSpecification=\"false\"/> </UML:Namespace.ownedElement> </UML:Namespace> ";
			temp += " </UML:ModelElement.namespace> </UML:Event> </UML:Transition.trigger> ";
			(*it).second =temp;
			break;
		case 5:
			temp = "";
			temp += "<UML:Transition.effect> <UML:UninterpretedAction xmi.id=\"XDE-6C3D3042-5C7A-4746-8A90-BEDB86FD2FF4\" ";
			temp += "isAsynchronous=\"false\" name=\"\" isSpecification=\"false\"> <UML:Action.script> ";
			temp += " <UML:ActionExpression language=\"\" body=\"^Environment.getBrightnessValue\"/> ";
			temp += "</UML:Action.script> </UML:UninterpretedAction> </UML:Transition.effect> ";
			(*it).second = temp;
			break;	
		case 6:
			temp = "";
			temp += "<UML:Transition.trigger> <UML:Event> <UML:ModelElement.namespace> <UML:Namespace> ";
			temp += " <UML:Namespace.ownedElement> <UML:CallEvent xmi.id=\"XDE-79243838-9C4E-4908-9637-9F9583043BE4\" ";
			temp += " operation=\"XDE-C8BD0DBA-E427-41A0-95F4-98FAA920ACA9\" name=\"setBrightnessValue\" ";
			temp += "isSpecification=\"false\"/> </UML:Namespace.ownedElement> </UML:Namespace> ";
			temp += " </UML:ModelElement.namespace> </UML:Event> </UML:Transition.trigger> ";
			(*it).second = temp;
			break;
		case 7:
			temp = "";
			temp += "<UML:Transition.guard> <UML:Guard> <UML:Guard.expression> ";
			temp += "<UML:BooleanExpression body=\"brightnessValue &gt;=0 &amp; brightnessValue&lt;=1000\" language=\"\"/>";
			temp += " </UML:Guard.expression> </UML:Guard> </UML:Transition.guard> <UML:Transition.effect> ";
			temp += " <UML:UninterpretedAction xmi.id=\"XDE-8E3B2DF6-D63B-4A70-9CD3-FF0DE13EEDAD\" ";
			temp += " isAsynchronous=\"false\" name=\"\" isSpecification=\"false\"> <UML:Action.script> ";
			temp += " <UML:ActionExpression language=\"\" body=\"correctedBrightnessValue:=brightnessValue\"/> ";
			temp += " </UML:Action.script> </UML:UninterpretedAction> </UML:Transition.effect> ";
			(*it).second = temp;
			break;
		case 8:
			temp = "";
			temp += "<UML:Transition.effect> <UML:UninterpretedAction xmi.id=\"XDE-101E5C46-12EA-4169-9DC9-D3661EE9836B\" ";
			temp += "isAsynchronous=\"false\" name="" isSpecification=\"false\"> <UML:Action.script> ";
			temp += " <UML:ActionExpression language=\"\" body=\"^ComputingComponent.setBrightnessValue(brightnessValue)\"/> ";
			temp += " </UML:Action.script> </UML:UninterpretedAction> </UML:Transition.effect> ";
			(*it).second = temp;
			break;
		case 9:
			temp = "";
			temp += "<UML:Transition.guard> <UML:Guard> <UML:Guard.expression> ";
			temp += "<UML:BooleanExpression body=\"brightnessValue&lt;0\" language=\"\"/> ";
			temp += " </UML:Guard.expression> </UML:Guard> </UML:Transition.guard>  <UML:Transition.effect> ";
			temp += " <UML:UninterpretedAction xmi.id=\"XDE-0B7A10EB-A9FC-4DE8-BBF1-AF1C9A970E7F\" ";
			temp += " isAsynchronous=\"false\" name=\"\" isSpecification=\"false\">  <UML:Action.script> ";
			temp += " <UML:ActionExpression language=\"\" body=\"correctedBrightnessValue:=0\"/> ";
			temp += " </UML:Action.script> </UML:UninterpretedAction> </UML:Transition.effect> ";
			(*it).second = temp;
			break;
		case 10:
			temp = "";
			temp += "<UML:Transition.guard> <UML:Guard> <UML:Guard.expression> ";
			temp += "<UML:BooleanExpression body=\"brightnessValue&gt;1000\" language=\"\"/> ";
			temp += " </UML:Guard.expression> </UML:Guard> </UML:Transition.guard>  <UML:Transition.effect> ";
			temp += " <UML:UninterpretedAction xmi.id=\"XDE-7D6DDE48-7568-4043-B00A-87EFBE1A6CB3\" ";
			temp += " isAsynchronous=\"false\" name=\"\" isSpecification=\"false\">  <UML:Action.script> ";
			temp += " <UML:ActionExpression language=\"\" body=\"correctedBrightnessValue:=1000\"/> ";
			temp += " </UML:Action.script> </UML:UninterpretedAction> </UML:Transition.effect> ";
			(*it).second = temp;
			break;			
		default:
			(*it).second = " ";
	}
		count++;
	}
	
*/			
}	
	


bool cOrganism::addTrans(int trans, int orig, int dest) 
{
	NameStateMap::iterator pos1, pos2;
	bool inserted1, inserted2, inserted3;
	bool exists = 0;
	State u, v;


	// create or find origin state
	tie(pos1, inserted1) = states.insert(std::make_pair(orig, State()));
        if (inserted1) {
                u = add_vertex(uml_state_diagram);
                uml_state_diagram[u].state_label = orig;
                pos1->second = u;
				
				// add to the map between position and label.... 
				PosToStateLabel[states.size()] = orig;
				
        } else  {
                u = pos1->second;
        }

	// create or find dest state
	tie(pos2, inserted2) = states.insert(std::make_pair(dest, State()));
        if (inserted2) {
                v = add_vertex(uml_state_diagram);
                uml_state_diagram[v].state_label = dest;
                pos2->second = v;
				
				// add to map betweek positions and label.....
				PosToStateLabel[states.size()] = orig;

				
        } else  {
                v = pos2->second;
        }
		// call isTrans...	
		exists = isTrans(u, v, trans);
		if (exists == 0) {
			tie(transitions, inserted3) = add_edge(u, v, uml_state_diagram);
			if (inserted3) {
				uml_state_diagram[transitions].edge_label = trans;
				uml_state_diagram[transitions].start_state = orig;
				uml_state_diagram[transitions].end_state = dest;
		
				// add trans to table...
				transGuardActionInfo.insert(std::make_pair(trans, ""));
			}
		}
	return true;
}


bool cOrganism::addTransConnect(int trans, int orig, int dest) 
{
	// find either the orig int or the dest int in the map of states & names
	
	// originally thought that either orig or dest could be connected, but actually, must be orig.
	if ((states.find(orig) == states.end()) && (numStates()>0)) 
	{
		return false;
	}
	
	return (addTrans(trans,orig,dest));
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
	
	if ((numStates() <= s0_pos) || (numStates() <= s1_pos)) {
		return found_entry;
	}
	
	cOrganism::State& s0 = getStateInPosition(s0_pos);
	cOrganism::State& s1 = getStateInPosition(s1_pos);

	if (isTrans(s0, s1, trans_name)) { 
		found_entry = 1;
	} 
	
	return found_entry;

}


void cOrganism::printXMI(cAvidaContext& ctx)
{

	Graph::vertex_iterator i, iend;
	Graph::edge_iterator e, eend;
	
	int trans_label;
	int dest_state;
	std::string temp, temp1, temp2;
	int tempint;
	
	initTransForMSXMI();

	xmi = "";
	// loop through all states
	// print initial information for the init state.

	tie(i, iend) = vertices(uml_state_diagram);
	
	if (numStates() > 0) {
		temp = stringifyAnInt(uml_state_diagram[0].state_label);
		xmi += "<UML:Pseudostate xmi.id=\"s" + temp + "\" kind=\"initial\" outgoing=\"\" name=\"s";
		xmi += temp + "\" isSpecification=\"false\"/>\n";
		++i;
	}
	
	
	for (; i != iend; ++i) {
		temp = "s" + stringifyAnInt(uml_state_diagram[*i].state_label);
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



	for (tie(e, eend) = edges(uml_state_diagram); e != eend; ++e) { 
		// info determined from the trans itself....
		trans_label = uml_state_diagram[*e].edge_label;
		temp = "t" + stringifyAnInt(uml_state_diagram[*e].edge_label);
		temp1 = "s" + stringifyAnInt(uml_state_diagram[*e].start_state);
		temp2 = "s" + stringifyAnInt(uml_state_diagram[*e].end_state);
		temp = temp + temp1 + temp2;
		// if I manage to set edge_info, I could then use that to print...
		// currently the start state and end state are already encoded. :)

		xmi+= "<UML:Transition xmi.id=\"" + temp + "\"";
		xmi+= " source=\"" + temp1 + "\"";
		xmi += " target=\"" + temp2 + "\" name=\"\" isSpecification=\"false\">\n";
		
		
		temp = transGuardActionInfo[trans_label];
		xmi += temp;

		xmi += "</UML:Transition>\n";

	
	}
}



// print the label. Change - signs to _
std::string cOrganism::stringifyAnInt(int x) { 

	std::ostringstream o;

	if (x < 0) {
		x = abs(x);
		o << "_";
	} 
	
	o << x;
	return o.str();
}

// return the number of states in a state diagram
double cOrganism::numStates() 
{
	return (double) num_vertices(uml_state_diagram);
}

double cOrganism::numTrans()
{
	return (double) num_edges(uml_state_diagram);
}

double cOrganism::numUniqueTransLabels() 
{
	return transGuardActionInfo.size();
}

std::string cOrganism::getXMI()
{
//	std::string temp = xmi_begin + xmi + xmi_end;
//	cout << "PRINT XMI" << endl;
//	cout << temp << endl;
//	cout << "END PRINT XMI" << endl;
	return (xmi_begin + xmi + xmi_end);
}

// This function takes in an integer and returns the state in that position within the states map. Recall that this
// map relates integers (the number assigned to a state) to a state in the graph.
// ... Potential candidate for optimization as per DBK ... 

cOrganism::State& cOrganism::getStateInPosition (int num)
{
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
	
}




// if you ask for something greater than the number of states then you get the highest numbered state.
int cOrganism::getStateLabelInPosition (int num)
{

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

	return i->first;
	
}

cOrganism::Graph& cOrganism::getGraph()
{
	return uml_state_diagram;
}

bool cOrganism::nextTrigger()
{
	triggers_it++;
	if (triggers_it == triggers.end()){
		triggers_it = triggers.begin();
	}
}

bool cOrganism::prevTrigger()
{
	if (triggers_it == triggers.begin()){
		triggers_it = triggers.end();
	} 
	triggers_it--;
}

bool cOrganism::nextGuard()
{
	guards_it++;
	if (guards_it == guards.end()){
		guards_it = guards.begin();
	}
}

bool cOrganism::prevGuard()
{
	if (guards_it == guards.begin()){
		guards_it = guards.end();
	} 
	guards_it--;
}

bool cOrganism::nextAction()
{	
	actions_it++;
	if (actions_it == actions.end()){
		actions_it = actions.begin();
	}
}

bool cOrganism::prevAction()
{
	if (actions_it == actions.begin()){
		actions_it = actions.end();
	} 
	actions_it--;
}

std::string cOrganism::getTrigger()
{
	std::string tmp;
	if (triggers.size() > 0) {
		tmp = *triggers_it;
	} else {
		tmp = ""; 
	}
	return tmp;
}

std::string cOrganism::getGuard()
{
	std::string tmp;
	if (guards.size() > 0) {
		tmp = *guards_it;
	} else {
		tmp = ""; 
	}
	return tmp;
}

std::string cOrganism::getAction()
{
	std::string tmp;
	if (actions.size() > 0) {
		tmp = *actions_it;
	} else {
		tmp = ""; 
	}
	return tmp;
}

bool cOrganism::crTransLabel()
{
//	std::string tmp_trigger = getTrigger();
//	std::string tmp_guard = getGuard();
//	std::string tmp_action = getAction();
	std::string tmp_label = getTrigger() + getGuard() + getAction();
//	  std::vector<std::string> transition_labels;
	transition_labels.push_back(tmp_label);

	return true;
}

bool cOrganism::isTransLabel(std::string desired_lab)
{
	std::vector<std::string>::iterator tr_lab_it;
	for(tr_lab_it = transition_labels.begin(); tr_lab_it != transition_labels.end(); tr_lab_it++)
	{
			if (desired_lab == *tr_lab_it) {
				return true;
			}
	} 
	return false;
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
