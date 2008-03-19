
#include <stdexcept>
#include <numeric>
#include <iomanip>

#include "../cpu/hardware_method.hh"
#include "fitness_matrix.hh"

#include "config.hh"
#include "inst_lib.hh"

#include "../cpu/test_cpu.hh"


using namespace std;


cFitnessMatrix::cFitnessMatrix( const cGenome & code,  cInstLib * inst_lib)
  :  m_start_genotype(code), m_inst_lib( inst_lib ), m_DFS_MaxDepth(0),
     m_DFS_NumRecorded(0)
{
  m_start_genotype.SetNumInstructions( m_inst_lib->GetSize());
  m_start_genotype.CalcFitness();
  m_data_set.insert(m_start_genotype);
}

cFitnessMatrix::~cFitnessMatrix()
{
}



/*
  A wrapper routine to call the depth limited search with appropriate
  exception catching.
 */

void cFitnessMatrix::CollectData(ofstream& log_file)
{
  try
    {

      log_file << endl << "DOING DEPTH LIMITED SEARCH..." << endl << endl;

      time(&m_search_start_time);
      DepthLimitedSearch(m_start_genotype, log_file);
      time(&m_search_end_time);
    }
  catch(bad_alloc& alloc_failure)
    {
      log_file << "DLS Exception: memory allocation failure" << endl;
    }
  catch(exception& std_ex)
    {
      log_file << "DLS Exception: " << std_ex.what() <<endl;
    }
  catch(...)
    {
      log_file << "DLS Exception: " << "unrecognized exception" << endl;
    }

}


/*
  Statistics collected at each node:
  ----------------------------------

  numNew = number of mutants alive and above threshold that have not
  been seen before

  numVisited = number of mutants that have been seen before (alive and
  above threshold are considered only, because these are the only ones
  stored in the data set)

  numOK = number of mutants alive and above threshold (and new)

  numBelowthresh = number of mutants alive and below threshold (these
  could have been seen before because there is no record of them)

  numDead = number of non-reproducing mutants (these could have been
  seen before because there is no record of dead mutants)

  Consequently, if we sum numOK+numBelowThresh+numDead for all nodes
  then we will get the number of _unique_ genotypes inspected

*/

void cFitnessMatrix::DepthLimitedSearch(const cMxCodeArray& startNode, ofstream& log_file, int currDepth)
{
  if (currDepth == 0)
    {
      m_DFSNumDead.clear();
      m_DFSNumBelowThresh.clear();
      m_DFSNumOK.clear();
      m_DFSNumNew.clear();
      m_DFSNumVisited.clear();
      m_DFSDepth.clear();
    }


  log_file << "cFitnessMatrix::DepthLimitedSearch, depth = " << currDepth << endl;

  currDepth++;
  if (currDepth > m_DFS_MaxDepth) m_DFS_MaxDepth = currDepth;
  m_DFSDepth.push_back(currDepth);

  list<cMxCodeArray> theMutants;

  // statistics
  short numDead = 0, numBelowThresh = 0, numOK = 0;
  short numVisited = 0, numNew = 0;

  // get the 1 mutants of the start node
  startNode.Get1Mutants(theMutants);

  list<cMxCodeArray>::iterator list_iter;
  vector<cMxCodeArray> deleteMutants;
  vector<cMxCodeArray>::iterator vect_iter;

  // MyCodeArrayLessThan myLess;

  for (list_iter = theMutants.begin(); list_iter != theMutants.end(); list_iter++)
    {
      // check if its already in the data set

      set<cMxCodeArray, MyCodeArrayLessThan >::iterator data_iter;
      pair< set<cMxCodeArray, MyCodeArrayLessThan >::iterator , bool> insert_result;

      cMxCodeArray test_array = *list_iter;

      data_iter = m_data_set.find(test_array);

      if (data_iter == m_data_set.end())
	{

	  list_iter->CalcFitness();
	  double fitness = list_iter->GetFitness();
	
	  if (fitness == 0.0)
	    {
	      numDead++;
	      deleteMutants.push_back(*list_iter);
	    }
	  else if (MutantOK(fitness))
	    {
	      insert_result = m_data_set.insert(*list_iter);

	      numNew++;
	      numOK++;
	      m_DFS_NumRecorded++;
	    }
	  else
	    {
	      numBelowThresh++;
	      deleteMutants.push_back(*list_iter);
	    }
	}
      // if its already in the data set delete from the list
      else
	{
	  // log_file << "Mutant already in Set!!!!" << endl;
	  numVisited++;
	  deleteMutants.push_back(*list_iter);
	}
    }

  // delete the ones that are dead or already visited from list
  for (vect_iter = deleteMutants.begin(); vect_iter != deleteMutants.end(); vect_iter++)
    {
      theMutants.remove(*vect_iter);
    }

  // save statistics of this node
  m_DFSNumDead.push_back(numDead);
  m_DFSNumBelowThresh.push_back(numBelowThresh);
  m_DFSNumOK.push_back(numOK);
  m_DFSNumNew.push_back(numNew);
  m_DFSNumVisited.push_back(numVisited);

  // call depth first search on each mutant in the list
  if (currDepth < m_depth_limit)
    {
      for (list_iter = theMutants.begin(); list_iter != theMutants.end(); list_iter++)
	{
	  DepthLimitedSearch(*list_iter, log_file, currDepth);
	}
    }

}


bool cFitnessMatrix::MutantOK(double fitness)
{
  return (fitness > m_fitness_threshhold);
}


/*
  Diagonaliztion via repeated multiplication by a random vector.
  Based on Claus' diagonalize.pl
 */

double cFitnessMatrix::Diagonalize(vector<double>& randomVect, int hamDistThresh, double errorRate, ofstream& log_file)
{
  // first all genotypes need their transition lists
  this->MakeTransitionProbabilities(hamDistThresh, errorRate, log_file);

  log_file << endl << "DIAGONALIZING...  (error prob=" << errorRate << ")" << endl << endl;

  // initialize the random vector
  vector<double> norms;
  MakeRandomVector(randomVect, m_data_set.size());

  vector<double> result;
  // do repeated multiplication
  for (int i = 0; i < m_diag_iters; i++)
    {
      /*
      norms.push_back(VectorNorm(randomVect));
      VectorDivideBy(randomVect, norms.back());
      MatrixVectorMultiply(randomVect, result);
      randomVect = result;
      */

      MatrixVectorMultiply(randomVect, result);
      norms.push_back(VectorNorm(result));
      VectorDivideBy(result, norms.back());
      randomVect = result;

      log_file << "Vect Norm(" << i << "):" << norms.back() << endl;
    }


  return norms.back();
}


/*
   Make transition lists for all the genotypes in the data set.  The
   algorithm for building the transition lists is O(n^2) - for every
   genotype we have to look at all the other genotypes to make its
   transition list.
*/


void cFitnessMatrix::MakeTransitionProbabilities(int hamDistThresh, double errorRate, ofstream& log_file)
{

  set<cMxCodeArray, MyCodeArrayLessThan >::iterator set_iter;
  int num = 0;
  int trans = 0;

  log_file << endl << "MAKING TRANSITION PROBABILITIES... (error prob=" << errorRate << ")" << endl << endl;

  for (set_iter = m_data_set.begin(); set_iter != m_data_set.end(); set_iter++, num++)
    {
      trans = (*set_iter).MakeTransitionList(m_data_set, hamDistThresh, errorRate);
      log_file << num << ") " << trans << " transitions for " << (*set_iter).AsString() << endl;
    }

}


void cFitnessMatrix::MakeRandomVector(vector<double>& newVect, int size)
{
  cRandom randy(time(NULL));

  /* vector should be the size of the set + 1 - because the matrix
     also has the "other" column.  So thats why its '<='
   */
  for (int i = 0; i <= size; i++)
    {
      newVect.push_back(randy.GetDouble());
    }

}

void cFitnessMatrix::VectorDivideBy(vector<double>& vect, double div)
{
  vector<double>::iterator vect_iter;

  for(vect_iter = vect.begin(); vect_iter != vect.end(); vect_iter++)
    {
      *vect_iter /= div;
    }
}

double cFitnessMatrix::VectorNorm(const vector<double> &vect)
{
  double norm = 0.0;
  vector<double>::const_iterator vect_iter;

  for(vect_iter = vect.begin(); vect_iter != vect.end(); vect_iter++)
    {
      norm = norm + (*vect_iter);
    }

  return norm;
}


/*
  Perform the matrix vector multiplication using the sparse matrix
  representation in the 'transition map' of the genotypes.  The
  transition map has entries for every genotype that it has a non-zero
  probability of giving birth to.

  This algorithm is iterating through the columns of the matrix and
  adding each one's contributions to the appropriate rows of the
  resulting vector.  This is sort of the reverse of the normal row by
  row dot product way of thinking about matrix-vector multiplication.

  The true transition matrix has a last column of all zeros - the
  probabilities that a dead genotype will give birth to a live one.
  This is simply ignored because it doesn't effect the calculation
  (and because we're using the actual data set to do the calculation
  there isn't any good way to make a cMxCodeArray to represent dead
  genotypes...)

 */

void cFitnessMatrix::MatrixVectorMultiply(const vector<double> &vect, vector<double>& result)
{

  result.clear();
  result.resize(vect.size(), 0.0);

  set<cMxCodeArray, MyCodeArrayLessThan >::iterator set_iter;

  int column = 0;
  for (set_iter = m_data_set.begin(); set_iter != m_data_set.end(); set_iter++, column++)
    {
      const map<int, double, less<int> > trans_map = (*set_iter).GetTransitionList();

      map<int, double, less<int> >::const_iterator trans_iter;
      for(trans_iter = trans_map.begin(); trans_iter != trans_map.end(); trans_iter++)
	{
	  int row = (*trans_iter).first;
	  double transition_probability = (*trans_iter).second;

	  result[row] += vect[column] * transition_probability;
	}

    }

}

void cFitnessMatrix::PrintGenotypes(ostream &fp)
{
  int totNumDead=0, totNumBelowThresh=0, totNumOK=0;
  int totNumNew=0, totNumVisited=0, totDepth=0;
  int totNumDead2=0, totNumBelowThresh2=0, totNumOK2=0;
  int totNumNew2=0, totNumVisited2=0, totDepth2=0;
  int numNodes;
  float avgNumDead=0.0, avgNumBelowThresh=0.0, avgNumOK=0.0;
  float avgNumNew=0.0, avgNumVisited=0.0, avgDepth=0.0;
  float sdNumDead=0.0, sdNumBelowThresh=0.0, sdNumOK=0.0;
  float sdNumNew=0.0, sdNumVisited=0.0, sdDepth=0.0;

  totNumDead = accumulate(m_DFSNumDead.begin(), m_DFSNumDead.end(), 0);
  totNumBelowThresh = accumulate(m_DFSNumBelowThresh.begin(), m_DFSNumBelowThresh.end(), 0);
  totNumOK = accumulate(m_DFSNumOK.begin(), m_DFSNumOK.end(), 0);
  totNumNew = accumulate(m_DFSNumNew.begin(), m_DFSNumNew.end(), 0);
  totNumVisited = accumulate(m_DFSNumVisited.begin(), m_DFSNumVisited.end(), 0);
  totDepth = accumulate(m_DFSDepth.begin(), m_DFSDepth.end(), 0);

  numNodes = m_DFSNumDead.size();

  avgNumDead = ((float)totNumDead)/((float)numNodes);
  avgNumBelowThresh = ((float)totNumBelowThresh)/((float)numNodes);
  avgNumOK = ((float)totNumOK)/((float)numNodes);
  avgNumNew = ((float)totNumNew)/((float)numNodes);
  avgNumVisited = ((float)totNumVisited)/((float)numNodes);
  avgDepth = ((float)totDepth)/((float)numNodes);

  for (int i = 0; i < numNodes; i++)
    {
      totNumDead2 += m_DFSNumDead[i]*m_DFSNumDead[i];
      totNumBelowThresh2 += m_DFSNumBelowThresh[i]*m_DFSNumBelowThresh[i];
      totNumOK2 += m_DFSNumOK[i]*m_DFSNumOK[i];
      totNumNew2 += m_DFSNumNew[i]*m_DFSNumNew[i];
      totNumVisited2 += m_DFSNumVisited[i]*m_DFSNumVisited[i];
      totDepth2 += m_DFSDepth[i]*m_DFSDepth[i];
    }

  sdNumDead = sqrt( ((float)totNumDead2)/((float)numNodes)
    - (avgNumDead*avgNumDead));
  sdNumBelowThresh = sqrt( ((float) totNumBelowThresh2)/((float) numNodes)
    - (avgNumBelowThresh *avgNumBelowThresh ));
  sdNumOK =sqrt( ((float) totNumOK2)/((float) numNodes)
    - ( avgNumOK* avgNumOK)) ;
  sdNumNew = sqrt( ((float) totNumNew2)/((float) numNodes)
    - (avgNumNew * avgNumNew) );
  sdNumVisited = sqrt( ((float) totNumVisited2)/((float) numNodes)
    - (avgNumVisited * avgNumVisited) );
  sdDepth = sqrt( ((float) totDepth2)/((float) numNodes)
    - (avgDepth * avgDepth) );


  fp << "#######################################################" << endl;
  fp << "# DEPTH FIRST SEARCH RESULTS " << endl;
  fp << "# Depth Limit: " << m_depth_limit << endl;
  fp << "# Fitness Threshold Ratio: " << m_fitness_threshold_ratio  << endl;
  fp << "# Start time: " << asctime(localtime(&m_search_start_time));
  fp << "# End Time: " << asctime(localtime(&m_search_end_time));
  fp << "#######################################################" << endl;
  fp << "# Total number of unique nodes checked: ";
  fp << totNumDead + totNumOK + totNumBelowThresh << endl;
  fp << "# Total number of unique living genotypes found: ";
  fp << totNumOK + totNumBelowThresh << endl;
  fp << "# Total number of unique dead genotypes found: ";
  fp << totNumDead << endl;
  fp << "# Total number of unique genotypes alive and below threshold found: " << endl;
  fp << "# " << totNumBelowThresh << endl;
  fp << "# Total number of unique genotypes alive and above threshold found: " << endl;
  fp << "# " << totNumOK << endl;
  fp << "# Number of Nodes Fully Explored (non-leaf, alive and above threshold): " << endl;
  fp << "# " << numNodes << endl;
  fp << "# Maximum Depth Visited: ";
  fp << m_DFS_MaxDepth << endl;
  fp << "# Number of 1-Mutants Dead at each node, avg - stand dev" << endl;
  fp << "# " << avgNumDead << "\t\t" << sdNumDead << endl;
  fp << "# Number of 1-Mutants Alive but Below Threshold at each node,  avg - stand dev" << endl;
  fp << "# " << avgNumBelowThresh << "\t\t" << sdNumBelowThresh << endl;
  fp << "# Number of 1-Mutants  alive and above threshold at each node,  avg - stand dev" << endl;
  fp << "# " << avgNumOK << "\t\t" << sdNumOK << endl;
  fp << "# Number of New living, above threshold, 1-Mutants at each Node, avg - stand dev" << endl;
  fp << "# " << avgNumNew << "\t\t" << sdNumNew << endl;
  fp << "# Number of Already found living, above threshold 1-Mutants at each Node, avg - stand dev" << endl;
  fp << "# " << avgNumVisited << "\t\t" << sdNumVisited << endl;
  fp << "# Depth of Nodes in search, avg - stand dev" << endl;
  fp << "# " << avgDepth << "\t\t" << sdDepth << endl;
  fp << "#######################################################" << endl;
  fp << "# Fitness:" << endl;

  set<cMxCodeArray, MyCodeArrayLessThan >::iterator iter;
  for(iter = m_data_set.begin(); iter != m_data_set.end(); iter++)
    {
      fp << (*iter).AsString() << "\t" <<(*iter).GetFitness() << "\t" << (*iter).GetMerit() << "\t" << (*iter).GetGestationTime() << endl;
    }

  fp << "#######################################################" << endl;

}

/*
  PrintTransitionList gets m_data_set.size()+1 because there is the
  extra row at the end for the "other" category.  We also add an extra
  row of zeros for the "other" (dead) genotypes so that we're actually
  writing out a square matrix that can be processed by other programs
  (Mathematica).

  Note that this is actually printing the transpose of the transition
  matrix, assuming the matrix rows represent the offspring genotypes
  and the columns represent the parents and that the norm vector is a
  column vector.  (as discussed with Claus in March, 2001).  But since
  we're printing one parent genotype at a time there is just no
  sensible way to print a column at a time...

  Used for testing the diagonalization - this isn't really useful for
  real experiments because the matrix will be thousands by thousands
  big and printing it out is not feasible.

 */

void cFitnessMatrix::PrintTransitionMatrix(ostream &fp, int hamDistThresh, double errorRate, double avg_fitness, bool printMatrix)
{

  fp << endl << endl;
  fp << "#######################################################" << endl;
  fp << "TRANSITION PROBABILITIES" << endl;
  fp << "#Hamming Distance Threshold" << endl;
  fp << "# " << hamDistThresh << endl;
  fp << "#Error Rate" << endl;
  fp << "# " << errorRate << endl;
  fp << "#Avg Fitness" << endl;
  fp << "# " << avg_fitness << endl;
  fp << "#######################################################" << endl;


  if (printMatrix)
    {
      set<cMxCodeArray, MyCodeArrayLessThan >::iterator iter;
      fp << "{" << endl;
      int index = 0;
      for(iter = m_data_set.begin(); iter != m_data_set.end(); iter++, index++)
	{
	  (*iter).PrintTransitionList(fp, m_data_set.size()+1);
	  fp << "," << endl;
	}


      /*
	Add the transition probabilities for "other" (dead) genotypes
	so that we're actually writing out a square matrix that can be
	processed by other programs (Mathematica)
      */

      fp << "{" ;
      for (index = 0; index <= (int) m_data_set.size(); index++)
	{
	  fp << 0.0;
	  if (index < (int) m_data_set.size())
	    fp << " , ";
	}
      fp << "}" << endl;

      fp << "}";
    }

}



void cFitnessMatrix::PrintHammingVector(ostream& fp,const vector<double>& dataVect, double errProb, double avgFit)
{
  vector<double> hamVect;

  for (int i = 0; i < m_start_genotype.GetSize(); i++)
    hamVect.push_back(0.0);

  set<cMxCodeArray, MyCodeArrayLessThan >::iterator data_iter;

  int index = 0;

  for (data_iter = m_data_set.begin(); data_iter != m_data_set.end(); data_iter++, index++)
    {
      int dist = m_start_genotype.HammingDistance(*data_iter);
      hamVect[dist] += dataVect[index];
    }



  fp << errProb << "\t";
  fp << avgFit << "\t";

  vector<double>::const_iterator vect_iter;

  for(vect_iter = hamVect.begin(); vect_iter != hamVect.end(); vect_iter++)
    {
      fp << (*vect_iter) << "\t";
    }

  fp << endl;

}


void cFitnessMatrix::PrintFitnessVector(ostream& fp,const vector<double>& dataVect, double errProb, double avgFit, double maxFit, double step)
{
  vector<double> fitVect;

  int maxi = (int) (maxFit /step) +1;

  for (int i = 0; i < maxi; i++)
    fitVect.push_back(0.0);

  set<cMxCodeArray, MyCodeArrayLessThan >::iterator data_iter;

  int index = 0;

  for (data_iter = m_data_set.begin(); data_iter != m_data_set.end(); data_iter++, index++)
    {
      double f = (*data_iter).GetFitness();
      fitVect[(int) (f/step)] += dataVect[index];
    }
  // the last contribution is of fitness zero:
  fitVect[0] += dataVect[index];



  fp << errProb << "\t";
  fp << avgFit << "\t";

  vector<double>::const_iterator vect_iter;

  for(vect_iter = fitVect.begin(); vect_iter != fitVect.end(); vect_iter++)
    {
      fp << (*vect_iter) << "\t";
    }

  fp << endl;

}

void cFitnessMatrix::PrintFullVector(ostream& fp,const vector<double>& dataVect, double errProb, double avgFit)
{
  fp << errProb << "\t";
  fp << avgFit << "\t";

  vector<double>::const_iterator vect_iter;

  for(vect_iter = dataVect.begin(); vect_iter != dataVect.end(); vect_iter++)
    {
      fp << (*vect_iter) << "\t";
    }

  fp << endl;
}



void cFitnessMatrix::CalcFitnessMatrix( int depth_limit, double fitness_threshold_ratio, int ham_thresh, double error_rate_min, double error_rate_max, double error_rate_step, double output_start, double output_step, int diag_iters, bool write_ham_vector, bool write_full_vector )
{

  /* set parameters (TODO: read these from event list) */

  m_depth_limit = depth_limit;
  m_fitness_threshold_ratio = fitness_threshold_ratio;
  m_ham_thresh = ham_thresh;
  m_error_rate_min = error_rate_min;
  m_error_rate_max = error_rate_max;
  m_error_rate_step = error_rate_step;
  m_diag_iters = diag_iters;

  m_fitness_threshhold = m_start_genotype.GetFitness() * m_fitness_threshold_ratio;

  /* open files for output */

  ofstream log_file("fitness_matrix.log");
  ofstream genotype_file("found_genotypes.dat");
  ofstream fit_vect_file("fitness_vect.dat");

  ofstream ham_vect_file;
  if (write_ham_vector)
    ham_vect_file.open("hamming_vect.dat");
  ofstream full_vect_file;
  if (write_full_vector)
    full_vect_file.open("full_vect.dat");


  /* do the depth first search */

  CollectData(log_file);
  PrintGenotypes(genotype_file);
  genotype_file.close();


  /* diagonalize transition matrices at different copy error rates */

  for (double error = m_error_rate_min; error <= m_error_rate_max; error += m_error_rate_step)
    {
      vector<double> dataVect;

      double avg_fitness = Diagonalize(dataVect, m_ham_thresh, error, log_file);

      PrintFitnessVector(fit_vect_file, dataVect, error, avg_fitness, output_start, output_step);

      if ( write_ham_vector )
	PrintHammingVector(ham_vect_file, dataVect, error, avg_fitness);

      if ( write_full_vector )
	PrintFullVector(full_vect_file, dataVect, error, avg_fitness);

    }


  /* close remaining files */

  log_file.close();
  fit_vect_file.close();
  if ( write_ham_vector )
    ham_vect_file.close();
  if ( write_full_vector )
    full_vect_file.close();
}





//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//  cMxCodeArray
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

const int cMxCodeArray::m_max_gestation_time = 1000000000;


cMxCodeArray::cMxCodeArray()
{
  // Initialize the code array as empty.
  size = 0;
  max_size = 0;
  data = NULL;
  m_merit = 0;
  m_gestation_time = m_max_gestation_time;
  m_num_instructions = 0;
}

cMxCodeArray::cMxCodeArray(int n_inst, int in_size, int in_max_size) :m_merit(0), m_gestation_time(m_max_gestation_time), m_num_instructions(n_inst)
{  
  assert (in_max_size == 0 || in_max_size >= in_size);

  // Initialize the code array randomly.
  size = in_size;
  if (in_max_size != 0) max_size = in_max_size;
  else max_size = size;

  data = new cInstruction[max_size];

  for (int i = 0; i < size; i++)
    {
      data[i].SetOp(g_random.GetUInt(cConfig::GetNumInstructions()));
    }

}

cMxCodeArray::cMxCodeArray(const cMxCodeArray &in_code)
{

  size = in_code.size;
  max_size = in_code.max_size;
  m_merit = in_code.m_merit;
  m_gestation_time = in_code.m_gestation_time;
  m_num_instructions = in_code.m_num_instructions;
  m_trans_probs = in_code.m_trans_probs;

  data = new cInstruction[max_size];

  for (int i = 0; i < size; i++)
    {
      data[i] = in_code.data[i];
    }

}


cMxCodeArray::cMxCodeArray(const cGenome &in_code, int in_max_size) :m_merit(0), m_gestation_time(m_max_gestation_time), m_num_instructions(0)
{
  assert (in_max_size == 0 || in_max_size >= in_code.GetSize());

  size = in_code.GetSize();
  if (in_max_size != 0) max_size = in_max_size;
  else max_size = size;

  data = new cInstruction[max_size];

  for (int i = 0; i < size; i++) {
    data[i] = in_code[i];
  }
}


cMxCodeArray::~cMxCodeArray()
{
  // TODO - what is this?
  // g_memory.Remove(C_CODE_ARRAY);

  if (data) delete [] data;
}

void cMxCodeArray::Resize(int new_size)
{
  assert (new_size <= MAX_CREATURE_SIZE);

  // If we are not changing the size, just return.
  if (new_size == size) return;

  // Re-construct the arrays only if we have to...
  if (new_size > max_size || new_size * 4 < max_size) {
    cInstruction * new_data = new cInstruction[new_size];

    for (int i = 0; i < size && i < new_size; i++) {
      new_data[i] = data[i];
    }

    if (data) delete [] data;

    data = new_data;
    max_size = new_size;
  }

  // Fill in the un-filled-in bits...
  for (int i = size; i < new_size; i++) {
    data[i].SetOp(g_random.GetUInt(cConfig::GetNumInstructions()));
  }

  size = new_size;
}

// Resize the arrays, and leave memeory uninitializes! (private method)
void cMxCodeArray::ResetSize(int new_size, int new_max)
{
  assert (new_size >= 0);
  assert (new_max == 0 || new_max >= new_size);

  // If we are clearing it totally, do so.
  if (!new_size) {
    if (data) delete [] data;
    data = NULL;
    size = 0;
    max_size = 0;
    return;
  }

  // If a new max is _not_ being set, see if one needs to be.
  if (new_max == 0 &&
      (max_size < new_size) || (max_size > new_size * 2)) {
    new_max = new_size;
  }

  // If the arrays need to be rebuilt, do so.
  if (new_max != 0 && new_max != max_size) {
    cInstruction * new_data = new cInstruction[new_max];

    if (data) delete [] data;
    data = new_data;
    max_size = new_max;
  }

  // Reset the array size.
  size = new_size;
}

/**
 * Resizes and reinitializes the genome. The previous data is lost.
 **/
void cMxCodeArray::Reset()
{
  // Initialze the array
  if( cConfig::GetAllocMethod() == ALLOC_METHOD_RANDOM ){
    // Randomize the initial contents of the new array.
    Randomize();
  }else{
    // Assume default instuction initialization
    Clear();
  }
}

/**
 * Sets all instructions to @ref cInstruction::GetInstDefault().
 **/
void cMxCodeArray::Clear()
{
  int i;
  for (i = 0; i < size; i++)
    {
      data[i] = cInstLib::GetInstDefault();
    }
}

/**
 * Replaces all instructions in the genome with a sequence of random
 * instructions.
 **/

void cMxCodeArray::Randomize()
{
  int i;
  for (i = 0; i < size; i++)
    {
      data[i].SetOp(g_random.GetUInt(cConfig::GetNumInstructions()));
    }
}

void cMxCodeArray::operator=(const cMxCodeArray & other_code)
{
  // If we need to resize, do so...

  ResetSize(other_code.size);

  // Now that both code arrays are the same size, copy the other one over.

  for (int i = 0; i < size; i++)
    {
      data[i] = other_code.data[i];
    }

    m_merit = other_code.m_merit;
    m_gestation_time = other_code.m_gestation_time;
    m_num_instructions = other_code.m_num_instructions;
    m_trans_probs = other_code.m_trans_probs;
}


/*
 * Let the 0th  instruction be the least significant,
 * so longer codes are always greater.
 */
bool cMxCodeArray::operator<(const cMxCodeArray &other_code) const
{

  if (size < other_code.size)
    return true;
  else if (size > other_code.size)
    return false;

  bool result = false;

  for (int i = size-1; i >= 0; i-- )
    {
      if (data[i].GetOp() < other_code.data[i].GetOp())
	{
	  result = true;
	  break;
	}
      else if (other_code.data[i].GetOp() < data[i].GetOp())
	{
	  result = false;
	  break;
	}
    }

  return result;
}


void cMxCodeArray::CopyDataTo(cGenome & other_code) const
{
  // @CAO this is ugly, but the only option I see...
  other_code = cGenome(size);

  for (int i = 0; i < size; i++) {
    other_code[i] = data[i];
  }
}

bool cMxCodeArray::OK() const
{
  bool result = true;

  assert (size >= 0 && size <= max_size && max_size <= MAX_CREATURE_SIZE);

  return result;
}


int cMxCodeArray::FindInstruction(const cInstruction & inst, int start_index)
{
  assert(start_index < GetSize());
  for(int i=0; i<GetSize(); ++i ){
    if( Get(i) == inst ){
      return i;
    }
  }
  return -1;
}

cString cMxCodeArray::AsString() const
{
  cString out_string(size);
  for (int i = 0; i < size; i++) {
    int cur_char = data[i].GetOp();
    if (cur_char < 26) {
      out_string[i] = cur_char + 'a';
    } else if (cur_char < 52) {
      out_string[i] = cur_char - 26 + 'A';
    } else if (cur_char < 62) {
      out_string[i] = cur_char - 52 + '1';
    } else {
      out_string[i] = '*';
    }
  }

  return out_string;
}


void cMxCodeArray::Get1Mutants(list<cMxCodeArray>& mutants) const
{
  for (int line_num = 0; line_num < size; line_num++)
    {
      int cur_inst = data[line_num].GetOp();

      for (int inst_num = 0; inst_num < m_num_instructions; inst_num++)
	{
	  if (cur_inst == inst_num) continue;

	  cMxCodeArray newMutant(*this);
	  newMutant[line_num].SetOp(inst_num);
	  mutants.push_back(newMutant);
	}
    }
}


/*
  Make a list of the transition probabilities only to those other
  genotypes that are within the given hamming distance of this
  genotype.

  The list is actually implemented as a map from indexes to
  probabilities, where the index is the place of the genotype in
  the complete ordered set of genotypes.

  TODO this seems kind of inefficient that you have to calculate the
  hamming distance twice - maybe I should pass it as a parameter to
  the TransitionProbability function after it is first calculated?  Or
  maybe the threshold should be in the transition probability?
*/

  /* dropped const for win32 -- kgn */
//int cMxCodeArray::MakeTransitionList(const set<cMxCodeArray, MyCodeArrayLessThan > &data_set, int hamDistThresh, double errorRate) const
int cMxCodeArray::MakeTransitionList(set<cMxCodeArray, MyCodeArrayLessThan > &data_set, int hamDistThresh, double errorRate) const
{
  set<cMxCodeArray, MyCodeArrayLessThan >::iterator set_iter;
  int index = 0;
  double totalProb = 0.0;
  int num = 0;

  for (set_iter = data_set.begin(); set_iter != data_set.end(); set_iter++, index++)
    {
      if (HammingDistance(*set_iter) <= hamDistThresh)
	{
	  double transProb =  TransitionProbability(*set_iter, errorRate);
	  m_trans_probs[index] = transProb * GetFitness();
	  totalProb += transProb;
	  num += 1;
	}
      else
	{
	  m_trans_probs[index]= 0.0;
	}
    }

  /* The transition probability for "other"
     index got incremented at the very end of the loop so its pointing
     to the next element now */

  m_trans_probs[index] = (1.0 - totalProb) * GetFitness();

  return num;

}


const map<int, double, less<int> >& cMxCodeArray::GetTransitionList() const
{
  return m_trans_probs;
}


int cMxCodeArray::HammingDistance(const cMxCodeArray &other_gene) const
{
  int distance = 0;

  assert (size == other_gene.GetSize());

  for (int i = 0; i < size; i++)
    {
      if (this->Get(i) != other_gene.Get(i))
	distance++;
    }

  return distance;
}


double cMxCodeArray::TransitionProbability(const cMxCodeArray &other_gene, double errorRate) const
{
  double Q = 0.0;
  assert (size == other_gene.GetSize());

  if ( (*this) == other_gene)
    {
      Q = pow((1-errorRate), size);
    }
  else
    {
      int distance = HammingDistance(other_gene);
      Q = pow((1.0-errorRate), (size -distance)) * pow((errorRate/m_num_instructions),distance);
    }

  return Q;
}

/*
  Used in testing the diagonalization.
*/

void cMxCodeArray::PrintTransitionList(ostream &fp, int size) const
{

  fp.setf(ios::fixed);

  fp << "{" ;

  for (int index=0 ; index < size; index++)
    {
      map<int, double, less<int> >::const_iterator find_results = m_trans_probs.find(index);
      if (find_results == m_trans_probs.end())
	{
	  fp << 0.0 ;
	}
      else
	{
	  fp << setprecision(10) << (*find_results).second ;
	}

      if (index < size-1)
	fp << " , " ;

    }

  fp << "}" ;
}


void cMxCodeArray::CalcFitness()
{
  cGenome temp(1);
  cCPUTestInfo test_info;
  CopyDataTo(temp); 
  cTestCPU::TestGenome(test_info, temp);
  if ( test_info.IsViable() )
    m_gestation_time =
      test_info.GetTestOrganism()->GetPhenotype().GetGestationTime();
  else // if not viable, set a really high gestation time
    m_gestation_time = m_max_gestation_time;
  m_merit = test_info.GetTestOrganism()->GetPhenotype().GetMerit().GetDouble();
}
