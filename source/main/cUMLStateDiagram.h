#ifndef _C_UMLSTATEDIAGRAM_H_
#define _C_UMLSTATEDIAGRAM_H_

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <utility>                   
#include <algorithm>              
#include <boost/config.hpp>   
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/graphviz.hpp>


struct transition_properties { 
	transition_properties() {}
	transition_properties(int trigger, int guard, int action) 
		: _tr(trigger), _gu(guard), _act(action) {}
	int _tr, _gu, _act; 
}; 

struct transition_label { 
	int trigger;
	int guard;
	int action;
};


struct state {
	int identifier;
	int num_incoming;
	int num_outgoing;
};

struct trigger_info {
	std::string operation_id;
	std::string label;
};


class cUMLStateDiagram { 
protected: 
  std::vector<trigger_info> triggers;
  std::vector<std::string> guards;
  std::vector<std::string> actions;
//  std::vector<transition> transitions;
  std::vector<transition_label> transition_labels;

//  int orig_state_index;
//  int dest_state_index;
  int trans_label_index;
  int trigger_index;
  int guard_index;
  int action_index;

  
  std::string xmi;
  	
public:

  cUMLStateDiagram();
  ~cUMLStateDiagram();

  void printXMI();	 // print the XMI version of the model
  std::string getXMI (); // get the xmi string (including beginning & end read from file.)
  std::string StringifyAnInt(int);
  int numStates();
  int numTrans();
  int numTriggers() { return triggers.size(); } 
  int numGuards() { return guards.size(); } 
  int numActions() { return actions.size(); }


  bool findTrans(int, int, int, int, int);
//  bool findTransLabel(transition_label); // find a specific transition label

// Boost graph type for a state diagram.
  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, 
	int, transition_properties> state_diagram;

  template <typename T>
  bool absoluteMoveIndex (T x, int &y, int z);

 
  template <typename T>
  bool relativeMoveIndex (T x, int &y, int z);  
 
 
 // pass in the path; state to know path step; examine_edge
 // Issue: the need to revisit the same node.
 //! A BGL breadth-first visitor used to see if a path is included in a state diagram.
class PathVisitor : public boost::bfs_visitor< > {
public:
  PathVisitor() {}
  template <class state_diagram>
  void tree_edge(typename boost::graph_traits<state_diagram>::edge_descriptor e, state_diagram& g) {
    typename boost::graph_traits<state_diagram>::vertex_descriptor u, v;
    u = boost::source(e, g);
    v = boost::target(e, g);
//    distance[v] = distance[u] + 1;
	std::cout << "origin: " << u << std::endl;
	std::cout << "destination: " << v << std::endl;
	std::cout << "tga: " << g[e]._tr << g[e]._gu << g[e]._act << std::endl;
	}
  };
  
struct transition_writer {
	transition_writer(state_diagram& sd) : _sd(sd) { }
	
	template<typename Edge>
	void operator()(std::ostream& out) {
		out << "working?";
	}
  
	state_diagram& _sd;
}; 
 
 
// The jump functions jump the index of the various vectors either forward (+ int) or backwards (- int)
  bool absoluteJumpGuard(int);
  bool absoluteJumpAction(int);
  bool absoluteJumpTrigger(int);
  bool absoluteJumpTransitionLabel(int);
  bool absoluteJumpOriginState(int);
  bool absoluteJumpDestinationState(int);
  
  
// Implement a scenario check.  
//  void checkForPath(std::deque<std::string>, boost::graph_traits<state_diagram>::vertex_descriptor, bool&, int&);
  int findPath(std::deque<std::string> p); 
  int checkForPathStep(std::deque<std::string>, boost::graph_traits<state_diagram>::vertex_descriptor, int); 
  
// The get functions get the value of the index of various vectors  
  int getTriggerIndex();
  int getGuardIndex();
  int getActionIndex();
  transition_label getTransLabel();
  
// Visit graph
  void executeVisitor();
  void printGraphViz();


// Add functions

//  bool addTransition();
  bool addTransitionTotal();
  bool addTrigger(std::string, std::string);
  bool addGuard(std::string);
  bool addAction(std::string);
  bool addTransitionLabel(int, int, int);
  // END UML functions
protected:
    state_diagram sd0;
	boost::graph_traits<state_diagram>::vertex_descriptor orig;
	boost::graph_traits<state_diagram>::vertex_descriptor dest;
	

};

#endif
