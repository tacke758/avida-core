/*
 *  cMDEResponseProperty.h
 *  
 *
 *  Created by Heather Goldsby on 11/20/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _C_MDERESPONSEPROPERTY_H_
#define _C_MDERESPONSEPROPERTY_H_

#include "cMDEProperty.h"
#include <string>
#include <fstream>
#include <iostream>
#include <cassert>


/* Used to represent properties of the form: Globally, P eventually holds  */

class cMDEResponseProperty : public cMDEProperty{
	
public:
	cMDEResponseProperty(std::string p, std::string q, std::string r) { 
		_expr_p = p;
		_expr_q = q;
		_name = ("Response" + r); _reward = -1;
		_property_file_name = "tmp-property.pr"; 
		_witness_file_name = "tmp-witness.pr";
		_properties = "properties_that_passed";
		_promela = "tmp.pr";
	}
	
	virtual ~cMDEResponseProperty() {}

	bool print(); 
	bool printWitness(); 
	bool printInEnglish();
	std::string getPropertyType() { return "Response"; } 
	std::string getPropertyParameters() { return (_expr_p + " " + _expr_q); }



private:
	std::string _expr_p;
	std::string _expr_q;	
};

#endif
