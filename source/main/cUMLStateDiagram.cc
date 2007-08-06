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


// This function accepts a path throught the state diagram as a deque and returns the length 
// of the longest path segment that the diagram satisfies. 
// The function is complicated by the fact that the longest path segment could start at the 
// beginning, middle, or end of the path itself. 
// Currently the path must begin at the 0 vertex. 
int cUMLStateDiagram::findPath(std::deque<std::string> p) { 
	unsigned int path_dist = 0; // the current path distance satisfied is 0. 
//	int path_longest_length = 0; 
	unsigned int len = 0;
	int num_vert = num_vertices(sd0);
	std::deque<std::string> p_temp = p;

	// Entire path must start at state 0. 
	len = checkForPathStep(p, vertex(0, sd0), 0);
	// If this returns a length, then the path is found & we can exit.
	if (len > 0) { 
		path_dist = len;
	} else { 
	// Else, check for partial paths, which can start from anywhere... 
		p.pop_front();
		// Must check each state other than state 0.
		while (!p.empty()) {
			for (int i = 1; i<num_vert; i++) { 
				len = checkForPathStep(p, vertex(i, sd0), 0);
				// check to see if this path is longer than other paths....
				if (len > path_dist) { 
					path_dist = len;
				}	
			}	
			p.pop_front(); 
		
			if (len > p.size()) break;
		} 
	 
	} 
	
	return path_dist;
}

int cUMLStateDiagram::checkForPathStep(std::deque<std::string> path, 
				boost::graph_traits<state_diagram>::vertex_descriptor v, 
				int curr_dist) { 
	
	boost::graph_traits<state_diagram>::out_edge_iterator out_edge_start, out_edge_end;
	boost::graph_traits<state_diagram>::edge_descriptor ed;
	std::string tt, tg, ta, ts;
	int longest_dist = curr_dist;
	int temp;

	if (path.empty()) return curr_dist;

	// Get the outgoing edges of v
	for(tie(out_edge_start, out_edge_end) = out_edges(v, sd0);
			 out_edge_start != out_edge_end; ++out_edge_start) { 
			ed = *out_edge_start;
			
			// Get the label of the edge
			tt = triggers[(sd0[ed]._tr)].operation_id;
			tg = guards[(sd0[ed]._gu)];
			ta = actions[(sd0[ed]._act)];
			
			// eliminate nulls
			if (tt == "<null>") tt = "";
			if (tg == "<null>") tg = "";
			if (ta == "<null>") ta = "";
			
			ts = tt + "[" + tg + "]" + "/" + ta;
//			std::cout << "transition named: " << ts << std::endl;
			
			
			if (ts == path.front()) { 
				//std::cout << "Looking for and found a path named: " << ts << std::endl;
				//std::cout << "Searching vertex " << target(ed, sd0) << "with distance " << curr_dist+1 << std::endl; 

				temp = checkForPathStep(std::deque<std::string>(++path.begin(), path.end()), target(ed,sd0), curr_dist+1);
				if (temp > longest_dist) longest_dist = temp;
			}
	}
	
	//std::cout << "Returning longest_dist " << longest_dist << std::endl;
	return longest_dist;

} 




bool cUMLStateDiagram::findTrans(int origin, int destination, int trig, int gu, int act) 
{

	bool result = false;
	int num_vert = (int) num_vertices(sd0); 
	
	if (num_edges(sd0) == 0) { 
		return false;
	}
	
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
		if (destination > num_vert) return result;
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
		if (origin > num_vert) return result;
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
		if ((destination > num_vert) || 
			(origin > num_vert)) return result;
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

	int x_size = (int) x.size();
	if (x_size == 0 || amount > x_size) {
		return false;
	}
	
	index = 0;
	return relativeMoveIndex(x, index, amount);
}

template <typename T>
bool cUMLStateDiagram::relativeMoveIndex (T x, int &index, int amount )
{
	int x_size = (int) x.size();

	if (x_size == 0) {
		return false;
	}
	
	if (amount > 0) { 
		index += (amount % x_size);

		// index is greater than vector
		if (index >= x_size) { 
			index -= x_size;
		} else if(index < 0) { 
			index += x_size;
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
	int num_vert = (int) num_vertices(sd0);
	
	if (num_vert > jump_amount) {  	
		orig = vertex(jump_amount, sd0);
		result = true;
	}
	return result;
}

bool cUMLStateDiagram::absoluteJumpDestinationState(int jump_amount) 
{	
	bool result = false;
	int num_vert = (int) num_vertices(sd0);

	if (num_vert > jump_amount) {  	
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
	
	return true;
}

bool cUMLStateDiagram::addTransitionTotal(int o, int d, int t, int g, int a) { 
	absoluteJumpGuard(g);
	absoluteJumpAction(a);
	absoluteJumpTrigger(t);
	absoluteJumpOriginState(o);
	absoluteJumpDestinationState(d);	
	return addTransitionTotal();

}

bool cUMLStateDiagram::addTransitionTotal()
{
	// Check that there are two vertices
	if (num_vertices(sd0) == 0) {
		return false;
	} 
		
/*  // Removed on 7/17/2007 - The scenarios should take care of this problem.
	// Check that the origin state of the transition is reachable.
	boost::graph_traits<state_diagram>::in_edge_iterator in_start, in_end;
	tie (in_start, in_end) = in_edges(orig, sd0);
	if ((in_start == in_end) && (orig != vertex(0, sd0))) { 
		return false;
	}
*/	

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

std::string cUMLStateDiagram::getXMI(std::string name)
{
	printXMI(name);
	return (xmi);
}

void cUMLStateDiagram::printXMI(std::string name)
{
	std::string temp, temp1, temp2, temp3;
	std::string trig_label, trig_op_name;
	std::string sd = name;
	

	boost::graph_traits<state_diagram>::edge_iterator edge_start, edge_end;
	boost::graph_traits<state_diagram>::edge_descriptor ed;

	int s_count = 0;
	int t_count = 0;
	xmi = "";

	// This state is the initial state; thus it should be printed regardless of whether it has an incoming
	// edge or not.
	if (numStates() > 0) {
		temp = "_1";
		xmi += "<UML:Pseudostate xmi.id=\"" + sd + "s"  + temp + "\" kind=\"initial\" outgoing=\"\" name=\"s";
		xmi += temp + "\" isSpecification=\"false\"/>\n";
	}
	
	for (; s_count < numStates(); ++s_count) {
				temp = sd + "s" + StringifyAnInt(s_count);
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
		temp = sd + "t" + StringifyAnInt(t_count);
		temp1 = sd + "s" + StringifyAnInt(source(ed, sd0));
		temp2 = sd + "s" + StringifyAnInt(target(ed, sd0));
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
	temp = sd + "t" + StringifyAnInt(t_count);
	temp1 = sd + "s" + StringifyAnInt(-1);
	temp2 = sd + "s" + StringifyAnInt(0);
	temp3 = temp + temp1 + temp2;

	xmi+= "<UML:Transition xmi.id=\"" + temp3 + "\"";
	xmi+= " source=\"" + temp1 + "\"";
	xmi += " target=\"" + temp2 + "\" name=\"\" isSpecification=\"false\">\n";
	xmi += "</UML:Transition>\n";
		
	return;
}



