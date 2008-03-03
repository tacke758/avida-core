#ifndef _C_MDEEXPRESSION_H_
#define _C_MDEEXPRESSION_H_
/*
 *  cMDEExpression.h
 *  
 *
 *  Created by Heather Goldsby on 2/12/2008.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */
#include <string>
#include <iostream>
#include <cassert>
#include <set>



class cMDEExpression{
	
public:


	virtual ~cMDEExpression() {}
	
	// A function that returns an expression
	std::string getExpr() const { return _expr; }

	bool operator==(const cMDEExpression &that) const {
		return (_expr == that.getExpr());
	}
	
	virtual std::string getInEnglish() =0;
	virtual std::string getInPromela() =0;
	
	// This function is used to see if an expression uses a given attribute
	virtual bool usesAttribute(std::string) { return false; }
	
	// This function is used to see if an expression uses a given operation
	virtual bool usesOperation(std::string) { return false; }
	
	// This function calculates the number of ANDs used by a given expression
	virtual int numANDs() { return 0; }
	
	// This function calculates the number of ORs used by a given expression
	virtual int numORs() { return 0; }
	
	virtual void interestingExpressionEval() { _interesting = 0; }
	virtual float getInterestingExpressionEval() { return _interesting; } 
	
	// the related classes are the classes that this expression may be 
	// related to.
	void addRelatedClassName(std::string c) { _related_class_names.insert(c); }
	void setRelatedClassNames(std::set<std::string> s) { _related_class_names = s; }	
	std::set<std::string> getRelatedClassNames() { return _related_class_names; }

	// the classes are the classes involved in the expression
	void addUsedClassName(std::string c) {_used_class_names.insert(c); }
	void setUsedClassNames(std::set<std::string> s) { _used_class_names = s; }	
	std::set<std::string> getUsedClassNames() { return _used_class_names; }
	
	bool isUsesRelatedClasses() { return _uses_related_classes; } 
	void setUsesRelatedClasses(bool t) { _uses_related_classes = t; }
		
protected:
	std::string _expr;
	float _interesting;
	std::set<std::string> _related_class_names;
	std::set<std::string> _used_class_names;
	bool _uses_related_classes; 
};




#endif
