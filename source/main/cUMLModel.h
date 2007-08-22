#ifndef _C_UMLMODEL_H_
#define _C_UMLMODEL_H_

#include "cUMLStateDiagram.h"

#include <string>
#include <iostream>
#include <map>
#include <utility>
#include <set>
#include <vector>
#include <fstream>


struct scenario_info { 
	int stateDiagramID;
	std::deque<std::string> path; 
	bool shouldLoop;
	int startState;
};

struct xmi_info { 
    std::string xmi_begin;
    std::string xmi_end;
	std::vector<std::string> classes_info;
};

class cUMLModel { 
public:
  //! Constructor that loads state diagram and scenario information from seed-model.cfg.
	cUMLModel();
	~cUMLModel();
	
	// Used to check if the diagrams satisfy the specified scenarios
	double checkForScenarios();
	
	// Used to generate and access the XMI version of the model.
	std::string getXMI(); // get the XMI version of the model.
	void printXMI(); // create the XMI version of the model.
	void printUMLModelToFile(std::string);  // print the UML model to a file.
	
	// Used to update and get the max number of trans. 
	int getMaxTrans() { return max_trans; } 
	void setMaxTrans( int m ) { max_trans = m; } 
	
	// Access the size of the state diagrams and also a specific state diagram
	unsigned int getStateDiagramSize() { return state_diagrams.size(); } 
	cUMLStateDiagram* getStateDiagram (unsigned int); 
	
	// Set and access the bonus info for a model. Should be used by the tasks
	void setBonusInfo (std::string s, float f) { bonus_info[s] = f; } 
	float getBonusInfo (std::string);  
	std::vector<double> getScenarioCompletion() { return scenario_completion; } 
	void setScenarioCompletion(std::vector<double> s)  { scenario_completion = s; }
		
	// Get the number of, well, everything
	int numStates();
	int numTrans();
	int numTriggers();
	int numGuards();
	int numActions();
	int numSDs();
	int numSCs();
	
	// check if the model is ready for hydra
	bool readyForHydra(); 

protected: 
	static xmi_info xi;
	std::string	xmi; 
	static int max_trans;
	std::vector<cUMLStateDiagram> state_diagrams;
	std::map<std::string, float> bonus_info;
	std::vector<scenario_info> scenarios;
	std::vector<double> scenario_completion;
	int hydraMode; 
	
  // The following are set once per Avida experiment, when the first cUMLModel is constructed:
  static bool _cfgLoaded; //!< Whether or not we've already loaded seed-model.cfg.
  static std::vector<cUMLStateDiagram> _cfg_state_diagrams; //!< State diagrams as read from seed-model.cfg.
  static std::vector<scenario_info> _cfg_scenarios; //!< Scenarios as read from seed-model.cfg.
  static int _cfg_hydra_mode; //!< Execution mode for hydra, as read from seed-model.cfg.
};



#endif
