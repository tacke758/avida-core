#ifndef _C_UMLSTATEDIAGRAM_H_
#define _C_UMLSTATEDIAGRAM_H_

#include <string>
#include <vector>
#include <iostream>
#include <sstream>


struct transition_label { 
	int trigger;
	std::string guard;
	std::string action;
};

struct  transition {
	int orig_state;
	int dest_state;
	transition_label trans;
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
  std::vector<state> states;											
  std::vector<trigger_info> triggers;
  std::vector<std::string> guards;
  std::vector<std::string> actions;
  std::vector<transition> transitions;
  std::vector<transition_label> transition_labels;
  
  int orig_state_index;
  int dest_state_index;
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

  bool findTrans(int, int, int, std::string, std::string) ;
  bool findTrans(int, int, std::string, std::string, std::string);
  bool findTransLabel(transition_label); // find a specific transition label
  bool currTrans(int, int, std::string, std::string, std::string);


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
  bool relativeJumpGuard(int);
  bool relativeJumpAction(int);
  bool relativeJumpTrigger(int);
  bool relativeJumpTransitionLabel(int);
  bool relativeJumpOriginState(int);
  bool relativeJumpDestinationState(int);
    

// The first functions jump the index to the beginning of various vectors   
  bool firstGuard() {guard_index = 0;}
  bool firstAction() {action_index = 0;}
  bool firstTrigger() {trigger_index = 0;}
  bool firstTransitionLabel() {trans_label_index = 0;}
  bool firstOriginState() {orig_state_index = 0;}
  bool firstDestinationState() {dest_state_index = 0;}	

// The last functions jump the index to the end of various vectors   
  bool lastGuard() {guard_index = (guards.size()-1);}
  bool lastAction() {action_index = (actions.size()-1);}
  bool lastTrigger() {trigger_index = (triggers.size()-1);}
  bool lastTransitionLabel() {trans_label_index = (transition_labels.size()-1);} 
  bool lastOriginState() {orig_state_index = (states.size()-1);}
  bool lastDestinationState() {dest_state_index = (states.size()-1);}
	
  
// The get functions get the value of the index of various vectors  
//  std::string getTriggerIndex();
  int getTriggerIndex();
  std::string getGuard();
  std::string getAction();
  transition_label getTransLabel();
  int getOrigStateIndex();
  int getDestStateIndex();

// Add functions
  bool addState();
  bool addTransitionLabel();
  bool addTransition();
  bool addTransitionTotal();
  bool addTrigger(std::string, std::string);
  bool addGuard(std::string);
  bool addAction(std::string);
  // END UML functions

};

#endif
