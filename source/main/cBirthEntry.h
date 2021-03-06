/*
 *  cBirthEntry.h
 *  Avida
 *
 *  Created by David Bryson on 4/1/09.
 *  Copyright 2009-2011 Michigan State University. All rights reserved.
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
 */

#ifndef cBirthEntry_h
#define cBirthEntry_h

#include "avida/core/Genome.h"

#ifndef cMerit_h
#include "cMerit.h"
#endif

class cBioGroup;

using namespace Avida;


class cBirthEntry
{
public:
  Genome genome;
  double energy4Offspring;
  cMerit merit;
  int timestamp; // -1 if empty
  tArray<cBioGroup*> groups;
  
  inline cBirthEntry() : timestamp(-1) { ; }
};

#endif
