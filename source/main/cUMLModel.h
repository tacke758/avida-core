#ifndef _C_UMLMODEL_H_
#define _C_UMLMODEL_H_

#include "cUMLClass.h"
#include "cUMLStateDiagram.h"
#include "cMDEPropertyGenerator.h"
#include "cMDEExpressionAttribute.h"

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
	std::map<std::string, float> utilityMap;
	std::string alt_set_name;
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
	
	void ResetModel();
	
	// Used to check if the diagrams satisfy the specified scenarios
	double checkForScenarios();
	
	// Used to generate and access the XMI version of the model.
	std::string getXMI(); // get the XMI version of the model.
	void printXMI(); // create the XMI version of the model.
	void printUMLModelToFile(std::string);  // print the UML model to a file.
	int getUMLValue(); // print movie info
	
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
	void printScenarioUtility(std::string);
		
	// Get the number of, well, everything
	int numStates();
	int numTrans();
	int numTriggers();
	int numGuards();
	int numActions();
	int numSDs();
	int numSCs();
	cMDEPropertyGenerator* getPropertyGenerator() { return gen; }
	void createExpressionsFromClasses();


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

	// check if the model is ready for hydra
	bool readyForHydra(); 
	bool getWitnessMode() {return witnessMode; }
	int getGenMode() { return genMode; }
	int getRelatedClassMode() { return relatedClassMode; }
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
	// Mode 0 = model generation; mode 1 = property generation
	int genMode;
	// Mode 0 = print
	// Mode 1 = contribute to interest + print
	// Mode 2 = forbid unrelated + print
	int relatedClassMode;
	double percent_scenario_complete;
	
  // The following are set once per Avida experiment, when the first cUMLModel is constructed:
  static bool _cfgLoaded; //!< Whether or not we've already loaded seed-model.cfg.
  static std::vector<cUMLStateDiagram> _cfg_state_diagrams; //!< State diagrams as read from seed-model.cfg.
  static std::vector<cUMLClass> _cfg_classes; // !< Classes read from seed model config
  
  static std::vector<scenario_info> _cfg_scenarios; //!< Scenarios as read from seed-model.cfg.
  static int _cfg_hydra_mode; //!< Execution mode for hydra, as read from seed-model.cfg.
  static bool _cfg_witness_mode; //!< Execution mode for hydra, as read from seed-model.cfg.
  static int _cfg_gen_mode; //!< Generation mode, as read from seed-model.cfg.
  static int _cfg_related_class_mode; //!< Related class mode, as read from seed-model.cfg
								  // where 0 = print, 1 = interest + print, 2 = forbid + print
  static cMDEPropertyGenerator _cfg_gen; 
  
  cMDEPropertyGenerator* gen; 

};



#endif
