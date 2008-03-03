#ifndef _C_MDEEXPRESSIONATTRIBUTE_H_
#define _C_MDEEXPRESSIONATTRIBUTE_H_
/*
 *  cMDEExpressionAttribute.h
 *  
 *
 *
 */
#include <string>
#include <iostream>
#include <cassert>
#include <set>



class cMDEExpressionAttribute {
	
public:
	cMDEExpressionAttribute() {}
	~cMDEExpressionAttribute() {}
	
	cMDEExpressionAttribute (std::string n, std::string t, std::string c) { 
		_att_name =n; _att_type = t; _att_class=c; }
	
	// get functions
	std::string getInEnglish() { return (_att_class + "." + _att_name); }
	std::string getInPromela() { return (_att_class + "_V." + _att_name); }	
	//std::set<std::string> getRelatedClasses() { return _related_classes; }
	std::string getAttName() { return _att_name; }
	std::string getClassName() { return _att_class; }
	
	// set functions
	//void setRelatedClasses(std::set<std::string> s) { _related_classes = s; }	

		
protected:
	std::string _att_name;
	std::string _att_type;
	std::string _att_class; 
	//std::set<std::string> _related_classes;
	
};




#endif
