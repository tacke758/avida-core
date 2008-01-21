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
	
	std::ofstream outfile;
//	outfile.open (_name.c_str());
	outfile.open ("property");
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
	
	std::ofstream outfile;
//	std::string file_name = "w" + _name;
	std::string file_name = "witness-property";
	outfile.open (file_name.c_str());
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
	std::string cmd;
	std::string work_prop = "properties_that_passed";
	
	// if this property passed, then save it to a file
	if (verify_reward) { 
//		cmd = "cat " + _name + " >> " + work_prop;
		cmd = "cat property >> " + work_prop;
		system(cmd.c_str());
	}
	
	_reward = verify_reward;
}



