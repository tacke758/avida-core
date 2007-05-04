#include "cUMLModel.h"


#include <iomanip>


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
	
	return data;
}

std::string cUMLModel::xmi_begin = loadFile("xmi_begin");
std::string cUMLModel::xmi_end = loadFile("xmi_end");

std::string cUMLModel::getXMI()
{
	std::string x;
	int v;
	
	x = xmi_begin; 
	
	// get the xmi for each state diagram
	for (v = 0; v < state_diagrams.size(); ++v) { 
		state_diagrams[v].printXMI();
		x+=state_diagrams[v].getXMI();
	}
	
	x += xmi_end;

}


