#ifndef _C_MDESIMPLEOPERATIONEXPRESSION_H_
#define _C_MDESIMPLEOPERATIONEXPRESSION_H_
/*
 *  cMDESimpleOperationExpression.h
 *  
 *
 *
 */
#include <string>
#include <iostream>
#include <cassert>
#include <set>
#include <cMDEExpression.h>



class cMDESimpleOperationExpression : public cMDEExpression{
	
public:
	cMDESimpleOperationExpression (std::string n, std::string c) { 
		_op_name =n; _op_class=c; _expr = getInPromela();}
	~cMDESimpleOperationExpression() {}
	
	// get functions
	std::string getInEnglish() { return (_op_class + "." + _op_name + "()"); }
	std::string getInPromela() { return (_op_class + "_q??[" + _op_name + "]"); }	
	std::set<std::string> getRelatedClasses() { return _related_classes; }
	
	// set functions
	void setRelatedClasses(std::set<std::string> s) { _related_classes = s; }	

	// This function is used to see if an expression uses a given operation
	bool usesOperation(std::string s) {
		if (s==_op_name) return true;
		return false;
	}
	
	void interestingExpressionEval() { 
		// Currently, the interesting reward is evaluated based on: 
		// - whether it includes one of the significant variables/operations
		float temp;
		// Check to see if the expressions use suspend or restart operations
		temp += usesOperation("suspend") + usesOperation("restart");
		_interesting = temp;		
	}
		
protected:
	std::string _op_name;
	std::string _op_class; 
	std::set<std::string> _related_classes;
	
};




#endif
