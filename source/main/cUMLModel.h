#ifndef _C_UMLMODEL_H_
#define _C_UMLMODEL_H_

#include "cUMLStateDiagram.h"
//#include "cWorld.h"
//#include "cStats.h"

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

	void seedDiagrams(); // add the building blocks for the diagrams.
	
	std::string getXMI(); // get the XMI version of the model.
	void printXMI(); // create the XMI version of the model.	
	
	int getStateDiagramSize() { return state_diagrams.size(); } 
	cUMLStateDiagram* getStateDiagram (int); 
	int numStates();
	int numTrans();
	int numTriggers();
	int numGuards();
	int numActions();
	int numSDs();
	

protected: 
	static std::string xmi_begin;
	std::string	xmi; // the XMI created by the model
	static std::string xmi_end;
	static std::string xmi_class1;
	static std::string xmi_class2;
	std::vector<cUMLStateDiagram> state_diagrams;
										
};

#endif
