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
#include <cassert>



class cMDEProperty{
	
public:
	virtual ~cMDEProperty() {}
	// A function that prints the property to a file.
	virtual void print() = 0;
	virtual void printWitness() = 0;
	virtual void printInEnglish() =0;
	virtual std::string getPropertyType() = 0;
	virtual std::string getPropertyParameters() { return ""; } 
	
	
	// A function that checks to see if there is a witness trace for the property
	float numWitnesses();
	// A function that verifies a property
	float verify();
	// A function that evaluates a property	
	virtual void evaluate(); 
	void setEvaluationInformation (float eval) { _reward = eval; }
	float getEvaluationInformation() { return _reward; }
	
	// These functions get and set how interesting a property is. This 
	// information is calculated by the property generator when
	// the property is created.
	void setInterestingProperty(float inter) {_interesting = inter; }
	float getInteresting() { return _interesting; }
	
		
protected:
	std::string _scope;
	float _reward;
	float _interesting;
	
};

struct ltcMDEProperty{ 
	bool operator() (cMDEProperty* p1, cMDEProperty* p2) const
	{
	std::string name1, name2;
	
	name1 = p1->getPropertyType() + p1->getPropertyParameters();
	name2 = p2->getPropertyType() + p2->getPropertyParameters(); 
	// 1st less than 2nd & 2nd never equal to the first. 
	
		return (name1 < name2);
	}
};

#endif
