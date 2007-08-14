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

xmi_info cUMLModel::xi = loadFile("xmi_info");
int cUMLModel::max_trans = 0;


cUMLModel::cUMLModel()
{}

cUMLModel::~cUMLModel()
{}

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

/* This is a crappy little function to read in from a file. 
It is not robust. It will not understand things unless you 
follow the *very* specific file format. */
void cUMLModel::seedDiagrams()
{
	std::string data, line; // or maybe stringstream? (strstream?)
	int num_states;
	int num_sd = 0;
	int cur_sd = -1;
	std::string tr_l, tr_o, gu, act, temp;
	int trig_i, guard_i, act_i, orig_i, dest_i;
	std::ifstream infile;
	infile.open("seed-model.cfg");
	assert(infile.is_open());
	scenario_info s;
	std::string path_step;
	
	while (getline (infile, line))
	{
		// Read in states
		if (line == "=STATES====================") {
			line.erase();
			infile >> num_states;
		// Read in number of state diagrams	
		} else if (line == "=HYDRA=====================") { 
			line.erase(); 
			infile >> hydraMode;
		} else if (line == "=STATE-DIAGRAM=============") { 
			line.erase();
			infile >> num_sd;
			state_diagrams.resize(num_sd);
		// Read in each state diagram	
		} else if (line == "=SD========================") { 
			line.erase();
			cur_sd++;
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
				state_diagrams[cur_sd].addTransitionTotal(orig_i, dest_i, trig_i, guard_i, act_i);
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
			
		}
		
		/* Missing code for reading in transition labels .... */
		
		line.erase();
	}
		
	//read from file; load into string/strstream, and return it.
	infile.close();

	return;
  
}


void cUMLModel::printXMI()
{
	xmi = "";
//	int v;
	
	xmi = xi.xmi_begin; 
//	std::cout << "xi.classes.size " << xi.classes_info.size() << " state_diagrams.size() " << state_diagrams.size() << std::endl;
	
	assert (xi.classes_info.size() == state_diagrams.size());
	
	for (unsigned int i=0; i<xi.classes_info.size(); i++) { 
		xmi += xi.classes_info[i];
		xmi += state_diagrams[i].getXMI("sd" + i);
	}
//	xmi += state_diagrams[0].getXMI("sd0");	
//	xmi += xmi_class2;
	
	xmi += xi.xmi_end;
	
}

std::string cUMLModel::getXMI()
{
//	printXMI();
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
//		std::cout << "TEMP BONUS: " << temp_bonus << std::endl;
		total_bonus += temp_bonus;
				
		complete_bonus = s.path.size() + s.shouldLoop; 
		if (s.startState >= 0) complete_bonus++;
		
//		if (temp_bonus == complete_bonus) {
////			bonus_info["scenario"+i] = 1;
//			scenario_completion[i] = temp_bonus / complete_bonus;
////			std::cout << "scenario complete " << std::endl;
//		} else {
////			bonus_info["scenario"+i] = 0;
//			scenario_completion[i] = 0;
////			std::cout << "scenario incomplete " << std::endl;
//
//		}
	scenario_completion[i] = temp_bonus / complete_bonus;
	}
		
	return total_bonus;
}

bool cUMLModel::readyForHydra() 
{
	// options: (0) ALL_COMPLETE, (1) ONE_COMPLETE, (2) ONE_NON_EMPTY, (3) ALL_NON_EMPTY
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
	default:
		ret_val = 0;
	}

	return ret_val;
}


