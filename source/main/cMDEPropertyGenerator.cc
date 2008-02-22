
#include "cMDEPropertyGenerator.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <iomanip>
#include <errno.h>


using namespace std;

cMDEPropertyGenerator::cMDEPropertyGenerator() {
  //init property data
  expression_p = 0;
  expression_q = 0;
  expression_r = 0;
  m_property_reward = 0;
  m_property_success =0;
  m_property_failure =0;
  m_absence_property_success =0;
  m_absence_property_failure =0;
  m_existence_property_success =0;
  m_existence_property_failure =0;
  m_universal_property_success =0;
  m_universal_property_failure =0;
  m_precedence_property_success =0;
  m_precedence_property_failure =0;
  m_response_property_success =0;
  m_response_property_failure =0;
}


cMDEPropertyGenerator::~cMDEPropertyGenerator()
{
	for( std::set<cMDEProperty*, ltcMDEProperty>::iterator it = mdeprops.begin(),
		e = mdeprops.end(); it != e; )
	{
		cMDEProperty *p = *it;
		mdeprops.erase(it++);
		delete p;
	}

}


float cMDEPropertyGenerator::addExistenceProperty(std::string s)
{
	// a pointer to the existence property
	float val = 0;	
	cMDEExistenceProperty e(s);
	
	// first, try to find the property
	//
	std::set<cMDEProperty*, ltcMDEProperty>::iterator mdepropiter = mdeprops.find(&e);
	if (mdepropiter != mdeprops.end()) {
		val = (*mdepropiter)->getEvaluationInformation();
	} else {
		e.evaluate();
		val = e.getEvaluationInformation();
		mdeprops.insert(new cMDEExistenceProperty(e));
		if (val >0) {
			m_property_success++;
			m_existence_property_success++;
		} else { 
			m_property_failure++;
			m_existence_property_failure++;
		}
	}
	
	return val;
}

float cMDEPropertyGenerator::addAbsenceProperty(std::string s)
{
	// a pointer to the absence property
	float val = 0;
	cMDEAbsenceProperty e(s);
	
	std::set<cMDEProperty*, ltcMDEProperty>::iterator mdepropiter = mdeprops.find(&e);
	if (mdepropiter != mdeprops.end()) {
		val = (*mdepropiter)->getEvaluationInformation();
	} else {
		e.evaluate();
		val = e.getEvaluationInformation();
		mdeprops.insert (new cMDEAbsenceProperty(e));
		if (val >0) {
			m_property_success++;
			m_absence_property_success++;
		} else { 
			m_property_failure++;
			m_absence_property_failure++;
		}
	}
	
	return val;
	
}

float cMDEPropertyGenerator::addUniversalProperty(std::string s)
{
	// a pointer to the universal property
	float val = 0;	
	cMDEUniversalProperty e(s);
	
	std::set<cMDEProperty*, ltcMDEProperty>::iterator mdepropiter = mdeprops.find(&e);
	if (mdepropiter != mdeprops.end()) {
		val = (*mdepropiter)->getEvaluationInformation();
	} else {
		e.evaluate();
		val = e.getEvaluationInformation();
		mdeprops.insert (new cMDEUniversalProperty(e));
		if (val >0) {
			m_property_success++;
			m_universal_property_success++;
		} else { 
			m_property_failure++;
			m_universal_property_failure++;
		}
	}
	
	return val;
	
}



// Check if the expression exists in the vector. If not, add it.
bool cMDEPropertyGenerator::addExpression(std::string s, std::set<std::string> t)
{ 
	bool val = false;
	cMDEExpression e; 
	e.setExpr(s);
	e.setRelatedClasses(t);
	std::vector<cMDEExpression>::iterator exprit;
	exprit = find(expressions.begin(), expressions.end(), e); 
	if (exprit == expressions.end()) { 
		expressions.push_back(e); 
		val = true;
	} 
	return val;
}

// AND expressions p & q to create a new expression
// return true if this is a new expression
// return false otherwise
bool cMDEPropertyGenerator::ANDExpressions()
{
	bool val = false;
	std::set<std::string> class_names, class_names2;
	std::set<std::string>::iterator c;
	std::string totalstring;
	cMDEExpression p, q;
	
	if (expression_p != expression_q){
		p = getP();
		q = getQ();
		totalstring = "(" + p.getExpr() + " && " + q.getExpr() + ")";

		class_names = p.getRelatedClasses();
		class_names2 = q.getRelatedClasses();
		
		for (c=class_names2.begin(); c != class_names2.end(); c++) { 
			class_names.insert(*c);
		}
		
		val = addExpression(totalstring, class_names); 
	}
	return (val);
}

// OR expressions p & q to create a new expression
// return true if this is a new expression
// return false otherwise
bool cMDEPropertyGenerator::ORExpressions()
{
	
	bool val = false;
	std::set<std::string> class_names, class_names2;
	std::set<std::string>::iterator c;
	std::string totalstring;
	cMDEExpression p, q;

	if (expression_p != expression_q){
		p = getP();
		q = getQ();
		
		class_names = p.getRelatedClasses();
		class_names2 = q.getRelatedClasses();

		for (c=class_names2.begin(); c != class_names2.end(); c++) { 
			class_names.insert(*c);
		}
		
		totalstring = "(" + p.getExpr() + " || " + q.getExpr() + ")";
		val = addExpression(totalstring, class_names); 
	}
	return (val);
}

float cMDEPropertyGenerator::addResponseProperty(std::string s1, std::string s2)
{
	// a pointer to the universal property
	float val = 0;	
	cMDEResponseProperty e(s1, s2);
	
	std::set<cMDEProperty*, ltcMDEProperty>::iterator mdepropiter = mdeprops.find(&e);
	if (mdepropiter != mdeprops.end()) {
		val = (*mdepropiter)->getEvaluationInformation();
	} else {
		e.evaluate();
		val = e.getEvaluationInformation();
		mdeprops.insert (new cMDEResponseProperty(e));
		if (val >0) {
			m_property_success++;
			m_response_property_success++;
		} else { 
			m_property_failure++;
			m_response_property_failure++;
		}
	}
	return val;
}

float cMDEPropertyGenerator::addPrecedenceProperty(std::string s1, std::string s2)
{
	// a pointer to the universal property
	float val = 0;	
	cMDEPrecedenceProperty e(s1, s2);
	
	std::set<cMDEProperty*, ltcMDEProperty>::iterator mdepropiter = mdeprops.find(&e);
	if (mdepropiter != mdeprops.end()) {
		val = (*mdepropiter)->getEvaluationInformation();
	} else {
		e.evaluate();
		val = e.getEvaluationInformation();
		mdeprops.insert (new cMDEPrecedenceProperty(e));
		if (val >0) {
			m_property_success++;
			m_precedence_property_success++;
		} else { 
			m_property_failure++;
			m_precedence_property_failure++;
		}
	}
	return val;	
	
}


	
void cMDEPropertyGenerator::printExpressions() 
{
	std::vector<cMDEExpression>::iterator exprit;
	for (exprit = expressions.begin(); exprit < expressions.end(); exprit++){
		std::cout << exprit->getExpr() << std::endl;
	}

}



