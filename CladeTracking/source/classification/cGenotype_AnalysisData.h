/*
 *  cGenotype_AnalysisData.h
 *  Avida
 *
 *  Copyright 1999-2008 Michigan State University. All rights reserved.
 *  Copyright 1993-2003 California Institute of Technology.
 *
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; version 2
 *  of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */


/*  This class is designed to handle some experiment-time analysis of Avida genotypes
 *  including genetic background tracking.
 */



#ifndef cGenotype_AnalysisData_h
#define cGenotype_AnalysisData_h

#ifndef cGenotype_h
#include "cGenotype.h"
#endif

class cGenotype_AnalysisData {
  
  private:  
    // This class is supposed to be an internal, general purpose means
    // of tracking epistasis information in real time.  Its purpose is
    // to keep track of the identity of the base genetic background --
    // the second genotype in the epistasis analysis pair.
    class cEpistasisInfo{         
      int    base_gentoype_id;    // Identity of base genetic background
      int    base_genotype_depth; // Depth of base genetic background
      double log_epistasis;
    };
    mutable cEpistasisInfo* m_SignEpistasisMRCA;   // Keep track of sign epistasis data to MRCA   (allowed to be invalid or null)
    mutable cEpistasisInfo* m_SignEpistasisParent; // Keep track of sign epistasis data to Parent (allowed to be invalid or null)
    void CheckSignEpistasisMRCA();    // Validate MRCA epistasis data
    void CheckSignEpistasisParent();  // Validate parent epistasis data
    
    int m_coalescese_time;  // Time the genotype became part of the genetic background of the entire population (-1 if never or not yet);
    
    
  public:
     //Common Ancestor Information
     int DepthToMRCA() {return 0;}    // Depth to most recent common ancestor
     int CoalescenseTime() {return -1;} // Time this genotype became the genetic background of the population (-1 if never or not yet)
    
     //Sign-Epistatic Mutation Tracking
      double EpistaticConversionToMRCA() {return 0.0;}   // Revert all background mutations back to MRCA (except last mutation)
     
     
};


#endif
