/*
 *  cUMLClass.cc
 *  
 *
 *  Created by Heather Goldsby on 1/18/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "cUMLClass.h"

void cUMLClass::addOperation(std::string n, std::string c) { 
	operation o; 
	o.op_name = n; 
	o.op_code = c;
	operations.push_back(o);
}

void cUMLClass::addAttribute(std::string n, std::string t, std::vector<std::string> vals) {
	class_attribute c;
	c.attribute_name = n;
	c.attribute_type = t;
//	c.attribute_values = vals;
	c.attribute_values.assign(vals.begin(), vals.end());
	attributes.push_back(c);
}
