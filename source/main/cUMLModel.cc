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
	
	return data;
}

std::string cUMLModel::xmi_begin = loadFile("xmi_begin");
std::string cUMLModel::xmi_end = loadFile("xmi_end");
std::string cUMLModel::xmi_class1 = loadFile("class1_xmi");
std::string cUMLModel::xmi_class2 = loadFile("class2_xmi");


cUMLModel::cUMLModel()
{
	// initialize / seed UML model here
	resetUMLModel();

}

cUMLModel::~cUMLModel()
{
}



void cUMLModel::resetUMLModel()
{
	// Currently there are 2 state diagrams...
	resetStateDiagrams(2);
	return;

}

cUMLStateDiagram* cUMLModel::getStateDiagram (int x) 
{
	// check to see whether this state diagram exists
	if (x < state_diagrams.size()) { 
		return (&(state_diagrams.begin()[x]));
	}

}

cUMLClassDiagram* cUMLModel::getClassDiagram (int x) 
{
	// check to see whether this state diagram exists
	if (x < class_diagrams.size()) { 
		return (&(class_diagrams.begin()[x]));
	}

}

void cUMLModel::resetStateDiagrams(int x)
{ 
	// reset the number of state diagrams.
  state_diagrams.clear();
	state_diagrams.resize(x);
  class_diagrams.clear();
	class_diagrams.resize(x);
	seedDiagrams();
}

void cUMLModel::seedDiagrams()
{
  self_bonus.clear();

/* 
 // Used for simple example - tagaaa model
 cUMLStateDiagram* temp = getStateDiagram(0);
 temp->addTransitionLabel(1, 2, 3);
 temp->addTransitionLabel(0, 0, 0);
 temp->addTransitionLabel(0, 3, 1);
 temp->addTransitionLabel(2, 1, 2);
 temp->addTransitionLabel(3, 0, 0);
 
 std::string trig_label = "ta";
 std::string trig_operation_id = "ta";
 temp->addTrigger(trig_label, trig_operation_id);
 
 trig_label = "tb";
 trig_operation_id = "tb";
 temp->addTrigger(trig_label, trig_operation_id);
 
 trig_label = "tc";
 trig_operation_id = "tc";
 temp->addTrigger(trig_label, trig_operation_id); 
 
 trig_label = "td";
 trig_operation_id = "td";
 temp->addTrigger(trig_label, trig_operation_id);
 
 temp->addGuard("ga");
 temp->addGuard("gb");
 temp->addGuard("gc");
 temp->addGuard("gd");
 
 temp->addAction("aa");
 temp->addAction("ab");
 temp->addAction("ac");
 temp->addAction("ad");

 
 
 
 cUMLStateDiagram* temp1 = getStateDiagram(1);
 temp1->addTransitionLabel(3, 2, 1);
 temp1->addTransitionLabel(0, 1, 0);
 temp1->addTransitionLabel(2, 0, 3);
 temp1->addTransitionLabel(2, 1, 0);
 temp1->addTransitionLabel(1, 1, 1); 
 */
 
 // Used for Temperature Sensor / MultiSensor example.

  // For the first state diagram... 
  // Software Sensor
  cUMLStateDiagram* soft_sense = getStateDiagram(1);
	
  // init triggers, guards, and actions
  // trig 0
  std::string trig_label = "<null>";
  std::string trig_operation_id = "<null>";
  soft_sense->addTrigger(trig_label, trig_operation_id);
  // trig 1
  trig_label = "setTempOpState";
  trig_operation_id = "XDE-4437EBF1-9C42-4EB4-B7CF-415697B567CD";
  soft_sense->addTrigger(trig_label, trig_operation_id);
  // trig 2
  trig_label = "setTempData";
  trig_operation_id = "XDE-9517D6BA-8666-4A82-AFEA-62D60FE37B07";
  soft_sense->addTrigger(trig_label, trig_operation_id);
  // guard 0
  soft_sense->addGuard("<null>");
  // action 0
  soft_sense->addAction("<null>");
  // action 1
  soft_sense->addAction("^TempSensor.getOpState()");
  // action 2
  soft_sense->addAction("^TempSensor.getTempData()");
  
  
  // Temperature Sensor
  cUMLStateDiagram* temp_sense = getStateDiagram(0);

  // init triggers, guards, and actions
  // trig 0
  trig_label = "<null>";
  trig_operation_id = "<null>";
  temp_sense->addTrigger(trig_label, trig_operation_id);  
  // trig 1
  trig_label = "getOpState";
  trig_operation_id = "XDE-73C1C501-493F-44F2-A70A-0C7BFA92160D";
  temp_sense->addTrigger(trig_label, trig_operation_id);
  // trig 2
  trig_label = "getTempData";
  trig_operation_id = "XDE-7C41CD1F-6E52-4E32-9C8E-999BA1919EC6";
  temp_sense->addTrigger(trig_label, trig_operation_id);  
  // guard 0
  temp_sense->addGuard("<null>");
  // action 0
  temp_sense->addAction("<null>");
  // action 1
  temp_sense->addAction("^SoftwareSensor.setTempOpState(op_state)");
  // action 2
  temp_sense->addAction("^SoftwareSensor.setTempData(data)");
  // action 3
  temp_sense->addAction("op_state:=1");
  // action 4
  temp_sense->addAction("op_state:=0");
  // action 5
  temp_sense->addAction("data:=100");
  // action 6
  temp_sense->addAction("data:=200");
  // action 7
  temp_sense->addAction("data:=300");

  
  
}

double cUMLModel::evaluateModel(int id, cWorld* world)
{
	double bonus = 0.0;
	double mod_bonus = 0.0;
	
	double size = state_diagrams.size();
	
	int s0_nt = getStateDiagram(0)->numTrans();
	int s1_nt = getStateDiagram(1)->numTrans();
	
	cUMLStateDiagram* temp_sense = getStateDiagram(0);
	cUMLStateDiagram* soft_sense = getStateDiagram(1);
	
	world->GetStats().addState(temp_sense->numStates() + soft_sense->numStates());
	world->GetStats().addTrans(s0_nt + s1_nt);


	// Check if the model meets the sequence diagram
//	bonus += checkForSequenceDiagram1();
		
		
	// Check if the model can be correctly formalized
//	if (bonus >= 5.0) {
	if (temp_sense->findTrans(-1, -1, -1, -1, 3)) {
		self_bonus["hydra_attempt"] = 1;
		mod_bonus = formalizeModel(world);
		self_bonus["hydra_pass"] = mod_bonus;		
		bonus += mod_bonus;
	}
	

	// Check if the model meets the properties. 
	if (mod_bonus > 0.0) {
		self_bonus["spin_attempt"] = 1;
		mod_bonus += propertyN1(id, world);
		self_bonus["spin_pass"] = mod_bonus;		
		bonus += mod_bonus;
	}

	// additional bonus for creating transitions...
	if (s0_nt <= 2) { 
		bonus += (s0_nt / 4);
	} else {
		bonus += .5;
	}
	
	if (s1_nt <= 2) { 
		bonus += (s1_nt / 4);
	} else {
		bonus += .5;
	}
	
	

	return bonus;
}


double cUMLModel::checkForSequenceDiagram1()
{
	double bonus = 0.0;
	double temp_bonus = 0.0;

	
	cUMLStateDiagram* soft_sense = getStateDiagram(1);
	cUMLStateDiagram* temp_sense = getStateDiagram(0);
	
	int nt = soft_sense->numTrans() + temp_sense->numTrans();
/*	// reward if number of transitions is greater than 10
	int nt = soft_sense->numTrans() + temp_sense->numTrans();
	if (nt <= 10) { 
		bonus += nt;
	} else {
		bonus += 10;
	}
*/	
	

				
	// Software Sensor
	
	// action: 
	// TempSensor.getOpState()
//	temp_bonus = soft_sense->findTrans(-1, -1, -1, "*", "^TempSensor.getOpState()");	
	temp_bonus = soft_sense->findTrans(-1, -1, -1, -1, 1);	
	self_bonus["seq_d_1"] = temp_bonus;
	bonus += temp_bonus;
			
	// trigger:
	// setTempOpState(op_state)
//	temp_bonus = soft_sense->findTrans(-1, -1, 1, "*", "*");
	temp_bonus = soft_sense->findTrans(-1, -1, 1, -1, -1);
	self_bonus["seq_d_2"] = temp_bonus;
	bonus += temp_bonus;		

	
	// Temperature Sensor		
	
	// trigger:
	// getOpState()
	temp_bonus = temp_sense->findTrans(-1, -1, 1, -1, -1);		
	self_bonus["seq_d_3"] = temp_bonus;
	bonus += temp_bonus;
	
			
	// action:
	// op_state := 1
	//temp_bonus = temp_sense->findTrans(-1, -1, -1, "*", "op_state:=1");		
	temp_bonus = temp_sense->findTrans(-1, -1, -1, -1, 3);		

	self_bonus["seq_d_4"] = temp_bonus;
	bonus += temp_bonus;

//	temp_bonus = temp_sense->findTrans(-1, -1, -1, "*", "^SoftwareSensor.setTempOpState(op_state)");
	temp_bonus = temp_sense->findTrans(-1, -1, -1, -1, 1);

	self_bonus["seq_d_5"] = temp_bonus;
	bonus += temp_bonus;
	
	// For each state diagram, look for the relevant transitions
/*	if(getStateDiagram(0)->findTrans(-1, -1, -1, "*", "<null>")) {
		bonus += 1.0;
	}
	if(getStateDiagram(1)->findTrans(-1, -1, -1, "*", "<null>")) {
		bonus += 1.0;
	}	
		if(getStateDiagram(2)->findTrans(-1, -1, -1, "*", "<null>")) {
		bonus += 1.0;
	}*/
	
	
//	bonus += formalizeModel();
//	bonus += propertyN1();

	return bonus;
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
	std::string x;
//	int v;
	
	x = xmi_begin; 
	
//	x += xmi_class1;
//	x += state_diagrams[0].getXMI();
//	x += xmi_class2;
	x += state_diagrams[1].getXMI();
	
	
	
	// get the xmi for each state diagram
//	for (v = 0; v < state_diagrams.size(); ++v) { 
//		state_diagrams[v].printXMI();
//		x+=state_diagrams[v].getXMI();
//	}
	
	x += xmi_end;
	
	return x;
}

double cUMLModel::formalizeModel(cWorld* world) 
{
	printXMI();
	std::string temp = xmi;


/*(	if (temp == organism->getParentXMI()) {
		ctx.task_success_complete += organism->getParentBonusInfo("hydra");
		organism->setBonusInfo("hydra", organism->getParentBonusInfo("hydra"));
		return organism->getParentBonusInfo("hydra");
	}*/


	world->GetStats().HydraAttempt();

	double bonus = 0.0;
	unsigned int status_total = 0;
	int status=0;

	int to_subavida[2]={0};
	int from_subavida[2]={0};
	
	pipe(to_subavida); //write to 1, read from 0
	pipe(from_subavida);
	
	pid_t subavida = fork();
	if(subavida == 0) {
		//child
		close(to_subavida[1]);
		close(from_subavida[0]);
		dup2(to_subavida[0], STDIN_FILENO); //oldd, newd
		dup2(from_subavida[1], STDOUT_FILENO);
		execl("/usr/bin/java", "-cp .", "-jar", "./hydraulic.jar", NULL);
		// We don't ever get here.
	} 
	//parent
	close(to_subavida[0]);
	close(from_subavida[1]);

	// At this point, forget about subavida - It's running.
	// Write the model to to_subavida[1].  Close to_subavida[1] (which wakes up subavida).
	// Then, read from from_subavida[0] as long as is possible, after which point subavida will die.

	// Write the model to STDIN of subavida (be careful; write may not write all that you ask!)
	do {
		status = write(to_subavida[1], temp.c_str()+status_total, temp.size());	
		if (status < 0) {
			break;
		} else {
			 status_total += status;
		}
	} while (status_total < temp.size());
	close(to_subavida[1]); // Wakes up subavida.

	// Time passes...

	// Read the output from subavida.  Keep reading until subavida closes the pipe.
	const int read_size=128; // The number of bytes that we're going to try to read from subavida.
	std::string subavida_output;
	char line[read_size]={0};
	do {
		status = read(from_subavida[0], line, read_size-1);
		if(status > 0) {
			subavida_output += line;
			memset(line, 0, read_size);
		}
	} while(((status==-1) && (errno == EINTR)) || (status>0));

	// Done with subavida.
	close(from_subavida[0]);
	// Make sure that subavida dies.
	pid_t done=0;
	while((done=waitpid(subavida, &status, 0))==-1 && (errno == EINTR)); 
	assert(done==subavida);
	
	// if there are no errors, return 0 from hydraulic.  otherwise, return non-zero.
	if(status != 0) {
//		ctx->task_failed = 0;
//		organism->setBonusInfo("hydra", 0.0); 
		return 0;
	} else {
	//	ctx->task_failed = ctx->task_failed && 1;
//		ctx.task_success_complete += 1;
		world->GetStats().HydraPassed();
//		organism->setBonusInfo("hydra", 1.0); 
		return 1;
	}
	
}


double cUMLModel::checkProperty(const std::string& neverclaimFile, int id, cWorld* world) const {
	world->GetStats().SpinAttempt();
	double status=0;
	std::string cmd = "cat " + neverclaimFile + " >> tmp.pr && ./spin -a tmp.pr &> /dev/null";
	if(system(cmd.c_str())!=0) return 0;
	world->GetStats().SpinPassed();
	world->GetStats().PanAttempt();
	
	if(system("/usr/bin/gcc -DMEMLIM=512 pan.c -o pan &> /dev/null")!=0) return 0;
	if(system("./pan -a &> ./pan.out")!=0) return 0;
	if(system("cat pan.out | perl -e 'while(<STDIN>) { if(/errors:\\s(\\d+)/) {exit($1);}}'")!=0) return 0;
		
	if(system("cat pan.out | perl -e 'while(<STDIN>) { if(/unreached/) {exit(1);}}'")!=0) return 1;

	std::ostringstream strstrm;
	strstrm << "cp tmp.xmi " << world->GetStats().GetUpdate() << "." << id;
	strstrm << ".xml";	
	if(system(strstrm.str().c_str())!=0) return 0.0;
			
	world->GetStats().PanPassed();
	return 3;
}

double cUMLModel::propertyN1(int deme_id, cWorld* world) const {
	std::string temp = xmi;
	double temp1 = 0;

/*	
	if (temp == organism->getParentXMI()) { 
		ctx.task_success_complete += organism->getParentBonusInfo("spinn1");
		organism->setBonusInfo("spinn1", organism->getParentBonusInfo("spinn1"));
		return organism->getParentBonusInfo("spinn1");
	}
*/	
	
	// check if the trigger is present in the relevant diagram... 
//	if (organism->getStateDiagram()->findTrans(-1,-1,1,"*","*")){
//		temp += 1;
		
		// check property
//		if (ctx.task_success_complete) {
			temp1 += checkProperty("N1", deme_id, world);
//		} 
//	}
	
//	organism->setBonusInfo("spinn1", temp1); 
	return temp1;
}


