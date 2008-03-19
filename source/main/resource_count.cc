//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <iostream>
#include <math.h>
#include "../tools/string_util.hh"
#include "../tools/functions.hh"

#include "resource_count.hh"


using namespace std;


const double cResourceCount::UPDATE_STEP(1.0 / 10000.0);
const int cResourceCount::PRECALC_DISTANCE(100);

cSpatialCountElem::cSpatialCountElem (double initamount) {
    amount = initamount;
    delta=0.0;
    elempt.Resize(MAXFLOWPTS);
    xdist.Resize(MAXFLOWPTS);
    ydist.Resize(MAXFLOWPTS);
    dist.Resize(MAXFLOWPTS);
}
cSpatialCountElem::cSpatialCountElem () {
    amount = 0.0;
    delta=0.0;
    elempt.Resize(MAXFLOWPTS);
    xdist.Resize(MAXFLOWPTS);
    ydist.Resize(MAXFLOWPTS);
    dist.Resize(MAXFLOWPTS);
}
void cSpatialCountElem::SetPtr (int innum, int inelempt, int inxdist, 
                        int inydist, double indist) {
  elempt[innum] = inelempt;
  xdist[innum] = inxdist;
  ydist[innum] = inydist;
  dist[innum] = indist;
}

cSpatialResCount::cSpatialResCount(int inworld_x, int inworld_y, 
                  double inxdiffuse, double inydiffuse, double inxgravity, 
                  double inygravity)
                 : grid(inworld_x * inworld_y) {

  int i;
 
  xdiffuse = inxdiffuse;
  ydiffuse = inydiffuse;
  xgravity = inxgravity;
  ygravity = inygravity;
  world_x = inworld_x;
  world_y = inworld_y;
  num_cells = world_x * world_y;
  for (i = 0; i < GetSize(); i++) {
    cSpatialCountElem tmpelem;
    grid[i] = tmpelem;
  } 
  SetPointers();
}

cSpatialResCount::cSpatialResCount(int inworld_x, int inworld_y) 
                 : grid(inworld_x * inworld_y) {
  int i;
 
  xdiffuse = 1.0;
  ydiffuse = 1.0;
  xgravity = 0.0;
  ygravity = 0.0;
  world_x = inworld_x;
  world_y = inworld_y;
  num_cells = world_x * world_y;
  for (i = 0; i < GetSize(); i++) {
    cSpatialCountElem tmpelem;
    grid[i] = tmpelem;
   } 
   SetPointers();
}

cSpatialResCount::cSpatialResCount() {
  int i;
 
  xdiffuse = 1.0;
  ydiffuse = 1.0;
  xgravity = 0.0;
  ygravity = 0.0;
}

void cSpatialResCount::ResizeClear(int inworld_x, int inworld_y) {

  int i;
 
  grid.ResizeClear(inworld_x * inworld_y); 
  world_x = inworld_x;
  world_y = inworld_y;
  num_cells = world_x * world_y;
  for (i = 0; i < GetSize(); i++) {
    cSpatialCountElem tmpelem;
    grid[i] = tmpelem;
   } 
   SetPointers();
}

void cSpatialResCount::SetPointers() {

  /* Pointer 0 will point to the cell above and to the left the current cell
     and will go clockwise around the cell.                               */

  int     i,ii;
  double  SQRT2 = sqrt(2.0);

  /* First treat all cells like they are in a torus */

  for (i = 0; i < GetSize(); i++) {
    grid[i].SetPtr(0 ,Neighbor(i, world_x, world_y, -1, -1), -1, -1, SQRT2);
    grid[i].SetPtr(1 ,Neighbor(i, world_x, world_y,  0, -1),  0, -1, 1.0);
    grid[i].SetPtr(2 ,Neighbor(i, world_x, world_y, +1, -1), +1, -1, SQRT2);
    grid[i].SetPtr(3 ,Neighbor(i, world_x, world_y, +1,  0), +1,  0, 1.0);
    grid[i].SetPtr(4 ,Neighbor(i, world_x, world_y, +1, +1), +1, +1, SQRT2);
    grid[i].SetPtr(5 ,Neighbor(i, world_x, world_y,  0, +1),  0, +1, 1.0);
    grid[i].SetPtr(6 ,Neighbor(i, world_x, world_y, -1, +1), -1, +1, SQRT2);
    grid[i].SetPtr(7 ,Neighbor(i, world_x, world_y, -1,  0), -1,  0, 1.0);
  }
 
  /* Fix links for top, bottom and sides for non-torus */
  
  if (geometry == GEOMETRY_GRID) {
    /* Top and bottom */

    for (i = 0; i < world_x; i++) {
      grid[i].SetPtr(0, -99, -99, -99, -99.0);
      grid[i].SetPtr(1, -99, -99, -99, -99.0);
      grid[i].SetPtr(2, -99, -99, -99, -99.0);
      ii = num_cells-1-i;
      grid[ii].SetPtr(4, -99, -99, -99, -99.0);
      grid[ii].SetPtr(5, -99, -99, -99, -99.0);
      grid[ii].SetPtr(6, -99, -99, -99, -99.0);
    }

    /* fix links for right and left sides */

    for (i = 0; i < world_y; i++) {
      ii = i * world_x;    
      grid[ii].SetPtr(0, -99, -99, -99, -99.0);
      grid[ii].SetPtr(7, -99, -99, -99, -99.0);
      grid[ii].SetPtr(6, -99, -99, -99, -99.0);
      ii = ((i + 1) * world_x) - 1;
      grid[ii].SetPtr(2, -99, -99, -99, -99.0);
      grid[ii].SetPtr(3, -99, -99, -99, -99.0);
      grid[ii].SetPtr(4, -99, -99, -99, -99.0);
    }
  }
}

void cSpatialResCount::CheckRanges() {

  // Check that the x, y ranges of the inflow and outflow rectangles 
  // are valid
  /* check range of inputs */
  if (inflowX1 < 0) { 
    inflowX1 = 0; 
  } else if (inflowX1 > world_x) { 
    inflowX1 = world_x; 
  }
  if (inflowX2 < 0) { 
     inflowX2 = 0; 
  } else if (inflowX2 > world_x) { 
     inflowX2 = world_x; 
  }
  if (inflowY1 < 0) { 
    inflowY1 = 0; 
  } else if (inflowY1 > world_y) { 
    inflowY1 = world_y; 
  }
  if (inflowY2 < 0) { 
    inflowY2 = 0; 
  } else if (inflowY2 > world_y) { 
    inflowY2 = world_y; 
  }

  /* allow for rectangles that cross over the zero X or zero Y boundry */
  if (inflowX2 < inflowX1) { inflowX2 += world_x; }
  if (inflowY2 < inflowY1) { inflowY2 += world_y; }
  if (outflowX1 < 0) { 
    outflowX1 = 0; 
  } else if (outflowX1 > world_x) { 
    outflowX1 = world_x; 
  }
  if (outflowX2 < 0) { 
     outflowX2 = 0; 
  } else if (outflowX2 > world_x) { 
     outflowX2 = world_x; 
  }
  if (outflowY1 < 0) { 
    outflowY1 = 0; 
  } else if (outflowY1 > world_y) { 
    outflowY1 = world_y; 
  }
  if (outflowY2 < 0) { 
    outflowY2 = 0; 
  } else if (outflowY2 > world_y) { 
    outflowY2 = world_y; 
  }

  /* allow for rectangles that cross over the zero X or zero Y boundry */
  if (outflowX2 < outflowX1) { outflowX2 += world_x; }
  if (outflowY2 < outflowY1) { outflowY2 += world_y; }

}

void cSpatialResCount::RateAll(double ratein) {

  int i;
 
  for (i = 0; i < num_cells; i++) {
    grid[i].Rate(ratein);
  } 
}

void cSpatialResCount::StateAll() {

  int i;
 
  for (i = 0; i < num_cells; i++) {
    grid[i].State();
  } 
}

void cSpatialResCount::FlowAll() {

  int     i,k,ii,xdist,ydist;
  double  dist;
 
  for (i = 0; i < num_cells; i++) {
      
    /* because flow is two way we must check only half the neighbors to 
       prevent double flow calculations */

    for (k = 3; k <= 6; k++) {
      ii = grid[i].GetElemPtr(k);
      xdist = grid[i].GetPtrXdist(k);
      ydist = grid[i].GetPtrYdist(k);
      dist = grid[i].GetPtrDist(k);
      if (ii >= 0) {
        FlowMatter(grid[i],grid[ii],xdiffuse,ydiffuse,xgravity,ygravity,
                   xdist, ydist, dist);
      }
    }
  }
}

const double cSpatialResCount::SumAll() const{

  int i;
  double sum = 0.0;

  for (i = 0; i < num_cells; i++) {
    sum += GetAmount(i);
  } 
  return sum;
}


void cSpatialResCount::Source(double amount) const {
  int     i, j, elem;
  double  totalcells;


  totalcells = (inflowY2 - inflowY1 + 1) * (inflowX2 - inflowX1 + 1) * 1.0;
  amount /= totalcells;

  for (i = inflowY1; i <= inflowY2; i++) {
    for (j = inflowX1; j <= inflowX2; j++) {
      elem = (Mod(i,world_y) * world_x) + Mod(j,world_x);
      Rate(elem,amount); 
    }
  }
}

void cSpatialResCount::Sink(double decay) const {

  int     i, j, elem;
  double  deltaamount;


  for (i = outflowY1; i <= outflowY2; i++) {
    for (j = outflowX1; j <= outflowX2; j++) {
      elem = (Mod(i,world_y) * world_x) + Mod(j,world_x);
      deltaamount = Max((GetAmount(elem) * (1.0 - decay)), 0.0);
      Rate(elem,-deltaamount); 
    }
  }
}


void FlowMatter(cSpatialCountElem &elem1, cSpatialCountElem &elem2, double inxdiffuse, 
                double inydiffuse, double inxgravity, double inygravity,
                int xdist, int ydist, double dist) {

  /* Routine to calculate the amount of flow from one Element to another.
     Amount of flow is a function of:

       1) Amount of material in each cell (will try to equalize)
       2) Distance between each cell
       3) x and y "gravity"

     This method only effect the delta amount of each element.  The State
     method will need to be called at the end of each time step to complete
     the movement of material.
  */

  double  diff, flowamt, xgravity, xdiffuse, ygravity,  ydiffuse;

  if (((elem1.amount == 0.0) && (elem2.amount == 0.0)) && (dist < 0.0)) return;
  diff = (elem1.amount - elem2.amount);
  if (xdist != 0) {

    /* if there is material to be effected by x gravity */

    if (((xdist>0) && (inxgravity>0.0)) || ((xdist<0) && (inxgravity<0.0))) {
      xgravity = elem1.amount * fabs(inxgravity)/3.0;
    } else {
      xgravity = -elem2.amount * fabs(inxgravity)/3.0;
    }
    
    /* Diffusion uses the diffusion constant x half the difference (as the 
       elements attempt to equalize) / the number of possible neighbors (8) */

    xdiffuse = inxdiffuse * diff / 16.0;
  } else {
    xdiffuse = 0.0;
    xgravity = 0.0;
  }  
  if (ydist != 0) {

    /* if there is material to be effected by y gravity */

    if (((ydist>0) && (inygravity>0.0)) || ((ydist<0) && (inygravity<0.0))) {
      ygravity = elem1.amount * fabs(inygravity)/3.0;
    } else {
      ygravity = -elem2.amount * fabs(inygravity)/3.0;
    }
    ydiffuse = inydiffuse * diff / 16.0;
  } else {
    ydiffuse = 0.0;
    ygravity = 0.0;
  }  

  flowamt = ((xdiffuse + ydiffuse + xgravity + ygravity)/
             (fabs(xdist*1.0) + fabs(ydist*1.0)))/dist;
  elem1.delta -= flowamt;
  elem2.delta += flowamt;
}


cResourceCount::cResourceCount(int num_resources)
  : resource_count(num_resources)
  , decay_rate(num_resources)
  , inflow_rate(num_resources)
  , decay_precalc(num_resources, PRECALC_DISTANCE+1)
  , inflow_precalc(num_resources, PRECALC_DISTANCE+1)
  , spatial_resource_count(num_resources)
  , geometry(num_resources)
  , curr_grid_res_cnt(num_resources)
  , curr_spatial_res_cnt(num_resources)
  , update_time(0.0)
  , spatial_update_time(0.0)
{
  resource_count.SetAll(0.0);
  decay_rate.SetAll(0.0);
  inflow_rate.SetAll(0.0);
  decay_precalc.SetAll(0.0);
  inflow_precalc.SetAll(0.0);
  geometry.SetAll(GEOMETRY_GLOBAL);
  curr_grid_res_cnt.SetAll(0.0);
}

cResourceCount::~cResourceCount()
{
}

void cResourceCount::Setup(int id, cString name, double initial, double inflow,
                           double decay, int in_geometry, double in_xdiffuse,
                           double in_xgravity, double in_ydiffuse, 
                           double in_ygravity, int in_inflowX1, 
                           int in_inflowX2, int in_inflowY1, 
                           int in_inflowY2, int in_outflowX1, 
                           int in_outflowX2, int in_outflowY1, 
                           int in_outflowY2)
{
  assert(id >= 0 && id < resource_count.GetSize());
  assert(initial >= 0.0);
  assert(decay >= 0.0);
  assert(inflow >= 0.0);
  assert(spatial_resource_count[id].GetSize() > 0);

  cString geo_name;
  if (in_geometry == GEOMETRY_GLOBAL) {
    geo_name = "GLOBAL";
  } else if (in_geometry == GEOMETRY_GRID) {
    geo_name = "GRID";
  } else if (in_geometry == GEOMETRY_TORUS) {
    geo_name = "TORUS";
  }
  cerr << "Setting up resource " << name
       << "(" << geo_name 
       << ") with initial quatity=" << initial
       << ", decay=" << decay
       << ", inflow=" << inflow
       << endl;
  if ((in_geometry == GEOMETRY_GRID) || (in_geometry == GEOMETRY_TORUS)) {
    cerr << "  Inflow rectangle (" << in_inflowX1 
         << "," << in_inflowY1 
         << ") to (" << in_inflowX2 
         << "," << in_inflowY2 
         << ")" << endl; 
    cerr << "  Outflow rectangle (" << in_outflowX1 
         << "," << in_outflowY1 
         << ") to (" << in_outflowX2 
         << "," << in_outflowY2 
         << ")" << endl;
    cerr << "  xdiffuse=" << in_xdiffuse
         << ", xgravity=" << in_xgravity
         << ", ydiffuse=" << in_ydiffuse
         << ", ygravity=" << in_ygravity
         << endl;
  }   

  resource_count[id] = initial;
  spatial_resource_count[id].RateAll
                              (initial/spatial_resource_count[id].GetSize());
  spatial_resource_count[id].StateAll();  
  decay_rate[id] = decay;
  inflow_rate[id] = inflow;
  geometry[id] = in_geometry;
  spatial_resource_count[id].SetGeometry(in_geometry);
  spatial_resource_count[id].SetPointers();

  double step_decay = pow(decay, UPDATE_STEP);
  double step_inflow = inflow * UPDATE_STEP;

  decay_precalc(id, 0) = 1.0;
  inflow_precalc(id, 0) = 0.0;
  for (int i = 1; i <= PRECALC_DISTANCE; i++) {
    decay_precalc(id, i)  = decay_precalc(id, i-1) * step_decay;
    inflow_precalc(id, i) = inflow_precalc(id, i-1) * step_decay + step_inflow;
  }
  spatial_resource_count[id].SetXdiffuse(in_xdiffuse);
  spatial_resource_count[id].SetXgravity(in_xgravity);
  spatial_resource_count[id].SetYdiffuse(in_ydiffuse);
  spatial_resource_count[id].SetYgravity(in_ygravity);
  spatial_resource_count[id].SetInflowX1(in_inflowX1);
  spatial_resource_count[id].SetInflowX2(in_inflowX2);
  spatial_resource_count[id].SetInflowY1(in_inflowY1);
  spatial_resource_count[id].SetInflowY2(in_inflowY2);
  spatial_resource_count[id].SetOutflowX1(in_outflowX1);
  spatial_resource_count[id].SetOutflowX2(in_outflowX2);
  spatial_resource_count[id].SetOutflowY1(in_outflowY1);
  spatial_resource_count[id].SetOutflowY2(in_outflowY2);
}

void cResourceCount::Update(double in_time) 
{ 
  update_time += in_time;
  spatial_update_time += in_time;
 }

 
const tArray<double> & cResourceCount::GetResources() const
{
  DoUpdates();
  return resource_count;
}
 
const tArray<double> & cResourceCount::GetCellResources(int cell_id) const

  // Get amount of the resource for a given cell in the grid.  If it is a
  // global resource pass out the entire content of that resource.

{
  int num_resources = resource_count.GetSize();
  DoUpdates();
  for (int i = 0; i < num_resources; i++) {
    if (geometry[i] == GEOMETRY_GLOBAL) {
      curr_grid_res_cnt[i] = resource_count[i];
    } else {
      curr_grid_res_cnt[i] = spatial_resource_count[i].GetAmount(cell_id);
    }
  }
  return curr_grid_res_cnt;

}

const tArray<int> & cResourceCount::GetResourcesGeometry() const
{
  return geometry;
}

const tArray< tArray<double> > &  cResourceCount::GetSpatialRes()
{
  const int num_spatial_resources = spatial_resource_count.GetSize();
  if (num_spatial_resources > 0) {
    const int num_cells = spatial_resource_count[0].GetSize();
    DoUpdates();
    for (int i = 0; i < num_spatial_resources; i++) {
      for (int j = 0; j < num_cells; j++) {
	curr_spatial_res_cnt[i][j] = spatial_resource_count[i].GetAmount(j);
      }
    }
  }

  return curr_spatial_res_cnt;
}

void cResourceCount::Modify(const tArray<double> & res_change)
{
  assert(resource_count.GetSize() == res_change.GetSize());

  for (int i = 0; i < resource_count.GetSize(); i++) {
    resource_count[i] += res_change[i];
    assert(resource_count[i] >= 0.0);
  }
}


void cResourceCount::Modify(int id, double change)
{
  assert(id < resource_count.GetSize());

  resource_count[id] += change;
  assert(resource_count[id] >= 0.0);
}

void cResourceCount::ModifyCell(const tArray<double> & res_change, int cell_id)
{
  assert(resource_count.GetSize() == res_change.GetSize());

  for (int i = 0; i < resource_count.GetSize(); i++) {
    if (geometry[i] == GEOMETRY_GLOBAL) {
      resource_count[i] += res_change[i];
      assert(resource_count[i] >= 0.0);
    } else {
      spatial_resource_count[i].Rate(cell_id, res_change[i]);
    }
  }
}


void cResourceCount::Set(int id, double new_level)
{
  assert(id < resource_count.GetSize());

  resource_count[id] = new_level;
}

void cResourceCount::ResizeSpatialGrids(int in_x, int in_y)
{
  for (int i = 0; i < resource_count.GetSize(); i++) {
    spatial_resource_count[i].ResizeClear(in_x, in_y);
    curr_spatial_res_cnt[i].Resize(in_x * in_y);
  }
}
///// Private Methods /////////

void cResourceCount::DoUpdates() const
{ 
  assert(update_time >= 0);

  // Determine how many update steps have progressed
  int num_steps = (int) (update_time / UPDATE_STEP);

  // Preserve remainder of update_time
  update_time -= num_steps * UPDATE_STEP;


  while (num_steps > PRECALC_DISTANCE) {
    for (int i = 0; i < resource_count.GetSize(); i++) {
      if (geometry[i] == GEOMETRY_GLOBAL) {
        resource_count[i] *= decay_precalc(i, PRECALC_DISTANCE);
        resource_count[i] += inflow_precalc(i, PRECALC_DISTANCE);
      }
    }
    num_steps -= PRECALC_DISTANCE;
  }

  for (int i = 0; i < resource_count.GetSize(); i++) {
    if (geometry[i] == GEOMETRY_GLOBAL) {
      resource_count[i] *= decay_precalc(i, num_steps);
      resource_count[i] += inflow_precalc(i, num_steps);
    }
  }

  // If one (or more) complete update has occured update the spatial resources

  while (spatial_update_time >= 1.0) { 
    spatial_update_time -= 1.0;
    for (int i = 0; i < resource_count.GetSize(); i++) {
      if (geometry[i] != GEOMETRY_GLOBAL) {
        spatial_resource_count[i].Source(inflow_rate[i]);
        spatial_resource_count[i].Sink(decay_rate[i]);
        spatial_resource_count[i].FlowAll();
        spatial_resource_count[i].StateAll();
        resource_count[i] = spatial_resource_count[i].SumAll();
      }
    }
  }
}
