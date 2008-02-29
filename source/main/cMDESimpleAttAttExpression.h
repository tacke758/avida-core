#ifndef _C_MDESIMPLEATTATTEXPRESSION_H_
#define _C_MDESIMPLEATTATTEXPRESSION_H_
/*
 *  cMDESimpleAttAttExpression.h
 *  
 *
 *  Created by Heather Goldsby on 2/28/2008.
 *  A class used to represent simple expressions that are created by relating two
 *  attributes.
 *
 */
#include "cMDEExpression.h"
#include "cMDEExpressionAttribute.h"
#include <string>
#include <iostream>
#include <cassert>
#include <set>



class cMDESimpleAttAttExpression : public cMDEExpression{
	
public:
	cMDESimpleAttAttExpression (cMDEExpressionAttribute* a1, 
								cMDEExpressionAttribute* a2, 
								std::string op) {
		_att_1 = a1;
		_att_2 = a2;
		_att_operator = op;
		_expr = getInEnglish();
	}
 	~cMDESimpleAttAttExpression() {}
	
	// get functions
	std::string getInEnglish() { 
		return (_att_1->getInEnglish() + " " + _att_operator + " " + _att_2->getInEnglish()); }
	std::string getInPromela() { 
		return (_att_1->getInPromela() + " " + _att_operator + " " + _att_2->getInPromela()); }
	
	// Check to see if the Expression uses a specific attribute.
	bool usesAttribute(std::string s) { 
		if ((s == _att_1->getAttName()) || (s==_att_2->getAttName())) return true;
		return false;
	}
	


		
	
	//std::set<std::string> getRelatedClasses() { return _related_classes; }
	
		
protected:
	cMDEExpressionAttribute* _att_1;
	cMDEExpressionAttribute* _att_2;
	std::string _att_operator;
	
};




#endif
