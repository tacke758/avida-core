#ifndef _C_MDESIMPLEATTVALEXPRESSION_H_
#define _C_MDESIMPLEATTVALEXPRESSION_H_
/*
 *  cMDESimpleAttValExpression.h
 *  
 *
 *
 */
#include "cMDEExpression.h"
#include "cMDEExpressionAttribute.h"
#include <string>
#include <iostream>
#include <cassert>
#include <set>



class cMDESimpleAttValExpression : public cMDEExpression{
	
public:
	cMDESimpleAttValExpression (cMDEExpressionAttribute* a1, 
								std::string val, 
								std::string op) {
		_att_1 = a1;
		_att_val = val;
		_att_operator = op;
		_compound = false;
		_expr = 
			getInPromela();
	}
 	~cMDESimpleAttValExpression() {}
	
	// get functions
	std::string getInEnglish() { 
		return (_att_1->getInEnglish() + " " + _att_operator + " " + _att_val); }
	std::string getInPromela() { 
		return (_att_1->getInPromela() + " " + _att_operator + " " + _att_val); }
	
	// Check to see if the Expression uses a specific attribute.
	bool usesAttribute(std::string s) { 
		if (s == _att_1->getAttName()) return true;
		return false;
	}
	
	//std::set<std::string> getRelatedClasses() { return _related_classes; }
	
		
protected:
	cMDEExpressionAttribute* _att_1;
	std::string _att_val;
	std::string _att_operator;
	
};




#endif
