/*
 *  cMDEResponseProperty.cc
 *  
 *
 *  Created by Heather Goldsby on 11/20/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "cMDEResponseProperty.h"

bool cMDEResponseProperty::print() {
	
	// Create the file...
	std::string cmd = "cp " + _promela + " " + _property_file_name;
	if(system(cmd.c_str())!=0) return false;
	
	// Open the file in append mode...
	std::ofstream outfile;
	outfile.open (_property_file_name.c_str(), std::ios_base::app);
	assert(outfile.is_open());
	
	
	// Add the response property to the end of the file...
	outfile << "/* Response property " << _expr_p  << " " << _expr_q << "*/" << std::endl;
	outfile << "#define s (" << _expr_q << ")" << std::endl;
	outfile << "#define p (" << _expr_p << ")" << std::endl;
	outfile << "never {  /*  !([](p -> <>s))  */ " << std::endl;
	outfile << "T0_init:" << std::endl;
	outfile << "if" << std::endl;
	outfile << ":: (! ((s)) && (p)) -> goto accept_S4" << std::endl;
	outfile << ":: (1) -> goto T0_init" << std::endl;
	outfile << "fi;" << std::endl;
	outfile << "accept_S4:" << std::endl;
	outfile << "if " << std::endl;
	outfile << ":: (! ((s))) -> goto accept_S4" << std::endl;
	outfile << "fi; }" << std::endl;
	
	outfile.close();
	return true;

}

bool cMDEResponseProperty::printWitness() {
	
	// Create the file
	std::string cmd = "cp " + _promela + " " + _witness_file_name;
	if(system(cmd.c_str())!=0) return false;
	
	// Open the file in append mode
	std::ofstream outfile;
	outfile.open (_witness_file_name.c_str(), std::ios_base::app);
	assert(outfile.is_open());
	
	// Add response property to the end of the file...
	outfile << "/* Response property " << _expr_p  << " " << _expr_q << "*/" << std::endl;
	outfile << "#define s (" << _expr_q << ")" << std::endl;
	outfile << "#define p (" << _expr_p << ")" << std::endl;
	outfile << "never {    /* !(!(<>(p && <>s))) */" << std::endl;
	outfile << "T0_init:" << std::endl;
	outfile << "if" << std::endl;
	outfile << ":: ((p) && (s)) -> goto accept_all" << std::endl;
	outfile << ":: ((p)) -> goto T0_S4" << std::endl;
	outfile << ":: (1) -> goto T0_init" << std::endl;
	outfile << "fi;" << std::endl;
	outfile << "T0_S4:" << std::endl;
	outfile << "if" << std::endl;
	outfile << ":: ((s)) -> goto accept_all" << std::endl;
	outfile << ":: (1) -> goto T0_S4" << std::endl;
	outfile << "fi;" << std::endl;	
	outfile << "accept_all:" << std::endl;
	outfile << "skip}" << std::endl;
	
	outfile.close();
	return true;
	
}


bool cMDEResponseProperty::printInEnglish() {
	
	std::ofstream outfile;
	outfile.open (_properties.c_str(), std::ios_base::app);
	assert(outfile.is_open());
	
	outfile << "Globally, it is always the case that if " << _expr_p  << " holds, ";
	outfile << "then "<< _expr_q << " eventually holds." << std::endl << std::endl;
	
	outfile.close();
	return true;
	
}
