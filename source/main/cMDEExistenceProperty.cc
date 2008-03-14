/*
 *  cMDEExistenceProperty.cc
 *  
 *
 *  Created by Heather Goldsby on 11/20/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "cMDEExistenceProperty.h"

void cMDEExistenceProperty::print() {
	
	// Create the file...
	std::string cmd = "cp " + _promela + " " + _property_file_name;
	if(system(cmd.c_str())!=0) return;
	
	// Open the file in append mode...
	std::ofstream outfile;
	outfile.open (_property_file_name.c_str(), std::ios_base::app);
	assert(outfile.is_open());
	
	// Add the existence property to the end of the file...
	outfile << "/* Existence property " << _expr_p << "*/" << std::endl;
	outfile << "#define p (" << _expr_p << ")" << std::endl;
	outfile << "never { /* !(<>(p)) */ " << std::endl;
	outfile << "accept_init :    /* init */" << std::endl;
	outfile << "if " << std::endl;
	outfile << ":: (!p) -> goto accept_init " << std::endl;
	outfile << "fi; }" << std::endl;
	
	// close the file...
	outfile.close();

}

void cMDEExistenceProperty::printWitness() {
	
	// Create the file
	std::string cmd = "cp " + _promela + " " + _witness_file_name;
	if(system(cmd.c_str())!=0) return;
	
	// Open the file in append mode
	std::ofstream outfile;
	outfile.open (_witness_file_name.c_str(), std::ios_base::app);
	assert(outfile.is_open());
	
	// Add existence witness
	outfile << "#define p (" << _expr_p << ")" << std::endl;
	outfile << "never { /* !([](!p)) */ " << std::endl;
	outfile << "T0_init :    /* init */ " << std::endl;
	outfile << "if " << std::endl;
	outfile << ":: (1) -> goto T0_init" << std::endl;
	outfile << ":: (p) -> goto accept_all " << std::endl;
	outfile << "fi; " << std::endl;
	outfile << "accept_all :    /* 1 */" << std::endl;
	outfile << "skip }" << std::endl;
	
	
	outfile.close();
	
}

void cMDEExistenceProperty::printInEnglish() {
	
	std::ofstream outfile;
	outfile.open (_properties.c_str(), std::ios_base::app);
	assert(outfile.is_open());
	
	outfile << _interesting << ", ";
	if (_uses_related_classes) {
		outfile << "true" << ", ";
	} else { outfile << "false" << ", "; }
	outfile << "Globally, " << _expr_p  << " eventually holds." << std::endl<< std::endl;
	
	outfile.close();
}

