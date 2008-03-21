
#include "cMDEPropertyGenerator.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <iomanip>
#include <errno.h>


using namespace std;

cMDEPropertyGenerator::cMDEPropertyGenerator(int rcm) {
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
  m_suppressed = 0;
  m_related_class_mode = rcm; 
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

float cMDEPropertyGenerator::addKnownExistenceProperty(std::string s){
	cMDEExistenceProperty* e = new cMDEExistenceProperty(s);
	e->setInterestingProperty(0);
	e->setUsesRelatedClasses(0);	
	e->setSuppressed(true);
	return 0;
}

float cMDEPropertyGenerator::addKnownAbsenceProperty(std::string s){
	cMDEAbsenceProperty* e = new cMDEAbsenceProperty(s);
	e->setInterestingProperty(0);
	e->setUsesRelatedClasses(0);	
	e->setSuppressed(true);
	return 0;
}

float cMDEPropertyGenerator::addKnownUniversalProperty(std::string s){
	cMDEUniversalProperty* e = new cMDEUniversalProperty(s);
	e->setInterestingProperty(0);
	e->setUsesRelatedClasses(0);	
	e->setSuppressed(true);	
	return 0;
}

float cMDEPropertyGenerator::addKnownPrecedenceProperty(std::string s1, std::string s2){
	cMDEPrecedenceProperty* e = new cMDEPrecedenceProperty(s1, s2);
	e->setInterestingProperty(0);
	e->setUsesRelatedClasses(0);	
	e->setSuppressed(true);
	return 0;
}

float cMDEPropertyGenerator::addKnownResponseProperty(std::string s1, std::string s2){
	cMDEResponseProperty* e = new cMDEResponseProperty(s1, s2);
	e->setInterestingProperty(0);
	e->setUsesRelatedClasses(0);	
	e->setSuppressed(true);
	return 0;
}

float cMDEPropertyGenerator::addExistenceProperty(cMDEExpression* expr)
{

	float val = 0;	
	float interesting = 1; // 1 point for an existence property.
	bool related;
	bool left = false;
	bool right = false;
	
	// get the string representation of the expression.
	cMDEExistenceProperty e(expr->getExpr());
	
	// exit if expression is null. (Should never be the case)
	if (expr->getExpr() == "<null>") return val;
	
	std::set<cMDEProperty*, ltcMDEProperty>::iterator mdeiterleft, mdeiterright;
	// if an OR compound, compute the left & right pieces of the OR. 
	if (expr->getCompound()) { 
		if (expr->getOp() == "||") { 
			cMDEExistenceProperty e_left(expr->getLeft()->getExpr()); 
			cMDEExistenceProperty e_right(expr->getRight()->getExpr()); 
			
			mdeiterleft = mdeprops.find(&e_left);
			mdeiterright = mdeprops.find(&e_right);
		
			if (mdeiterleft != mdeprops.end()) {
				left = ((*mdeiterleft)->getEvaluationInformation());
			}
			if (mdeiterright != mdeprops.end()) {
				right = ((*mdeiterright)->getEvaluationInformation());
			}

		}
	}
	
	// first, try to find the property & check for redundancy....
	std::set<cMDEProperty*, ltcMDEProperty>::iterator mdepropiter = mdeprops.find(&e);
	
	if (mdepropiter != mdeprops.end()) {
		val = (*mdepropiter)->getEvaluationInformation();
		val += (*mdepropiter)->getInteresting();
	} else {
		// add in how interesting the expression is. Use the STRONG and form. 
		expr->interestingStrongANDExpressionEval(); 
		interesting += expr->getInterestingExpressionEval();
		interesting += getExpressionRelevancy(expr);
			
		// determine if they are related
		related = expr->getUsesRelatedClasses();
		e.setInterestingProperty(interesting);
		e.setUsesRelatedClasses(related);

		if (left) { 
//			std::cout << "LEFT" << std::endl;
			// the left expression is the stronger form of this expression... 
			val = (*mdeiterleft)->getEvaluationInformation();
			e.setSuppressed(true);
			m_suppressed++;
		} else if (right) {
//			std::cout << "RIGHT" << std::endl;
			// the left expression is the stronger form of this expression... 
			val = (*mdeiterright)->getEvaluationInformation();
			e.setSuppressed(true);
			m_suppressed++;
		} else {
			e.evaluate();
			val = e.getEvaluationInformation();
			e.setSuppressed(false);
		}
		mdeprops.insert(new cMDEExistenceProperty(e));
//		std::cout << "val: " << val << std::endl;
		if (val >0) {
			m_property_success++;
			m_existence_property_success++;
		} else { 
			m_property_failure++;
			m_existence_property_failure++;
			e.setInterestingProperty(0);
		}
	}
	
	if ((m_related_class_mode == 2) && (related == 1)) { val += .5; }
	if ((m_related_class_mode == 3) && (related == 0)) { val =0; }
	
	return val;
}



float cMDEPropertyGenerator::addAbsenceProperty(cMDEExpression* expr)
{
	// a pointer to the absence property
	float val = 0;
	float interesting = 3; // 3 points for an absence property.
	bool related;	
	bool left = false;
	bool right = false;


	// get the string representation of the expression.
	cMDEAbsenceProperty e(expr->getExpr());
	
	// exit if expression is null. (Should never be the case)
	if (expr->getExpr() == "<null>") return val;
	
	std::set<cMDEProperty*, ltcMDEProperty>::iterator mdeiterleft, mdeiterright;
	// if an AND compound, compute the left & right pieces of the OR. 
	if (expr->getCompound()) { 
		if (expr->getOp() == "&&") { 
			cMDEExistenceProperty e_left(expr->getLeft()->getExpr()); 
			cMDEExistenceProperty e_right(expr->getRight()->getExpr()); 
			
			mdeiterleft = mdeprops.find(&e_left);
			mdeiterright = mdeprops.find(&e_right);
		
			if (mdeiterleft != mdeprops.end()) {
				left = ((*mdeiterleft)->getEvaluationInformation());
			}
			if (mdeiterright != mdeprops.end()) {
				right = ((*mdeiterright)->getEvaluationInformation());
			}

		}
	}
	
	// first, try to find the property & check for redundancy....
	std::set<cMDEProperty*, ltcMDEProperty>::iterator mdepropiter = mdeprops.find(&e);
	if (mdepropiter != mdeprops.end()) {
		val = (*mdepropiter)->getEvaluationInformation();
		val += (*mdepropiter)->getInteresting();
	} else {
	
		// add in how interesting the expression is. Use the WEAK and form. 
		expr->interestingWeakANDExpressionEval(); 
		interesting += expr->getInterestingExpressionEval();
		interesting += getExpressionRelevancy(expr);
		
		// determine if they are related
		related = expr->getUsesRelatedClasses();
		
		e.setInterestingProperty(interesting);
		
		if (left) { 
			std::cout << "LEFT" << std::endl;
			// the left expression is the stronger form of this expression... 
			val = (*mdeiterleft)->getEvaluationInformation();
			e.setSuppressed(true);
			m_suppressed++;
		} else if (right) {
//			std::cout << "RIGHT" << std::endl;
			// the left expression is the stronger form of this expression... 
			val = (*mdeiterright)->getEvaluationInformation();
			e.setSuppressed(true);
			m_suppressed++;
		} else {
			e.evaluate();
			val = e.getEvaluationInformation();
			e.setSuppressed(false);
		}
		
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
	
	
	if ((m_related_class_mode == 2) && (related == 1)) { val += .5; }
	if ((m_related_class_mode == 3) && (related == 0)) { val =0; }
	
	return val;
	
}

float cMDEPropertyGenerator::addUniversalProperty(cMDEExpression* expr)
{
	// a pointer to the universal property
	float val = 0;	
	float interesting = 3; // 3 points for a universal property.
	bool related;
	bool left = false;
	bool right = false;
	
	cMDEUniversalProperty e(expr->getExpr());
	// exit if expression is null. (Should never be the case)
	if (expr->getExpr() == "<null>") return val;
	
	std::set<cMDEProperty*, ltcMDEProperty>::iterator mdeiterleft, mdeiterright;
	// if an OR compound, compute the left & right pieces of the OR. 
	if (expr->getCompound()) { 
		if (expr->getOp() == "||") { 
			cMDEExistenceProperty e_left(expr->getLeft()->getExpr()); 
			cMDEExistenceProperty e_right(expr->getRight()->getExpr()); 
			
			mdeiterleft = mdeprops.find(&e_left);
			mdeiterright = mdeprops.find(&e_right);
					
			if (mdeiterleft != mdeprops.end()) {
				left = ((*mdeiterleft)->getEvaluationInformation());
			}
			if (mdeiterright != mdeprops.end()) {
				right = ((*mdeiterright)->getEvaluationInformation());
			}
		}
	}
	
	
	std::set<cMDEProperty*, ltcMDEProperty>::iterator mdepropiter = mdeprops.find(&e);
	if (mdepropiter != mdeprops.end()) {
		val = (*mdepropiter)->getEvaluationInformation();
		val += (*mdepropiter)->getInteresting();
	} else {
	
		// add in how interesting the expression is. Use the STRONG and form. 
		expr->interestingStrongANDExpressionEval(); 
		interesting += expr->getInterestingExpressionEval();
		e.setInterestingProperty(interesting);
		interesting += getExpressionRelevancy(expr);
		
		// determine if they are related
		related = expr->getUsesRelatedClasses();
		
		if (left) { 
			std::cout << "LEFT" << std::endl;
			// the left expression is the stronger form of this expression... 
			val = (*mdeiterleft)->getEvaluationInformation();
			e.setSuppressed(true);
			m_suppressed++;
		} else if (right) {
			std::cout << "RIGHT" << std::endl;
			// the left expression is the stronger form of this expression... 
			val = (*mdeiterright)->getEvaluationInformation();
			e.setSuppressed(true);
			m_suppressed++;
		} else {
			e.evaluate();
			val = e.getEvaluationInformation();
			e.setSuppressed(false);
		}
		
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
	
	if ((m_related_class_mode == 2) && (related == 1)) { val += .5; }
	if ((m_related_class_mode == 3) && (related == 0)) { val =0; }
	
	return val;
	
}


float cMDEPropertyGenerator::addResponseProperty(cMDEExpression* e1, cMDEExpression* e2)
{
	// a pointer to the universal property
	float val = 0;	
	float interesting = 2; // 2 points for a response property.
	bool related;
	

	cMDEResponseProperty e(e1->getExpr(), e2->getExpr());
	bool dependent = areExpressionsAtsOpsDependent(e1, e2);
	// exit if the expressions are dependent.
	if (dependent) return val;

	std::set<cMDEProperty*, ltcMDEProperty>::iterator mdepropiter = mdeprops.find(&e);
	if (mdepropiter != mdeprops.end()) {
		val = (*mdepropiter)->getEvaluationInformation();
		val += (*mdepropiter)->getInteresting();
	} else {
	
		e1->interestingStrongANDExpressionEval(); 
		e2->interestingStrongANDExpressionEval(); 
		interesting += e1->getInterestingExpressionEval() + e1->getInterestingExpressionEval();
		interesting += getExpressionRelevancy(e1) + getExpressionRelevancy(e2);

		bool related = areExpressionsRelated(e1, e2);

		e.setSuppressed(false);
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
	
	if ((m_related_class_mode == 2) && (related == 1)) { val += .5; }
	if ((m_related_class_mode == 3) && (related == 0)) { val =0; }

	return val;
}

float cMDEPropertyGenerator::addPrecedenceProperty(cMDEExpression* e1, cMDEExpression* e2)
{
	// a pointer to the universal property
	float val = 0;	
	float interesting = 2; // 2 points for a precedence property.
	bool related;

	
	bool dependent = areExpressionsAtsOpsDependent(e1, e2);
	// exit if the expressions are dependent.
	if (dependent) return val;

	cMDEPrecedenceProperty e(e1->getExpr(), e2->getExpr());
	
	std::set<cMDEProperty*, ltcMDEProperty>::iterator mdepropiter = mdeprops.find(&e);
	if (mdepropiter != mdeprops.end()) {
		val = (*mdepropiter)->getEvaluationInformation();
		val += (*mdepropiter)->getInteresting();
	} else {
	
		e.setSuppressed(false);
		e1->interestingStrongANDExpressionEval(); 
		e2->interestingStrongANDExpressionEval(); 
		interesting += e1->getInterestingExpressionEval() + e1->getInterestingExpressionEval();
		interesting += getExpressionRelevancy(e1) + getExpressionRelevancy(e2);
		bool related = areExpressionsRelated(e1, e2);
		
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
	
	if ((m_related_class_mode == 2) && (related == 1)) { val += .5; }
	if ((m_related_class_mode == 3) && (related == 0)) { val = 0; }

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
	e->addRelatedClassName(c);
	e->addAttOpName(c + "." + n);
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
	e->addRelatedClassName(a1->getClassName());
	e->addAttOpName(a1->getClassName() + "." + a1->getAttName());
	e->addAttOpName(a1->getClassName() + "." + a2->getAttName());
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
	e->addRelatedClassName(a1->getClassName());
	e->addAttOpName(a1->getClassName() + "." + a1->getAttName());
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
	
	// If ats/ops are dependent, then don't create the expression
	if (areExpressionsAtsOpsDependent(e1, e2)) return false;
	
	// determine if it uses related classes
	e->setUsesRelatedClasses(areExpressionsRelated(e1, e2));
	
	
	// set at op names.
	e->setAttOpNames(e1->getAtOpNames()); 
	std::set<std::string> rcns = e2->getAtOpNames();
	for (std::set<std::string>::iterator it = rcns.begin(); it!=rcns.end(); it++) { 
		e->addAttOpName(*it);
	}
	
	// set related class names
	e->setRelatedClassNames(e1->getRelatedClassNames()); 
	rcns = e2->getRelatedClassNames();
	for (std::set<std::string>::iterator it = rcns.begin(); it!=rcns.end(); it++) { 
		e->addRelatedClassName(*it);
	}
	
	// set used class names
	e->setUsedClassNames(e1->getUsedClassNames()); 
	std::set<std::string> ucns = e2->getUsedClassNames();
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
	std::set<std::string> used_class_names;
	std::set<std::string> related_class_names;
	std::set<std::string>::iterator it;
	
	int count =0; 
	for (exprit = expressions.begin(); exprit < expressions.end(); exprit++){
		used_class_names = (*exprit)->getUsedClassNames(); 
		related_class_names = (*exprit)->getRelatedClassNames();
		std::cout << count << " " << (*exprit)->getExpr() << std::endl;
		std::cout << "     uses related classes: " << (*exprit)->getUsesRelatedClasses() << std::endl;
		std::cout << "     used class names: " << std::endl;
		for (it = used_class_names.begin(); it!=used_class_names.end(); it++) {
			std::cout << "          " << (*it) << std::endl;
		}
		std::cout << "     related class names: " << std::endl;
		for (it = related_class_names.begin(); it!=related_class_names.end(); it++) {
			std::cout << "          " << (*it) << std::endl;
		}
		
		count++;
	}

}


bool cMDEPropertyGenerator::areExpressionsRelated(cMDEExpression* e1, cMDEExpression* e2)
{
	// Get the related class names of expression 1
	// Get the used class names of expression 2
	std::set<std::string> rcns = e1->getRelatedClassNames();
	std::set<std::string> ucns = e2->getUsedClassNames();
	
	// determine if it uses related classes
	bool test = includes(rcns.begin(), rcns.end(), ucns.begin(), ucns.end());
	
	return test;
}

// try a set intersection and check that it is null.
bool cMDEPropertyGenerator::areExpressionsAtsOpsDependent(cMDEExpression* e1, cMDEExpression* e2)
{
	// Get the at op names of expression 1
	// Get the at op names of expression 2
	std::set<std::string> atop1 = e1->getAtOpNames();
	std::set<std::string> atop2 = e2->getAtOpNames();
	std::set<std::string> results;
	insert_iterator<std::set<std::string> > it1(results, results.begin());
	
	// determine if it uses related classes
	set_intersection(atop1.begin(), atop1.end(), atop2.begin(), atop2.end(), it1);
	
	// If the set is empty then the expressions are independent.
	if (results.size() == 0) return false; 
	
	return true;
}


float cMDEPropertyGenerator::getExpressionRelevancy(cMDEExpression* e) 
{
	float total =0;
	// check if the expression uses the relevant attributes or operations
	for (int i=0; i<relevant_attributes.size(); i++) {
		total += e->usesAttribute(relevant_attributes[i]);
//		std::cout << "relevant at " << relevant_attributes[i] << " " << total << std::endl;
	}
	
	for (int i=0; i<relevant_operations.size(); i++) {
		total += e->usesOperation(relevant_operations[i]);
//		std::cout << "relevant op " << relevant_operations[i] << " " << total << std::endl;

	}
	
}



