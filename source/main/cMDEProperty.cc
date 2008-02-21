/*
 *  cMDEProperty.cc
 *  
 *
 *  Created by Heather Goldsby on 11/20/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "cMDEProperty.h"

#include <cstdlib>
#include <cmath>
#include <climits>
#include <iomanip>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

float cMDEProperty::numWitnesses() {
/*	
	std::string file_name = "tmp-witness.pr";
	std::string cmd = "cp tmp.pr "+ file_name;
	if(system(cmd.c_str())!=0) return 0.0;*/
	
///	std::string file_name = "tmp-witness.pr";
	int num_witness = 0;

	std::string cmd = "./spin -a " +  _witness_file_name + " &> /dev/null";
	if(system(cmd.c_str())!=0) return 0.0;
	
	if(system("/usr/bin/gcc -DMEMLIM=512 pan.c -o pan &> /dev/null")!=0) return 0.0;
	if(system("./pan -e -n -a -w19  -m100000 -c1 &> ./pan.out")!=0) return 0.0;
	num_witness = (system("cat pan.out | perl -e 'while(<STDIN>) { if(/errors:\\s(\\d+)/) {exit($1);}}'"));
	if (num_witness != 0) {
		num_witness = 1;
	}
	
	return num_witness;
}


float cMDEProperty::verify() { 

//	std::string file_name = "tmp-property.pr";
	std::string cmd;
	
	cmd = "./spin -a " +  _property_file_name + " &> /dev/null";
	if(system(cmd.c_str())!=0) return 0.0;
	
	if(system("/usr/bin/gcc -DMEMLIM=512 pan.c -o pan &> /dev/null")!=0) return 0.0;
	if(system("./pan -a &> ./pan.out")!=0) return 0.0;
	int num = (system("cat pan.out | perl -e 'while(<STDIN>) { if(/errors:\\s(\\d+)/) {exit($1);}}'")); 
	if (num != 0) return 0;
	return 1.0;
}


void cMDEProperty::evaluate() { 
	float wit_reward = 0;
	float verify_reward = 0;
	std::string cmd;
	std::string work_prop = "properties_that_passed";

	
	// print the witness property
	printWitness();
	
	// call numWitnesses
	wit_reward = numWitnesses();
	
	// call verify
	if (wit_reward > 0) { 
		// print the property
		print();
		verify_reward = verify();
	}
	
	// if this property passed, then save it to a file
	if (verify_reward) { 
		printInEnglish();
//		cmd = "cat english-property >> " + work_prop;
//		system(cmd.c_str());
	}
	
//	_reward = wit_reward + verify_reward;
	_reward = verify_reward;
	
}

