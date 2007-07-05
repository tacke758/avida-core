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

cUMLStateDiagram* cUMLModel::getStateDiagram (int x) 
{
	// check to see whether this state diagram exists
	if (x < state_diagrams.size()) { 
		return (&(state_diagrams.begin()[x]));
	}

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
	char c;
	std::string tr_l, tr_o, gu, act;
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
		}
		
		/* Missing code for reading in transition labels and transitions.... */
		

		
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
	
//	xmi += xmi_class1;
//	xmi += state_diagrams[0].getXMI();
//	xmi += xmi_class2;
	xmi += state_diagrams[1].getXMI();

	xmi += xmi_end;
	
}

std::string cUMLModel::getXMI()
{
	printXMI();
	return xmi;
}


