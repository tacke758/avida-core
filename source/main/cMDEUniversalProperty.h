/*
 *  cMDEUniversalProperty.h
 *  
 *
 *  Created by Heather Goldsby on 11/20/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef _C_MDEUNIVERSALPROPERTY_H_
#define _C_MDEUNIVERSALPROPERTY_H_
#include "cMDEProperty.h"

#include <string>
#include <fstream>
#include <iostream>



/* Used to represent properties of the form: Globally, it is always the case that P holds  */

class cMDEUniversalProperty : public cMDEProperty{
	
public:
	cMDEUniversalProperty(std::string expr) { _expr_p = expr; _reward = -1;}
	virtual ~cMDEUniversalProperty() {}
	void print(); // { std::cout << _scope << " " << _expr_p  << std::endl; }
	void printWitness(); // { std::cout << _scope << " " << _expr_p  << std::endl; }
	void printInEnglish();
	void evaluate();
	std::string getPropertyType() { return "Universal"; } 
	std::string getPropertyParameters() { return _expr_p; }


private:
	std::string _expr_p;
};

#endif

