#ifndef _C_MDEPROPERTYGENERATOR_H_
#define _C_MDEPROPERTYGENERATOR_H_

#include "cMDEProperty.h"
#include "cMDEExistenceProperty.h"
#include "cMDEAbsenceProperty.h"
#include "cMDEUniversalProperty.h"
#include "cMDEResponseProperty.h"
#include "cMDEPrecedenceProperty.h"
#include "cMDEExpression.h"
#include "cMDEExpressionAttribute.h"
#include "cMDESimpleOperationExpression.h"
#include "cMDESimpleAttAttExpression.h"
#include "cMDESimpleAttValExpression.h"
#include "cMDECompoundExpression.h"

#include <string>
#include <iostream>
#include <map>
#include <utility>
#include <algorithm>              
#include <set>
#include <vector>
#include <fstream>


class cMDEPropertyGenerator { 
public:
	cMDEPropertyGenerator(bool);
	~cMDEPropertyGenerator();
	
	// Get info about the properties satisfied by the model.
	int numSuccess() { return m_property_success; } 
	int numFailure() { return m_property_failure; }
	int numTotalProperty() { return m_property_success + m_property_failure; }
	int numAbsencePropertySuccess() { return m_absence_property_success; }
	int numAbsencePropertyFailure() { return m_absence_property_failure; }
	int numAbsencePropertyTotal() { return m_absence_property_success + m_absence_property_failure; }
	int numExistencePropertySuccess() { return m_existence_property_success; }
	int numExistencePropertyFailure() { return m_existence_property_failure; }
	int numExistencePropertyTotal() { 
		return m_existence_property_success + m_existence_property_failure; }
	int numUniversalPropertySuccess() { return m_universal_property_success; }
	int numUniversalPropertyFailure() { return m_universal_property_failure; }
	int numUniversalPropertyTotal() { 
		return m_universal_property_success + m_universal_property_failure; }	
	int numPrecedencePropertySuccess() { return m_precedence_property_success; }
	int numPrecedencePropertyFailure() { return m_precedence_property_failure; }
	int numPrecedencePropertyTotal() { 
		return m_precedence_property_success + m_precedence_property_failure; }	
	int numResponsePropertySuccess() { return m_response_property_success; }
	int numResponsePropertyFailure() { return m_response_property_failure; }
	int numResponsePropertyTotal() { return m_response_property_success + m_response_property_failure; }	
	
	int propertySize() { return mdeprops.size(); }
	
	void resetPropertyReward() { m_property_reward = 0; }
	void addPropertyReward(float x) { m_property_reward += x; }
	float getPropertyReward() { return m_property_reward; }
	
	bool addSimpleOperationExpression(std::string, std::string, std::set<std::string>); 
	bool addSimpleAttAttExpression(cMDEExpressionAttribute*, cMDEExpressionAttribute*, std::string, std::set<std::string>);
	bool addSimpleAttValExpression(cMDEExpressionAttribute*, std::string, std::string, std::set<std::string>); 
	bool addCompoundExpression(cMDEExpression*, cMDEExpression*, std::string);

	void printExpressions();
																											  

	// Properties
	float addExistenceProperty(std::string, float, bool);
	float addAbsenceProperty(std::string, float, bool);
	float addUniversalProperty(std::string, float, bool);
	float addResponseProperty(std::string, std::string, float, bool);
	float addPrecedenceProperty(std::string, std::string, float, bool);

//	bool addExpression(std::string, std::set<std::string>);  
//	bool relatedExpressions(cMDEExpression*, cMDEExpression*); 

	cMDEExpression* getP() { return expressions[expression_p]; } 
	cMDEExpression* getQ() { return (expressions[expression_q]);  }
	cMDEExpression* getR() { return (expressions[expression_r]); }
	std::string getPstring() { return (expressions[expression_p])->getExpr(); } 
	std::string getQstring() { return (expressions[expression_q])->getExpr();  }
	std::string getRstring() { return (expressions[expression_r])->getExpr(); }
	
	float getPInterest() { 
		(expressions[expression_p])->interestingExpressionEval(); 
		return (expressions[expression_p])->getInterestingExpressionEval() ; }
	float getQInterest() { 		
		(expressions[expression_q])->interestingExpressionEval(); 
		return (expressions[expression_q])->getInterestingExpressionEval() ; }
	float getRInterest() { 
		(expressions[expression_r])->interestingExpressionEval(); 
		return (expressions[expression_r])->getInterestingExpressionEval() ; }
	
	bool relativeMoveExpressionP(int x) { return relativeMoveIndex(expressions, expression_p, x); }
	bool absoluteMoveExpressionP(int x) { return absoluteMoveIndex(expressions, expression_p, x); }
	bool relativeMoveExpressionQ(int x) { return relativeMoveIndex(expressions, expression_q, x); }
	bool absoluteMoveExpressionQ(int x) { return absoluteMoveIndex(expressions, expression_q, x); }
	bool relativeMoveExpressionR(int x) { return relativeMoveIndex(expressions, expression_r, x); }
	bool absoluteMoveExpressionR(int x) { return absoluteMoveIndex(expressions, expression_r, x); }
	bool areExpressionsRelated(cMDEExpression*, cMDEExpression*);
	bool areExpressionsAtsOpsDependent(cMDEExpression*, cMDEExpression*);

	
	bool ANDExpressions();
	bool ORExpressions();
 
	template <typename T>
		bool absoluteMoveIndex (T x, int &y, int z)
	{
			
			int x_size = (int) x.size();
			if (x_size == 0 || z > x_size) {
				return false;
			}
			
			y = 0;
			return relativeMoveIndex(x, y, z);
	}
	
	template <typename T>
		bool relativeMoveIndex (T x, int &y, int z)
	{
			int x_size = (int) x.size();
			
			if (x_size == 0) {
				return false;
			}
			
			if (z > 0) { 
				y += (z % x_size);
				
				// index is greater than vector
				if (y >= x_size) { 
					y -= x_size;
				} else if(index < 0) { 
					y += x_size;
				}
			}	
			return true;
	}	
	

protected: 
	int expression_p;
	int expression_q;
	int expression_r;
	
	// set of properties.
	std::set<cMDEProperty*, ltcMDEProperty> mdeprops;
	int m_property_success;
	int m_property_failure;
	int m_absence_property_success;
	int m_absence_property_failure;
	int m_existence_property_success;
	int m_existence_property_failure;
	int m_universal_property_success;
	int m_universal_property_failure;
	int m_precedence_property_success;
	int m_precedence_property_failure;
	int m_response_property_success;
	int m_response_property_failure;
	float m_property_reward;
	int m_related_class_mode;
	

	// vector of expressions
	std::vector<cMDEExpression*> expressions;
	
};



#endif
