#include "cUMLModel.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <iomanip>
#include <errno.h>


using namespace std;

std::string loadFile(const char* filename) {
	std::string data, line; // or maybe stringstream? (strstream?)
	std::ifstream infile;
	infile.open(filename);
	assert(infile.is_open());
	
	while (getline (infile, line))
	{
		data.append(line);
		line.erase();
	}
	
	//read from file; load into string/strstream, and return it.
	infile.close();

	return data;
}

std::string cUMLModel::xmi_begin = loadFile("xmi_begin");
std::string cUMLModel::xmi_end = loadFile("xmi_end");
std::string cUMLModel::xmi_class1 = loadFile("class1_xmi");
std::string cUMLModel::xmi_class2 = loadFile("class2_xmi");


cUMLModel::cUMLModel()
{
	// initialize / seed UML model here
//	state_diagrams.clear();
//	state_diagrams.resize(2);
}

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
//	char c;
	std::string tr_l, tr_o, gu, act, temp;
	int trig_i, guard_i, act_i, orig_i, dest_i;
	std::ifstream infile;
	infile.open("seed-model.cfg");
	assert(infile.is_open());
	
	while (getline (infile, line))
	{
		// Read in states
		if (line == "=STATES====================") {
//			std::cout << "Ooh, I found a state! " << std::endl;
			line.erase();
			infile >> num_states;
//			std::cout << "Numer of states: "<< num_states << std::endl;
		// Read in number of state diagrams	
		} else if (line == "=STATE-DIAGRAM=============") { 
//			std::cout << "Yippee! A state diagram for me! " << std::endl;
			line.erase();
			infile >> num_sd;
//			std::cout << "Numer of sds: "<< num_sd << std::endl;
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
//				std::cout << "Adding a trigger " << tr_l << " " << tr_o << std::endl;
				infile >> tr_l;
			}
		}else if (line == "-GUARDS--------------------") { 
			line.erase();
			infile >> gu;
			while (gu != "-END---------------------------") { 
				state_diagrams[cur_sd].addGuard(gu);
//				std::cout << "Adding a guard " << gu << std::endl;
				infile >> gu;
			}
		} else if (line == "-ACTIONS--------------------") { 
			line.erase();
			infile >> act;
			while (act != "-END---------------------------") { 
				state_diagrams[cur_sd].addAction(act);
//				std::cout << "Adding an action " << act << std::endl;
				infile >> act;
			}
		} else if (line == "-TRANSITIONS---------------") { 
			line.erase();
			infile >> temp; 
			while (temp != "-END---------------------------") { 
//				std::cout << "Ug in here again. Curr sd: " << cur_sd << std::endl;
				infile >> orig_i >> dest_i >> trig_i >> guard_i >> act_i; 
				state_diagrams[cur_sd].addTransitionTotal(orig_i, dest_i, trig_i, guard_i, act_i);
				infile >> temp; 
			}
		
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
	
	xmi = xmi_begin; 
	xmi += xmi_class1;
	xmi += state_diagrams[0].getXMI("sd0");	
	xmi += xmi_class2;
//	state_diagrams[1].printXMI();
	xmi += state_diagrams[1].getXMI("sd1");

	xmi += xmi_end;
	
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

