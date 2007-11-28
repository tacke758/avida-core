#ifndef _C_MDEPROPERTY_H_
#define _C_MDEPROPERTY_H_
/*
 *  cMDEProperty.h
 *  
 *
 *  Created by Heather Goldsby on 11/20/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */
#include <string>
#include <iostream>


class cMDEProperty{
	
public:
	virtual ~cMDEProperty() {}
	// A function that prints the property to a file.
	virtual void print() = 0;
	virtual void printWitness() = 0;
	// A function that checks to see if there is a witness trace for the property
	float numWitnesses();
	// A function that verifies a property
	float verify();
	// A function that evaluates a property	
	virtual void evaluate(); 
	std::string getMDEPropertyName() { return _name; } 
	void setEvaluationInformation (float eval) { _reward = eval; }
	float getEvaluationInformation() { return _reward; }
		
protected:
	std::string _name;
	std::string _scope;
	float _reward;
	
};

#endif
