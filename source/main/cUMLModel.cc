#include "cUMLModel.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <iomanip>
#include <errno.h>


using namespace std;

xmi_info loadFile(const char* filename) {
	std::string data, line; // or maybe stringstream? (strstream?)
	std::ifstream infile;
	infile.open(filename);
	assert(infile.is_open());
	std::string s;
	xmi_info x;
	
	while (getline (infile, line))
	{
		// handle xmi_begin
		if (line == "=XMI-BEGIN==================") {
			line.erase();
			infile >> line;
			while (line != "=END========================") { 
				// cat line to the xmi_begin string...
				x.xmi_begin += (line + " ");
				infile >> line;
			}
//			std::cout << "XMI BEGIN" << x.xmi_begin << std::endl;
		} else if (line == "=XMI-END====================") { 
		// handle xmi_end
//			std::cout << "XMI END"<< std::endl;
			line.erase();
			infile >> line;
			while (line != "=END========================") { 
				// cat line to the xmi_begin string...
				x.xmi_end += (line + " ");
				infile >> line;
			}
//			std::cout << "XMI END" << x.xmi_end << std::endl;
		} else if (line == "=CLASS======================") { 
		// handle xmi_end
			line.erase();
			infile >> line;
			data = "";
			while (line != "=END========================") { 
				// cat line to the xmi_begin string...
				data += (line + " ");
				infile >> line;
			}
			x.classes_info.push_back(data);			
//			std::cout << "CLASS" << data << std::endl;
		}

		line.erase();
	}

	infile.close();
	return x;
}


/*! This function intializes variables with data read from a file named "seed-model.cfg". 

"This is a crappy little function to read in from a file. 
It is not robust. It will not understand things unless you 
follow the *very* specific file format."
*/
void seed_diagrams(const char* seed_model,
                   std::vector<cUMLStateDiagram>& state_diagrams,
                   std::vector<scenario_info>& scenarios,
                   int& hydra_mode, 
				   bool& witness_mode) {
  std::string data, line; // or maybe stringstream? (strstream?)
	int num_states;
	int num_sd = 0;
	int cur_sd = -1;
	bool include_trans = 0;
	std::string tr_l, tr_o, gu, act, temp;
	int trig_i, guard_i, act_i, orig_i, dest_i;
	std::ifstream infile;
	infile.open(seed_model);
	assert(infile.is_open());
	scenario_info s;
	std::string path_step;
	std::string class_name;
	
	while (getline (infile, line))
	{
		// Read in states
		if (line == "=STATES====================") {
			line.erase();
			infile >> num_states;
      // Read in number of state diagrams	
		} else if (line == "=HYDRA=====================") { 
			line.erase(); 
			infile >> hydra_mode;
		} else if (line == "=WITNESS===================") { 
			line.erase(); 
			infile >> witness_mode;
		} else if (line == "=INCLUDE-TRANSITIONS=======") { 
			line.erase(); 
			infile >> include_trans;
		} else if (line == "=STATE-DIAGRAM=============") { 
			line.erase();
			infile >> num_sd;
			state_diagrams.resize(num_sd);
      // Read in each state diagram	
		} else if (line == "=SD========================") { 
			line.erase();
			infile >> class_name;
			cur_sd++;
			state_diagrams[cur_sd].addName(class_name);
		} else if (line == "-TRIGGERS------------------") { 
			line.erase();
			infile >> tr_l;
			while (tr_l != "-END---------------------------") { 
				infile >> tr_o;
				state_diagrams[cur_sd].addTrigger(tr_l, tr_o);
				infile >> tr_l;
			}
		}else if (line == "-GUARDS--------------------") { 
			line.erase();
			infile >> gu;
			while (gu != "-END---------------------------") { 
				state_diagrams[cur_sd].addGuard(gu);
				infile >> gu;
			}
		} else if (line == "-ACTIONS--------------------") { 
			line.erase();
			infile >> act;
			while (act != "-END---------------------------") { 
				state_diagrams[cur_sd].addAction(act);
				infile >> act;
			}
		} else if (line == "-TRANSITIONS---------------") { 
			line.erase();
			infile >> temp; 
			while (temp != "-END---------------------------") { 
				infile >> orig_i >> dest_i >> trig_i >> guard_i >> act_i; 
				if (include_trans) { 
					state_diagrams[cur_sd].addTransitionTotal(orig_i, dest_i, trig_i, guard_i, act_i);
				}	
				infile >> temp; 
			}
		}  else if (line == "-SCENARIO----------------------") { 
			line.erase();
			s.path.clear();
			s.stateDiagramID = cur_sd;
			infile >> s.shouldLoop >> s.startState;
			infile >> temp;
			while (temp!= "-END---------------------------") { 
				infile >> path_step;
				s.path.push_back(path_step);
				infile >> temp;
			}
			scenarios.push_back(s);
			
		} else if (line == "-TRANSITION-LABELS-------------") { 
			line.erase();
			infile >> temp;			
			while (temp!= "-END---------------------------") { 
				infile >> trig_i >> guard_i >> act_i; 
				state_diagrams[cur_sd].addTransitionLabel(trig_i, guard_i, act_i); 
				infile >> temp; 
			}
		//	infile >> temp;
		}

		line.erase();
  }
  
	infile.close();
	return;
}


xmi_info cUMLModel::xi = loadFile("xmi_info");
int cUMLModel::max_trans = 0;


bool cUMLModel::_cfgLoaded = false;
std::vector<cUMLStateDiagram> cUMLModel::_cfg_state_diagrams;
std::vector<scenario_info> cUMLModel::_cfg_scenarios;
int cUMLModel::_cfg_hydra_mode;
bool cUMLModel::_cfg_witness_mode;


cUMLModel::cUMLModel(const char* seed_model) {
  if(!_cfgLoaded) {
    seed_diagrams(seed_model, _cfg_state_diagrams, _cfg_scenarios, _cfg_hydra_mode, _cfg_witness_mode);
    _cfgLoaded = true;
  }
  
  state_diagrams = _cfg_state_diagrams;
  scenarios = _cfg_scenarios;
  hydraMode = _cfg_hydra_mode; 
  witnessMode = _cfg_witness_mode;
}


cUMLModel::~cUMLModel()
{
	for( std::set<cMDEProperty*, ltcMDEProperty>::iterator it = mdeprops.begin(),
		e = mdeprops.end(); it != e; )
	{
		cMDEProperty *p = *it;
		mdeprops.erase(it++);
		delete p;
	}

}

float cUMLModel::getBonusInfo (std::string s)  
{ 
	float val;
	std::map<std::string, float>::iterator bonus_info_pointer;
	
	bonus_info_pointer = bonus_info.find(s);
	
	if (bonus_info_pointer != bonus_info.end()) { 
		val = bonus_info[s];
	} else { 
		val = 0;
	}
	return val; 
}


cUMLStateDiagram* cUMLModel::getStateDiagram (unsigned int x) 
{
  assert(x<state_diagrams.size());
  return &state_diagrams[x];
}


void cUMLModel::printXMI()
{
	xmi = "";
//	int v;
	
	xmi = xi.xmi_begin; 
	
	assert (xi.classes_info.size() == state_diagrams.size());
	
	for (unsigned int i=0; i<xi.classes_info.size(); i++) { 
		xmi += xi.classes_info[i];
		xmi += state_diagrams[i].getXMI("sd" + i);
	}
	
	xmi += xi.xmi_end;
	
}

std::string cUMLModel::getXMI()
{
	return xmi;
}

int cUMLModel::numStates() 
{ 
	int temp_states = 0;
	for (unsigned int i=0; i<state_diagrams.size(); i++) { 
		temp_states += getStateDiagram(i)->numStates();
	}
	return temp_states;
}

int cUMLModel::numTrans()
{
int temp_trans = 0;
	for (unsigned int i=0; i<state_diagrams.size(); i++) { 
		temp_trans += getStateDiagram(i)->numTrans();
	}
	return temp_trans;
}

int cUMLModel::numTriggers()
{
int temp_trigger = 0;
	for (unsigned int i=0; i<state_diagrams.size(); i++) { 
		temp_trigger += getStateDiagram(i)->numTriggers();
	}
	return temp_trigger;
}

int cUMLModel::numGuards() 
{
int temp = 0;
	for (unsigned int i=0; i<state_diagrams.size(); i++) { 
		temp += getStateDiagram(i)->numGuards();
	}
	return temp;
}

int cUMLModel::numActions()
{
int temp = 0;
	for (unsigned int i=0; i<state_diagrams.size(); i++) { 
		temp += getStateDiagram(i)->numActions();
	}
	return temp;
}

int cUMLModel::numSDs()
{
	return state_diagrams.size();
}

int cUMLModel::numSCs()
{
	return scenarios.size();
}

double cUMLModel::checkForScenarios()
{
	double total_bonus = 0;
	double temp_bonus = 0;
	double complete_bonus;
	scenario_info s;
	
	// Should check to see if each scenario is satisfied.
	// Iterate through list of scnearios; Call each scenario on the appropriate state diagram
	// Accrue results.
	// Save bonus info.
	scenario_completion.resize(scenarios.size());
	for (unsigned int i=0; i< scenarios.size(); i++) { 
		s = scenarios[i];
		temp_bonus = getStateDiagram(s.stateDiagramID)->findPath(s.path, s.shouldLoop, s.startState);

		complete_bonus = s.path.size() + s.shouldLoop; 
		if (s.startState >= 0) complete_bonus++;
		
		// The next line is commented out to increase the reward for a given scenario. 
		// total_bonus += (temp_bonus / complete_bonus);
		total_bonus += temp_bonus;
		
	scenario_completion[i] = temp_bonus / complete_bonus;
	}
		
	return total_bonus;
}

bool cUMLModel::readyForHydra() 
{
	// options: (0) ALL_COMPLETE, (1) ONE_COMPLETE, (2) ONE_NON_EMPTY, (3) ALL_NON_EMPTY
	//          (4) ALL COMPLETE && DETERMINISTIC, (5) NONE
	// check which option was selected in the seed-model.cfg
	// check to see if this condition is true. If so, return 1; otherwise, return 0.
	
	bool ret_val = 0;
//	double complete_bonus = 0;

	switch (hydraMode){
	case 0:
		ret_val = 1;
		for (unsigned int i=0; i< scenario_completion.size(); i++) { 
				//ret_val &= scenario_completion[i];
				if (scenario_completion[i] != 1) ret_val &= 0;
		}
		break;
	case 1:
		ret_val = 0;
		for (unsigned int i=0; i< scenario_completion.size(); i++) { 
				if (scenario_completion[i] == 1) ret_val = 1;
		}
		break;
	case 2:
		ret_val = 0;
		break;
	case 3:
		ret_val = 0;
		break;
	case 4:
		ret_val = 1;
		if (!(getBonusInfo("isDeterministic"))) { ret_val=0; break; }
		for (unsigned int i=0; i< scenario_completion.size(); i++) { 
			//ret_val &= scenario_completion[i];
			if (scenario_completion[i] != 1) ret_val &= 0;
		}
		break;
	case 5: 
		ret_val = 1;
		break;
	default:
		ret_val = 0;
	}

	return ret_val;
}

void cUMLModel::printUMLModelToFile(std::string file_name) 
{
	// open outfile. 
	std::ofstream outfile;
	outfile.open(file_name.c_str());
//	outfile.open("out.xmi");

	assert(outfile.is_open());
	
	// export xmi to outfile.
	outfile << xmi << endl;
	
	// close outfile.
	outfile.close();
	
	return;	
}

float cUMLModel::checkProperties() 
{
	// for all properties:
	std::set<cMDEProperty*>::iterator prop_ptr;
	float total = 0;
	float temp_val = 0;
	m_property_success = 0;
	m_property_failure = 0;

	for (prop_ptr=mdeprops.begin(); prop_ptr!=mdeprops.end(); prop_ptr++)  
	{
		temp_val = (*prop_ptr)->getEvaluationInformation();
		if (temp_val == -1) { 
			(*prop_ptr)->evaluate();
			temp_val = (*prop_ptr)->getEvaluationInformation();
			// increment the temp_val by 1 more, since this is a new property
			if (temp_val >0) {
				temp_val += 1;
				m_property_success++;
			} else { 
				m_property_failure++;
			}
		} 
		
		total += temp_val;
	}
	
	
	// return the reward.
	return total;
}


bool cUMLModel::addExistenceProperty(std::string s)
{
	// a pointer to the existence property
	std::string temp = getStateDiagram(0)->StringifyAnInt(mdeprops.size());
	cMDEExistenceProperty* e = new cMDEExistenceProperty(s, temp);
	mdeprops.insert (e);
	//int q = mdeprops.size();
	return true;
}

bool cUMLModel::addAbsenceProperty(std::string s)
{
	// a pointer to the absence property
	std::string temp = getStateDiagram(0)->StringifyAnInt(mdeprops.size());
	cMDEAbsenceProperty* e = new cMDEAbsenceProperty(s, temp);
	mdeprops.insert (e);
	//int q = mdeprops.size();
	return true;
	
}

bool cUMLModel::addUniversalProperty(std::string s)
{
	// a pointer to the universal property
	std::string temp = getStateDiagram(0)->StringifyAnInt(mdeprops.size());
	cMDEUniversalProperty* e = new cMDEUniversalProperty(s, temp);
	mdeprops.insert (e);
	//int q = mdeprops.size();
	return true;
	
}
