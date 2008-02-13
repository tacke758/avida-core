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



class cMDEExpression{
	
public:
	// A function that returns an expression
	std::string getExpr() const { return _expr; }
	void setExpr(std::string s) { _expr = s; }
	bool operator==(const cMDEExpression that) {
//		std::cout << _expr << " " << that.getExpr() << std::endl;
		return (_expr == that.getExpr());
	}
	void addRelatedClass(std::string c) { _related_classes.insert(c); }
	void setRelatedClasses(std::set<std::string> s) { _related_classes = s; }
	std::set<std::string> getRelatedClasses() { return _related_classes; }

		
protected:
	std::string _expr;
	std::set<std::string> _related_classes;
	// Ok, eventually, to do more complex expression creation, we need to get it to check the types...
	
//	std::string _types;
	
};




/*
struct ltcMDEExpression{ 
	bool operator() (cMDEExpression* p1, cMDEExpression* p2) const
	{
		std::string name1, name2;
		
		name1 = p1->getExpr();
		name2 = p2->getExpr(); 
		// 1st less than 2nd & 2nd never equal to the first. 
		
		return (name1 < name2);
	}
};
*/
#endif
