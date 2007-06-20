#include "cUMLStateDiagram.h"

cUMLStateDiagram::cUMLStateDiagram()
{

  // initialize / seed UML state diagram here
  orig_state_index = 0;
  dest_state_index = 0;
  trans_label_index = 0;
  trigger_index = 0;
  guard_index = 0;
  action_index = 0;

  xmi = "";

   
  // initialize w/ 10 states
  
  state s;
  for (int i=0; i<11; i++) {
	s.identifier = i;
	s.num_incoming = 0;
	s.num_outgoing = 0;
	states.push_back(s);
  }
   
}

cUMLStateDiagram::~cUMLStateDiagram()
{
}


/*
bool cUMLStateDiagram::findTransLabel(transition_label t) { 
	for(std::vector<transition_label>::iterator i=transition_labels.begin(); i!=transition_labels.end(); ++i){
		if ((i->trigger == t.trigger) && (i->guard == t.guard) && (i->action == t.action)) {
			return true;
		}
	}
	return false;
}



bool cUMLStateDiagram::findTrans(int orig, int dest, std::string tr, std::string gu, std::string act) 
{
	int tracker = 0;
	
	for(std::vector<trigger_info>::iterator i=triggers.begin(); i!=triggers.end(); i++) {
		if (tr == i->label) { 
			return findTrans(orig, dest, tracker, gu, act );
		}
		tracker++;
	}

	return false;
}

// Check to see if i->trans.trigger is ever equal to trig
bool cUMLStateDiagram::findTrans(int orig, int dest, int trig, std::string gu, std::string act) 
{
	// the wild cards for there are 
	// -1 for orig, dest & trigger
	// "*" for guard, and action

	for(std::vector<transition>::iterator i=transitions.begin(); i!=transitions.end(); ++i){
		if (((orig == -1) || (orig == i->orig_state)) && 
			((dest == -1) || (dest == i->dest_state)) && 
			((trig == -1) || (trig == i->trans.trigger)) && 
			((gu == "*") || (gu == i->trans.guard)) &&
			((act == "*") || (act == i->trans.action))) { 
						return true;
			}
	}
	return false;
}
*/

bool cUMLStateDiagram::findTrans(int orig, int dest, int trig, int gu, int act) 
{
	for(std::vector<transition>::iterator i=transitions.begin(); i!=transitions.end(); ++i){
		if (((orig == -1) || (orig == i->orig_state)) && 
			((dest == -1) || (dest == i->dest_state)) && 
			((trig == -1) || (trig == i->trans.trigger)) && 
			((gu == -1) || (gu == i->trans.guard)) &&
			((act == -1) || (act == i->trans.action))) { 
						return true;
			}
	}
	return false;

}


template <typename T>
bool cUMLStateDiagram::absoluteMoveIndex (T x, int &index, int amount )
{

	if (x.size() == 0 || amount > x.size()) {
		return false;
	}
	
	index = 0;
	return relativeMoveIndex(x, index, amount);
}

template <typename T>
bool cUMLStateDiagram::relativeMoveIndex (T x, int &index, int amount )
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



bool cUMLStateDiagram::absoluteJumpTrigger(int jump_amount)
{
	return absoluteMoveIndex(triggers, trigger_index, jump_amount);
}

bool cUMLStateDiagram::absoluteJumpGuard(int jump_amount)
{
	return absoluteMoveIndex(guards, guard_index, jump_amount);	
}

bool cUMLStateDiagram::absoluteJumpAction(int jump_amount)
{
	return absoluteMoveIndex(actions, action_index, jump_amount);
}

bool cUMLStateDiagram::absoluteJumpTransitionLabel(int jump_amount)
{
	return absoluteMoveIndex(transition_labels, trans_label_index, jump_amount);
}

bool cUMLStateDiagram::absoluteJumpOriginState(int jump_amount) 
{
	return absoluteMoveIndex(states, orig_state_index, jump_amount);
}

bool cUMLStateDiagram::absoluteJumpDestinationState(int jump_amount) 
{
	return absoluteMoveIndex(states, dest_state_index, jump_amount);
}

/*
bool cUMLStateDiagram::relativeJumpTrigger(int jump_amount)
{
	return relativeMoveIndex(triggers, trigger_index, jump_amount);
}

bool cUMLStateDiagram::relativeJumpGuard(int jump_amount)
{
	return relativeMoveIndex(guards, guard_index, jump_amount);	
}

bool cUMLStateDiagram::relativeJumpAction(int jump_amount)
{
	return relativeMoveIndex(actions, action_index, jump_amount);
}

bool cUMLStateDiagram::relativeJumpTransitionLabel(int jump_amount)
{
	return relativeMoveIndex(transition_labels, trans_label_index, jump_amount);
}

bool cUMLStateDiagram::relativeJumpOriginState(int jump_amount) 
{
	return relativeMoveIndex(states, orig_state_index, jump_amount);
}

bool cUMLStateDiagram::relativeJumpDestinationState(int jump_amount) 
{
	return relativeMoveIndex(states, dest_state_index, jump_amount);
}
*/

int cUMLStateDiagram::getTriggerIndex()
{
	return trigger_index;
}

int cUMLStateDiagram::getGuardIndex()
{
		return guard_index;
}

int cUMLStateDiagram::getActionIndex()
{
		return action_index;
	
}

int cUMLStateDiagram::getOrigStateIndex()
{
	return orig_state_index;
}
 
int cUMLStateDiagram::getDestStateIndex()
{
	return dest_state_index;
}

transition_label cUMLStateDiagram::getTransLabel()
{
	return transition_labels[trans_label_index];
}

bool cUMLStateDiagram::addState()
{	
	state s;
	s.identifier = states.size();
	s.num_incoming = 0;
	s.num_outgoing = 0;
	states.push_back(s);
	
	return true;
}

bool cUMLStateDiagram::addTrigger(std::string op_id, std::string lab) 
{
	trigger_info t;
	t.operation_id = op_id;
	t.label = lab;
	
	triggers.push_back(t);
	
	return true;
}

bool cUMLStateDiagram::addGuard(std::string gu) 
{
	guards.push_back(gu);
	return true;
}

bool cUMLStateDiagram::addAction(std::string act)
{
	actions.push_back(act);
	return true;
}

/*
// Broken - 5/17
bool cUMLStateDiagram::addTransitionLabel()
{
	transition_label t;
	t.trigger = getTriggerIndex();
	t.guard = getGuard();
	t.action = getAction();
	
	// no dupes
	if (findTransLabel(t)){
		return false;
	 }
	
	transition_labels.push_back(t);
	
	// Move the transition label index to the most recently created
	trans_label_index = transition_labels.size() - 1;

	return false;
}
*/


bool cUMLStateDiagram::addTransition()
{
	
	if ((states.size() == 0) || (transition_labels.size() == 0)) {

		return false;
	} 

	transition t;
	t.orig_state = getOrigStateIndex();
	t.dest_state = getDestStateIndex();
	// increment number of edges for a state
	states[getOrigStateIndex()].num_outgoing += 1;
	states[getDestStateIndex()].num_incoming += 1;
	
	t.trans = getTransLabel();
	
	if ((t.orig_state != 0) && (states[t.orig_state].num_incoming == 0)) {
		return false;
	}
	
	// no dupes
    if (findTrans(t.orig_state, t.dest_state, t.trans.trigger, t.trans.guard, t.trans.action)) {
		return false;
	}

	transitions.push_back(t);

	orig_state_index = 0;
	dest_state_index = 0;
	trans_label_index = 0;
	trigger_index = 0;
	guard_index = 0 ;
	action_index = 0;
	
	return true;

}

bool cUMLStateDiagram::addTransitionLabel(int tr, int gu, int act)
{
	transition_label t;
	t.trigger = tr;
	t.guard = gu;
	t.action = act;

	// currently, not checking for dupes, since we are seeding the labels.
	
	transition_labels.push_back(t);
}




bool cUMLStateDiagram::addTransitionTotal(int o, int d, int t, int g, int a)
{
	if ((states.size() == 0)) {

		return false;
	} 
		
	absoluteMoveIndex(states, orig_state_index, o);
	absoluteMoveIndex(states, dest_state_index, d);
	absoluteMoveIndex(triggers, trigger_index, t);
	absoluteMoveIndex(guards, guard_index, g);
	absoluteMoveIndex(actions, action_index, a);

	transition trany;
	trany.orig_state = getOrigStateIndex();
	trany.dest_state = getDestStateIndex();
	
	
	// Do not create transition if the origin state is unreachable.
// Taken out on 5/22
	if ((trany.orig_state != 0) && (states[trany.orig_state].num_incoming == 0)) {
		return false;
	}
	
	// increment number of edges for a state
	states[getOrigStateIndex()].num_outgoing += 1;
	states[getDestStateIndex()].num_incoming += 1;

	
	transition_label tl;
	tl.trigger = getTriggerIndex();
	tl.guard = getGuardIndex();
	tl.action = getActionIndex();
	trany.trans = tl;
	
	
	// no dupes
    if (findTrans(trany.orig_state, trany.dest_state, trany.trans.trigger, trany.trans.guard, trany.trans.action)) {
		return false;
	}
	

	transitions.push_back(trany);
	
	
/*	// reset all indices
	orig_state_index = 0;
	dest_state_index = 0;
	trigger_index = 0;
	action_index = 0;
	guard_index = 0;*/
		
	return true;

}


bool cUMLStateDiagram::addTransitionTotal()
{
	if ((states.size() == 0)) {

		return false;
	} 
		
	transition trany;
	trany.orig_state = getOrigStateIndex();
	trany.dest_state = getDestStateIndex();
	
	
	// Do not create transition if the origin state is unreachable.
// Taken out on 5/22
	if ((trany.orig_state != 0) && (states[trany.orig_state].num_incoming == 0)) {
		return false;
	}
	
	// increment number of edges for a state
	states[getOrigStateIndex()].num_outgoing += 1;
	states[getDestStateIndex()].num_incoming += 1;

	
	transition_label tl;
	tl.trigger = getTriggerIndex();
	tl.guard = getGuardIndex();
	tl.action = getActionIndex();
	trany.trans = tl;
	
	
	// no dupes
    if (findTrans(trany.orig_state, trany.dest_state, trany.trans.trigger, trany.trans.guard, trany.trans.action)) {
		return false;
	}
	

	transitions.push_back(trany);
	
	
	// reset all indices
	orig_state_index = 0;
	dest_state_index = 0;
	trigger_index = 0;
	action_index = 0;
	guard_index = 0;
		
	return true;

}


bool cUMLStateDiagram::currTrans(int orig, int dest, int tr, int gu, int act)
{

	if (((orig == -1) || (orig == getOrigStateIndex())) &&
		((dest == -1) || (dest == getDestStateIndex())) && 
		((tr == -1)  || (tr == getTriggerIndex())) &&
		((gu == -1)  || (gu == getGuardIndex())) &&
		((act == -1) || (act == getActionIndex()))) { 

			return true;
	}
	
	return false;

}



int cUMLStateDiagram::numStates()
{
	return states.size();
}

int cUMLStateDiagram::numTrans()
{
	return transitions.size();
}

// print the label. Change - signs to _
std::string cUMLStateDiagram::StringifyAnInt(int x) { 

	std::ostringstream o;

	if (x < 0) {
		x = abs(x);
		o << "_";
	} 
	
	o << x;
	return o.str();
}

std::string cUMLStateDiagram::getXMI()
{
	printXMI();
	return (xmi);
}

void cUMLStateDiagram::printXMI()
{
	std::string temp, temp1, temp2, temp3;
	std::string trig_label, trig_op_name;

	int s_count = 0;
	int t_count = 0;
	xmi = "";

	// This state is the initial state; thus it should be printed regardless of whether it has an incoming
	// edge or not.
	if (numStates() > 0) {
		temp = StringifyAnInt(s_count);
		xmi += "<UML:Pseudostate xmi.id=\"s" + temp + "\" kind=\"initial\" outgoing=\"\" name=\"s";
		xmi += temp + "\" isSpecification=\"false\"/>\n";
		++s_count;
	}
	
	for (; s_count < numStates(); ++s_count) {
	
		// only print if this state has an incoming edge. 
//		if ((states[s_count]).num_incoming > 0) {
			temp = "s" + StringifyAnInt(s_count);
			xmi+="<UML:CompositeState xmi.id=\"";
			xmi+=temp;
			xmi+= "\" isConcurrent=\"false\" name=\""; 
			xmi+= temp; 
			xmi+= "\" isSpecification=\"false\"/>\n";
//		}
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
		temp3 = temp + temp1 + temp2;

		xmi+= "<UML:Transition xmi.id=\"" + temp3 + "\"";
		xmi+= " source=\"" + temp1 + "\"";
		xmi += " target=\"" + temp2 + "\" name=\"\" isSpecification=\"false\">\n";

		// Get guard, trigger, and action
//		temp = transitions[t_count].trans.trigger;
		temp1 = guards[transitions[t_count].trans.guard];
		temp2 = actions[transitions[t_count].trans.action];
		trig_label = triggers[transitions[t_count].trans.trigger].label;
		trig_op_name = triggers[transitions[t_count].trans.trigger].operation_id;


		// print trigger, if any
		if (trig_label != "<null>") {
			xmi+= "<UML:Transition.trigger> <UML:Event> <UML:ModelElement.namespace> <UML:Namespace> ";
			xmi+= "<UML:Namespace.ownedElement> <UML:CallEvent xmi.id=\"" + temp3;
			xmi+= "tt\"  operation=\""+ trig_label + "\" ";
			xmi+= "name=\"" + trig_op_name + "\" isSpecification=\"false\"/> "; 
			xmi+= "</UML:Namespace.ownedElement> </UML:Namespace> </UML:ModelElement.namespace> ";
			xmi+= "</UML:Event>  </UML:Transition.trigger> ";
		}
		
		// print guard, if any
		// Note: for guard to work, '<' => '&lt'
		if (temp1 != "<null>"){
			xmi+= "<UML:Transition.guard> <UML:Guard> <UML:Guard.expression> ";
			xmi+= "<UML:BooleanExpression body=\"" + temp1 + "\" language=\"\"/> ";
			xmi+= "</UML:Guard.expression> </UML:Guard> </UML:Transition.guard> ";
		}
		
		// print action, if any
		if (temp2 != "<null>") { 
			xmi+= "<UML:Transition.effect> <UML:UninterpretedAction xmi.id=\"" + temp3 + "ui\" ";
			xmi+= "isAsynchronous=\"false\" name=\"\" isSpecification=\"false\"> ";
			xmi+= "<UML:Action.script> <UML:ActionExpression language=\"\" body=\""; 
			xmi+= temp2 + "\"/> </UML:Action.script> </UML:UninterpretedAction> </UML:Transition.effect> ";		
		}
		
		xmi += "</UML:Transition>\n";

	
	}

	return;
}
