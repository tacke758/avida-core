/*
 *  cHardwareManager.cc
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

#include "cHardwareManager.h"

#include "cHardwareCPU.h"
#include "cHardwareExperimental.h"
#include "cHardwareSMT.h"
#include "cHardwareTransSMT.h"
#include "cHardwareGX.h"
#include "cInitFile.h"
#include "cInstSet.h"
#include "cWorld.h"
#include "cWorldDriver.h"
#include "cOrganism.h"
#include "cPopulation.h"
#include "cPopulationCell.h"
#include "tDictionary.h"
#include "cStats.h"

cHardwareManager::cHardwareManager(cWorld* world)
: m_world(world), m_type(world->GetConfig().HARDWARE_TYPE.Get()) /*, m_testres(world) */
{
  cString filename = world->GetConfig().INST_SET.Get();
  
  // Setup the instruction library and collect the default filename
  cString default_filename;
  switch (m_type)
	{
		case HARDWARE_TYPE_CPU_ORIGINAL:
			default_filename = cHardwareCPU::GetDefaultInstFilename();
			break;
		case HARDWARE_TYPE_CPU_SMT:
			default_filename = cHardwareSMT::GetDefaultInstFilename();
			break;
		case HARDWARE_TYPE_CPU_TRANSSMT:
			default_filename = cHardwareTransSMT::GetDefaultInstFilename();
			break;
		case HARDWARE_TYPE_CPU_EXPERIMENTAL:
			default_filename = cHardwareExperimental::GetDefaultInstFilename();
			break;
    case HARDWARE_TYPE_CPU_GX:
			default_filename = cHardwareGX::GetDefaultInstFilename();
			break;      
		default:
      m_world->GetDriver().RaiseFatalException(1, "Unknown/Unsupported HARDWARE_TYPE specified");
  }
  if (filename == "" || filename == "-") {
    filename = default_filename;
    m_world->GetDriver().NotifyComment(cString("Using default instruction set: ") + filename);
  }
  AddInstSet(filename);
}


bool cHardwareManager::AddInstSet(const cString& filename, int id)
{
  
  cInstSet* new_inst_set;
  switch (m_type)
	{
		case HARDWARE_TYPE_CPU_ORIGINAL:
      new_inst_set = new cInstSet(m_world, cHardwareCPU::GetInstLib());
			break;
		case HARDWARE_TYPE_CPU_SMT:
      new_inst_set = new cInstSet(m_world, cHardwareSMT::GetInstLib());
			break;
		case HARDWARE_TYPE_CPU_TRANSSMT:
      new_inst_set = new cInstSet(m_world, cHardwareTransSMT::GetInstLib());
			break;
		case HARDWARE_TYPE_CPU_EXPERIMENTAL:
      new_inst_set = new cInstSet(m_world, cHardwareExperimental::GetInstLib());
			break;
    case HARDWARE_TYPE_CPU_GX:
      new_inst_set = new cInstSet(m_world, cHardwareGX::GetInstLib());
			break;      
		default:
      m_world->GetDriver().RaiseFatalException(1, "Unknown/Unsupported HARDWARE_TYPE specified");
  }
  
  
  if (m_world->GetConfig().INST_SET_FORMAT.Get()) {
    new_inst_set->LoadFromConfig();
  } else {
    new_inst_set->LoadFromLegacyFile(filename);
  }
  
  if (m_inst_sets.GetSize() == 0 || (*m_inst_sets[0] == *new_inst_set) ){
    if (id <= m_inst_sets.GetSize() - 1){ //If our array is large enough
      if (m_inst_sets[id] != NULL){ //Old instruction set exists, migrate hardware and delete it
        int sz = m_world->GetPopulation().GetSize();
        for (int k = 0; k < sz; k++){  //Go through every organism and see if it's using the depricated instruction set
          cInstSet* old = (m_world->GetPopulation().GetCell(k).IsOccupied()) ? m_world->GetPopulation().GetCell(k).GetHardware()->GetInstSetPtr() : NULL;
          if (old != NULL && old == m_inst_sets[id])
            old = new_inst_set;  //replace it if necessary
        }
        delete m_inst_sets[id];
      }
      m_inst_sets[id] = new_inst_set;
    } else{  //If we have to resize the array to accomodate the ID
      m_inst_sets.Resize(id+1, NULL);
      if (m_inst_sets.GetSize() > 1) //Update our stats object if more than one inst set is added.   Initial size is set in cStats constructor.
        m_world->GetStats().GetInstSetCounts().Resize(id+1,0);
      m_inst_sets[id] = new_inst_set;
    }
  } else{ //Our instruction set is incompatible
    m_world->GetDriver().RaiseFatalException(1, "Additional instruction set addition is incompatible.");
  }
  
  return true;
}


cHardwareBase* cHardwareManager::Create(cOrganism* in_org)
{
  assert(in_org != NULL);
  
  int inst_id = in_org->GetInstSetID();
  assert(inst_id <= m_inst_sets.GetSize()-1 && m_inst_sets[inst_id] != NULL);
  switch (m_type)
  {
    case HARDWARE_TYPE_CPU_ORIGINAL:
      return new cHardwareCPU(m_world, in_org, m_inst_sets[inst_id]);
    case HARDWARE_TYPE_CPU_SMT:
      return new cHardwareSMT(m_world, in_org, m_inst_sets[inst_id]);
    case HARDWARE_TYPE_CPU_TRANSSMT:
      return new cHardwareTransSMT(m_world, in_org, m_inst_sets[inst_id]);
    case HARDWARE_TYPE_CPU_EXPERIMENTAL:
      return new cHardwareExperimental(m_world, in_org, m_inst_sets[inst_id]);
    case HARDWARE_TYPE_CPU_GX:
      return new cHardwareGX(m_world, in_org, m_inst_sets[inst_id]);
    default:
      return NULL;
  }
}
