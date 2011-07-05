/*
 *  core/Types.h
 *  avida-core
 *
 *  Created by David on 5/20/11.
 *  Copyright 2011 Michigan State University. All rights reserved.
 *  http://avida.devosoft.org/
 *
 *
 *  This file is part of Avida.
 *
 *  Avida is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 *  Avida is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License along with Avida.
 *  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Authors: David M. Bryson <david@programerror.com>
 *
 */

#ifndef AvidaCoreTypes_h
#define AvidaCoreTypes_h

#include "apto/core.h"

namespace Avida {
  
  // Class Declarations
  // --------------------------------------------------------------------------------------------------------------
  
  class Context;
  class Feedback;
  class Genome;
  class GlobalObject;
  class Sequence;
  class World;
  class WorldDriver;
  class WorldFacet;

  
  // Type Declarations
  // --------------------------------------------------------------------------------------------------------------  
  
  typedef int Update; // Discrete unit of activity in Avida
  
  typedef Apto::String WorldFacetID;
  typedef Apto::SmartPtr<WorldFacet, Apto::InternalRCObject> WorldFacetPtr;
  
};

#endif