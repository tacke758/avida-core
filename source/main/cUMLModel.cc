
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
				   std::vector<cUMLClass>& classes,
//                   std::vector<cUMLStateDiagram>& state_diagrams,
                   std::vector<scenario_info>& scenarios,
                   int& hydra_mode, 
				   bool& witness_mode) {
  std::string data, line; 
	//int num_states;
	//int num_sd = 0;
	int cur_class = -1;
	//bool include_trans = 0;
	//std::string tr_l, tr_o, gu, act, temp;
	//int trig_i, guard_i, act_i, orig_i, dest_i;
	int num_classes;
	std::ifstream infile;
	infile.open(seed_model);
	assert(infile.is_open());
	//scenario_info s;
	//std::string path_step;
	std::string class_name;
	std::string temp1, temp2;
	std::string op_name, op_code;
	std::string att_name, att_type, att_val;
	std::vector<std::string> att_vals;
	
	while (getline (infile, line))
	{
		if (line == "==MODEL==") {
			line.erase();
			infile >> temp1 >> num_classes;
			infile >> temp1 >> hydra_mode;
			infile >> temp1 >> witness_mode;
			//std::cout << "num_classes " << num_classes << std::endl;
			//std::cout << "hydra_mode " << hydra_mode << std::endl;
			//std::cout << "witness_mode " << witness_mode << std::endl;
			classes.resize(num_classes);
		} else if (line == "==CLASS==") { 
			line.erase();
			infile >> temp1 >> class_name;
			cur_class++;
			classes[cur_class].addClassName(class_name);
			//std::cout << "current class " << cur_class << std::endl;
			//std::cout << "class name " << class_name << std::endl;
		} else if (line == "==ATTRIBUTES==") {
			line.erase();
			infile >> att_name;
			att_vals.clear();
			while (att_name != "==END==") { 
				infile >> att_type >> temp1 >> att_val;
			//	std::cout << "attribute " << att_name << " " << att_type << std::endl;
				while (att_val != "]") { // && att_val != "==END=CLASS==") { 
					att_vals.push_back(att_val);
			//		std::cout << "value " << att_val << std::endl;
					infile >> att_val;
				}
				
				classes[cur_class].addAttribute(att_name, att_type, att_vals);
				infile >> att_name;				
			}
		} else if (line == "==OPERATIONS==") {
			line.erase();
			infile >> op_name;
			while (op_name != "==END==") { 
				infile >> op_code;
//				state_diagrams[cur_sd].addTrigger(tr_l, tr_o);
				classes[cur_class].addOperation(op_name, op_code);
			//	std::cout << "operation " << op_name << " " << op_code << std::endl;
				infile >> op_name;				
			}
		} else if (line == "==ASSOCIATIONS==") {
			line.erase();
			infile >> class_name;
			while (class_name != "==END==") { 
				classes[cur_class].addAssociatedClass(class_name);
			//	std::cout << "Associated Class " << class_name << std::endl;
				infile >> class_name;			
			}
		}
		
	}
  
	infile.close();
	return;
}


xmi_info cUMLModel::xi = loadFile("xmi_info");
int cUMLModel::max_trans = 0;


bool cUMLModel::_cfgLoaded = false;
std::vector<cUMLStateDiagram> cUMLModel::_cfg_state_diagrams;
std::vector<cUMLClass> cUMLModel::_cfg_classes;
std::vector<scenario_info> cUMLModel::_cfg_scenarios;
int cUMLModel::_cfg_hydra_mode;
bool cUMLModel::_cfg_witness_mode;


cUMLModel::cUMLModel(const char* seed_model) {
  if(!_cfgLoaded) {
    seed_diagrams(seed_model, _cfg_classes, _cfg_scenarios, _cfg_hydra_mode, _cfg_witness_mode);
    _cfgLoaded = true;
  }
  
  //state_diagrams = _cfg_state_diagrams;
  classes = _cfg_classes;
  scenarios = _cfg_scenarios;
  hydraMode = _cfg_hydra_mode; 
  witnessMode = _cfg_witness_mode;
  expression_p = 0;
  expression_q = 0;
  expression_r = 0;
  createExpressionsFromClasses();
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
  assert(x<classes.size());
  return (getStateDiagram(x));
}


void cUMLModel::printXMI()
{
	xmi = "";
//	int v;
	
	xmi = xi.xmi_begin; 
	
	assert (xi.classes_info.size() == state_diagrams.size());
	
	for (unsigned int i=0; i<xi.classes_info.size(); i++) { 
		xmi += xi.classes_info[i];
		xmi += (getStateDiagram(i))->getXMI("sd" + i);
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
	for (int i=0; i<numSDs(); i++) { 
		temp_states += getStateDiagram(i)->numStates();
	}
	return temp_states;
}

int cUMLModel::numTrans()
{
int temp_trans = 0;
	for (int i=0;  i<numSDs(); i++) { 
		temp_trans += getStateDiagram(i)->numTrans();
	}
	return temp_trans;
}

int cUMLModel::numTriggers()
{
int temp_trigger = 0;
	for (int i=0;  i<numSDs(); i++) { 
		temp_trigger += getStateDiagram(i)->numTriggers();
	}
	return temp_trigger;
}

int cUMLModel::numGuards() 
{
int temp = 0;
	for (int i=0;  i<numSDs(); i++) { 
		temp += getStateDiagram(i)->numGuards();
	}
	return temp;
}

int cUMLModel::numActions()
{
int temp = 0;
	for (int i=0;  i<numSDs(); i++) { 
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

/*float cUMLModel::checkProperties() 
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
	
//		int size = mdeprops.size();
	// return the reward.
	return total;
}*/


float cUMLModel::addExistenceProperty(std::string s)
{
	// a pointer to the existence property
	std::string temp = StringifyAnInt(mdeprops.size());
	float val = 0;
	cMDEExistenceProperty* e = new cMDEExistenceProperty(s, temp);
	// first, try to find the property
	//
	std::set<cMDEProperty*, ltcMDEProperty>::iterator mdepropiter = mdeprops.find(e);
	if (mdepropiter != mdeprops.end()) {
		val = (*mdepropiter)->getEvaluationInformation();
	} else {
		e->evaluate();
		val = e->getEvaluationInformation();
		mdeprops.insert (e);
		if (val >0) {
				m_property_success++;
				m_existence_property_success++;
			} else { 
				m_property_failure++;
				m_existence_property_failure++;
			}
	}
	
	return val;
}

float cUMLModel::addAbsenceProperty(std::string s)
{
	// a pointer to the absence property
	std::string temp = StringifyAnInt(mdeprops.size());
	float val = 0;
	cMDEAbsenceProperty* e = new cMDEAbsenceProperty(s, temp);
//	mdeprops.insert (e);
	//int q = mdeprops.size();
	std::set<cMDEProperty*, ltcMDEProperty>::iterator mdepropiter = mdeprops.find(e);
	if (mdepropiter != mdeprops.end()) {
		val = (*mdepropiter)->getEvaluationInformation();
	} else {
		e->evaluate();
		val = e->getEvaluationInformation();
		mdeprops.insert (e);
		if (val >0) {
				m_property_success++;
				m_absence_property_success++;
			} else { 
				m_property_failure++;
				m_absence_property_failure++;
			}
	}

	return val;
	
}

float cUMLModel::addUniversalProperty(std::string s)
{
	// a pointer to the universal property
	std::string temp = StringifyAnInt(mdeprops.size());
	float val = 0;	
	cMDEUniversalProperty* e = new cMDEUniversalProperty(s, temp);
//	mdeprops.insert (e);
	//int q = mdeprops.size();
	std::set<cMDEProperty*, ltcMDEProperty>::iterator mdepropiter = mdeprops.find(e);
	if (mdepropiter != mdeprops.end()) {
		val = (*mdepropiter)->getEvaluationInformation();
	} else {
		e->evaluate();
		val = e->getEvaluationInformation();
		mdeprops.insert (e);
		if (val >0) {
				m_property_success++;
				m_universal_property_success++;
			} else { 
				m_property_failure++;
				m_universal_property_failure++;
			}
	}

	return val;
	
}




// Create expressions for each class. Expressions are: 
//   Equality of an attribute and each of its values
//   Inequality of an attribute and each of its values
//   Methods
void cUMLModel::createExpressionsFromClasses()
{
	std::string class_name;
	class_attribute a;
	operation o;
	cUMLClass c;
	int temp_size;
	std::string temp1, temp2, temp3;
	
	// For each class, create its set of expressions. 
	for (unsigned int i=0; i<classes.size(); i++) { 
		c = classes[i];
		class_name = c.getClassName();
		temp_size = c.numAttributes();
		
		// For each attribute...
		for (int j=0; j<temp_size; j++) {
			a = c.getAttribute(j);
			temp1 = class_name + "_V." + a.attribute_name;
			
			// For each attribute value
			for (unsigned int k=0; k<a.attribute_values.size(); k++){
				// create both an equality and an inequality expression
				temp2 = a.attribute_values[k];
				temp3 = temp1 + "==" + temp2;
				addExpression(temp3);
				//std::cout << temp3 << std::endl;
				temp3 = temp1 + "!=" + temp2;
				//std::cout << temp3 << std::endl;
				addExpression(temp3);
			}
		}
		
		// For each method
		temp_size = c.numOperations();
		for (int m=0; m<temp_size; m++) {
			o = c.getOperation(m);
			temp1 = class_name + "_q??[" + o.op_name + "]";
			addExpression(temp1);
			//std::cout << temp1 << std::endl;
		}
	}
}


std::string cUMLModel::getP() {
	return expressions[expression_p]; 
} 


// print the label. Change - signs to _
std::string cUMLModel::StringifyAnInt(int x) { 
	
	std::ostringstream o;
	if (x < 0) {
		x = abs(x);
		o << "_";
	} 
	o << x;
	return o.str();
}

// Check if the expression exists in the vector. If not, add it.
bool cUMLModel::addExpression(std::string s)
{ 
	bool val = false;
	std::vector<std::string>::iterator exprit;
	exprit = find(expressions.begin(), expressions.end(), s); 
	if (exprit == expressions.end()) { 
		expressions.push_back(s); 
		val = true;
	}
	return val;
}

// AND expressions p & q to create a new expression
// return true if this is a new expression
// return false otherwise
bool cUMLModel::ANDExpressions()
{
	bool val = false;
	std::string pstring, qstring, totalstring;
	if (expression_p != expression_q){
		pstring = getP();
		qstring = getQ();
		totalstring = pstring + " && " + qstring;
		val = addExpression(totalstring); 
	}
	return (val);
}

// OR expressions p & q to create a new expression
// return true if this is a new expression
// return false otherwise
bool cUMLModel::ORExpressions()
{
	
	bool val = false;
	std::string pstring, qstring, totalstring;
	if (expression_p != expression_q){
		pstring = getP();
		qstring = getQ();
		totalstring = pstring + " || " + qstring;
		val = addExpression(totalstring); 
	}
	return (val);
}

float cUMLModel::addResponseProperty(std::string s1, std::string s2)
{
	// a pointer to the universal property
	std::string temp = StringifyAnInt(mdeprops.size());
	float val = 0;	
	cMDEResponseProperty* e = new cMDEResponseProperty(s1, s2, temp);
	//	mdeprops.insert (e);
	//int q = mdeprops.size();
	std::set<cMDEProperty*, ltcMDEProperty>::iterator mdepropiter = mdeprops.find(e);
	if (mdepropiter != mdeprops.end()) {
		val = (*mdepropiter)->getEvaluationInformation();
	} else {
		e->evaluate();
		val = e->getEvaluationInformation();
		mdeprops.insert (e);
		if (val >0) {
			m_property_success++;
			m_response_property_success++;
		} else { 
			m_property_failure++;
			m_response_property_failure++;
		}
	}
	return val;
}

float cUMLModel::addPrecedenceProperty(std::string s1, std::string s2)
{
	// a pointer to the universal property
	std::string temp = StringifyAnInt(mdeprops.size());
	float val = 0;	
	cMDEPrecedenceProperty* e = new cMDEPrecedenceProperty(s1, s2, temp);
	//	mdeprops.insert (e);
	//int q = mdeprops.size();
	std::set<cMDEProperty*, ltcMDEProperty>::iterator mdepropiter = mdeprops.find(e);
	if (mdepropiter != mdeprops.end()) {
		val = (*mdepropiter)->getEvaluationInformation();
	} else {
		e->evaluate();
		val = e->getEvaluationInformation();
		mdeprops.insert (e);
		if (val >0) {
			m_property_success++;
			m_precedence_property_success++;
		} else { 
			m_property_failure++;
			m_precedence_property_failure++;
		}
	}
	return val;	
	
}

	

