#ifndef _C_UMLMODEL_H_
#define _C_UMLMODEL_H_

#include "cUMLStateDiagram.h"
#include "cUMLClassDiagram.h"

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
	std::string getXMI(); // get the XMI version of the model
	void printXMI();
	double formalizeModel(); // formalize the model using Hydra
	double checkProperty(const std::string& neverclaimFile) const; // check for property satisfaction using Spin
	double propertyN1() const; // check for property N1
	
	void resetStateDiagrams (int);  
	void seedDiagrams();
	
	// Track bonus info for model
	std::map <std::string, double> getBonus() {return self_bonus;}
	void setBonusInfo(std::string x, double y) {self_bonus[x] = y;}
	
	cUMLStateDiagram* getStateDiagram (int); 
	cUMLClassDiagram* getClassDiagram (int);
	
	// Determine the "fitness" of the model
	// does the model satisfy the sequence diagrams? 
	// does the model meet the properties? 
	double evaluateModel();
	double checkForSequenceDiagram1();


protected: 
	static std::string xmi_begin;
	std::string	xmi; // the XMI created by the model
	static std::string xmi_end;
	static std::string xmi_class1;
	static std::string xmi_class2;
	std::vector<cUMLStateDiagram> state_diagrams;
	std::vector<cUMLClassDiagram> class_diagrams;
	
	// UML - Track information about the bonus received for this model.
	std::map <std::string, double> self_bonus;											
	
};

#endif
