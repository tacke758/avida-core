#include "cUMLStateDiagram.h"

cUMLStateDiagram::cUMLStateDiagram()
{

  // initialize the state diagram with 10 states.
  sd0 = state_diagram(10); 

  // initialize / seed UML state diagram here
//  orig_state_index = 0;
//  dest_state_index = 0;
	orig = vertex(0, sd0);
	dest = vertex(0, sd0);
  
  trans_label_index = 0;
  trigger_index = 0;
  guard_index = 0;
  action_index = 0;

  xmi = "";
    
}

cUMLStateDiagram::~cUMLStateDiagram()
{
}

float cUMLStateDiagram::findPath(std::deque<std::string> p) { 
	bool res = false;
	int num_vert = num_vertices(sd0);
	std::deque<std::string> p_temp = p;
	int possible_length = p.size();
	int current_length = 0; 
	int len = 0;
	float val;
	
	while (!p.empty()) {
		for (int i = 0; i<num_vert; i++) { 
			checkForPath(p_temp, vertex(i, sd0), res, len);
			// check to see if this path is longer than other paths....
			if (len > current_length) { 
				current_length = len;
			}
		}
		p.pop_front();
		p_temp = p;
		res = false;
		len = 0;
	}	
	
//	std::cout << "Longest path length: " << current_length << std::endl;
//	std::cout << "Possible path length: " << possible_length << std::endl;
	val = current_length / possible_length;
	return (val);
}


void cUMLStateDiagram::checkForPath(std::deque<std::string> path, 
	boost::graph_traits<state_diagram>::vertex_descriptor v, bool& result, 
	int& dist) { 
	
	// declare temp strings for triggers, guards, and actions
	std::string tt, tg, ta, ts;
	
	// If the path is empty or the result is already true, then 
	// return true to indicate that the path has been found.
	if ((path.empty()) || (result == true)) { 
		result = true;
		return;
	}
	
	// Get all outgoing edges 
	boost::graph_traits<state_diagram>::out_edge_iterator out_edge_start, out_edge_end;
	boost::graph_traits<state_diagram>::edge_descriptor ed;

	// Check to see if any have the same label as the first element of path
	// If so, then call this function on them with the tail of the path
	for(tie(out_edge_start, out_edge_end) = out_edges(v, sd0);
			 out_edge_start != out_edge_end; ++out_edge_start) { 
			ed = *out_edge_start;
			tt = triggers[(sd0[ed]._tr)].operation_id;
			tg = guards[(sd0[ed]._gu)];
			ta = actions[(sd0[ed]._act)];
			
			// eliminate nulls
			if (tt == "<null>") tt = "";
			if (tg == "<null>") tg = "";
			if (ta == "<null>") ta = "";
			
			ts = tt + tg + ta;
//			std::cout << "Looking for an edge with " << path.front() << std::endl;
//			std::cout << "Found an edge with " << ts << std::endl;
			
			if (ts == path.front()) { 
//				std::cout << "Found an edge with " << ts << std::endl;
				dist++;
				path.pop_front();
				checkForPath(path, target(ed, sd0), result, dist);	
			}
		}	
}


bool cUMLStateDiagram::findTrans(int origin, int destination, int trig, int gu, int act) 
{

	bool result = false;
	
	boost::graph_traits<state_diagram>::vertex_descriptor o_temp, d_temp;
	boost::graph_traits<state_diagram>::edge_iterator edge_start, edge_end;
	boost::graph_traits<state_diagram>::out_edge_iterator out_edge_start, out_edge_end;
	boost::graph_traits<state_diagram>::in_edge_iterator in_edge_start, in_edge_end;
	boost::graph_traits<state_diagram>::edge_descriptor ed;

	// Get the set of transitions.
	if ((origin == -1) && (destination == -1)) { 
		for (tie(edge_start, edge_end) = edges(sd0);
			 edge_start != edge_end; ++edge_start) { 
			ed = *edge_start;
			if (((trig == -1)	|| (sd0[ed]._tr == trig))		&& 
				((gu == -1)		|| (sd0[ed]._gu == gu))			&& 
				((act == -1)	|| (sd0[ed]._act == act)))  { 
				result = true; 
				break;
			}	
		}
	} else if (origin == -1 && destination != -1) { 
		if (destination > num_vertices(sd0)) return result;
		d_temp = vertex(destination, sd0);
		for (tie(in_edge_start, in_edge_end) = in_edges(d_temp, sd0);
			 in_edge_start != in_edge_end; ++in_edge_start) { 
			ed = *in_edge_start;
			if (((trig == -1)	|| (sd0[ed]._tr == trig))		&& 
				((gu == -1)		|| (sd0[ed]._gu == gu))			&& 
				((act == -1)	|| (sd0[ed]._act == act)))  { 
				result = true; 
				break;
			}	
		}
	} else if (origin != -1 && destination == -1) { 
		if (origin > num_vertices(sd0)) return result;
		o_temp = vertex(origin, sd0);

		for(tie(out_edge_start, out_edge_end) = out_edges(o_temp, sd0);
			 out_edge_start != out_edge_end; ++out_edge_start) { 
			ed = *out_edge_start;
			if (((trig == -1)	|| (sd0[ed]._tr == trig))		&& 
				((gu == -1)		|| (sd0[ed]._gu == gu))			&& 
				((act == -1)	|| (sd0[ed]._act == act)))  { 
				result = true; 
				break;
			}	
		}
	} else if (origin != -1 && destination != -1) { 
		if ((destination > num_vertices(sd0)) || 
			(origin > num_vertices(sd0))) return result;
		o_temp = vertex(origin, sd0);
		d_temp = vertex(destination, sd0);
		for(tie(out_edge_start, out_edge_end) = edge_range (o_temp, d_temp, sd0);
			 out_edge_start != out_edge_end; ++out_edge_start) { 
			ed = *out_edge_start;
			if (((trig == -1)	|| (sd0[ed]._tr == trig))		&& 
				((gu == -1)		|| (sd0[ed]._gu == gu))			&& 
				((act == -1)	|| (sd0[ed]._act == act)))  { 
				result = true; 
				break;
			}	
		}
	}
	
	return result;

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
	bool result = false;
	if (num_vertices(sd0) > jump_amount) {  	
		orig = vertex(jump_amount, sd0);
		result = true;
	}
	return result;
}

bool cUMLStateDiagram::absoluteJumpDestinationState(int jump_amount) 
{	
	bool result = false;
	if (num_vertices(sd0) > jump_amount) {  	
		dest = vertex(jump_amount, sd0);
		result = true;
	}
	return result;
}


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


transition_label cUMLStateDiagram::getTransLabel()
{
	return transition_labels[trans_label_index];
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
//	boost::add_edge(transition_properties());

	orig_state_index = 0;
	dest_state_index = 0;
	trans_label_index = 0;
	trigger_index = 0;
	guard_index = 0 ;
	action_index = 0;
	
	return true;

}
*/

bool cUMLStateDiagram::addTransitionLabel(int tr, int gu, int act)
{
	transition_label t;
	t.trigger = tr;
	t.guard = gu;
	t.action = act;

	// currently, not checking for dupes, since we are seeding the labels.
	
	transition_labels.push_back(t);
}


bool cUMLStateDiagram::addTransitionTotal()
{
	// Check that there are two vertices
	if (num_vertices(sd0) == 0) {
		return false;
	} 
		
	// Check that the origin state of the transition is reachable.
	boost::graph_traits<state_diagram>::in_edge_iterator in_start, in_end;
	tie (in_start, in_end) = in_edges(orig, sd0);
	if ((in_start == in_end) && (orig != vertex(0, sd0))) { 
		return false;
	}

	// Check that there is not a duplicate transition
	if (findTrans(orig, dest, trigger_index, guard_index, action_index)) { 
		return false;
	}
	
	// Create the transition properties
	transition_properties tp = transition_properties(trigger_index, guard_index, action_index); 
	
	// Create the transition
	add_edge(orig, dest, tp, sd0);
		
	// Reset all the indices
	trigger_index = 0;
	action_index = 0;
	guard_index = 0;
	orig = vertex(0, sd0);
	dest = vertex(0, sd0);
		
	return true;

}




int cUMLStateDiagram::numStates()
{
	return num_vertices(sd0);
}

int cUMLStateDiagram::numTrans()
{
	return num_edges(sd0);
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

void cUMLStateDiagram::executeVisitor() {

	PathVisitor visitor; 
	boost::graph_traits<state_diagram>::vertex_descriptor o_temp;
	o_temp = vertex(0, sd0);
	
	boost::breadth_first_search(sd0, o_temp, boost::visitor(visitor));

}


struct transition_writer {
	transition_writer(cUMLStateDiagram::state_diagram& sd) : _sd(sd) { }
	
	template<typename Edge>
	void operator()(std::ostream& out) {
		out << "working?";
	}
  
	cUMLStateDiagram::state_diagram& _sd;
}; 

void cUMLStateDiagram::printGraphViz() { 
	std::ofstream outfile("gv");
    boost::write_graphviz(outfile, sd0); //, transition_writer(sd0)); //, topo_vertex_writer(_topo_last_network));
    outfile.close();

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
	
	boost::graph_traits<state_diagram>::edge_iterator edge_start, edge_end;
	boost::graph_traits<state_diagram>::edge_descriptor ed;

	int s_count = 0;
	int t_count = 0;
	xmi = "";

	// This state is the initial state; thus it should be printed regardless of whether it has an incoming
	// edge or not.
	if (numStates() > 0) {
		temp = "_1";
		xmi += "<UML:Pseudostate xmi.id=\"s" + temp + "\" kind=\"initial\" outgoing=\"\" name=\"s";
		xmi += temp + "\" isSpecification=\"false\"/>\n";
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


	// Get the set of transitions.
	for (tie(edge_start, edge_end) = edges(sd0);
			 edge_start != edge_end; ++edge_start) {
		
		ed = *edge_start;
	 	 
		// info determined from the trans itself....
		temp = "t" + StringifyAnInt(t_count);
		temp1 = "s" + StringifyAnInt(source(ed, sd0));
		temp2 = "s" + StringifyAnInt(target(ed, sd0));
		temp3 = temp + temp1 + temp2;
		
		xmi+= "<UML:Transition xmi.id=\"" + temp3 + "\"";
		xmi+= " source=\"" + temp1 + "\"";
		xmi += " target=\"" + temp2 + "\" name=\"\" isSpecification=\"false\">\n";

		// Get guard, trigger, and action
		temp1 = guards[(sd0[ed]._gu)];
		temp2 = actions[(sd0[ed]._act)];
		trig_label = triggers[(sd0[ed]._tr)].label;
		trig_op_name = triggers[(sd0[ed]._tr)].operation_id;

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
		t_count++;
	}
	
	// Add one transition to connect the init state to state 0
	temp = "t" + StringifyAnInt(t_count);
	temp1 = "s" + StringifyAnInt(-1);
	temp2 = "s" + StringifyAnInt(0);
	temp3 = temp + temp1 + temp2;

	xmi+= "<UML:Transition xmi.id=\"" + temp3 + "\"";
	xmi+= " source=\"" + temp1 + "\"";
	xmi += " target=\"" + temp2 + "\" name=\"\" isSpecification=\"false\">\n";
	xmi += "</UML:Transition>\n";
		
	return;
}


