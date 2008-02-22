/*
 *  cMDEExistenceProperty.h
 *  
 *
 *  Created by Heather Goldsby on 11/20/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _C_MDEEXISTENCEPROPERTY_H_
#define _C_MDEEXISTENCEPROPERTY_H_

#include "cMDEProperty.h"
#include <string>
#include <fstream>
#include <iostream>
#include <cassert>


/* Used to represent properties of the form: Globally, P eventually holds  */

class cMDEExistenceProperty : public cMDEProperty{
	
public:
	cMDEExistenceProperty(std::string expr) { _expr_p = expr; _reward = -1;}
	
	virtual ~cMDEExistenceProperty() {}

	void print(); // { std::cout << _scope << " " << _expr_p  << std::endl; }
	void printWitness(); // { std::cout << _scope << " " << _expr_p  << std::endl; }
	void printInEnglish();
	std::string getPropertyType() { return "Existence"; } 
	std::string getPropertyParameters() { return _expr_p; }



private:
	std::string _expr_p;
};

#endif
