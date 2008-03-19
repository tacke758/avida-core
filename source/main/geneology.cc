//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

/* geneology.cc ***************************************************************
 Geneology recording classes

 charles@krl.caltech.edu & travc@ugcs.caltech.edu
 Time-stamp: <1999-03-09 18:48:48 travc>

 Copyright (C) 1993 - 1996 California Institute of Technology
 Read the LICENSE and README files, or contact 'charles@krl.caltech.edu',
 before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.
******************************************************************************/
#include "../defs.hh"

#ifdef GENEOLOGY

#include <limits.h>
#include "geneology.hh"
#include "genebank.hh"
#include "population.hh"
#include "species.hh"


// cGTElement -----------------------------------------------------------------

cGTElement::cGTElement(cGenotype * _genotype) :
 genotype(_genotype),
 flag(FALSE),
 depth(_genotype->GetDepth()),
 birth_update(_genotype->GetUpdateBorn()),
 death_update(-1),
 fitness(0),
 parent(_genotype->GetParentID()),
 daughters(NULL),
 daughter_it(NULL),
 breed_true(0),
 num_daughter_gt(0),
 num_dead_daughter_gt(0),
 num_dead_daughter_tree(0),
 sub_tree_depth(0),
 breed_daughters(0),
 d_fit_sum(0),
 d_fit_sum_square(0),
 worse_fit_daughters(0),
 same_fit_daughters(0),
 better_fit_daughters(0) {
   for( int i=0; i<GT_NUM_DECEND; i++ ) decend[i] = 0;
}


cGTElement::~cGTElement(){
  delete daughter_it;
  delete daughters;
}

void cGTElement::AddDaughter(int _id){
  if( daughters == NULL ){
    daughters = new tKeyList<int,int>;
    daughter_it = new tKeyListIterator<int,int>(daughters);
  }
  if( daughter_it->Seek(_id) ){
    daughter_it->CurrentData()++;
  }else{
    daughters->Add(_id,1);
    ++num_daughter_gt;
  }
}

Boolean cGTElement::RemoveDaughter(int _id){
  assert( daughters != NULL );
  if( daughter_it->Seek(_id) ){
    daughter_it->Remove();
    return TRUE;
  }
  return FALSE;
}

void cGTElement::Die(){  // called when genotype goes extinct
#ifdef GENEOLOGY_DELETES_GENOTYPES
  delete genotype;
#endif
  fitness = genotype->GetFitness();
  species = genotype->GetSpecies()->GetID();
  parent_species = genotype->GetSpecies()->GetParentID();
  parent_distance = genotype->GetParentDistance();
  genotype = NULL;
  death_update = cStats::GetUpdate();
}

Boolean cGTElement::KillDaughter(int _id){
  if( HasDaughters() && daughter_it->Seek(_id) ){
    daughter_it->Remove();
    ++num_dead_daughter_gt;  // Inc number of dead daugthers
    ++decend[0];             // This is level 0 decendant, so mark that too
    // If this guy's sub-tree depth is 0, well it had a daugthter so mark 1
    if( sub_tree_depth < 1 )
      sub_tree_depth = 1;
    return TRUE;
  }
  return FALSE;
}


void cGTElement::Print(ostream & out){
  out<<"\tParent:   "<<parent<<endl;
  out<<"\tBreedTrue: "<<breed_true<<endl;
  out<<"\tDaughters: ";
  if( daughters == NULL ){
    out<<"<none>"<<endl;
  }else{
    for( daughter_it->Reset(); daughter_it->Good(); daughter_it->Next() )
      out<<daughter_it->CurrentKey()<<"("<<daughter_it->CurrentData()<<")\t";
    out<<endl;
  }
}


// cGeneology -----------------------------------------------------------------

Boolean cGeneology::AddNode(cGenotype * _genotype){
  return hash.Add(_genotype->GetID(), new cGTElement(_genotype));
}

Boolean cGeneology::Remove(int _id){
  if(hash_it.Seek(_id)){
    Remove(hash_it);
    return TRUE;
  }
  return FALSE;
}

void cGeneology::Remove(HashIterator & it){
  assert( it.CurrentData() != NULL );
  LogDeath(it);
  delete it.CurrentData();
  it.Remove();
}


void cGeneology::LogDeath(HashIterator & it){
  assert( it.CurrentData()->num_daughter_gt == it.CurrentData()->num_dead_daughter_gt );
  assert( it.CurrentData()->num_daughter_gt == it.CurrentData()->decend[0] );
  ostream & out =  cStats::GetGeneologyLog();

  // Need parent iterator for "parent fitness"
  // If can't get it, use this geneotype (assume same fitness)
  HashIterator pit(&hash);  // points to parent (if no parent, points to self)
  if( it.CurrentData()->parent > 0 ){
    if( !pit.Seek(it.CurrentData()->parent) ){
      pit.Seek(it.CurrentKey());
    }
  }else{
    pit.Seek(it.CurrentKey());
  }

  if( out.good() ){
    out <<cStats::GetUpdate()<<" "                      // 1 update
	<<it.CurrentKey()<<" "                          // 2 id
	<<it.CurrentData()->parent<<" "                 // 3 parent_id
	<<it.CurrentData()->parent_distance<<" "        // 4 parent_distance
	<<it.CurrentData()->species<<" "                // 5 species
	<<it.CurrentData()->parent_species<<" "         // 6 parent_species
	<<it.CurrentData()->birth_update<<" "           // 7 update born
	<<it.CurrentData()->death_update<<" "           // 8 update extinct
	<<it.CurrentData()->GetFitness()<<" "           // 9 fitness
	<<it.CurrentData()->depth<<" "                 // 10 depth
	<<it.CurrentData()->sub_tree_depth<<" "        // 11 sub-tree depth
	<<it.CurrentData()->num_dead_daughter_tree<<" "// 12 # sub-trees
	<<it.CurrentData()->breed_true<<" "            // 13 # breed true
	<<pit.CurrentData()->GetFitness()<<" "         // 14 parent fitness
	<<it.CurrentData()->breed_daughters<<" "       // 15 breeding daughters
	<<it.CurrentData()->worse_fit_daughters<<" "   // 16 d_fit < p_fit
	<<it.CurrentData()->same_fit_daughters<<" "    // 17 d_fit = p_fit
	<<it.CurrentData()->better_fit_daughters<<" "  // 18 d_fit > p_fit
	<<it.CurrentData()->d_fit_sum<<" "             // 19 daughter fit sum
	<<it.CurrentData()->d_fit_sum_square<<" ";     // 20 d fit sum sq
    for( int i=0; i<GT_NUM_DECEND; ++i ){
      out<<it.CurrentData()->decend[i]<<" ";           // 21+ decendant counts
    }
    out<<endl;
  }
}


void cGeneology::LogLeafDeath(cGenotype * _genotype, HashIterator & pit){
  ostream & out =  cStats::GetGeneologyLog();
  // out<<" LEAF LOGGING IS DEPRECATED "<<endl;
  if( out.good() ){
    out<<cStats::GetUpdate()<<" "        // 1 update
       <<_genotype->GetID()<<" "         // 2 id
       <<pit.CurrentKey()<<" "           // 3 parent_id
       <<_genotype->GetUpdateBorn()<<" " // 4 update born (Unknown)
       <<cStats::GetUpdate()<<" "        // 5 update extinct (Now)
       <<"0 "                            // 6 fitness
       <<pit.CurrentData()->depth+1<<" " // 7 depth
       <<"0 "                            // 8 sub-tree depth (0)
       <<"0 "                            // 9 # sub-trees (0)
       <<"0 "                            // 10 # breed true (0)
       <<pit.CurrentData()->GetFitness()<<" " // 11 parent fitness
       <<"0 "                            // 12 breeding daughters
       <<"0 "                            // 13 d_fit < p_fit (0)
       <<"0 "                            // 14 d_fit = p_fit (0)
       <<"0 "                            // 15 d_fit > p_fit (0)
       <<"0 "                            // 16 daughter fit sum (0)
       <<"0 ";                           // 17 d fit sum sq (0)
    for( int i=0; i<GT_NUM_DECEND; ++i ){
      out<<"0 ";                         // 18+ decendant counts (all 0)
    }
    out<<endl;
    // out<<" LEAF LOGGING IS DEPRECATED "<<endl;
  }
}


Boolean cGeneology::AddDaughter(int _parent_id, int _daughter_id){
  if( hash_it.Seek(_parent_id) ){
    hash_it.CurrentData()->AddDaughter(_daughter_id);
    return TRUE;
  }
  return FALSE;
}


// Big functions

void cGeneology::AddBirth(cGenotype * _parent, cGenotype * _daughter){
  if( _daughter->GetParentID() != -1 ){ // if not birth of injected

    // if it is a mismatch, then log? @TCC
    if( _parent->GetID()!=_daughter->GetParentID()
	&& _parent->GetID()!=_daughter->GetID() ){
      // @TCC  MAKE THIS OUTPUT TO A LOG
#ifdef GENEOLOGY_DEBUG
      cerr<<"Birth Mismatch: "<<_daughter->GetID()
	<<" real parent="<<_parent->GetID()
          <<" genotype parent="<<_daughter->GetParentID()<<endl;
#endif
    }else{  // not a mismatch, so go on

      // If Parent is not there, add it.  (Also seek hash_it to parent)
      if( !hash_it.Seek(_daughter->GetParentID()) ){
	AddNode(_parent);
	hash_it.Seek(_daughter->GetParentID());
      }
      // If Parent == Daughter, then inc Breed_True
      if( _parent->GetID() == _daughter->GetID() ){
	hash_it.CurrentData()->IncBreedTrue();
      }else{
	AddDaughter(_parent->GetID(),_daughter->GetID());
      }
    }
  }else{ // Breed True of injected?
    if( _parent->GetID() == _daughter->GetID() ){
      if( !hash_it.Seek(_parent->GetID()) ){
	AddNode(_parent);
	hash_it.Seek(_parent->GetID());
      }
      hash_it.CurrentData()->IncBreedTrue();
    }
  }
}


void cGeneology::GenotypeDeath(cGenotype * _genotype){
  HashIterator it(&hash);  // iterator pointing to dead node
  // HashIterator pit(&hash); // iterator pointing to parent of dead node
  assert(_genotype->GetNumCPUs()==0 && "GenotypeDeath on Alive Genotype");

  if( it.Seek(_genotype->GetID()) ){ // if it is the hashtable
    it.CurrentData()->Die(); // Mark as dead (can't use genotype anymore)

    // We know that it isn't alive since this is being called...
    if( !it.CurrentData()->HasDaughters() ){
      HashIterator pit(&hash);
      if( it.CurrentData()->parent>=0 && pit.Seek(it.CurrentData()->parent) ){
	// Notify Parent
	if( pit.CurrentData()->KillDaughter(it.CurrentKey()) ){
	  RecordDecendants(pit, it);
	  ParentTrim(pit.CurrentKey()); // Recurse to Parent
	}else{
	  cerr<<"Child "<<it.CurrentKey()<<" Not in Parent's ("
	    <<pit.CurrentKey()<<") Daughter List"<<endl;
	}
	Remove(it.CurrentKey());
      }else{
	assert( "Parent is undefined!!?!" );
      }
    }
  }else{ // it isn't in the hash table

    if( _genotype->GetParentID()>=0 && it.Seek(_genotype->GetParentID()) ){
      if( it.CurrentData()->KillDaughter(_genotype->GetID()) ){
	if( cConfig::GetLogGeneologyLeaves() )
	  LogLeafDeath(_genotype, it);
	ParentTrim(it.CurrentKey()); // Recurse to Parent
      }
    }else{
      assert( "Removing ID from Non-Existant Parent!!!" );
    }
  }
}


void cGeneology::ParentTrim(int _id){
  // Should a parent be killed?  Called when a child genotype dies
  HashIterator it(&hash);
  if( it.Seek(_id) ){
    if( !it.CurrentData()->IsAlive() && !it.CurrentData()->HasDaughters() ){
      HashIterator pit(&hash); // parent iterator
      if( it.CurrentData()->parent>=0 && pit.Seek(it.CurrentData()->parent) ){
	if( pit.CurrentData()->KillDaughter(_id) ){
	  RecordDecendants(pit, it);
	  ParentTrim(pit.CurrentKey()); // Recurse to Parent
	}
	Remove(it.CurrentKey());
      }else{
	if( it.CurrentData()->parent>=0 )
	  cerr<<"CANNOT FIND PARENT "<<it.CurrentData()->parent
	    <<" TRYING TO DELETE "<<it.CurrentKey()<<endl;
	Remove(it.CurrentKey());
      }
    }
  }else{
    cerr<<_id<<" NOT FOUND by ParentTrim"<<endl;
  }
}


void cGeneology::RecordDecendants(HashIterator & pit, HashIterator & dit){
  int i;
  // if this daughter is a node
  if( dit.CurrentData() != NULL ){
    // Merge it's decendant counts with the parents
    for( i=0; i<GT_NUM_DECEND-1; i++ ){
      pit.CurrentData()->decend[i+1] += dit.CurrentData()->decend[i];
    }

    // if daughter had daughters (it is a sub tree) then mark that in parent
    if( dit.CurrentData()->num_daughter_gt>0 )
      pit.CurrentData()->num_dead_daughter_tree++;

    // Sub Tree Depth
    if( pit.CurrentData()->sub_tree_depth<dit.CurrentData()->sub_tree_depth+1 )
      pit.CurrentData()->sub_tree_depth = dit.CurrentData()->sub_tree_depth+1;

    // Update the Sum and Sum of Squares of Daughter genotpyes
    pit.CurrentData()->d_fit_sum += dit.CurrentData()->GetFitness();
    pit.CurrentData()->d_fit_sum_square += dit.CurrentData()->GetFitness()
                                         * dit.CurrentData()->GetFitness();

    // Update the counts of daughter genotypes (breeding, better, worse, same)
    pit.CurrentData()->breed_daughters++;

    // fx*(1-e) <= fy
    if( pit.CurrentData()->GetFitness() * (1-GT_SAME_FIT_EPSILON)
	<= dit.CurrentData()->GetFitness() ){
      // fy <= fx*(1+e)
      if( dit.CurrentData()->GetFitness() <=
	  pit.CurrentData()->GetFitness() * (1+GT_SAME_FIT_EPSILON) ){
	pit.CurrentData()->same_fit_daughters++;
      }else{
	// fy > fx*(1+e)
	pit.CurrentData()->better_fit_daughters++;
      }
    }else{
      // fy < fx*(1-e)
      pit.CurrentData()->worse_fit_daughters++;
    }

  }else{ // Daughter wasn't a node and never gave birth
  }
}


void cGeneology::TrimLeaves(){
  hash_it.Reset();
  while( hash_it.Good() ){
    if( hash_it.CurrentData()->daughters == NULL ){
      //cout<<"Trimming off "<<hash_it()
	//<<"("<<hash_it.CurrentData()->breed_true<<")"<<endl;
      Remove(hash_it.CurrentKey());
    }else{
      hash_it.Next();
    }
  }
}


void cGeneology::ClearFlags(){
  hash_it.Reset();
  while( hash_it.Good() ){
    hash_it.CurrentData()->flag=FALSE;
    hash_it.Next();
  }
}


void cGeneology::Print(ostream & out){
  hash_it.Reset();
  while( hash_it.Good() ){
    out<<hash_it()<<" ";
    hash_it.CurrentData()->Print(out);
    hash_it.Next();
  }
}


void cGeneology::PrintDepthHistogram(){
  static ofstream hist_ofs("depth_hist.out");
  // Put in error handling here!!!!!! @TCC
  PrintDepthHistogram(hist_ofs);
}


void cGeneology::PrintDepthHistogram(ostream & out){
  // Uses genebank to facilitate looking at only living types
  // Two pass method... slow, but who cares, it's IO
  // If we need it to be faster, we can have stats record min/max of depth

  // Figure the size we need for the histogram
  cGenotype * cur_genotype = genebank->GetBestGenotype();  // Best==Front
  int min = cur_genotype->GetDepth();
  int max = cur_genotype->GetDepth();
  int i;
  for( i=0; i < genebank->GetSize(); i++ ){
    if( cur_genotype->GetDepth() < min )
      min = cur_genotype->GetDepth();
    if( cur_genotype->GetDepth() > max )
      max = cur_genotype->GetDepth();
    cur_genotype = cur_genotype->GetNext();
  }
  int size = max-min+1;

  // Now make the actual histogram
  int * creature_depth_hist = new int[size];  // entry for each creature
  int * bt_creature_depth_hist = new int[size];  // only breed_true
  int * genotype_depth_hist = new int[size];  // entry only for genotypes
  int * bt_genotype_depth_hist = new int[size];  // only breed_true
  for( i=0; i < size; i++ ){ // zero out
    creature_depth_hist[i] = 0;
    bt_creature_depth_hist[i] = 0;
    genotype_depth_hist[i] = 0;
    bt_genotype_depth_hist[i] = 0;
  }
  cur_genotype = genebank->GetBestGenotype();  // Best==Front
  for( i=0; i < genebank->GetSize(); i++ ){
    creature_depth_hist[cur_genotype->GetDepth()-min]
      += cur_genotype->GetNumCPUs();
    genotype_depth_hist[cur_genotype->GetDepth()-min] ++;

    // See if it has any breed_true
    if( hash_it.Seek(cur_genotype->GetID())
	&& hash_it.CurrentData()->breed_true>0 ){
      bt_creature_depth_hist[cur_genotype->GetDepth()-min]
	+= cur_genotype->GetNumCPUs();
      bt_genotype_depth_hist[cur_genotype->GetDepth()-min] ++;
    }
    cur_genotype = cur_genotype->GetNext();
  }

  // Output
  for( i=0; i < size; i++ ){
    out<<i+min<<" "
       <<creature_depth_hist[i]<<" "
       <<bt_creature_depth_hist[i]<<" "
       <<genotype_depth_hist[i]<<" "
       <<bt_genotype_depth_hist[i]<<endl;
  }

  out<<endl;

  // Cleanup
  delete [] creature_depth_hist;
  delete [] genotype_depth_hist;
}


void cGeneology::PrintTree(int root, ostream & out){
  ClearFlags();
  PrintTreeRecurse(root,out);
}


void cGeneology::PrintTrimTree(int root, ostream & out){
  ClearFlags();
  PrintTrimTreeRecurse(root,out);
}


void cGeneology::PrintTreeRecurse(int root, ostream & out, int depth){
  HashIterator it(&hash);
  for( int i=0; i<depth; ++i )
    out<<"| ";
  out<<root;
  if( it.Seek(root) ){ // if node
    assert( root!=1 || depth==it.CurrentData()->depth ); // depth should match
    cGTElement * c = it.CurrentData();
    if( c->flag == FALSE ){
      c->flag=TRUE;
      if( c->IsAlive() )
	out<<"<"<<c->genotype->GetNumCPUs()<<">";
      else
	out<<"<0>";
      if( it.Good() && c->daughters != NULL ){ // if daughters
	out<<"("
	   <<c->breed_true<<","
	   <<c->num_daughter_gt<<"-"
	   <<c->num_dead_daughter_gt<<"="
	   <<c->daughters->CalcSize()<<")"
	   <<endl;
	c->daughter_it->Reset();
	while( c->daughter_it->Good() ){
	    if( c->daughter_it->CurrentKey() != root ){
	      PrintTreeRecurse(c->daughter_it->CurrentKey(), out, depth+1);
	    }
	  c->daughter_it->Next();
	}
      }else{ // Node with no daughters
	out<<"(0)"<<endl;
      }
    }else{
      out<<" ALREADY VISITED? "<<endl;
    }
  }else{
    out<<endl;
  }
}


void cGeneology::PrintTrimTreeRecurse(int root, ostream & out, int depth){
  HashIterator it(&hash);
  if( it.Seek(root) ){
    for( int i=0; i<depth; ++i )
      out<<"| ";
    cGTElement * c = it.CurrentData();
    out<<root<<"["<<(int)(c->IsAlive())<<"]";
    if( c->flag == FALSE ){
      c->flag=TRUE;
      if( it.Good() && c->daughters != NULL ){
	out<<"("<<c->daughters->CalcSize()<<","
	  <<c->num_dead_daughter_gt<<"/"<<c->num_daughter_gt<<")"<<endl;
	c->daughter_it->Reset();
	while( c->daughter_it->Good() ){
	  if( hash_it.Seek(c->daughter_it->CurrentKey()) ){
	    if( hash_it.CurrentData()->parent == root ){
	      PrintTrimTreeRecurse(c->daughter_it->CurrentKey(), out, depth+1);
	    }
	  }
	  c->daughter_it->Next();
	}
      }else{
	out<<"(0)"<<endl;
      }
    }else{
      out<<endl;
    }
  }
}


#endif
