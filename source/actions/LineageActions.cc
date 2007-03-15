/*
 *  LineageActions.cc
 *  Avida
 *
 *  Created by Art Covert on 01/03/07 - all your genomes are belonged to us
 *  Copyright 2007 Michigan State University. All rights reserved.
 *
 */

//all the headers I could possiably need and more that I probably don't
#include "LandscapeActions.h"

#include "cAction.h"
#include "cActionLibrary.h"
#include "cAnalyze.h"
#include "cAnalyzeGenotype.h"
#include "tAnalyzeJob.h"
#include "cClassificationManager.h"
#include "cGenotype.h"
#include "cGenotypeBatch.h"
#include "cHardwareManager.h"
#include "cLandscape.h"
#include "cMutationalNeighborhood.h"
#include "cMutationalNeighborhoodResults.h"
#include "cOrganism.h"
#include "cPhenotype.h"
#include "cPopulation.h"
#include "cPopulationCell.h"
#include "cStats.h"
#include "cString.h"
#include "cTestUtil.h"
#include "cWorld.h"
#include "cWorldDriver.h"
#include "tSmartArray.h"
#include "defs.h"

#include <float.h>
#include <iomanip>

class cActionAnalyzeSingleMutation : public cAction
{
protected:


  cString m_filename; // name of the output file

  int m_totalDesendents; //total number of desendents for the current mutation being inspected

  double m_fitChange; // change in fitness caused by the mutation in question


  /*** These variables describe the effects subsequent mutations had ***/

  int m_removed; //all mutations are generally removed, 1 if it was removed, -1 if it persisted at the end of the lineage

 
  int m_compensitory; //any muation that restores fitness is compensitory
  double m_totalCompensationFit;

  int m_pureCompensitory; //any mutation that restores fitness only in the presence of the original mutaiton is purely compensitory
  double m_totalPureCompensationFit;

  int m_superCompensitory; //any mutation that restores more fitness than was originally lost
  double m_totalSuperCompensationFit;

  int m_pureSuperCompensitory; //any super compensitory mutation only in the presence of the original mutation
  double m_pureSuperCompensationFit;

  double m_genomeFitEffect; //effect of the fitness ratio for the child relative to parent (-1 del, 0 neut, 1 ben)
  double m_mutationFitEffect; //effect of fitness ratio for child without original deleterious

  tArray<int> m_used; //the mutation is used as part of a task
  //double m_totalUsedFit;

  int m_usedTasks;  // was the mutation needed to perform some task...
  double m_usedTasksFit;

  int m_hangingAround; //number of molcular clock ticks the mutation persisted for

  /*** end ***/

  /*** Private helper functions ***/

  //isolate the difference between two genomes, returns -1 if there is more than one positions difference
  virtual const int find_diff_pos(const cGenome& a, const cGenome& b){

    //in this case they muts both be the same size
    if(a.GetSize() != b.GetSize()) return -1;

    //variable to hold the size of the genome
    const int size = a.GetSize();

    //position of the last mutation found
    int mut_pos = -1;

    //loop thorugh and try to find one and only one mutation.
    for(int i = 0; i < size; i++){
      if(a[i].GetOp() != b[i].GetOp()){
	
	//more than one mutation - for now we bail out
	if(mut_pos == -1)
	  mut_pos = i;
	else {
	  cerr << "Too many mutations, bailing out" << endl;
	  return -1;
	}
      }
    }
    return mut_pos;

  }

  //recalculate the lineage -- not used
  virtual void recalc(cAvidaContext& ctx, 
	      cTestCPU* testcpu, 
	      tListIterator<cAnalyzeGenotype>& batch_it){
    while(!batch_it.AtEnd()){
      batch_it.Next();
      batch_it.Get()->Recalculate(ctx, testcpu, batch_it.PrevData());
    }

  }


  virtual bool inFinalDom(const int& pos, 
			  tListIterator<cAnalyzeGenotype> batch_it)
  {

    int mutated_op = batch_it.Get()->GetGenome()[pos].GetOp();

    do{
      batch_it.Next();
      if(batch_it.Get()->GetGenome()[pos].GetOp() != mutated_op) break;
    }while(!batch_it.AtEnd() && (batch_it.Get()->GetUpdateDead() != -1));

    return (batch_it.Get()->GetUpdateDead() == -1);
  }

  virtual int generate_optimal_batch(const int& pos,
				     tListIterator<cAnalyzeGenotype> batch_it, 
				     tList<cAnalyzeGenotype>& optimal) {}


  virtual int generate_revert_batch(const int& pos,
				    tListIterator<cAnalyzeGenotype> batch_it, 
				    tList<cAnalyzeGenotype>& reverted) {

    reverted.EraseList();
    //cerr << "Pos : " << pos << endl;
    //cerr << "Size: " << "" << endl;
    //ancestral op and it's mutated pair
    int reverted_op = batch_it.PrevData()->GetGenome()[pos].GetOp();
    int mutated_op = batch_it.Get()->GetGenome()[pos].GetOp();

    int mutated_id = batch_it.Get()->GetID();

    //lets just be a bit anal while were developing
    if(reverted_op == mutated_op){
      cerr << "ERROR: reverted and muteated instructions are the same!" << endl;
      return -1;
    }

    tListIterator<cAnalyzeGenotype> control_it(m_world->GetAnalyze().GetCurrentBatch().List());

    control_it.Next();
    //isolate each genome with the mutation in it - make two lists, one with the ancestral state and one with the reverted state
    if(!control_it.AtEnd()){
      //get all the positions before the mutation entered
      do{
	cGenome newGene(control_it.Get()->GetGenome());
	newGene[pos].SetOp(mutated_op);
	reverted.PushRear(new cAnalyzeGenotype(m_world,newGene,control_it.Get()->GetInstSet()));
	control_it.Next();
      }while(control_it.Get()->GetID() != mutated_id);

      //after the mutation entered
      do{
      
	cGenome newGene(control_it.Get()->GetGenome());
	newGene[pos].SetOp(reverted_op);
	reverted.PushRear(new cAnalyzeGenotype(m_world,newGene,control_it.Get()->GetInstSet()));
	
	//need to do it like this so we always check the last one... aparently Charles dosen't belive in tail nodes...
	if(control_it.AtEnd()) break;
	control_it.Next();
      }while(control_it.Get()->GetGenome()[pos].GetOp() == mutated_op);

      int break_id = control_it.Get()->GetID();

      //after the mutation left (may never get here.)
      //      if(!control_it.AtEnd())
	do{
	  
	  cGenome newGene(control_it.Get()->GetGenome());
	  newGene[pos].SetOp(mutated_op);
	  reverted.PushRear(new cAnalyzeGenotype(m_world,newGene,control_it.Get()->GetInstSet()));
	  
	  //need to do it like this so we always check the last one... aparently Charles dosen't belive in tail nodes...
	  if(control_it.AtEnd()) break;
	  control_it.Next();
	}while(1);

      
      //cerr << "Here" << endl;
      return break_id;
    }
    return -1;
  }

  //find the slice of the lineage that contains the muation and its counterpart without the mutation
  virtual void generate_new_batches(const int& pos,
			    tListIterator<cAnalyzeGenotype> batch_it, 
			    tList<cAnalyzeGenotype>& org, 
			    tList<cAnalyzeGenotype>& reverted) {

    //ancestral op and it's mutated pair
    int reverted_op = batch_it.PrevData()->GetGenome()[pos].GetOp();
    int mutated_op = batch_it.Get()->GetGenome()[pos].GetOp();

    //lets just be a bit anal while were developing
    if(reverted_op == mutated_op){
      cerr << "ERROR: reverted and muteated instructions are the same!" << endl;
      return;
    }
    
    //int i = 0;
    
    //isolate each genome with the mutation in it - make two lists, one with the ancestral state and one with the reverted state
    if(!batch_it.AtEnd())
      while(batch_it.Get()->GetGenome()[pos].GetOp() == mutated_op){
	org.PushRear(batch_it.Get());
	//reverted.Push((*batch_it.Get()));
	cGenome newGene(org.GetLast()->GetGenome());
	newGene[pos].SetOp(reverted_op);
	reverted.PushRear(new cAnalyzeGenotype(m_world,newGene,batch_it.Get()->GetInstSet()));
	
	//need to do it like this so we always check the last one... aparently Charles dosen't belive in tail nodes...
	if(batch_it.AtEnd()) break;
	batch_it.Next();
      }
    //cerr << "Here" << endl;
    return;
  }

  //output the current mutation to a file 
  virtual void output_mutation(cDataFile& file, 
		       tListIterator<cAnalyzeGenotype> org, 
		       tListIterator<cAnalyzeGenotype> revert,
		       double& effect,
		       double& net_revert_effect,
		       int& mut_pos){
    file.Write(org.Get()->GetID(),"Genotype ID");
    file.Write(org.Get()->GetParentID(),"Parent ID");
    file.Write(m_world->GetHardwareManager().GetInstSet().GetName(org.Get()->GetGenome()[mut_pos].GetOp()),"From");
    file.Write(m_world->GetHardwareManager().GetInstSet().GetName(revert.Get()->GetGenome()[mut_pos].GetOp()),"To");
    //file
  }

  //reset all of the member variables for the next mutation.
  virtual void reset_private(){
    m_totalDesendents = 0;
    m_fitChange = 0.0;
    m_removed = -1;
    m_compensitory = 0;
    m_totalCompensationFit = 0.0;
    m_pureCompensitory = 0;
    m_totalPureCompensationFit = 0.0;
    m_superCompensitory = 0;
    m_pureSuperCompensationFit = 0.0;
    m_totalSuperCompensationFit = 0.0;
    m_pureSuperCompensitory = 0;
    m_hangingAround = 0;
    m_used.SetAll(0);
    m_usedTasks = 0;
    m_usedTasksFit = 0.0;
    m_genomeFitEffect = 0.0;
    m_mutationFitEffect = 0.0;
  }

  virtual void find_all_mutations(tList<tListIterator<cAnalyzeGenotype> > & all_muts, const int & mut_pos){
    tListIterator<cAnalyzeGenotype> batch_it(m_world->GetAnalyze().GetCurrentBatch().List());

    //advance to ancestral genome -- first one in the batch
    batch_it.Next();

    do{
      //get the next genome
      batch_it.Next();
      if(batch_it.Get()->GetGenome()[mut_pos].GetOp() 
	 != batch_it.PrevData()->GetGenome()[mut_pos].GetOp()){
	all_muts.Push(new tListIterator<cAnalyzeGenotype>(batch_it));
      }
    }while(!batch_it.AtEnd());
  }


  /*
    classify fitness effects based on the log transfomred fitness ratio i.e. the fitness ratio is 
    transformed so that 0 is neutral(0.0), greater than is benifical(1.0), less than is deleterious(-1.0)

    threshold of 1.0% is used...
   */
  virtual double classify(const double& effect){
    if(effect > log10(1.01)){
      return 1.0;
       }else if(effect < log10(0.99)){
      return -1.0;
    }else
      return 0.0;
  }

  virtual double abs(const double& x){
    if(x < 0) return x*-1.0;
    else return x;
  }

  /*** end ***/

public:

  cActionAnalyzeSingleMutation(cWorld* world, const cString& args)
    : cAction(world, args), 
      m_filename("singleMutResults.dat"), 
      m_totalDesendents(0), 
      m_fitChange(0.0),
      m_removed(-1),
      m_compensitory(0),
      m_totalCompensationFit(0.0),
      m_pureCompensitory(0),
      m_totalPureCompensationFit(0.0),
      m_superCompensitory(0),
      m_totalSuperCompensationFit(0.0),
      m_pureSuperCompensitory(0),
      m_pureSuperCompensationFit(0.0),
      m_genomeFitEffect(0.0),
      m_mutationFitEffect(0.0),
      m_usedTasks(0),
      m_usedTasksFit(0.0),                                                   
      //m_used(0.0),
      //m_totalUsedFit(0.0),
      m_hangingAround(0)
  {
    cString largs(args);
    if (largs.GetSize()) m_filename = largs.PopWord(); 

    //set up m_used
    

  }

  static const cString GetDescription()
  {
    return "For each genome in the lineage evaluate the fate of its mutation.  Output is placed in a dat file, one dat file per batch, each dat file contains one line per organism.\nArguments: [filename='singleMutResults.dat']";
  }

  virtual void Process(cAvidaContext& ctx)
  {
    int mut_pos = -1;
    cTestCPU *my_testCPU = m_world->GetHardwareManager().CreateTestCPU();
    cDataFile& outfile = m_world->GetDataFile(m_filename);
    outfile.WriteComment("Fate of deleterious mutations in the line of descent.  One line per deleterious mutation.");

    if(ctx.GetAnalyzeMode()){
      tListIterator<cAnalyzeGenotype> batch_it(m_world->GetAnalyze().GetCurrentBatch().List());

      //advance to the first genome
      batch_it.Next();

      while(batch_it.Next() && !batch_it.AtEnd()){

	//only bother with deleterious mutations
	if(batch_it.GetConst()->GetFitness() < batch_it.PrevData()->GetFitness()){

	  tList<cAnalyzeGenotype> curr_mutation, reverted_mutation;
	
	  //locate the mutated site
	  mut_pos = find_diff_pos(batch_it.GetConst()->GetGenome(), batch_it.PrevData()->GetGenome());

	  //if there is no mutation, or more than one skip
	  if(mut_pos == 0 || mut_pos == -1) continue;

	  //set up the arrays
	  generate_new_batches(mut_pos, batch_it, curr_mutation, reverted_mutation);

	  if(curr_mutation.GetSize() == 1) continue;

	  tListIterator<cAnalyzeGenotype> curr_it(curr_mutation);
	  tListIterator<cAnalyzeGenotype> revert_it(reverted_mutation);


	  //advance to the first org
	  curr_it.Next();
	  revert_it.Next();

	  m_totalDesendents = curr_mutation.GetSize();


	  //what was the change in fitness due to this deleterious mutation
	  m_fitChange = revert_it.Get()->GetFitness() - curr_it.Get()->GetFitness();

	  m_used.Resize(curr_it.Get()->GetTaskCounts().GetSize(),0);

	  int count = 1;

	  double 
	    effect_size = 0.0,
	    revert_effect_size = 0.0, //revert effect_size may now be useless... think about taking it out
	    effect = 0,
	    revert_effect = 0;
	  

	  //recalculate stats for all of the genomes -- determine the effect of reversion
	  while(!curr_it.AtEnd() && !revert_it.AtEnd()){
	    //for(int i = 1; i < m_totalDesendents; i++){

	    if(curr_mutation.GetLast()->GetUpdateDead() == -1) m_hangingAround = 1;

	    count++;

	    curr_it.Get()->Recalculate(ctx,my_testCPU, reverted_mutation.GetFirst()); 
	    revert_it.Get()->Recalculate(ctx,my_testCPU, curr_it.Get());

	    //transform the ratio into a number representing the effect, 0 neutral, negative deleterious, positive benifical
	    effect = log10(curr_it.Get()->GetFitnessRatio());

	    //same transformation, but in reverse, ie fitness going up here means that the mutation is delterious and so on
	    revert_effect = -log10(revert_it.Get()->GetFitnessRatio());

	    m_genomeFitEffect = classify(effect);
	    m_mutationFitEffect = classify(revert_effect);
	    
	    //compute the ratio of the ancestral fitness to the final fitness of each sub-lineage
	    //note: the first item in the revert sublineage is effectivly the ancestral state	      
	    effect_size = curr_it.Get()->GetFitness() /  reverted_mutation.GetFirst()->GetFitness(); 
	    revert_effect_size = revert_it.Get()->GetFitness() / reverted_mutation.GetFirst()->GetFitness();
	    
	    
	    if(m_genomeFitEffect > 0 && m_mutationFitEffect > 0){
	      break;
	    }else if( m_mutationFitEffect >= 0){
	      break;
	    }
	    
	    //advance to next step
	    curr_it.Next();
	    revert_it.Next();
	    
	  }

	  outfile.Write(batch_it.Get()->GetID(),"Genotype ID");
	  outfile.Write(batch_it.Get()->GetParentID(),"Parent ID");
	  outfile.Write(1.0 - (curr_mutation.GetFirst()->GetFitness() / reverted_mutation.GetFirst()->GetFitness()), "% fitness lost");
	  outfile.Write(m_totalDesendents,"Total number of descendents");
	  outfile.Write(count, "Depth of break");

	  outfile.Write(m_genomeFitEffect,"Final mutation effect on genome");
	  outfile.Write(m_mutationFitEffect,"Final mutation effect on delterious mutation");
	  outfile.Write(effect_size, "Total effect size of original sub-lineage");
	  outfile.Write(revert_effect_size, "Total effect size of reverted sub-lineage");
	  outfile.Write(revert_effect_size - effect_size, "Final effect of deleterious mutation");

	  outfile.Write(m_usedTasks,"Number of tasks using the mutaiton");
	  outfile.Write(m_usedTasksFit,"Fitness from tasks using this mutation.");
	  outfile.Write(m_hangingAround, "Alive at end of run...");
	  outfile.Endl();
	}

	//get us ready for the next pass
	reset_private();

      }
      
      
    }
  }
};

class  cActionMarginalMutationEffectMatrix0 : public cActionAnalyzeSingleMutation
{
protected:

  virtual void generate_new_batches(const int& pos,
			    tListIterator<cAnalyzeGenotype> batch_it, 
			    tList<cAnalyzeGenotype>& org, 
			    tList<cAnalyzeGenotype>& reverted) {

    //ancestral op and it's mutated pair
    int reverted_op = batch_it.PrevData()->GetGenome()[pos].GetOp();
    int mutated_op = batch_it.Get()->GetGenome()[pos].GetOp();

    //lets just be a bit anal while were developing
    if(reverted_op == mutated_op){
      cerr << "ERROR: reverted and muteated instructions are the same!" << endl;
      return;
    }

    //isolate each genome with the mutation in it - make two lists, one with the ancestral state and one with the reverted state
    if(!batch_it.AtEnd())
      while(batch_it.Get()->GetGenome()[pos].GetOp() == mutated_op){
	org.PushRear(batch_it.Get());
	//reverted.Push((*batch_it.Get()));
	cGenome newGene(org.GetLast()->GetGenome());
	newGene[pos].SetOp(reverted_op);
	reverted.PushRear(new cAnalyzeGenotype(m_world,newGene,batch_it.Get()->GetInstSet()));
	
	//need to do it like this so we always check the last one... aparently Charles dosen't belive in tail nodes...
	if(batch_it.AtEnd()) break;
	batch_it.Next();
      }
    //cerr << "Here" << endl;
    return;
  }

public:

  cActionMarginalMutationEffectMatrix0(cWorld* world, const cString& args)
    : cActionAnalyzeSingleMutation(world, args)
  {
    cString largs(args);
    if (largs.GetSize()) m_filename = largs.PopWord();
  }

  virtual void Process(cAvidaContext& ctx)
  {
    int mut_pos = -1;
    cTestCPU *my_testCPU = m_world->GetHardwareManager().CreateTestCPU();
    cDataFile& outfile = m_world->GetDataFile(m_filename);
    ofstream& out = outfile.GetOFStream();
    //    outfile.WriteComment("Effects of all mutations resulting in the dominant organism.");

    tList<tArray<double> > matrix;
    tListIterator<tArray<double> > column_it(matrix);
    tArray<double> *marginalEffect; // represents a temp pointer to one column

    int count = 0;
    int listDiff = -1;

    if(ctx.GetAnalyzeMode()){
      tListIterator<cAnalyzeGenotype> batch_it(m_world->GetAnalyze().GetCurrentBatch().List());
      //advance to the first genome
      batch_it.Next();

      while(batch_it.Next() && !batch_it.AtEnd()){

	tList<cAnalyzeGenotype> reverted_mutation, control_sub_lineage;
	
	//locate the mutated site
	mut_pos = find_diff_pos(batch_it.GetConst()->GetGenome(), batch_it.PrevData()->GetGenome());
	
	//if there is no mutation, or more than one skip
	if(mut_pos == 0 || mut_pos == -1) continue;
	
	//set up the arrays
// 	if(count == 0)
	generate_new_batches(mut_pos, batch_it, control_sub_lineage, reverted_mutation);
// 	else
// 	  generate_revert_batch(mut_pos, batch_it, reverted_mutation);

	if(reverted_mutation.GetSize() == 1) continue;

	//cerr << curr_mutation.GetLast()->GetUpdateDead() << endl;


	// is this mutation alive at the end of the run...
	if(control_sub_lineage.GetLast()->GetUpdateDead() == -1){

	  count++;

	  tListIterator<cAnalyzeGenotype> curr_it(control_sub_lineage);
	  tListIterator<cAnalyzeGenotype> revert_it(reverted_mutation);
	  marginalEffect = new tArray<double>(); //create a new column
	
	  //advance to the first org
	  curr_it.Next();
	  revert_it.Next();

	  //line up the current genotype iterator
	  //lastDiff = control_sub_lineage.GetSize() - reverted_mutation.GetSize();
	  //for(int i = lastDiff; 0 < i; i--)
	  //  curr_it.Next();
	  
	  
	  m_totalDesendents = reverted_mutation.GetSize();
	  
	  
	  //what was the change in fitness due to this deleterious mutation
	  m_fitChange = revert_it.Get()->GetFitness() - curr_it.Get()->GetFitness();
	  
	  m_used.Resize(curr_it.Get()->GetTaskCounts().GetSize(),0);
	  
	  int count = 1;
	  
	  double 
	    effect_size = 0.0,
	    revert_effect_size = 0.0, //revert effect_size may now be useless... think about taking it out
	    effect = 0,
	    revert_effect = 0;
	  
	  
	  //recalculate stats for all of the genomes -- determine the effect of reversion
	  while(!curr_it.AtEnd() && !revert_it.AtEnd()){
	    
	    count++;
	    
	    curr_it.Get()->Recalculate(ctx,my_testCPU, reverted_mutation.GetFirst()); 
	    //curr_it.Get()->Recalculate(ctx,my_testCPU, batch_it.PrevData()); 
	    revert_it.Get()->Recalculate(ctx,my_testCPU, curr_it.Get());
	    
	    //transform the ratio into a number representing the effect, 0 neutral, negative deleterious, positive benifical
	    if(curr_it.Get()->GetFitnessRatio() != 0)
	      effect = log10(curr_it.Get()->GetFitnessRatio());
	    else {
	      effect = -9.9;
	    }
	    
	    //same transformation, but in reverse, ie fitness going up here means that the mutation is delterious and so on
	    if(revert_it.Get()->GetFitnessRatio() != 0)
	      revert_effect = -log10(revert_it.Get()->GetFitnessRatio());
	    else revert_effect = 9.9;
	    
	    if(count == 2){
	      m_genomeFitEffect = classify(effect);
	      m_mutationFitEffect = classify(revert_effect);
	      marginalEffect->Push(m_mutationFitEffect);
	    }
	    
	    marginalEffect->Push(revert_effect);

	    //compute the ratio of the ancestral fitness to the final fitness of each sub-lineage
	    //note: the first item in the revert sublineage is effectivly the ancestral state	      
	    effect_size = curr_it.Get()->GetFitness() /  reverted_mutation.GetFirst()->GetFitness(); 
	    revert_effect_size = revert_it.Get()->GetFitness() / reverted_mutation.GetFirst()->GetFitness();
	    
	    //advance to next step
	    curr_it.Next();
	    revert_it.Next();
	    
	  }

	  //at the column to the matrix -- should be in sorted order by size
	  matrix.Insert(column_it,marginalEffect);
	}  
	  
	//get us ready for the next pass
	reset_private();
	
      } // end of big while

      tListIterator<tArray<double> > matrix_it(matrix);
      //matrix_it.Next();

      int numRows = matrix.GetFirst()->GetSize();
      int offset = 0;

      //      out << m_mutationFitEffect << endl;
      do{
	matrix_it.Next();
	offset = numRows - matrix_it.Get()->GetSize();
	out << setw(20) << (*matrix_it.Get())[0];
	for(int i = 1; i < numRows; i++){
	  if(i  < offset)
	    out << setw(20) << "NaN";
	  else
	    out << setw(20) << setprecision(4) << scientific << (*matrix_it.Get())[i - offset]; 
	}
	out << endl;

      }while(!matrix_it.AtEnd());
      //cin.ignore();
      reset_private();
    }
  }


};

class  cActionMarginalMutationEffectMatrix0a : public cActionAnalyzeSingleMutation
{
protected:

  virtual void generate_new_batches(const int& pos,
			    tListIterator<cAnalyzeGenotype> batch_it, 
			    tList<cAnalyzeGenotype>& org, 
			    tList<cAnalyzeGenotype>& reverted) {

    //ancestral op and it's mutated pair
    int reverted_op = batch_it.PrevData()->GetGenome()[pos].GetOp();
    int mutated_op = batch_it.Get()->GetGenome()[pos].GetOp();

    //lets just be a bit anal while were developing
    if(reverted_op == mutated_op){
      cerr << "ERROR: reverted and muteated instructions are the same!" << endl;
      return;
    }

    //isolate each genome with the mutation in it - make two lists, one with the ancestral state and one with the reverted state
    if(!batch_it.AtEnd())
      while(batch_it.Get()->GetGenome()[pos].GetOp() == mutated_op){
	org.PushRear(batch_it.Get());
	//reverted.Push((*batch_it.Get()));
	cGenome newGene(org.GetLast()->GetGenome());
	newGene[pos].SetOp(reverted_op);
	reverted.PushRear(new cAnalyzeGenotype(m_world,newGene,batch_it.Get()->GetInstSet()));
	
	//need to do it like this so we always check the last one... aparently Charles dosen't belive in tail nodes...
	if(batch_it.AtEnd()) break;
	batch_it.Next();
      }
    //cerr << "Here" << endl;
    return;
  }

public:

  cActionMarginalMutationEffectMatrix0a(cWorld* world, const cString& args)
    : cActionAnalyzeSingleMutation(world, args)
  {
    cString largs(args);
    if (largs.GetSize()) m_filename = largs.PopWord();
  }

  virtual void Process(cAvidaContext& ctx)
  {
    //int mut_pos = -1;
    cTestCPU *my_testCPU = m_world->GetHardwareManager().CreateTestCPU();
    cDataFile& outfile = m_world->GetDataFile(m_filename);
    ofstream& out = outfile.GetOFStream();
    //    outfile.WriteComment("Effects of all mutations resulting in the dominant organism.");

    tList<tArray<double> > matrix;
    tListIterator<tArray<double> > column_it(matrix);
    tArray<double> *marginalEffect; // represents a temp pointer to one column

    int count = 0;
    int listDiff = -1;

    if(ctx.GetAnalyzeMode()){
      tListIterator<cAnalyzeGenotype> batch_it(m_world->GetAnalyze().GetCurrentBatch().List());
      //advance to the first genome
      batch_it.Next();

      //      while(batch_it.Next() && !batch_it.AtEnd()){
	
	//locate the mutated site
	//mut_pos = find_diff_pos(batch_it.GetConst()->GetGenome(), batch_it.PrevData()->GetGenome());
	for(unsigned mut_pos = 0; mut_pos < batch_it.Get()->GetGenome().GetSize(); mut_pos++){
	  tList<cAnalyzeGenotype> reverted_mutation, control_sub_lineage;
	

	  //if there is no mutation, or more than one skip
	  //if(mut_pos == 0 || mut_pos == -1) continue;
	  tList<tListIterator<cAnalyzeGenotype> > all_muts;

	  find_all_mutations(all_muts, mut_pos);

	  tListIterator<tListIterator<cAnalyzeGenotype> > all_muts_it(all_muts);

	  //find the first mutation still alive at the end of the run
	  all_muts_it.Prev();
	  while(1){
	    all_muts_it.Prev();
	  
	    if(all_muts_it.Get()->NextData()->GetUpdateDead() == -1 && 
	       all_muts_it.Get()->Get()->GetUpdateDead() != -1)
	      break;
	  }

	  generate_new_batches(mut_pos, *all_muts_it.Get(), control_sub_lineage, reverted_mutation);

	  if(reverted_mutation.GetSize() == 1) continue;

	  //cerr << curr_mutation.GetLast()->GetUpdateDead() << endl;


	  // is this mutation alive at the end of the run...
	  if(control_sub_lineage.GetLast()->GetUpdateDead() == -1){

	    count++;

	    tListIterator<cAnalyzeGenotype> curr_it(control_sub_lineage);
	    tListIterator<cAnalyzeGenotype> revert_it(reverted_mutation);
	    marginalEffect = new tArray<double>(); //create a new column
	
	    //advance to the first org
	    curr_it.Next();
	    revert_it.Next();

	    //line up the current genotype iterator
	    //lastDiff = control_sub_lineage.GetSize() - reverted_mutation.GetSize();
	    //for(int i = lastDiff; 0 < i; i--)
	    //  curr_it.Next();
	  
	  
	    m_totalDesendents = reverted_mutation.GetSize();
	  
	  
	    //what was the change in fitness due to this deleterious mutation
	    m_fitChange = revert_it.Get()->GetFitness() - curr_it.Get()->GetFitness();
	  
	    m_used.Resize(curr_it.Get()->GetTaskCounts().GetSize(),0);
	  
	    int count = 1;
	  
	    double 
	      effect_size = 0.0,
	      revert_effect_size = 0.0, //revert effect_size may now be useless... think about taking it out
	      effect = 0,
	      revert_effect = 0;
	  
	  
	    //recalculate stats for all of the genomes -- determine the effect of reversion
	    while(!curr_it.AtEnd() && !revert_it.AtEnd()){
	    
	      count++;
	    
	      curr_it.Get()->Recalculate(ctx,my_testCPU, reverted_mutation.GetFirst()); 
	      //curr_it.Get()->Recalculate(ctx,my_testCPU, batch_it.PrevData()); 
	      revert_it.Get()->Recalculate(ctx,my_testCPU, curr_it.Get());
	    
	      //transform the ratio into a number representing the effect, 0 neutral, negative deleterious, positive benifical
	      if(curr_it.Get()->GetFitnessRatio() != 0)
		effect = log10(curr_it.Get()->GetFitnessRatio());
	      else {
		effect = -9.9;
	      }
	    
	      //same transformation, but in reverse, ie fitness going up here means that the mutation is delterious and so on
	      if(revert_it.Get()->GetFitnessRatio() != 0)
		revert_effect = -log10(revert_it.Get()->GetFitnessRatio());
	      else revert_effect = 9.9;
	    
	      if(count == 2){
		m_genomeFitEffect = classify(effect);
		m_mutationFitEffect = classify(revert_effect);
		marginalEffect->Push(m_mutationFitEffect);
	      }
	    
	      marginalEffect->Push(revert_effect);

	      //compute the ratio of the ancestral fitness to the final fitness of each sub-lineage
	      //note: the first item in the revert sublineage is effectivly the ancestral state	      
	      effect_size = curr_it.Get()->GetFitness() /  reverted_mutation.GetFirst()->GetFitness(); 
	      revert_effect_size = revert_it.Get()->GetFitness() / reverted_mutation.GetFirst()->GetFitness();
	    
	      //advance to next step
	      curr_it.Next();
	      revert_it.Next();
	    
	    }

	    //at the column to the matrix -- should be in sorted order by size
	    matrix.Insert(column_it,marginalEffect);
	  }  
	  
	  //get us ready for the next pass
	  reset_private();
	}
	//      } // end of big while

      tListIterator<tArray<double> > matrix_it(matrix);
      int numRows = matrix.GetFirst()->GetSize();
      do{
	matrix_it.Next();
	if(numRows < matrix_it.Get()->GetSize())
	  numRows = matrix_it.Get()->GetSize();
	   
      }while(!matrix_it.AtEnd());
      int offset = 0;

      //      out << m_mutationFitEffect << endl;
      do{
	matrix_it.Next();
	offset = numRows - matrix_it.Get()->GetSize();
	
	if(matrix_it.Get()->GetSize() > 0)
	  out << setw(20) << (*matrix_it.Get())[0];
	else
	  out << setw(20) << "NaN";
	

	for(int i = 1; i < numRows; i++){
	  if(i  < offset)
	    out << setw(20) << "NaN";
	  else
	    out << setw(20) << setprecision(4) << scientific << (*matrix_it.Get())[i - offset]; 
	}
	out << endl;

      }while(!matrix_it.AtEnd());
      //cin.ignore();
      reset_private();
    }
  }


};

/*
class  cActionMarginalMutationEffectMatrix1 : public cActionAnalyzeSingleMutation
{
protected:

  tList<cAnalyzeGenotype>  control_sub_lineage;
  
public:

  cActionMarginalMutationEffectMatrix1(cWorld* world, const cString& args)
    : cActionAnalyzeSingleMutation(world, args)
  {
    cString largs(args);
    if (largs.GetSize()) m_filename = largs.PopWord();
  }

  virtual void Process(cAvidaContext& ctx)
  {
    int mut_pos = -1;
    cTestCPU *my_testCPU = m_world->GetHardwareManager().CreateTestCPU();
    cDataFile& outfile = m_world->GetDataFile(m_filename);
    ofstream& out = outfile.GetOFStream();
    //    outfile.WriteComment("Effects of all mutations resulting in the dominant organism.");

    tList<tArray<double> > matrix;
    tListIterator<tArray<double> > column_it(matrix);
    tArray<double> *marginalEffect; // represents a temp pointer to one column

    int count = 0;
    int listDiff = -1;

    if(ctx.GetAnalyzeMode()){
      tListIterator<cAnalyzeGenotype> batch_it(m_world->GetAnalyze().GetCurrentBatch().List());
      //advance to the first genome
      batch_it.Next();

      while(batch_it.Next() && !batch_it.AtEnd()){

	tList<cAnalyzeGenotype> reverted_mutation;
	
	//locate the mutated site
	mut_pos = find_diff_pos(batch_it.GetConst()->GetGenome(), batch_it.PrevData()->GetGenome());
	
	//if there is no mutation, or more than one skip--should never happen
	if(mut_pos == 0 || mut_pos == -1) continue;
	
	if(inFinalDom(mut_pos, batch_it)){

	  //set up the arrays
	  generate_revert_batch(mut_pos, batch_it, reverted_mutation);

	  if(reverted_mutation.GetSize() == 1) continue;

	  // is this mutation alive at the end of the run...
	  count++;
	  
	  tListIterator<cAnalyzeGenotype> curr_it(m_world->GetAnalyze().GetCurrentBatch().List());
	  tListIterator<cAnalyzeGenotype> revert_it(reverted_mutation);
	  marginalEffect = new tArray<double>(); //create a new column
	
	  //advance to the first org
	  curr_it.Next();
	  revert_it.Next();
	  
	  int count = 1;
	  
	  double 
	    effect_size = 0.0,
	    revert_effect_size = 0.0, //revert effect_size may now be useless... think about taking it out
	    effect = 0,
	    revert_effect = 0;

	  //	  cerr << "Processing site " << mut_pos << endl;
	  
	  //recalculate stats for all of the genomes -- determine the effect of reversion
	  while(!curr_it.AtEnd() && !revert_it.AtEnd()){
	    
	    count++;
	    
	    //curr_it.Get()->Recalculate(ctx,my_testCPU, batch_it.PrevData()); 
	    curr_it.Get()->Recalculate(ctx,my_testCPU, batch_it.PrevData()); 
	    revert_it.Get()->Recalculate(ctx,my_testCPU, curr_it.Get());

	    //going all the way back now--this part is obsolete
	    if(curr_it.Get()->GetID() >= batch_it.Get()->GetID()){
	      //same transformation, but in reverse, ie fitness going up here means that the mutation is delterious and so on
	      if(revert_it.Get()->GetFitnessRatio() != 0)
		revert_effect = -log10(revert_it.Get()->GetFitnessRatio());
	      else revert_effect = DBL_MAX;
	    }
	    else{
	      if(revert_it.Get()->GetFitnessRatio() != 0)
		revert_effect = log10(revert_it.Get()->GetFitnessRatio());
	      else revert_effect = DBL_MIN;
	    }	      
	    
	    if(count == 2){
	      //m_genomeFitEffect = classify(effect);
	      m_mutationFitEffect = classify(revert_effect);
	      marginalEffect->Push(m_mutationFitEffect);
	    }
	    
	    marginalEffect->Push(revert_effect);

	    //advance to next step
	    curr_it.Next();
	    revert_it.Next();
	    
	  }

	  //at the column to the matrix -- should be in sorted order by size
	  matrix.Insert(column_it,marginalEffect);
	}  
	  
	//get us ready for the next pass
	reset_private();
	
      } // end of big while

      tListIterator<tArray<double> > matrix_it(matrix);
      //matrix_it.Next();

      int numRows = matrix.GetFirst()->GetSize();
      int offset = 0;

      //      out << m_mutationFitEffect << endl;
      do{
	matrix_it.Next();
	//	offset = numRows - matrix_it.Get()->GetSize();
	out << setw(20) << (*matrix_it.Get())[0];
	for(int i = 1; i < numRows; i++){
	  if(i  < offset)
	    out << setw(20) << "NaN";
	  else
	    out << setw(20) << setprecision(4) << scientific << (*matrix_it.Get())[i - offset]; 
	}
	out << endl;

      }while(!matrix_it.AtEnd());
      //cin.ignore();
      reset_private();
    }
  }


};*/


class  cActionMarginalMutationEffectMatrix2 : public cActionAnalyzeSingleMutation
{
protected:

  tList<cAnalyzeGenotype>  control_sub_lineage;


public:

  cActionMarginalMutationEffectMatrix2(cWorld* world, const cString& args)
    : cActionAnalyzeSingleMutation(world, args)
  {
    cString largs(args);
    if (largs.GetSize()) m_filename = largs.PopWord();
  }

  virtual void Process(cAvidaContext& ctx)
  {
    //int mut_pos = -1;
    //int numRows = 0;
    cTestCPU *my_testCPU = m_world->GetHardwareManager().CreateTestCPU();
    cDataFile& outfile = m_world->GetDataFile(m_filename);
    ofstream& out = outfile.GetOFStream();

    //tArray<tArray<double> > matrix;
    //tListIterator<tArray<double> > column_it(matrix);
    //tArray<double> *marginalEffect; // represents a temp pointer to one column
    tArray<int> site_finished;

    tArray<double> blackBar;
    blackBar.Resize(m_world->GetAnalyze().GetCurrentBatch().List().GetSize()+2,-DBL_MAX);
    
    int numRows = m_world->GetAnalyze().GetCurrentBatch().List().GetSize()+2;
    int count = 0;
    int listDiff = -1;

    if(ctx.GetAnalyzeMode()){
      tList<tListIterator<cAnalyzeGenotype> > all_muts;
      tList<cAnalyzeGenotype> reverted_mutation;
      tListIterator<cAnalyzeGenotype> batch_it(m_world->GetAnalyze().GetCurrentBatch().List());
      //advance to the first genome
      batch_it.Next();

      site_finished.Resize(batch_it.Get()->GetGenome().GetSize(),0);

      for(unsigned mut_pos = 0; mut_pos < batch_it.Get()->GetGenome().GetSize(); mut_pos++){
	
	site_finished[mut_pos] = 1;

	//find all mutations at a particular site
	find_all_mutations(all_muts, mut_pos);

	if(all_muts.GetSize() > 0){
	  tListIterator<tListIterator<cAnalyzeGenotype> > all_muts_it(all_muts);

	  cerr << "Processing ... pos: " << mut_pos << " with " << all_muts.GetSize() << " mutations ... " << endl;

	  int count = 0;
	  //for each mutation at a site, evaluate it's effects and add a line to the final matrix
	  //for(int mut = 0; mut < all_muts.GetSize(); mut++){
	  do{	  
	    all_muts_it.Next();
	    tArray<double> marginalEffect;
	    //cerr << "Num muts: " << all_muts.GetSize() << " on mut: " << count << endl;
	    
	    //set up the array -- get the last id of a genome with this mutation
	    int break_id = generate_revert_batch(mut_pos, (*all_muts_it.Get()), reverted_mutation);

	    if(reverted_mutation.GetSize() == 1) continue;

	    // is this mutation alive at the end of the run...
	    //	    count++;
	  
	    tListIterator<cAnalyzeGenotype> curr_it(m_world->GetAnalyze().GetCurrentBatch().List());
	    tListIterator<cAnalyzeGenotype> revert_it(reverted_mutation);
	    //marginalEffect = new tArray<double>(); //create a new column
	
	    //advance to the first org
	    curr_it.Next();
	    revert_it.Next();
	  
	    int count = 1;
	    int out_count = 0;

	    double 
	      effect_size = 0.0,
	      revert_effect_size = 0.0, //revert effect_size may now be useless... think about taking it out
	      effect = 0,
	      revert_effect = 0;

	    //	  cerr << "Processing site " << mut_pos << endl;
	  
	    //recalculate stats for all of the genomes -- determine the effect of reversion
	    while(!curr_it.AtEnd() && !revert_it.AtEnd()){
	    
	      count++;

	      //if mutant came back into the pop, or it happens to be the same as the lineage at this point
	      //then the recalc is moot
	      if(revert_it.Get()->GetGenome()[mut_pos].GetOp() ==
		 curr_it.Get()->GetGenome()[mut_pos].GetOp()){
		revert_effect = 10.0;
	      }
	      else{
		//the parent pointer for curr_it doesn't really matter in this case...
		//if(count == 2)
		//curr_it.Get()->Recalculate(ctx,my_testCPU, all_muts_it.Get()->PrevData()); 
		revert_it.Get()->Recalculate(ctx,my_testCPU, curr_it.Get());

		if(curr_it.Get()->GetID() >= all_muts_it.Get()->Get()->GetID()){

		  //mark the beginning and end of the mutated section
		  if((curr_it.Get()->GetID() == all_muts_it.Get()->Get()->GetID()) ||
		     (curr_it.Get()->GetID() == break_id)){
		    out << setw(20) << setprecision(4) << scientific << -DBL_MAX;
		    out_count++;
		  }

		  //post-mutation section -- mutation effect after it leaves the pop
		  if(curr_it.Get()->GetID() >= break_id){
		    if(revert_it.Get()->GetFitnessRatio() != 0)
		      revert_effect = log10(revert_it.Get()->GetFitnessRatio());
		    else revert_effect = -9.9;
		  }
		  else{
		    //reverted secion -- fitness going up here means that the mutation is delterious and so on
		    if(revert_it.Get()->GetFitnessRatio() != 0)
		      revert_effect = -log10(revert_it.Get()->GetFitnessRatio());
		    else revert_effect = 9.9;
		  }
		}
		else{
		  //mutated section -- mutation added prematurally
		  if(revert_it.Get()->GetFitnessRatio() != 0)
		    revert_effect = log10(revert_it.Get()->GetFitnessRatio());
		  else revert_effect = -9.9;
		}	   
	      }

	      out << setw(20) << setprecision(4) << scientific << revert_effect;	    	    
	      out_count++;
	      //marginalEffect.Push(revert_effect);

	      //advance to next step
	      curr_it.Next();
	      revert_it.Next();
	    
	    }

	    //line up all the rows
	    if(out_count < numRows)
	      for(int i = out_count; i < numRows; i++)
		out << setw(20) << setprecision(4) << scientific << -DBL_MAX;
	    out << endl;

	    //get us ready for the next pass
	    reset_private();
	  
	  }while(!all_muts_it.AtEnd());

	  all_muts.EraseList();
	  reverted_mutation.EraseList();

	  for(int i=0; i < numRows; i++)
	    out << setw(20) << setprecision(4) << scientific << -DBL_MAX;
	  out << endl;
	}
      }
    }

  }


};

class  cActionMarginalMutationEffectMatrix2a : public cActionAnalyzeSingleMutation
{
protected:

  /*
    Generate a batch with the most immidiatly benefical mutaiton at the given site and step
   */
  virtual int generate_optimal_batch(const int& pos,
				     tListIterator<cAnalyzeGenotype> batch_it, 
				     tList<cAnalyzeGenotype>& optimal,
				     cAvidaContext& ctx) {
    optimal.EraseList();

    cTestCPU *my_testCPU = m_world->GetHardwareManager().CreateTestCPU();
    
    cInstSet inst_set = batch_it.Get()->GetInstSet();
    int inst_set_size = inst_set.GetSize();
    double max_fit = 0.0;
    int opt_op = -1;

    for(unsigned mut = 0; mut < inst_set_size; mut++){
	cGenome newGene(batch_it.Get()->GetGenome());
	newGene[pos].SetOp(mut);
	cAnalyzeGenotype candidate(m_world,newGene,inst_set);
	
	candidate.Recalculate(ctx, my_testCPU, batch_it.Get());

	if(candidate.GetFitness() >= max_fit){
	  max_fit = candidate.GetFitness();
	  opt_op = mut;
	}
    }


    tListIterator<cAnalyzeGenotype> control_it(m_world->GetAnalyze().GetCurrentBatch().List());

    do{
      control_it.Next();      
      cGenome newGene(control_it.Get()->GetGenome());
      newGene[pos].SetOp(opt_op);
      optimal.PushRear(new cAnalyzeGenotype(m_world,newGene,control_it.Get()->GetInstSet()));
    }while(!control_it.AtEnd());

      //cerr << "Here" << endl;
    return 1;
  }
  
  tList<cAnalyzeGenotype>  control_sub_lineage;

  virtual void find_all_mutations(tList<tListIterator<cAnalyzeGenotype> > & all_muts, const int & mut_pos){
    tListIterator<cAnalyzeGenotype> batch_it(m_world->GetAnalyze().GetCurrentBatch().List());

    //advance to ancestral genome -- first one in the batch
    batch_it.Next();

    do{
      //get the next genome
      batch_it.Next();
      if(batch_it.Get()->GetGenome()[mut_pos].GetOp() 
	 != batch_it.PrevData()->GetGenome()[mut_pos].GetOp()){
	all_muts.Push(new tListIterator<cAnalyzeGenotype>(batch_it));
      }
    }while(!batch_it.AtEnd());
  }

public:

  cActionMarginalMutationEffectMatrix2a(cWorld* world, const cString& args)
    : cActionAnalyzeSingleMutation(world, args)
  {
    cString largs(args);
    if (largs.GetSize()) m_filename = largs.PopWord();
  }

  virtual void Process(cAvidaContext& ctx)
  {
    //int mut_pos = -1;
    //int numRows = 0;
    cTestCPU *my_testCPU = m_world->GetHardwareManager().CreateTestCPU();
    cDataFile& outfile = m_world->GetDataFile(m_filename);
    ofstream& out = outfile.GetOFStream();

    //tArray<tArray<double> > matrix;
    //tListIterator<tArray<double> > column_it(matrix);
    //tArray<double> *marginalEffect; // represents a temp pointer to one column
    tArray<int> site_finished;

    tArray<double> blackBar;
    blackBar.Resize(m_world->GetAnalyze().GetCurrentBatch().List().GetSize()+2,-DBL_MAX);
    
    int numRows = m_world->GetAnalyze().GetCurrentBatch().List().GetSize()+2;
    int count = 0;
    int listDiff = -1;

    if(ctx.GetAnalyzeMode()){
      tList<tListIterator<cAnalyzeGenotype> > all_muts;

      tList<cAnalyzeGenotype> reverted_mutation;
      tList<cAnalyzeGenotype> optimal_mutation;
      tArray<double> optimal_effect;

      tListIterator<cAnalyzeGenotype> batch_it(m_world->GetAnalyze().GetCurrentBatch().List());
      //advance to the first genome
      batch_it.Next();

      site_finished.Resize(batch_it.Get()->GetGenome().GetSize(),0);

      for(unsigned mut_pos = 0; mut_pos < batch_it.Get()->GetGenome().GetSize(); mut_pos++){
	
	site_finished[mut_pos] = 1;

	//find all mutations at a particular site
	find_all_mutations(all_muts, mut_pos);

	if(all_muts.GetSize() > 0){
	  tListIterator<tListIterator<cAnalyzeGenotype> > all_muts_it(all_muts);

	  cerr << "Processing ... pos: " << mut_pos << " with " << all_muts.GetSize() << " mutations ... " << endl;

	  int count = 0;
	  //for each mutation at a site, evaluate it's effects and add a line to the final matrix
	  //for(int mut = 0; mut < all_muts.GetSize(); mut++){
	  do{	  
	    all_muts_it.Next();
	    tArray<double> marginalEffect;
	    //cerr << "Num muts: " << all_muts.GetSize() << " on mut: " << count << endl;
	    
	    //set up the array -- get the last id of a genome with this mutation
	    int break_id = generate_revert_batch(mut_pos, (*all_muts_it.Get()), reverted_mutation);
	    generate_optimal_batch(mut_pos, (*all_muts_it.Get()),optimal_mutation,ctx);
	    if(reverted_mutation.GetSize() == 1) continue;

//  	    cerr << " optimal list: " << optimal_mutation.GetSize() << endl
//  		 << "reverted list: " << reverted_mutation.GetSize() << endl;
// 	    if(optimal_mutation.GetSize() != reverted_mutation.GetSize())
// 	      cin.ignore();

	    // is this mutation alive at the end of the run...
	    //	    count++;
	  
	    tListIterator<cAnalyzeGenotype> curr_it(m_world->GetAnalyze().GetCurrentBatch().List());
	    tListIterator<cAnalyzeGenotype> revert_it(reverted_mutation);
	    tListIterator<cAnalyzeGenotype> optimal_it(optimal_mutation);
		  
	    int count = 1;
	    int out_count = 0;

	    double 
	      effect_size = 0.0,
	      revert_effect_size = 0.0, //revert effect_size may now be useless... think about taking it out
	      effect = 0,
	      revert_effect = 0;

	    //	  cerr << "Processing site " << mut_pos << endl;
	  
	    //recalculate stats for all of the genomes -- determine the effect of reversion
	    //    while(!curr_it.AtEnd() && !revert_it.AtEnd()){
	    do{
	      //advance to the first org
	      curr_it.Next();
	      revert_it.Next();
	      optimal_it.Next();


	      //	      cerr << setw(20) << setprecision(4) << scientific ;
	      count++;

	      if(optimal_it.Get()->GetGenome()[mut_pos].GetOp() ==
		 curr_it.Get()->GetGenome()[mut_pos].GetOp()){
		optimal_effect.Push(10.0);
	      }
	      else{
		optimal_it.Get()->Recalculate(ctx,my_testCPU, curr_it.Get());
		if(optimal_it.Get()->GetFitnessRatio() != 0){
		  //		  cerr << "optimal: " << optimal_it.Get()->GetFitness() << " " << curr_it.Get()->GetFitness() << " " << optimal_it.Get()->GetFitnessRatio() << " " << log10(optimal_it.Get()->GetFitnessRatio()) << endl;
		  optimal_effect.Push(log10(optimal_it.Get()->GetFitnessRatio()));
		}
		else{ 
		  //		  cerr << "optimal: " << optimal_it.Get()->GetFitness() << " " << curr_it.Get()->GetFitness() << " " << optimal_it.Get()->GetFitnessRatio() << " " << -9.9 << endl;
		  optimal_effect.Push(-9.9);}
	      }

	      //if mutant came back into the pop, or it happens to be the same as the lineage at this point
	      //then the recalc is moot
	      if(revert_it.Get()->GetGenome()[mut_pos].GetOp() ==
		 curr_it.Get()->GetGenome()[mut_pos].GetOp()){
		revert_effect = 10.0;
	      }
	      else{
		//the parent pointer for curr_it doesn't really matter in this case...
		//if(count == 2)
		//curr_it.Get()->Recalculate(ctx,my_testCPU, all_muts_it.Get()->PrevData()); 
		revert_it.Get()->Recalculate(ctx,my_testCPU, curr_it.Get());

		if(curr_it.Get()->GetID() >= all_muts_it.Get()->Get()->GetID()){

		  //mark the beginning and end of the mutated section
		  if((curr_it.Get()->GetID() == all_muts_it.Get()->Get()->GetID()) ||
		     (curr_it.Get()->GetID() == break_id)){
		    out << setw(20) << setprecision(4) << scientific << -DBL_MAX;
		    out_count++;
		  }

		  //post-mutation section -- mutation effect after it leaves the pop
		  if(curr_it.Get()->GetID() >= break_id){
		    if(revert_it.Get()->GetFitnessRatio() != 0)
		      revert_effect = log10(revert_it.Get()->GetFitnessRatio());
		    else revert_effect = -9.9;
		  }
		  else{
		    //reverted secion -- fitness going up here means that the mutation is delterious and so on
		    if(revert_it.Get()->GetFitnessRatio() != 0)
		      revert_effect = -log10(revert_it.Get()->GetFitnessRatio());
		    else revert_effect = 9.9;
		  }
		}
		else{
		  //mutated section -- mutation added prematurally
		  if(revert_it.Get()->GetFitnessRatio() != 0)
		    revert_effect = log10(revert_it.Get()->GetFitnessRatio());
		  else revert_effect = -9.9;
		}	   
	      }

	      out << setw(20) << setprecision(4) << scientific << revert_effect;	    	    
	      out_count++;
	      //marginalEffect.Push(revert_effect);

	      //advance to next step
	      //curr_it.Next();
	      //revert_it.Next();
	      //optimal_it.Next();
	      
	    }while(!curr_it.AtEnd() && !revert_it.AtEnd());

	    //line up all the rows
	    if(out_count < numRows)
	      for(int i = out_count; i < numRows; i++)
		out << setw(20) << setprecision(4) << scientific << -DBL_MAX;
	    out << endl;
	    
	    for(int i = 0; i < optimal_effect.GetSize(); i++){
	      out << setw(20) << setprecision(4) << scientific << optimal_effect[i];
	    }
	    out << setw(20) << setprecision(4) << scientific << -DBL_MAX
		<< setw(20) << setprecision(4) << scientific << -DBL_MAX
		<< endl;
	    
	    //get us ready for the next pass
	    optimal_effect.Resize(0);
	    reset_private();
	    //cin.ignore();
	  }while(!all_muts_it.AtEnd());

	  all_muts.EraseList();
	  reverted_mutation.EraseList();

	  for(int i=0; i < numRows; i++)
	    out << setw(20) << setprecision(4) << scientific << -DBL_MAX;
	  out << endl;
	}
      }
    }

  }


};


class  cActionMarginalMutationEffectMatrix3 : public cActionAnalyzeSingleMutation
{
protected:

  tList<cAnalyzeGenotype>  control_sub_lineage;

  virtual void find_all_mutations(tList<tListIterator<cAnalyzeGenotype> > & all_muts, const int & mut_pos){
    tListIterator<cAnalyzeGenotype> batch_it(m_world->GetAnalyze().GetCurrentBatch().List());

    //advance to ancestral genome -- first one in the batch
    batch_it.Next();

    do{
      //get the next genome
      batch_it.Next();
      if(batch_it.Get()->GetGenome()[mut_pos].GetOp() 
	 != batch_it.PrevData()->GetGenome()[mut_pos].GetOp()){
	all_muts.Push(new tListIterator<cAnalyzeGenotype>(batch_it));
      }
    }while(!batch_it.AtEnd());
  }

public:

  cActionMarginalMutationEffectMatrix3(cWorld* world, const cString& args)
    : cActionAnalyzeSingleMutation(world, args)
  {
    cString largs(args);
    if (largs.GetSize()) m_filename = largs.PopWord();
  }

  virtual void Process(cAvidaContext& ctx)
  {
    //int mut_pos = -1;
    //int numRows = 0;
    cTestCPU *my_testCPU = m_world->GetHardwareManager().CreateTestCPU();
    cDataFile& outfile = m_world->GetDataFile(m_filename);
    ofstream& out = outfile.GetOFStream();

    //tArray<tArray<double> > matrix;
    //tListIterator<tArray<double> > column_it(matrix);
    //tArray<double> *marginalEffect; // represents a temp pointer to one column
    tArray<int> site_finished;

    tArray<double> blackBar;
    blackBar.Resize(m_world->GetAnalyze().GetCurrentBatch().List().GetSize()+2,-DBL_MAX);
    
    int numRows = m_world->GetAnalyze().GetCurrentBatch().List().GetSize()+2;
    int count = 0;
    int listDiff = -1;

    if(ctx.GetAnalyzeMode()){
      tList<tListIterator<cAnalyzeGenotype> > all_muts;
      tList<cAnalyzeGenotype> reverted_mutation;
      tListIterator<cAnalyzeGenotype> batch_it(m_world->GetAnalyze().GetCurrentBatch().List());
      //advance to the first genome
      batch_it.Next();

      site_finished.Resize(batch_it.Get()->GetGenome().GetSize(),0);

      for(unsigned mut_pos = 0; mut_pos < batch_it.Get()->GetGenome().GetSize(); mut_pos++){
	
	site_finished[mut_pos] = 1;

	//find all mutations at a particular site
	find_all_mutations(all_muts, mut_pos);

	if(all_muts.GetSize() > 0){
	  tListIterator<tListIterator<cAnalyzeGenotype> > all_muts_it(all_muts);

	  cerr << "Processing ... pos: " << mut_pos << " with " << all_muts.GetSize() << " mutations ... " << endl;

	  int count = 0;
	  //for each mutation at a site, evaluate it's effects and add a line to the final matrix
	  //for(int mut = 0; mut < all_muts.GetSize(); mut++){
	  do{	  
	    all_muts_it.Next();
	    tArray<double> marginalEffect;
	    //cerr << "Num muts: " << all_muts.GetSize() << " on mut: " << count << endl;
	    
	    //set up the array -- get the last id of a genome with this mutation
	    int break_id = generate_revert_batch(mut_pos, (*all_muts_it.Get()), reverted_mutation);

	    if(reverted_mutation.GetSize() == 1) continue;

	    // is this mutation alive at the end of the run...
	    //	    count++;
	  
	    tListIterator<cAnalyzeGenotype> curr_it(m_world->GetAnalyze().GetCurrentBatch().List());
	    tListIterator<cAnalyzeGenotype> revert_it(reverted_mutation);
	    //marginalEffect = new tArray<double>(); //create a new column
	
	    //advance to the first org
	    curr_it.Next();
	    revert_it.Next();
	  
	    int count = 1;
	    int out_count = 0;

	    double 
	      effect_size = 0.0,
	      revert_effect_size = 0.0, //revert effect_size may now be useless... think about taking it out
	      effect = 0,
	      revert_effect = 0;

	    //	  cerr << "Processing site " << mut_pos << endl;
	  
	    //recalculate stats for all of the genomes -- determine the effect of reversion
	    while(!curr_it.AtEnd() && !revert_it.AtEnd()){
	    
	      count++;

	      //if mutant came back into the pop, or it happens to be the same as the lineage at this point
	      //then the recalc is moot
	      if(revert_it.Get()->GetGenome()[mut_pos].GetOp() ==
		 curr_it.Get()->GetGenome()[mut_pos].GetOp()){
		revert_effect = 10.0;
	      }
	      else{
		//the parent pointer for curr_it doesn't really matter in this case...
		//if(count == 2)
		//curr_it.Get()->Recalculate(ctx,my_testCPU, all_muts_it.Get()->PrevData()); 
		revert_it.Get()->Recalculate(ctx,my_testCPU, curr_it.Get());

		if(curr_it.Get()->GetID() >= all_muts_it.Get()->Get()->GetID()){

		  //mark the beginning and end of the mutated section
		  if((curr_it.Get()->GetID() == all_muts_it.Get()->Get()->GetID()) ||
		     (curr_it.Get()->GetID() == break_id)){
		    out << setw(20) << setprecision(4) << scientific << -DBL_MAX;
		    out_count++;
		  }

		  //post-mutation section -- mutation effect after it leaves the pop
		  if(curr_it.Get()->GetID() >= break_id){
		    if(revert_it.Get()->GetFitnessRatio() != 0)
		      revert_effect = log10(revert_it.Get()->GetFitnessRatio());
		    else revert_effect = -9.9;
		  }
		  else{
		    //reverted secion -- fitness going up here means that the mutation is delterious and so on
		    if(revert_it.Get()->GetFitnessRatio() != 0)
		      revert_effect = -log10(revert_it.Get()->GetFitnessRatio());
		    else revert_effect = 9.9;
		  }
		}
		else{
		  //mutated section -- mutation added prematurally
		  if(revert_it.Get()->GetFitnessRatio() != 0)
		    revert_effect = log10(revert_it.Get()->GetFitnessRatio());
		  else revert_effect = -9.9;
		}	   
	      }

	      out << setw(20) << setprecision(4) << scientific << revert_effect;	    	    
	      out_count++;
	      //marginalEffect.Push(revert_effect);

	      //advance to next step
	      curr_it.Next();
	      revert_it.Next();
	    
	    }

	    //line up all the rows
	    if(out_count < numRows)
	      for(int i = out_count; i < numRows; i++)
		out << setw(20) << setprecision(4) << scientific << -DBL_MAX;
	    out << endl;

	    //get us ready for the next pass
	    reset_private();
	  
	  }while(!all_muts_it.AtEnd());

	  all_muts.EraseList();
	  reverted_mutation.EraseList();

	  for(int i=0; i < numRows; i++)
	    out << setw(20) << setprecision(4) << scientific << -DBL_MAX;
	  out << endl;
	}
      }
    }

  }


};


class cActionFindPercentDeleterious :public cActionAnalyzeSingleMutation 
{
protected:

  int m_totalMutations;
  int m_totalInitalBenifical;
  int m_totalInitalNeutral;
  int m_totalIntialDeleterious;
  int m_totalBenifical;
  int m_totalNeutral;
  int m_totalDeleterious;

  //reset all of the member variables for the next mutation.
  virtual void reset_private(){
    m_totalDesendents = 0;
    m_fitChange = 0.0;
    m_removed = -1;
    m_compensitory = 0;
    m_used.SetAll(0);
    m_usedTasks = 0;
    m_usedTasksFit = 0.0;
    m_genomeFitEffect = 0.0;
    m_mutationFitEffect = 0.0;
    m_totalMutations = 0;
    m_totalInitalBenifical = 0;
    m_totalInitalNeutral = 0;
    m_totalIntialDeleterious = 0;
    m_totalBenifical = 0;
    m_totalNeutral = 0;
    m_totalDeleterious = 0;
    m_genomeFitEffect = 0.0;
    m_mutationFitEffect = 0.0;


  }


public:
  cActionFindPercentDeleterious(cWorld* world, const cString& args)
    : cActionAnalyzeSingleMutation(world, args), 
      //      m_filename("percentDeleteriousDom.dat"),
      m_totalMutations(0),
      m_totalInitalBenifical(0),
      m_totalInitalNeutral(0),
      m_totalIntialDeleterious(0),
      m_totalBenifical(0),
      m_totalNeutral(0),
      m_totalDeleterious(0)
      
  
  {
    cString largs(args);
    if (largs.GetSize()) m_filename = largs.PopWord(); 
  }
   static const cString GetDescription()
   {
     return "For each dominant genome, at the end of a batch, evaluate which of it's instructions were initally deletiorus and which were delterious at some point during the run.  Output is placed in a dat file, one dat file per batch, each dat file contains one line for the dominant organism.\nArguments: [filename='percentDeleteriousDom.dat']";
   }

  virtual void Process(cAvidaContext& ctx)
  {
    int mut_pos = -1;
    cTestCPU *my_testCPU = m_world->GetHardwareManager().CreateTestCPU();
    cDataFile& outfile = m_world->GetDataFile(m_filename);
    outfile.WriteComment("Effects of all mutations resulting in the dominant organism.");

    if(ctx.GetAnalyzeMode()){
      tListIterator<cAnalyzeGenotype> batch_it(m_world->GetAnalyze().GetCurrentBatch().List());

      //advance to the first genome
      batch_it.Next();

      while(batch_it.Next() && !batch_it.AtEnd()){

	tList<cAnalyzeGenotype> curr_mutation, reverted_mutation;
	
	//locate the mutated site
	mut_pos = find_diff_pos(batch_it.GetConst()->GetGenome(), batch_it.PrevData()->GetGenome());
	
	//if there is no mutation, or more than one skip
	if(mut_pos == 0 || mut_pos == -1) continue;
	
	//set up the arrays
	generate_new_batches(mut_pos, batch_it, curr_mutation, reverted_mutation);

	if(curr_mutation.GetSize() == 1) continue;

	//cerr << curr_mutation.GetLast()->GetUpdateDead() << endl;


	// is this mutation alive at the end of the run...
	if(curr_mutation.GetLast()->GetUpdateDead() == -1){

	  tListIterator<cAnalyzeGenotype> curr_it(curr_mutation);
	  tListIterator<cAnalyzeGenotype> revert_it(reverted_mutation);
	
	
	  //advance to the first org
	  curr_it.Next();
	  revert_it.Next();
	  
	  m_totalDesendents = curr_mutation.GetSize();
	  
	  
	  //what was the change in fitness due to this deleterious mutation
	  m_fitChange = revert_it.Get()->GetFitness() - curr_it.Get()->GetFitness();
	  
	  m_used.Resize(curr_it.Get()->GetTaskCounts().GetSize(),0);
	  
	  int count = 1;
	  
	  double 
	    effect_size = 0.0,
	    revert_effect_size = 0.0, //revert effect_size may now be useless... think about taking it out
	    effect = 0,
	    revert_effect = 0;
	  
	  
	  //recalculate stats for all of the genomes -- determine the effect of reversion
	  while(!curr_it.AtEnd() && !revert_it.AtEnd()){
	    
	    count++;
	    
	    curr_it.Get()->Recalculate(ctx,my_testCPU, reverted_mutation.GetFirst()); 
	    revert_it.Get()->Recalculate(ctx,my_testCPU, curr_it.Get());
	    
	    //transform the ratio into a number representing the effect, 0 neutral, negative deleterious, positive benifical
	    if(curr_it.Get()->GetFitnessRatio() != 0)
	      effect = log10(curr_it.Get()->GetFitnessRatio());
	    else {
	      effect = -1000000;
	    }
	    
	    //same transformation, but in reverse, ie fitness going up here means that the mutation is delterious and so on
	    if(revert_it.Get()->GetFitnessRatio() != 0)
	      revert_effect = -log10(revert_it.Get()->GetFitnessRatio());
	    else revert_effect = -1000000;
	    
	    m_genomeFitEffect = classify(effect);
	    m_mutationFitEffect = classify(revert_effect);

	    if(count == 2){
	      if(m_mutationFitEffect > 0) m_totalInitalBenifical++;
	      else if(m_mutationFitEffect == 0) m_totalInitalNeutral++;
	      else m_totalIntialDeleterious++;
	    }
	    else{
	      if(m_mutationFitEffect > 0) m_totalBenifical++;
	      else if(m_mutationFitEffect == 0) m_totalNeutral++;
	      else m_totalDeleterious++;
	    }

	    //compute the ratio of the ancestral fitness to the final fitness of each sub-lineage
	    //note: the first item in the revert sublineage is effectivly the ancestral state	      
	    effect_size = curr_it.Get()->GetFitness() /  reverted_mutation.GetFirst()->GetFitness(); 
	    revert_effect_size = revert_it.Get()->GetFitness() / reverted_mutation.GetFirst()->GetFitness();
	    
	    //advance to next step
	    curr_it.Next();
	    revert_it.Next();
	    
	  }
	  
	  outfile.Write(batch_it.Get()->GetID(),"Genotype ID");
	  outfile.Write(batch_it.Get()->GetParentID(),"Parent ID");
	  outfile.Write(1.0 - (curr_mutation.GetFirst()->GetFitness() / reverted_mutation.GetFirst()->GetFitness()), "% fitness lost");
	  outfile.Write(m_totalDesendents,"Total number of descendents");
	  outfile.Write(effect, "Final genome effect of mutation");
	  outfile.Write(revert_effect, "Final mutation effect of mutation");

	  outfile.Write(m_totalInitalBenifical, "Initially Benifical");
	  outfile.Write(m_totalInitalNeutral, "Initially Neutral");
	  outfile.Write(m_totalIntialDeleterious, "Initially Deleterious");
	  outfile.Write(m_totalBenifical, "Total number of benifical steps");
	  outfile.Write(m_totalNeutral, "Total number of neutral steps");
	  outfile.Write(m_totalDeleterious, "Total number of deletroius steps");

	  outfile.Endl();

	  //cerr << "count: " << count << endl;

	}  
	  
	//get us ready for the next pass
	reset_private();
	
      } // end of big while
      //cin.ignore();
      
      
    }
  }
  
  
};

void RegisterLineageActions(cActionLibrary* action_lib){
  action_lib->Register<cActionAnalyzeSingleMutation>("AnalyzeSingleMutation");
  action_lib->Register<cActionMarginalMutationEffectMatrix0>("MarginalMutationEffectMatrix0");
  action_lib->Register<cActionMarginalMutationEffectMatrix0a>("MarginalMutationEffectMatrix0a");
  //override generate_revert_batch to get this working again
  //action_lib->Register<cActionMarginalMutationEffectMatrix1>("MarginalMutationEffectMatrix1");
  action_lib->Register<cActionMarginalMutationEffectMatrix2>("MarginalMutationEffectMatrix2");
  action_lib->Register<cActionMarginalMutationEffectMatrix2a>("MarginalMutationEffectMatrix2a");
  action_lib->Register<cActionFindPercentDeleterious>("FindPercentDeleterious");
}
