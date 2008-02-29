#ifndef _C_MDEEXPRESSIONATTRIBUTE_H_
#define _C_MDEEXPRESSIONATTRIBUTE_H_
/*
 *  cMDEExpressionAttribute.h
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



class cMDEExpressionAttribute{
	
public:
	cMDEExpressionAttribute (std::string n, std::string t, std::string c) { 
		_att_name =n; _att_type = t; _att_class=c; }
	~cMDEExpressionAttribute() {}
	
	// get functions
	std::string getInEnglish() { return (_att_class + "." + _att_name); }
	std::string getInPromela() { return (_att_class + "_V." + _att_name); }	
	std::set<std::string> getRelatedClasses() { return _related_classes; }
	
	// set functions
	void setRelatedClasses(std::set<std::string> s) { _related_classes = s; }	

		
protected:
	std::string _att_name;
	std::string _att_type;
	std::string _att_class; 
	std::set<std::string> _related_classes;
	
};




#endif
