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

  // used for both alphabet options
  std::vector<trigger_info> triggers;
  std::vector<std::string> guards;
  std::vector<std::string> actions;
	
  // used for alphabet where a word is a transition label
  std::vector<transition_properties> transition_labels;

  int trans_label_index;
  int trigger_index;
  int guard_index;
  int action_index;

  
  std::string xmi;
  	
public:

  cUMLStateDiagram();
  ~cUMLStateDiagram();

  void printXMI(std::string);	 // print the XMI version of the model
  std::string getXMI (std::string); // get the xmi string (including beginning & end read from file.)
  std::string StringifyAnInt(int);
  int numStates();
  int numTrans();
  int numTriggers() { return triggers.size(); } 
  int numGuards() { return guards.size(); } 
  int numActions() { return actions.size(); }
  void addName(std::string s) { sd_name = s; } 

  unsigned int getNumberOfNonDeterministicStates();


  bool findTrans(int, int, int, int, int);

// Boost graph type for a state diagram.
  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, 
	int, transition_properties> state_diagram;

  template <typename T>
  bool absoluteMoveIndex (T x, int &y, int z);

 
  template <typename T>
  bool relativeMoveIndex (T x, int &y, int z);  
 
// The jump functions jump the index of the various vectors either forward (+ int) or backwards (- int)
  bool absoluteJumpGuard(int);
  bool absoluteJumpAction(int);
  bool absoluteJumpTrigger(int);
  bool absoluteJumpTransitionLabel(int);
  bool absoluteJumpOriginState(int);
  bool absoluteJumpDestinationState(int);
  
  
// Implement a scenario check.  
  int findPath(std::deque<std::string>, bool, int); 
  int checkForPathStep(std::deque<std::string>, 
		boost::graph_traits<state_diagram>::vertex_descriptor, int); 
  int getEndState (std::deque<std::string>, boost::graph_traits<state_diagram>::vertex_descriptor);

  
// The get functions get the value of the index of various vectors  
  int getTriggerIndex() { return trigger_index; } 
  int getGuardIndex() { return guard_index; } 
  int getActionIndex() { return action_index; } 
  int getTransitionLabelIndex() { return trans_label_index; }
  std::string getGuard() { return guards[getGuardIndex()]; } 
  std::string getName() { return sd_name; } 
  
// Add functions

  bool addTransitionTotal();
  bool addTransitionFromLabel(); 
  bool addTransitionTotal(int, int, int, int, int);
  bool addTrigger(std::string, std::string);
  bool addGuard(std::string);
  bool addAction(std::string);
  bool addTransitionLabel(int, int, int);

  
protected:
    state_diagram sd0;
	boost::graph_traits<state_diagram>::vertex_descriptor orig;
	boost::graph_traits<state_diagram>::vertex_descriptor dest;
	int actual_end_state;
	std::string sd_name;
	

};

#endif
