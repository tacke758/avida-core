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
protected: 
	static std::string xmi_begin;
	std::string	xmi; // the XMI created by the model
	static std::string xmi_end;
	std::vector<cUMLStateDiagram> state_diagrams;											
	
public:

	std::string getXMI(); // get the XMI version of the model

};

#endif
