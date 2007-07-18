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


class cUMLModel { 
public:
	cUMLModel();
	~cUMLModel();

	// Read in from file seed-model.cfg and add the building blocks for the diagrams.
	void seedDiagrams(); 
	
	// Used to generate and access the XMI version of the model.
	std::string getXMI(); // get the XMI version of the model.
	void printXMI(); // create the XMI version of the model.	
	
	// Access the size of the state diagrams and also a specific state diagram
	unsigned int getStateDiagramSize() { return state_diagrams.size(); } 
	cUMLStateDiagram* getStateDiagram (unsigned int); 
	
	// Set and access the bonus info for a model. Should be used by the tasks
	void setBonusInfo (std::string s, float f) { bonus_info[s] = f; } 
	float getBonusInfo (std::string);  
		
	// Get the number of, well, everything
	int numStates();
	int numTrans();
	int numTriggers();
	int numGuards();
	int numActions();
	int numSDs();
	

protected: 
	static std::string xmi_begin;
	std::string	xmi; 
	static std::string xmi_end;
	static std::string xmi_class1;
	static std::string xmi_class2;
	std::vector<cUMLStateDiagram> state_diagrams;
	std::map<std::string, float> bonus_info;
										
};

#endif
