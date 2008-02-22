/*
 *  cMDEProperty.h
 *  
 *
 *  Created by Heather Goldsby on 11/20/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _C_MDEPRECEDENCEPROPERTY_H_
#define _C_MDEPRECEDENCEPROPERTY_H_

#include "cMDEProperty.h"
#include <string>
#include <fstream>
#include <iostream>
#include <cassert>


/* Used to represent properties of the form: Globally, P eventually holds  */

class cMDEPrecedenceProperty : public cMDEProperty{
	
public:
	cMDEPrecedenceProperty(std::string p, std::string q) { 
		_expr_p = p;
		_expr_q = q;
		_reward = -1;
	}
	
	virtual ~cMDEPrecedenceProperty() {}

	void print(); 
	void printWitness(); 
	void printInEnglish();
	std::string getPropertyType() { return "Precedence"; } 
	std::string getPropertyParameters() { return (_expr_p + " " + _expr_q); }



private:
	std::string _expr_p;
	std::string _expr_q;	
};

#endif
