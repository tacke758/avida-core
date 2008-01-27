/*
 *  cMDEResponseProperty.cc
 *  
 *
 *  Created by Heather Goldsby on 11/20/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "cMDEResponseProperty.h"

void cMDEResponseProperty::print() {
	
	std::ofstream outfile;
//	outfile.open (_name.c_str());
	outfile.open ("property");
	assert(outfile.is_open());
	
	outfile << "/* Response property " << _expr_p  << " " << _expr_q << "*/" << std::endl;
	outfile << "#define s (" << _expr_q << ")" << std::endl;
	outfile << "#define p (" << _expr_p << ")" << std::endl;
	outfile << "never {  /*  !([](p -> <>s))  */ " << std::endl;
	outfile << "T0_init:" << std::endl;
	outfile << "if" << std::endl;
	outfile << "(! ((s)) && (p)) -> goto accept_S4" << std::endl;
	outfile << ":: (1) -> goto T0_init" << std::endl;
	outfile << "fi;" << std::endl;
	outfile << "accept_S4:" << std::endl;
	outfile << "if " << std::endl;
	outfile << ":: (! ((s))) -> goto accept_S4" << std::endl;
	outfile << "fi; }" << std::endl;
	
	outfile.close();

}

void cMDEResponseProperty::printWitness() {
	
	std::ofstream outfile;
	std::string file_name = "witness-property";
	outfile.open (file_name.c_str());
	assert(outfile.is_open());
	
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
	
}

