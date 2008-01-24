#ifndef _C_UMLMODEL_H_
#define _C_UMLMODEL_H_

#include "cUMLClass.h"
#include "cUMLStateDiagram.h"
#include "cMDEProperty.h"
#include "cMDEExistenceProperty.h"
#include "cMDEAbsenceProperty.h"
#include "cMDEUniversalProperty.h"

#include <string>
#include <iostream>
#include <map>
#include <utility>
#include <algorithm>              
#include <set>
#include <vector>
#include <fstream>


struct scenario_info { 
	int stateDiagramID;
	std::deque<std::string> path; 
	bool shouldLoop;
	int startState;
};

struct xmi_info { 
    std::string xmi_begin;
    std::string xmi_end;
	std::vector<std::string> classes_info;
};

class cUMLModel { 
public:
  //! Constructor that loads state diagram and scenario information from seed-model.cfg.
	cUMLModel(const char* seed_model);
	~cUMLModel();
	
	// Used to check if the diagrams satisfy the specified scenarios
	double checkForScenarios();
	
	// Used to generate and access the XMI version of the model.
	std::string getXMI(); // get the XMI version of the model.
	void printXMI(); // create the XMI version of the model.
	void printUMLModelToFile(std::string);  // print the UML model to a file.
	
	// Used to update and get the max number of trans. 
	int getMaxTrans() { return max_trans; } 
	void setMaxTrans( int m ) { max_trans = m; } 
	
	// Access the size of the state diagrams and also a specific state diagram
	unsigned int getStateDiagramSize() { return state_diagrams.size(); } 
	cUMLStateDiagram* getStateDiagram (unsigned int); 
	
	// Set and access the bonus info for a model. Should be used by the tasks
	void setBonusInfo (std::string s, float f) { bonus_info[s] = f; } 
	float getBonusInfo (std::string);  
	std::vector<double> getScenarioCompletion() { return scenario_completion; } 
	void setScenarioCompletion(std::vector<double> s)  { scenario_completion = s; }
		
	// Get the number of, well, everything
	int numStates();
	int numTrans();
	int numTriggers();
	int numGuards();
	int numActions();
	int numSDs();
	int numSCs();
	int numSuccess() { return m_property_success; } 
	int numFailure() { return m_property_failure; }
	int numTotalProperty() { return m_property_success + m_property_failure; }
	int numAbsencePropertySuccess() { return m_absence_property_success; }
	int numAbsencePropertyFailure() { return m_absence_property_failure; }
	int numAbsencePropertyTotal() { return m_absence_property_success + m_absence_property_failure; }
	int numExistencePropertySuccess() { return m_existence_property_success; }
	int numExistencePropertyFailure() { return m_existence_property_failure; }
	int numExistencePropertyTotal() { return m_existence_property_success + m_existence_property_failure; }
	int numUniversalPropertySuccess() { return m_universal_property_success; }
	int numUniversalPropertyFailure() { return m_universal_property_failure; }
	int numUniversalPropertyTotal() { return m_universal_property_success + m_universal_property_failure; }	
	int propertySize() { return mdeprops.size(); }
	
	void resetPropertyReward() { m_property_reward = 0; }
	void addPropertyReward(float x) { m_property_reward += x; }
	float getPropertyReward() { return m_property_reward; }
	
	
	
	// Properties
	float addExistenceProperty(std::string);
	float addAbsenceProperty(std::string);
	float addUniversalProperty(std::string);
	bool addExpression(std::string s);  
	void createExpressionsFromClasses();
	std::string StringifyAnInt(int);
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
	std::string getP(); // { return expressions[expression_p]; } 
	std::string getQ() { return expressions[expression_q]; }
	std::string getR() { return expressions[expression_r]; }
	bool relativeMoveExpressionP(int x) { return relativeMoveIndex(expressions, expression_p, x); }
	bool absoluteMoveExpressionP(int x) { return absoluteMoveIndex(expressions, expression_p, x); }
	bool relativeMoveExpressionQ(int x) { return relativeMoveIndex(expressions, expression_q, x); }
	bool absoluteMoveExpressionQ(int x) { return absoluteMoveIndex(expressions, expression_q, x); }
	bool relativeMoveExpressionR(int x) { return relativeMoveIndex(expressions, expression_r, x); }
	bool absoluteMoveExpressionR(int x) { return absoluteMoveIndex(expressions, expression_r, x); }
	
	bool ANDExpressions();
	bool ORExpressions();
	
	// check if the model is ready for hydra
	bool readyForHydra(); 
	bool getWitnessMode() {return witnessMode; }
//	float checkProperties();
	

protected: 
	static xmi_info xi;
	std::string	xmi; 
	static int max_trans;
	std::vector<cUMLStateDiagram> state_diagrams;
	std::vector<cUMLClass> classes;
	std::map<std::string, float> bonus_info;
	std::vector<scenario_info> scenarios;
	std::vector<double> scenario_completion;
	int hydraMode; 
	bool witnessMode;
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
	float m_property_reward;
	

	// vector of expressions
	std::vector<std::string> expressions;
	
	
  // The following are set once per Avida experiment, when the first cUMLModel is constructed:
  static bool _cfgLoaded; //!< Whether or not we've already loaded seed-model.cfg.
  static std::vector<cUMLStateDiagram> _cfg_state_diagrams; //!< State diagrams as read from seed-model.cfg.
  static std::vector<cUMLClass> _cfg_classes; // !< Classes read from seed model config
  
  static std::vector<scenario_info> _cfg_scenarios; //!< Scenarios as read from seed-model.cfg.
  static int _cfg_hydra_mode; //!< Execution mode for hydra, as read from seed-model.cfg.
  static bool _cfg_witness_mode; //!< Execution mode for hydra, as read from seed-model.cfg.

};



#endif
