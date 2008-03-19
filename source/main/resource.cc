//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "resource.hh"


using namespace std;


cResource::cResource(const cString & _name, int _id)
  : name(_name)
  , id(_id)
  , initial(0.0)
  , inflow(0.0)
  , outflow(0.0)
  , geometry(GEOMETRY_GLOBAL)
  , inflowX1(-99)
  , inflowX2(-99)
  , inflowY1(-99)
  , inflowY2(-99)
  , outflowX1(-99)
  , outflowX2(-99)
  , outflowY1(-99)
  , outflowY2(-99)
  , xdiffuse(1.0)
  , xgravity(0.0)
  , ydiffuse(1.0)
  , ygravity(0.0)
{
}

bool cResource::SetGeometry(cString _geometry)
{
     _geometry.ToLower();
     if (_geometry == "global") {
          geometry = GEOMETRY_GLOBAL;
          return true;
     } else if (_geometry == "grid") {
          geometry = GEOMETRY_GRID;
          return true;
     } else if (_geometry == "torus") {
          geometry = GEOMETRY_TORUS;
          return true;
     } else {
          return false;
     }
}

cResourceLib::~cResourceLib()
{
  for (int i = 0; i < resource_array.GetSize(); i++) {
    delete resource_array[i];
  }
}

cResource * cResourceLib::AddResource(const cString & res_name)
{
  const int new_id = resource_array.GetSize();
  cResource * new_resource = new cResource(res_name, new_id);
  resource_array.Resize(new_id + 1);
  resource_array[new_id] = new_resource;
  return new_resource;
}

cResource * cResourceLib::GetResource(const cString & res_name) const
{
  for (int i = 0; i < resource_array.GetSize(); i++) {
    if (resource_array[i]->GetName() == res_name) return resource_array[i];
  }
  return NULL;
}

cResource * cResourceLib::GetResource(int id) const
{
  return resource_array[id];
}

