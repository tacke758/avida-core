//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef RESOURCE_COUNT_HH
#define RESOURCE_COUNT_HH

#define GEOMETRY_GLOBAL 0
#define GEOMETRY_GRID   1
#define GEOMETRY_TORUS  2


#include "../tools/tArray.hh"
#include "../tools/tMatrix.hh"
#include "../tools/string.hh"

#define MAXFLOWPTS 8

class cSpatialCountElem {
  mutable double amount, delta;
  tArray<int> elempt, xdist, ydist;
  tArray<double> dist;
public:
  cSpatialCountElem (double initamount);
  cSpatialCountElem ();
  void Rate (double ratein) const {delta += ratein;}
  void State () {amount += delta; delta = 0.0;}
  const double GetAmount() const {return amount;}
  void SetPtr (int innum, int inelempt, int inxdist, int  inydist, 
               double indist);
  int GetElemPtr (int innum) {return elempt[innum];}
  int GetPtrXdist (int innum) {return xdist[innum];}
  int GetPtrYdist (int innum) {return ydist[innum];}
  double GetPtrDist (int innum) {return dist[innum];}
  friend void FlowMatter (cSpatialCountElem&, cSpatialCountElem&, double, 
                          double, double, double, int, int, double);
};


class cSpatialResCount {
  tArray<cSpatialCountElem> grid;
  double xdiffuse, xgravity, ydiffuse, ygravity;
  int    inflowX1, inflowX2, inflowY1, inflowY2;
  int    outflowX1, outflowX2, outflowY1, outflowY2;
  int    geometry;
  int    world_x, world_y, num_cells;
public:
  cSpatialResCount();
  cSpatialResCount(int inworld_x, int inworld_y);
  cSpatialResCount(int inworld_x, int inworld_y, double inxdiffuse,
        double inydiffuse, double inxgravity, double inygravity);
  void ResizeClear(int inworld_x, int inworld_y);
  void SetPointers();
  void CheckRanges();
  int GetSize () {return grid.GetSize();}
  int GetX () {return world_x;}
  int GetY () {return world_y;}
  cSpatialCountElem Element(int x) {return grid[x];}
  void Rate (int x, double ratein) const {grid[x].Rate(ratein);}
  void Rate (int x, int y, double ratein) const 
      {grid[y * world_x + x].Rate(ratein);}
  void State (int x) {grid[x].State();}
  void State (int x, int y) {grid[y*world_x + x].State();}
  const double GetAmount (int x) const {return grid[x].GetAmount();}
  const double GetAmount (int x, int y) const 
                           {return grid[y*world_x + x].GetAmount();}
  void RateAll (double ratein);
  void StateAll ();
  void FlowAll ();
  const double SumAll() const;
  void Source(double amount) const;
  void Sink(double percent) const;
  void SetGeometry(int in_geometry) { geometry = in_geometry; }
  void SetXdiffuse(double in_xdiffuse) { xdiffuse = in_xdiffuse; }
  void SetXgravity(double in_xgravity) { xgravity = in_xgravity; }
  void SetYdiffuse(double in_ydiffuse) { ydiffuse = in_ydiffuse; }
  void SetYgravity(double in_ygravity) { ygravity = in_ygravity; }
  void SetInflowX1(int in_inflowX1) { inflowX1 = in_inflowX1; }
  void SetInflowX2(int in_inflowX2) { inflowX2 = in_inflowX2; }
  void SetInflowY1(int in_inflowY1) { inflowY1 = in_inflowY1; }
  void SetInflowY2(int in_inflowY2) { inflowY2 = in_inflowY2; }
  void SetOutflowX1(int in_outflowX1) { outflowX1 = in_outflowX1; }
  void SetOutflowX2(int in_outflowX2) { outflowX2 = in_outflowX2; }
  void SetOutflowY1(int in_outflowY1) { outflowY1 = in_outflowY1; }
  void SetOutflowY2(int in_outflowY2) { outflowY2 = in_outflowY2; }
};


class cResourceCount {
private:
  mutable tArray<double> resource_count;  // Current quantity of each resource
  tArray<double> decay_rate;      // Multiplies resource count at each step
  tArray<double> inflow_rate;     // An increment for resource at each step
  tMatrix<double> decay_precalc;  // Precalculation of decay values
  tMatrix<double> inflow_precalc; // Precalculation of inflow values
  tArray<int> geometry;           // Spatial layout of each resource
  mutable tArray<cSpatialResCount> spatial_resource_count;
  mutable tArray<double> curr_grid_res_cnt;
  mutable tArray< tArray<double> > curr_spatial_res_cnt;

  // Setup the update process to use lazy evaluation...
  mutable double update_time;     // Portion of an update compleated...
  mutable double spatial_update_time; 
  void DoUpdates() const;         // Update resource count based on update time

  // A few constants to describe update process...
  static const double UPDATE_STEP;   // Fraction of an update per step
  static const int PRECALC_DISTANCE; // Number of steps to precalculate
public:
  cResourceCount(int num_resources);
  ~cResourceCount();

  void Setup(int id, cString name, double initial, double inflow, 
             double decay, int in_geometry, double in_xdiffuse,
             double in_xgravity, double in_ydiffuse, 
             double in_ygravity, int in_inflowX1, 
             int in_inflowX2, int in_inflowY1, 
             int in_inflowY2, int in_outflowX1, 
             int in_outflowX2, int in_outflowY1, 
             int in_outflowY);
  void Update(double in_time);

  const tArray<double> & GetResources() const;
  const tArray<double> & GetCellResources(int cell_id) const;
  const tArray<int> & GetResourcesGeometry() const;
  const tArray< tArray<double> > & GetSpatialRes();
  void Modify(const tArray<double> & res_change);
  void Modify(int id, double change);
  void ModifyCell(const tArray<double> & res_change, int cell_id);
  void Set(int id, double new_level);
  void ResizeSpatialGrids(int in_x, int in_y);
  cSpatialResCount GetSpatialResource(int id) 
       { return spatial_resource_count[id]; }
};

#endif
