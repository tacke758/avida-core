#ifndef _C_MDECOMPOUDEXPRESSION_H_
#define _C_MDECOMPOUDEXPRESSION_H_
/*
 *  cMDECompoundExpression.h
 *  
 *
 *
 */
#include "cMDEExpression.h"
#include <string>
#include <iostream>
#include <cassert>
#include <set>



class cMDECompoundExpression : public cMDEExpression{
	
public:
	cMDECompoundExpression (cMDEExpression* e1, 
							cMDEExpression* e2, 
							std::string op) {
		_exp_1 = e1;
		_exp_2 = e2;
		// Op must be either && or ||
		_exp_op = op;
		_compound = true;
		_expr = getInPromela();
	}
// 	~cMDECompoundExpression() {}
	
	// get functions
	std::string getInEnglish() { 
		std::string s; 
		s = "(" + _exp_1->getInEnglish() + " " + _exp_op + " "; 
		s = s + _exp_2->getInEnglish() + ")";
		return (s); }
	std::string getInPromela() { 
		std::string s; 
		s = "(" + _exp_1->getInPromela() + " " + _exp_op + " "; 
		s = s + _exp_2->getInPromela() + ")";
		return (s);}
	
	std::string getOp() { return _exp_op; }
	cMDEExpression* getLeft() { return _exp_1; }
	cMDEExpression* getRight() { return _exp_2; }
	
	// This function is used to see if an expression uses a given attribute
	bool usesAttribute(std::string s) { 
		return (_exp_1->usesAttribute(s) || _exp_2->usesAttribute(s)); }
	
	// This function is used to see if an expression uses a given operation
	bool usesOperation(std::string s) { 
		return (_exp_1->usesOperation(s) || _exp_2->usesOperation(s)); }

	// This function returns the number of ANDs in a given expression
	int numANDs() { 
		int num = 0;
		if (_exp_op == "&&") num = 1;

		return (num + _exp_1->numANDs() + _exp_2->numANDs());
	}
	
	// This function returns the number of ORs in a given expression
	int numORs() { 
		int num = 0;
		if (_exp_op == "||") num = 1;
		return (num + _exp_1->numORs() + _exp_2->numORs());
	}

	void interestingStrongANDExpressionEval() { 
		float temp =0;
		float no = numORs();
		
		// Increase interesting based on the number of ANDs
		temp += numANDs();
		
		// Decrease interesting based on the number of ORs
		temp = temp - (no/2);
		
		_interesting = temp;
		
	}
	
	void interestingWeakANDExpressionEval() {
		float temp =0;
		float na = numANDs();
		
		// Increase interesting based on the number of ORs
		temp += numORs(); 
		// Decrease interesting based on the number of ANDs
		temp = temp - (na/2); 
		_interesting = temp;
	}
	
	
		
protected:
	cMDEExpression* _exp_1;
	cMDEExpression* _exp_2;
	std::string _exp_op;
	
};




#endif
