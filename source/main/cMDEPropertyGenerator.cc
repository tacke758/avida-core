
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
	
	for( std::vector<cMDEExpression*>::iterator it = expressions.begin();
		 it != expressions.end(); it++ )
	{
		delete *it;
	}
	expressions.erase(expressions.begin(), expressions.end());
}


float cMDEPropertyGenerator::addExistenceProperty(std::string s, float i)
{
	// a pointer to the existence property
	float val = 0;	
	float interesting = (i/5) + 1; // 1 point for an existence property.
	
	cMDEExistenceProperty e(s);
	
	// first, try to find the property
	std::set<cMDEProperty*, ltcMDEProperty>::iterator mdepropiter = mdeprops.find(&e);
	if (mdepropiter != mdeprops.end()) {
		val = (*mdepropiter)->getEvaluationInformation();
		val += (*mdepropiter)->getInteresting();
	} else {
		e.setInterestingProperty(interesting);
		e.evaluate();
		val = e.getEvaluationInformation();
		mdeprops.insert(new cMDEExistenceProperty(e));
		if (val >0) {
			m_property_success++;
			m_existence_property_success++;
		} else { 
			m_property_failure++;
			m_existence_property_failure++;
			e.setInterestingProperty(0);
		}
	}
	
	return val;
}

float cMDEPropertyGenerator::addAbsenceProperty(std::string s, float i)
{
	// a pointer to the absence property
	float val = 0;
	float interesting = (i/5) + 3; // 3 points for an absence property.

	cMDEAbsenceProperty e(s);
	
	std::set<cMDEProperty*, ltcMDEProperty>::iterator mdepropiter = mdeprops.find(&e);
	if (mdepropiter != mdeprops.end()) {
		val = (*mdepropiter)->getEvaluationInformation();
		val += (*mdepropiter)->getInteresting();
	} else {
		e.setInterestingProperty(interesting);
		e.evaluate();
		val = e.getEvaluationInformation();
		mdeprops.insert (new cMDEAbsenceProperty(e));
		if (val >0) {
			m_property_success++;
			m_absence_property_success++;
		} else { 
			e.setInterestingProperty(0);
			m_property_failure++;
			m_absence_property_failure++;
		}
	}
	
	return val;
	
}

float cMDEPropertyGenerator::addUniversalProperty(std::string s, float i)
{
	// a pointer to the universal property
	float val = 0;	
	float interesting = (i/5) + 3; // 3 points for a universal property.

	cMDEUniversalProperty e(s);
	
	std::set<cMDEProperty*, ltcMDEProperty>::iterator mdepropiter = mdeprops.find(&e);
	if (mdepropiter != mdeprops.end()) {
		val = (*mdepropiter)->getEvaluationInformation();
		val += (*mdepropiter)->getInteresting();
	} else {
		e.setInterestingProperty(interesting);
		e.evaluate();
		val = e.getEvaluationInformation();
		mdeprops.insert (new cMDEUniversalProperty(e));
		if (val >0) {
			m_property_success++;
			m_universal_property_success++;
		} else { 
			m_property_failure++;
			m_universal_property_failure++;
			e.setInterestingProperty(0);
		}
	}
	
	return val;
	
}


float cMDEPropertyGenerator::addResponseProperty(std::string s1, std::string s2, float i)
{
	// a pointer to the universal property
	float val = 0;	
	float interesting = (i/5) + 2; // 2 points for a response property.

	cMDEResponseProperty e(s1, s2);
	
	std::set<cMDEProperty*, ltcMDEProperty>::iterator mdepropiter = mdeprops.find(&e);
	if (mdepropiter != mdeprops.end()) {
		val = (*mdepropiter)->getEvaluationInformation();
		val += (*mdepropiter)->getInteresting();
	} else {
		e.setInterestingProperty(interesting);
		e.evaluate();
		val = e.getEvaluationInformation();
		mdeprops.insert (new cMDEResponseProperty(e));
		if (val >0) {
			m_property_success++;
			m_response_property_success++;
		} else { 
			m_property_failure++;
			m_response_property_failure++;
			e.setInterestingProperty(0);
		}
	}
	return val;
}

float cMDEPropertyGenerator::addPrecedenceProperty(std::string s1, std::string s2, float i)
{
	// a pointer to the universal property
	float val = 0;	
	float interesting = (i/5) + 2; // 2 points for a precedence property.

	cMDEPrecedenceProperty e(s1, s2);
	
	std::set<cMDEProperty*, ltcMDEProperty>::iterator mdepropiter = mdeprops.find(&e);
	if (mdepropiter != mdeprops.end()) {
		val = (*mdepropiter)->getEvaluationInformation();
		val += (*mdepropiter)->getInteresting();
	} else {
		e.setInterestingProperty(interesting);
		e.evaluate();
		val = e.getEvaluationInformation();
		
		mdeprops.insert (new cMDEPrecedenceProperty(e));
		if (val >0) {
			m_property_success++;
			m_precedence_property_success++;
		} else { 
			m_property_failure++;
			m_precedence_property_failure++;
			e.setInterestingProperty(0);

		}
	}
	return val;	
	
}


bool cMDEPropertyGenerator::addSimpleOperationExpression(std::string n, 
														std::string c, 
														std::set<std::string> rcs) 
{
	bool val = false;
	cMDESimpleOperationExpression* e = new cMDESimpleOperationExpression(n, c); 
	// set related class names
	e->setRelatedClassNames(rcs);
	e->addUsedClassName(c);
	e->setUsesRelatedClasses(true);
		
	std::vector<cMDEExpression*>::iterator exprit;
	exprit = find(expressions.begin(), expressions.end(), e); 
	if (exprit == expressions.end()) { 
		expressions.push_back(e); 
		val = true;
	} else {
		delete e;
	}
	return val;
}


bool cMDEPropertyGenerator::addSimpleAttAttExpression(cMDEExpressionAttribute* a1, 
												   cMDEExpressionAttribute* a2, 
												   std::string op ,
												   std::set<std::string> rcs) 
{
	bool val = false;
	cMDESimpleAttAttExpression* e = new cMDESimpleAttAttExpression(a1, a2, op); 
	// set related class names
	e->setRelatedClassNames(rcs);
	e->addUsedClassName(a1->getClassName());
	e->setUsesRelatedClasses(true);
	
	std::vector<cMDEExpression*>::iterator exprit;
	exprit = find(expressions.begin(), expressions.end(), e); 
	if (exprit == expressions.end()) { 
		expressions.push_back(e); 
		val = true;
	} else {
		delete e;
	}
	return val;
}

bool cMDEPropertyGenerator::addSimpleAttValExpression(cMDEExpressionAttribute* a1, 
												   std::string value, 
												   std::string op, 
												   std::set<std::string> rcs)
{
	bool val = false;
	cMDESimpleAttValExpression* e = new cMDESimpleAttValExpression(a1, value, op); 
	// set related class names
	e->setRelatedClassNames(rcs);
	e->addUsedClassName(a1->getClassName());
	e->setUsesRelatedClasses(true);

	std::vector<cMDEExpression*>::iterator exprit;
	exprit = find(expressions.begin(), expressions.end(), e); 
	if (exprit == expressions.end()) { 
		expressions.push_back(e); 
		val = true;
	} else {
		delete e;
	}
	return val;
	
}

bool cMDEPropertyGenerator::addCompoundExpression(cMDEExpression* e1, 
												  cMDEExpression* e2,
												  std::string op)
{
	bool val = false;
	cMDECompoundExpression* e = new cMDECompoundExpression(e1, e2, op); 
	
	// Get the related class names of expression 1
	// Get the used class names of expression 2
	std::set<std::string> rcns = e1->getRelatedClassNames();
	std::set<std::string> ucns = e2->getUsedClassNames();
	
	// determine if it uses related classes
	bool test = includes(rcns.begin(), rcns.end(), ucns.begin(), ucns.end());
	e->setUsesRelatedClasses(test);
	
	// set related class names
	e->setRelatedClassNames(e1->getRelatedClassNames()); 
	rcns = e2->getRelatedClassNames();
	for (std::set<std::string>::iterator it = rcns.begin(); it!=rcns.end(); it++) { 
		e->addRelatedClassName(*it);
	}
	
	// set used class names
	e->setUsedClassNames(e1->getUsedClassNames()); 
	ucns = e2->getUsedClassNames();
	for (std::set<std::string>::iterator it = ucns.begin(); it!=ucns.end(); it++) { 
		e->addUsedClassName(*it);
	}	
	
	std::vector<cMDEExpression*>::iterator exprit;
	exprit = find(expressions.begin(), expressions.end(), e); 
	if (exprit == expressions.end()) { 
		expressions.push_back(e); 
		val = true;
	} else {
		delete e;
	}
	return val;
	
}




// AND expressions p & q to create a new expression
// return true if this is a new expression
// return false otherwise
bool cMDEPropertyGenerator::ANDExpressions()
{
	bool val = false;
	
	if (expression_p != expression_q){
		val = addCompoundExpression(getP(), getQ(), "&&");
	}
	return (val);
}

// OR expressions p & q to create a new expression
// return true if this is a new expression
// return false otherwise
bool cMDEPropertyGenerator::ORExpressions()
{
	
	bool val = false;
	
	if (expression_p != expression_q){
		val = addCompoundExpression(getP(), getQ(), "||");
	}
	return (val);
}



	
void cMDEPropertyGenerator::printExpressions() 
{
	std::vector<cMDEExpression*>::iterator exprit;
	int count =0; 
	for (exprit = expressions.begin(); exprit < expressions.end(); exprit++){
		std::cout << count << " " << (*exprit)->getExpr() << std::endl;
		count++;
	}

}



