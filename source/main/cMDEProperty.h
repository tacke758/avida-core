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
#include<fstream>



class cMDEProperty{
	
public:
	virtual ~cMDEProperty() {
		_property_file_name = "tmp-property.pr"; 
		_witness_file_name = "tmp-witness.pr";
		_properties = "properties_that_passed";
		_promela = "tmp.pr";
	}
	// A function that prints the property to a file.
	virtual bool print() = 0;
	virtual bool printWitness() = 0;
	virtual bool printInEnglish() =0;
	virtual std::string getPropertyType() = 0;
	virtual std::string getPropertyParameters() { return ""; } 
	
	
	// A function that checks to see if there is a witness trace for the property
	float numWitnesses();
	// A function that verifies a property
	float verify();
	// A function that evaluates a property	
	virtual void evaluate(); 
	std::string getMDEPropertyName() { return _name; } 
	void setEvaluationInformation (float eval) { _reward = eval; }
	float getEvaluationInformation() { return _reward; }
	void incCount() { _count++; }
	int getCount() {return _count; }
		
protected:
	std::string _name;
	std::string _scope;
	float _reward;
	std::string _property_file_name; // = "tmp-property.pr"; 
	std::string _witness_file_name; // = "tmp-witness.pr";
	std::string _properties; // = "properties_that_passed";
	std::string _promela; 
	int _count; 
	
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
