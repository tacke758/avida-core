/*
 *  cMDEUniversalProperty.cc
 *  
 *
 *  Created by Heather Goldsby on 11/20/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "cMDEUniversalProperty.h"

void cMDEUniversalProperty::print() {
	
	// Create the file...
	std::string cmd = "cp " + _promela + " " + _property_file_name;
	if (system(cmd.c_str())!=0) return;
	
	// Open the file in append mode...
	std::ofstream outfile;
	outfile.open (_property_file_name.c_str(), std::ios_base::app);
	assert(outfile.is_open());
	
	outfile << "/* Universal property " << _expr_p << "*/" << std::endl;
	outfile << "#define p (" << _expr_p << ")" << std::endl;
	outfile << "never { /* ![]p */" << std::endl;
	outfile << "T0_init :    /* init */" << std::endl;
	outfile << "if " << std::endl;
	outfile << ":: (1) -> goto T0_init " << std::endl;
	outfile << ":: (!p) -> goto accept_all" << std::endl;
	outfile << "fi;" << std::endl;
	outfile << "accept_all :    /* 1 */" << std::endl;
	outfile << "skip " << std::endl;
	outfile << "}" << std::endl;
	
	outfile.close();

}

void cMDEUniversalProperty::printWitness() {
	
	// Create the file
	std::string cmd = "cp " + _promela + " " + _witness_file_name;
	if(system(cmd.c_str())!=0) return;
	
	// Open the file in append mode
	std::ofstream outfile;
	outfile.open (_witness_file_name.c_str(), std::ios_base::app);
	assert(outfile.is_open());
	
	outfile << "#define p (" << _expr_p << ")" << std::endl;
	outfile << "never {    /*  !([](p))  */" << std::endl;
	outfile << "T0_init :    /* init */" << std::endl;
	outfile << "if " << std::endl;
	outfile << ":: (! ((p))) -> goto accept_all" << std::endl;
	outfile << ":: (1) -> goto T0_init" << std::endl;
	outfile << "fi;" << std::endl;
	outfile << "accept_all :    " << std::endl;
	outfile << "skip " << std::endl;
	outfile << "}" << std::endl;
	
	outfile.close();
	
}


void cMDEUniversalProperty::evaluate()
{
	float verify_reward = 0;
	
	// print the property
	print();
	verify_reward = verify();
//	std::string cmd;
//	std::string work_prop = "properties_that_passed";
	
	// if this property passed, then save it to a file
	if (verify_reward) { 
		printInEnglish();
//		cmd = "cat " + _name + " >> " + work_prop;
//		cmd = "cat english-property >> " + work_prop;
//		system(cmd.c_str());
	}
	
	_reward = verify_reward;
}



void cMDEUniversalProperty::printInEnglish() {
	
	std::ofstream outfile;
	outfile.open (_properties.c_str(), std::ios_base::app);
	assert(outfile.is_open());	
	
	outfile << _interesting << ", "; 
	if (_uses_related_classes) {
		outfile << "true" << ", ";
	} else { outfile << "false" << ", "; }
	outfile << "Universal, ";
	outfile << "Globally, it is always the case that " << _expr_p  << " holds.     ";
	outfile << std::endl << std::endl;
	
	outfile.close();	
}
