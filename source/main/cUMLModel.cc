#include "cUMLModel.h"


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

cUMLModel::cUMLModel()
{
	// initialize / seed UML model here
}

cUMLModel::~cUMLModel()
{
}

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

int cUMLModel::formalizeModel() const
{
	std::string temp;

	temp = xmi;

/*(	if (temp == organism->getParentXMI()) {
		ctx.task_success_complete += organism->getParentBonusInfo("hydra");
		organism->setBonusInfo("hydra", organism->getParentBonusInfo("hydra"));
		return organism->getParentBonusInfo("hydra");
	}*/


//	m_world->GetStats().HydraAttempt();

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
////		m_world->GetStats().HydraPassed();
//		organism->setBonusInfo("hydra", 1.0); 
		return 1;
	}
	
}


int cUMLModel::checkProperty(const std::string& neverclaimFile) const {
//	m_world->GetStats().SpinAttempt();
	int status=0;
	std::string cmd = "cat " + neverclaimFile + " >> tmp.pr && ./spin -a tmp.pr &> /dev/null";
	if(system(cmd.c_str())!=0) return 0;
//	m_world->GetStats().SpinPassed();
//	m_world->GetStats().PanAttempt();
	
	if(system("/usr/bin/gcc -DMEMLIM=512 pan.c -o pan &> /dev/null")!=0) return 0;
	if(system("./pan -a &> ./pan.out")!=0) return 0;
	if(system("cat pan.out | perl -e 'while(<STDIN>) { if(/errors:\\s(\\d+)/) {exit($1);}}'")!=0) return 0;
	if(system("cat pan.out | perl -e 'while(<STDIN>) { if(/unreached/) {exit(1);}}'")!=0) return 0;
	
	
	std::ostringstream strstrm;
//	strstrm << "cp tmp.xmi " << m_world->GetStats().GetUpdate() << "." << organism->GetID();
//	strstrm << ".xml";	
	if(system(strstrm.str().c_str())!=0) return 0;
			
//	m_world->GetStats().PanPassed();
	return 3;
}

int cUMLModel::propertyN1() const {
	std::string temp = xmi;
	int temp1 = 0;

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
			temp1 += checkProperty("N1");
//		} 
//	}
	
//	organism->setBonusInfo("spinn1", temp1); 
	return temp1;
}


