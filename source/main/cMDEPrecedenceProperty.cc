/*
 *  cMDEPrecedenceProperty.cc
 *  
 *
 *  Created by Heather Goldsby on 11/20/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "cMDEPrecedenceProperty.h"

bool cMDEPrecedenceProperty::print() {
	
	// Create the file...
	std::string cmd = "cp " + _promela + " " + _property_file_name;
	if(system(cmd.c_str())!=0) return false;
	
	// Open the file in append mode...
	std::ofstream outfile;
	outfile.open (_property_file_name.c_str(), std::ios_base::app);
	assert(outfile.is_open());
	
	
	// Add the precedence property to the end of the file...
	outfile << "/* Precedence property " << _expr_p  << " " << _expr_q << "*/" << std::endl;
	outfile << "#define s (" << _expr_q << ")" << std::endl;
	outfile << "#define p (" << _expr_p << ")" << std::endl;
	outfile << "never {    /*  !(([](!p)) || ((!p) U  s))  */" << std::endl;
	outfile << "T0_init:" << std::endl;
	outfile << "if" << std::endl;
	outfile << ":: (! ((s)) && (p)) -> goto accept_S4" << std::endl;
	outfile << ":: (! ((s)) && (p)) -> goto accept_all" << std::endl;
	outfile << ":: (! ((s))) -> goto T0_init" << std::endl;
	outfile << ":: (! ((s)) && (p)) -> goto accept_S13" << std::endl;
	outfile << "fi;" << std::endl;
	outfile << "accept_S4:" << std::endl;
	outfile << "if" << std::endl;
	outfile << ":: (! ((s))) -> goto accept_S4" << std::endl;
	outfile << ":: (! ((s)) && (p)) -> goto accept_all" << std::endl;
	outfile << "fi;" << std::endl;
	outfile << "accept_S13:" << std::endl;
	outfile << "if" << std::endl;
	outfile << ":: ((p)) -> goto accept_all" << std::endl;
	outfile << ":: (1) -> goto T0_S13" << std::endl;
	outfile << "fi;" << std::endl;
	outfile << "T0_S13:" << std::endl;
    outfile << "if" << std::endl;
	outfile << ":: ((p)) -> goto accept_all" << std::endl;
	outfile << ":: (1) -> goto T0_S13" << std::endl;
	outfile << "fi;" << std::endl;
	outfile << "accept_all:" << std::endl;
	outfile << "skip }" << std::endl;
	
	outfile.close();
	return true;

}

bool cMDEPrecedenceProperty::printWitness() {
	
	// Create the file
	std::string cmd = "cp " + _promela + " " + _witness_file_name;
	if(system(cmd.c_str())!=0) return false;
	
	// Open the file in append mode
	std::ofstream outfile;
	outfile.open (_witness_file_name.c_str(), std::ios_base::app);
	assert(outfile.is_open());
	
	// Add precedence property witness to the end of the file....
	outfile << "/* Precedence property " << _expr_p  << " " << _expr_q << "*/" << std::endl;
	outfile << "#define q (" << _expr_q << ")" << std::endl;
	outfile << "#define p (" << _expr_p << ")" << std::endl;
	outfile << "never {    /* !p U (q && <>p) */" << std::endl;
	outfile << "T0_init:" << std::endl;
	outfile << "if" << std::endl;
	outfile << ":: ((p) && (q)) -> goto accept_all" << std::endl;
	outfile << ":: ((q)) -> goto T0_S4" << std::endl;
	outfile << ":: (! ((p))) -> goto T0_init" << std::endl;
	outfile << "fi;" << std::endl;
	outfile << "T0_S4:" << std::endl;
	outfile << "if" << std::endl;
	outfile << 	":: ((p)) -> goto accept_all" << std::endl;
	outfile << ":: (1) -> goto T0_S4" << std::endl;
	outfile << "fi;" << std::endl;
	outfile << "accept_all:" << std::endl;
    outfile << "skip }" << std::endl;

	outfile.close();
	return true;
	
}

bool cMDEPrecedenceProperty::printInEnglish() {
	
	std::ofstream outfile;
	outfile.open (_properties.c_str(), std::ios_base::app);
	assert(outfile.is_open());
	
	outfile << "Globally, it is always the case that if " << _expr_p  << " holds, ";
	outfile << "then "<< _expr_q << " previously held." << std::endl << std::endl;
	
	outfile.close();
	return true;
	
}

