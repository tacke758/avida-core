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
	// A function that returns an expression
	std::string getExpr() const { return _expr; }
	void setExpr(std::string s) { _expr = s; }
	bool operator==(const cMDEExpression that) {
		return (_expr == that.getExpr());
	}
	
	// the related classes are the classes that this expression may be 
	// related to.
	void addRelatedClass(std::string c) { _related_classes.insert(c); }
	void setRelatedClasses(std::set<std::string> s) { _related_classes = s; }	
	std::set<std::string> getRelatedClasses() { return _related_classes; }
	bool isRelatedClass(std::string c) { 
		return false;// find(_related_classes.begin(), _related_classes.end(), c)
	}

	// the classes are the classes involved in the expression
	void addClass(std::string c) {_class_names.insert(c); }
	bool isClass(std::string c) { 
		return false; //find(_class_names.begin(), _class_names.end(), c)
	}

		
protected:
	std::string _expr;
	std::set<std::string> _related_classes;
	std::set<std::string> _class_names;
	// Ok, eventually, to do more complex expression creation, we need to get it to check the types...
	
//	std::string _types;
	
};




#endif
