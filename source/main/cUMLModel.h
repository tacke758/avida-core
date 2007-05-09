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
	std::string getXMI(); // get the XMI version of the model
	int formalizeModel() const; // formalize the model using Hydra
	int checkProperty(const std::string& neverclaimFile) const; // check for property satisfaction using Spin
	int propertyN1() const; // check for property N1
	
	// Track bonus info for model
	std::map <std::string, double> getBonus() {return self_bonus;}
	void setBonusInfo(std::string x, double y) {self_bonus[x] = y;}
	
	// Determine the "fitness" of the model
	// does the model satisfy the sequence diagrams? 
	// does the model meet the properties? 


protected: 
	static std::string xmi_begin;
	std::string	xmi; // the XMI created by the model
	static std::string xmi_end;
	std::vector<cUMLStateDiagram> state_diagrams;
	
	// UML - Track information about the bonus received for this model.
	std::map <std::string, double> self_bonus;											
	
};

#endif
