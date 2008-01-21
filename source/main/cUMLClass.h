/*
 *  cUMLClass.h
 *  
 *
 *  Created by Heather Goldsby on 1/18/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _C_UMLCLASS_H_
#define _C_UMLCLASS_H_

//#include "cUMLStateDiagram.h"


#include <string>
#include <iostream>
#include <map>
#include <utility>
#include <set>
#include <vector>
#include <fstream>

struct class_attribute{
	std::string attribute_type;
	std::string attribute_name;
	std::vector<std::string> attribute_values;
};

struct operation{
	std::string op_name; 
	std::string op_code;
};

class cUMLClass{
	
public:
	void addOperation(std::string, std::string);
	void addAssociatedClass(std::string s) { associated_class_names.push_back(s); }
	void addAttribute(std::string, std::string, std::vector<std::string>);
	void addClassName(std::string s) { name = s; }
	
	std::string getClassName() { return name; }
	int numAttributes() { return attributes.size(); }
	int numOperations() { return operations.size(); }
	class_attribute getAttribute(int i) { return attributes[i]; }
	operation getOperation(int i) { return operations[i]; }
	
		
protected:
	std::vector<std::string> associated_class_names;
	std::vector<operation> operations;
	std::vector<class_attribute> attributes;
	std::string name;

};

#endif
