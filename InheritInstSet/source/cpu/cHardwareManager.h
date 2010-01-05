/*
 *  cHardwareManager.h
 *  Avida
 *
 *  Created by David on 10/18/05.
 *  Copyright 1999-2007 Michigan State University. All rights reserved.
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

#ifndef cHardwareManager_h
#define cHardwareManager_h

#include <cassert>

#ifndef cTestCPU_h
#include "cTestCPU.h"
#endif

#if USE_tMemTrack
# ifndef tMemTrack_h
#  include "tMemTrack.h"
# endif
#endif

#ifndef tList_h
#include "tList.h"
#endif

#ifndef cInstSet_h
#include "cInstSet.h"
#endif

#ifndef tList_h
#include "tArray.h"
#endif

class cHardwareBase;
class cOrganism;
class cWorld;


class cHardwareManager
{
#if USE_tMemTrack
  tMemTrack<cHardwareManager> mt;
#endif
private:
  cWorld* m_world;
  tArray<cInstSet*> m_inst_sets;  //Global instruction sets
  bool m_inherited_instset;       //Are we using per-organism instruction sets?
  tArray< tArray<int> > m_allowed_redundancies;  //Global restriction on per-organsim instruction sets.
  int m_init_redundancy;

  int m_type;
  
//  cTestResources m_testres;
  
  void LoadRedundancyFile(cString path);
  
  cHardwareManager(); // @not_implemented
  cHardwareManager(const cHardwareManager&); // @not_implemented
  cHardwareManager& operator=(const cHardwareManager&); // @not_implemented
  

public:
  cHardwareManager(cWorld* world);
  ~cHardwareManager() { for(int i = 0; i < m_inst_sets.GetSize(); i++) delete m_inst_sets[i]; }
  
  cHardwareBase* Create(cOrganism* in_org, cOrganism* parent_org = NULL);
  cTestCPU* CreateTestCPU() { return new cTestCPU(m_world /*, &m_testres*/); }

  const cInstSet& GetInstSet(int id=0) const { assert(id < m_inst_sets.GetSize()); return *(m_inst_sets[id]); }
  cInstSet& GetInstSet(int id=0) { assert(id < m_inst_sets.GetSize()); return *(m_inst_sets[id]); }
  
  int GetNumInstSets() const { return m_inst_sets.GetSize(); }
  
  bool AddInstSet(const cString& filename, int id = 0);
};


#ifdef ENABLE_UNIT_TESTS
namespace nHardwareManager {
  /**
   * Run unit tests
   *
   * @param full Run full test suite; if false, just the fast tests.
   **/
  void UnitTests(bool full = false);
}
#endif  

#endif
