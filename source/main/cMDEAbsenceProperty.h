/*
 *  cMDEAbsenceProperty.h
 *  
 *
 *  Created by Heather Goldsby on 11/20/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _C_MDEABSENCEPROPERTY_H_
#define _C_MDEABSENCEPROPERTY_H_

#include "cMDEProperty.h"
#include <string>
#include <fstream>
#include <iostream>


/* Used to represent properties of the form: Globally, it is never the case that P holds  */

class cMDEAbsenceProperty : public cMDEProperty{
	
public:
	cMDEAbsenceProperty(std::string expr) { 
		_expr_p = expr;
		_reward = -1; 		
		_property_file_name = "tmp-property.pr"; 
		_witness_file_name = "tmp-witness.pr";
		_properties = "properties_that_passed";
		_promela = "tmp.pr";
	}
	virtual ~cMDEAbsenceProperty() {}
	void print(); // { std::cout << _scope << " " << _expr_p  << std::endl; }
	void printWitness(); // { std::cout << _scope << " " << _expr_p  << std::endl; }
	void printInEnglish();
	void evaluate();
	std::string getPropertyType() { return "Absence"; } 
	std::string getPropertyParameters() { return _expr_p; }



private:
	std::string _expr_p;
};

#endif
