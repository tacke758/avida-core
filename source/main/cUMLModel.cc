
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
		} else if (line == "=XMI-END====================") { 
		// handle xmi_end
			line.erase();
			infile >> line;
			while (line != "=END========================") { 
				// cat line to the xmi_begin string...
				x.xmi_end += (line + " ");
				infile >> line;
			}
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
                   std::vector<cUMLStateDiagram>& state_diagrams,
                   std::vector<scenario_info>& scenarios,
                   int& hydra_mode, 
				   bool& witness_mode, 
				   int& gen_mode) {
  std::string data, line; 
	int cur_class = -1;
	int num_classes;
	std::ifstream infile;
	infile.open(seed_model);
	assert(infile.is_open());
	std::string class_name;
	std::string temp, temp1, temp2;
	std::string op_name, op_code;
	std::string att_name, att_type, att_val;
	int trig_i, guard_i, act_i, orig_i, dest_i;
	std::vector<std::string> att_vals;
	scenario_info s;
	std::string path_step;

	
	while (getline (infile, line))
	{
		if (line == "==MODEL==") {
			line.erase();
			infile >> temp1 >> num_classes;
			infile >> temp1 >> hydra_mode;
			infile >> temp1 >> witness_mode;
			infile >> temp1 >> gen_mode;
			// resize state diagrams & classes to correspond to the number of classes.
			classes.resize(num_classes);
			state_diagrams.resize(classes.size());
		} else if (line == "==CLASS==") { 
			line.erase();
			infile >> temp1 >> class_name;
			cur_class++;
			classes[cur_class].addClassName(class_name);
		} else if (line == "==ATTRIBUTES==") {
			line.erase();
			infile >> att_name;
			while (att_name != "==END==") { 
				infile >> att_type >> temp1 >> att_val;
				att_vals.clear();
				while (att_val != "]") { 
					att_vals.push_back(att_val);
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
				classes[cur_class].addOperation(op_name, op_code);
				infile >> op_name;				
			}
		} else if (line == "==ASSOCIATIONS==") {
			line.erase();
			infile >> class_name;
			while (class_name != "==END==") { 
				classes[cur_class].addAssociatedClass(class_name);
				infile >> class_name;			
			}
		} else if (line == "==SCENARIO==") { 
			line.erase();
			s.path.clear();
			s.stateDiagramID = cur_class;
			infile >> s.shouldLoop >> s.startState;
			infile >> temp;
			while (temp != "==END==") { 
				infile >> path_step;
				s.path.push_back(path_step);
				infile >> temp;
			} 
			scenarios.push_back(s);
		} else if (line == "==TRANSITIONS==") { 
			line.erase();
			infile >> temp; 
			while (temp != "==END==") { 
				infile >> orig_i >> dest_i >> trig_i >> guard_i >> act_i; 
				if (temp == "1") { 
					state_diagrams[cur_class].addTransitionTotal(orig_i, dest_i, trig_i, guard_i, act_i);
				}	
				infile >> temp; 
			}
		}	else if (line == "==TRANSITION=LABELS==") { 
			line.erase();
			infile >> temp;
			while (temp != "==END==") { 
				infile >> trig_i >> guard_i >> act_i; 
				if (temp=="1") { 
					state_diagrams[cur_class].addTransitionLabel(trig_i, guard_i, act_i);
				}
				infile >> temp;
			} 
		} else if (line == "==ACTIONS==") { 
			line.erase();
			infile >> temp;
			state_diagrams[cur_class].addAction("<null>");
			while (temp != "==END==") { 
				infile >> temp2; 
				if (temp=="1") { 
					state_diagrams[cur_class].addAction(temp2);
				}
				infile >> temp;
			} 
		}
		
	}
		
//	seedTriggersGuardsActions(classes, state_diagrams);
	// For each class... 
	// triggers = methods
	// guards = attribute equality / inequality
	// actions = methods of related classes.
	cUMLClass c;
	class_attribute a;
	operation o;
	int temp_size;
	std::string temp3;
	std::string at_type;
	std::set<std::string> rc;
	std::set<std::string>::iterator rcit;

	for (unsigned int i=0; i<classes.size(); i++) { 
		c = classes[i];
		temp_size = c.numAttributes();
		
		// add nulls.
		state_diagrams[i].addGuard("<null>");
		state_diagrams[i].addTrigger("<null>", "<null>");
//		state_diagrams[i].addAction("<null>");
		
		// For each attribute...
		for (int j=0; j<temp_size; j++) {
			a = c.getAttribute(j);
			for (unsigned int k=0; k<a.attribute_values.size(); k++){
				// create both an equality and an inequality expression
				temp2 = a.attribute_values[k];
				temp3 = a.attribute_name + "=" + temp2;
				state_diagrams[i].addGuard(temp3);
				temp3 = a.attribute_name + "!=" + temp2;
				state_diagrams[i].addGuard(temp3);
			}
		}
		
		// For each method
		temp_size = c.numOperations();
		for (int m=0; m<temp_size; m++) {
			o = c.getOperation(m);
			state_diagrams[i].addTrigger(o.op_name, o.op_code);
		}
		
		// For each of the related classes, add an action for each of 
		// its methods... (yucky...)
		rc.clear();
		rc = classes[i].getAssociatedClasses();
		for (rcit=rc.begin(); rcit!=rc.end(); rcit++) { 
			// Find the related class in the list of classes...
			for (unsigned int k=0; k<classes.size(); k++){
				if (classes[k].getClassName() == (*rcit)){
					// For each of its operations, add an action...
					temp_size = classes[k].numOperations();
					for (int m=0; m<temp_size; m++) {
						o = classes[k].getOperation(m);
						temp2 = "^" + classes[k].getClassName() + "." + o.op_name + "()";
						state_diagrams[i].addAction(temp2);
					}
				}
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
int cUMLModel::_cfg_gen_mode;


cUMLModel::cUMLModel(const char* seed_model) {
  if(!_cfgLoaded) {
    seed_diagrams(seed_model, _cfg_classes, _cfg_state_diagrams, _cfg_scenarios, _cfg_hydra_mode, _cfg_witness_mode, _cfg_gen_mode);
    _cfgLoaded = true;
  }
  
  classes = _cfg_classes;
	state_diagrams = _cfg_state_diagrams;
  scenarios = _cfg_scenarios;
  hydraMode = _cfg_hydra_mode; 
  witnessMode = _cfg_witness_mode;
  genMode = _cfg_gen_mode;
  
  // Initialize the property generator.
  gen = new cMDEPropertyGenerator();
  createExpressionsFromClasses();
}


cUMLModel::~cUMLModel()
{
	delete gen;
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
  return (&state_diagrams[x]);
}


void cUMLModel::printXMI()
{
	xmi = "";	
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
	//          (4) ALL COMPLETE && DETERMINISTIC, (5) NONE, (6) PERCENTAGE COMPLETE
	// check which option was selected in the seed-model.cfg
	// check to see if this condition is true. If so, return 1; otherwise, return 0.
	
	bool ret_val = 0;
	double temp = 0;


	switch (hydraMode){
	case 0:
		ret_val = 1;
		for (unsigned int i=0; i< scenario_completion.size(); i++) { 
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
			if (scenario_completion[i] != 1) ret_val &= 0;
		}
		break;
	case 5: 
		ret_val = 1;
		break;
	case 6: 
		ret_val = 1;
		for (unsigned int i=0; i< scenario_completion.size(); i++) { 
			temp += scenario_completion[i];
		}
		if ((temp/scenario_completion.size()) < .75){ ret_val = 0; }
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
	std::string at_type;
	cMDEExpressionAttribute* a1, a2;
			
	// For each class, create its set of expressions. 
	for (unsigned int i=0; i<classes.size(); i++) { 
		c = classes[i];
		class_name = c.getClassName();
		temp_size = c.numAttributes();

		// For each attribute...
		for (int j=0; j<temp_size; j++) {
			
			a = c.getAttribute(j);
			at_type = a.attribute_type;
			a1 = new cMDEExpressionAttribute(a.attribute_name, at_type, class_name);
			

			if ((at_type == "int") || (at_type == "integer")) {
				
				// For each attribute value
				for (unsigned int k=0; k<a.attribute_values.size(); k++){

					// create both an equality and an inequality expression
					temp2 = a.attribute_values[k];
					gen->addSimpleAttValExpression(a1, temp2, "==", c.getAssociatedClasses());
					gen->addSimpleAttValExpression(a1, temp2, "!=", c.getAssociatedClasses());
					gen->addSimpleAttValExpression(a1, temp2, ">", c.getAssociatedClasses());
					gen->addSimpleAttValExpression(a1, temp2, ">=", c.getAssociatedClasses());
					gen->addSimpleAttValExpression(a1, temp2, "<=", c.getAssociatedClasses());
					gen->addSimpleAttValExpression(a1, temp2, "<", c.getAssociatedClasses());
				
				}
			} else if ((at_type == "bool")||(at_type == "boolean")) {
				for (unsigned int k=0; k<a.attribute_values.size(); k++){
					// create both an equality and an inequality expression
					temp2 = a.attribute_values[k];
					gen->addSimpleAttValExpression(a1, temp2, "==", c.getAssociatedClasses());
					gen->addSimpleAttValExpression(a1, temp2, "!=", c.getAssociatedClasses());
					
				}
			}
		}
		
		// For each method
		temp_size = c.numOperations();
		for (int m=0; m<temp_size; m++) {
			o = c.getOperation(m);
			gen->addSimpleOperationExpression(o.op_name, class_name, c.getAssociatedClasses());
		}
	}
}


int cUMLModel::getUMLValue() 
{
	float temp = 0;
	int val;
	float w1 = getBonusInfo("spinw1");
	float w2 = getBonusInfo("spinw2");
	float n1 = getBonusInfo("spinn1");
	float n2 = getBonusInfo("spinn2");
	for (unsigned int i=0; i< scenario_completion.size(); i++) { 
		temp += scenario_completion[i];
	}
	val = (int)((temp/scenario_completion.size()) * 10);
	if (w1 || w2) val = 11;
	if (w1 && w2) val = 12;
	if (n1 || n2) val = 13;
	if (n1 && n2) val = 14;
	
	//	(organism->getUMLModel()->getBonusInfo("spinw1") == 0)	 &&
	//		(organism->getUMLModel()->getBonusInfo("spinw2")
	return val;
	
}


