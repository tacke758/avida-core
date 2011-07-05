/*
 *  cDynamicCount.h
 *  Avida
 *
 *  Copyright 2010-2011 Michigan State University. All rights reserved.
 *  http://avida.devosoft.org/
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
 *  along with this program; if not, write to the Free Software Foundation, 
 *  Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *  Authors: Jacob Walker <walke434@msu.edu>
 */

#ifndef cDynamicCount_h
#define cDynamicCount_h

#ifndef cSpatialResCount_h
#include "cSpatialResCount.h"
#endif


class cDynamicCount : public cSpatialResCount
{
public:
  void FlowAll();
  void StateAll();
  
  cDynamicCount(int peaks, double in_min_height, double in_min_radius, double in_radius_range, double in_ah, double in_ar,
		double in_acx, double in_acy, double in_hstepscale, double in_rstepscale, double in_cstepscalex, double in_cstepscaley, double in_hstep, double in_rstep,
		double in_cstepx, double in_cstepy, int in_worldx, int in_worldy, int in_geometry, int in_update_dynamic);
  ~cDynamicCount();
  void UpdateCount();
  
private:  
   cDynamicCount();
/*JW This is code adapted from DeJong and Morrison's 
Dynamic Function generator
*/
  
  /************************************************************
   This is the header file for df1.c               
   
   This file contains the global variables and definitions for 
   functions df1, df1_chg_H, df1_chg_R, df1_chg_C, and def1_init.

  This file allows you to set the initialization values that
  define the dynamic behavior of test function generated by
  the df1 Test Function Generator.

  Details regarding this test function generator can be found
  in "A Test Problem Generator for Non-Stationary Environments"
  by Ronald Morrison and Kenneth De Jong in the Proceeding of
  the Congress on Evolutionary Computation (CEC-99).

  The functions themselves are located in df1.c       

  Permission is hereby granted to copy all or any part of  
  this program for free distribution.               
                                                           
 File: df1.h     version 1.4 June 25, 2000
  Modified to change "bounce" characteristics at the end of the
    range of the changes.
 
   Direct comments and suggestions to:
   Ronald Morrison at ronald.morrison@mitretek.org
*************************************************************/
/*  Declare some variables.  These variables are all
    randomized in function df1_init.  If you prefer
    specific values for some or all of these peaks 
    (for example, if you want one high peak of a specific
    value) you may set any values you like at the end of the
    randomization process in appropriate section of df_init.*/

double* H;  /* H refers to the height of the peaks */
//double H[NUMBER_PEAKS]; 
double* R;  /* R controls the slope of the peaks
//double R[NUMBER_PEAKS];  It is the radius value of the 
                         right circular cones.*/

double** x;
//double x[NUMBER_PEAKS][2]; /* These are the spatial coordinates */

int* IUPH;
int* IUPR;
int** IUPC;

/*
int IUPH[NUMBER_PEAKS];
int IUPR[NUMBER_PEAKS];
int IUPC[NUMBER_PEAKS][2];
*/


/* The following variables establish the static landscape for 
   the test problem and establish the dynamic behavior of the
   environment.  Default values are provided, but the values 
   in this file may be changed to generate a wide variety 
   of dynamic behaviors, from very simple, to very complex. */

/*  First we establish the static environment that we will
    apply dynamics to. The static environment consists of
    the maximum of a field of right circular k-dimensional 
    cones, where the fitness is defined, for example in 2
    dimensions as:
    f(X,Y) = max(over i=1 to N) (Hi - Ri*(sqrt((X-Xi)2 +
                                               (Y-Yi2))))
    The height of the cones range from Hbase to (Hbase + Hrange).
    The radii of the cones range from Rbase to (Rbse + Rrange). */

int NoPeaks;  /* Actual number of peaks */
int NoDim;      /* Actual number of dimensions */

double Hbase;  /* min height */
double Hrange;  

double Hmax; /* This always equals Hbase + Hrange, and is set
                 in df1_init */
double Hminpct; /* This always equals Hbase/Hmax, and is set
                 in df1_init */

double Rbase;  /* min r value */
double Rrange;

double Rmax; /*  This always equals Rbase + Rrange, and is set 
                   in df1_init  */
double Rminpct; /* This ways equals Rbase/Rmax, and is set in
                   df1_init  */

/* The next section sets the variables that control the 
   dynamics of the problem.  The dynamics are controlled by
   the Logistics function given by:

   Y(t) = A*Y(t-1)*(1-Y(t-1))

   where Y(t) is Y at time t, and 
         Y(t-1) is Y at time t-1.

  The logistics function allows a wide range of dynamic
  performance by a simple change of the value of A, from simple.
  constant step sizes, to step sizes that alternate between two
  values, to step sizes that rotate through several values, to
  completely chaotic step size selection.  It is recommended that
  you plot the logistics function Y values vs. A values (1<A<4)
  to pick the value best for you.  The defaults have been set
  3.3, which creates oscillation between two step-size values:
  .8236 and .4794.  */

double Ah;  /* The A value for changing the peak heights */
double Ar;  /* The A value for changing the peak slopes */
double Ac[2]; /* An Ac value is needed for each 
                    spatial dimension as contained in NoDim.*/

/*  Since the logistics function creates step sizes that 
    may be larger than intended for each step, a single, linear
    scaling factor is provided for each moving parameter (height,
    slope, and spatial dimension).  
    
    Default values are set to 0.2, meaning that at the default
    values, each scaled step-size will oscillate between the 
    values of 0.16472 and 0.09588.     */
    
double Hstepscale;
double Rstepscale;
double cstepscale[2];/* One for each spatial
                                       dimension */

/*  These variables contain the step sizes.
    The A values (Ah, Ar, ...) will actually determine the 
    step values. Values are set here to allow the logistics 
    function to work through function transients during 
    initialization  */

double Hstep; 
double Rstep;
double cstep[2]; /* Need one for each 
                                    Dimension from above */
//JW End                                    

double** raw_grid;
int* ChangedPeaks;

double df1(double coord[2]);
void df1_init();
void df1_chg_R(int NoToChg, int* Peaks);
void df1_chg_H (int NoToChg, int* Peaks);
void df1_chg_c(int CoordNo, int NoToChg, int* Peaks);
int df1_flip();

int m_update_dynamic;
int m_counter;
};

#endif
