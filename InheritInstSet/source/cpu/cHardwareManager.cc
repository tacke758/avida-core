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

#include "cEventContext.h"
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
#include "cInheritedInstSet.h"


cHardwareManager::cHardwareManager(cWorld* world)
: m_world(world), m_allowed_redundancies(NULL), m_type(world->GetConfig().HARDWARE_TYPE.Get())  /*, m_testres(world) */
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
  LoadRedundancyFile(m_world->GetConfig().EIS_REDUNDANCY_FILE.Get());
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
        cEventContext state(m_world->GetDefaultContext());
        state << cCntxEntry("id",id) << cCntxEntry("ptr", (int) new_inst_set);
        cPopulation::ForAllOrganisms(cHardwareManager::ReplaceDeprecatedInstSet, m_world->GetPopulation(), state);
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

                          
void cHardwareManager::LoadRedundancyFile(cString path){
  
  if (path == "-"){
    m_inherited_instset = false;
    return;
  }
  
  cInitFile file(path);
    
  if (file.WasOpened() == false) {
    tConstListIterator<cString> err_it(file.GetErrors());
    const cString* errstr = NULL;
    while ((errstr = err_it.Next())) m_world->GetDriver().RaiseException(*errstr);
    m_world->GetDriver().RaiseFatalException(1, cString("Could not open redundancy allowance file '") + path + "'.");
  }
  
  if (file.GetNumLines() != m_inst_sets[0]->GetSize())
    m_world->GetDriver().RaiseFatalException(1, cString("Allowed instruction redundancies has an incorrect number of entries for the loaded instruction set."));
  
  m_allowed_redundancies = new tArray< tArray<int> >( m_inst_sets[0]->GetSize(), tArray<int>(0));
  for (int line_id = 0; line_id < file.GetNumLines(); line_id++) {
    cString cur_line = file.GetLine(line_id);
    cString inst_name = cur_line.PopWord();
    if (inst_name != m_inst_sets[0]->GetName(line_id))
      m_world->GetDriver().RaiseFatalException(1, cString("Allowed instruction redundancies are not in the same order as the loaded instruction set."));
    while (cur_line.GetSize())
      (*m_allowed_redundancies)[line_id].Push( cur_line.PopWord().AsInt() );
  }
  m_inherited_instset = true;
  m_init_redundancy = m_world->GetConfig().EIS_INIT_RED.Get();
  return;
}
                          

cHardwareBase* cHardwareManager::Create(cOrganism* in_org, const cInstSet* parent_instset)
{
  assert(in_org != NULL);
  
  int inst_id = in_org->GetInstSetID();
  assert(inst_id <= m_inst_sets.GetSize()-1 && m_inst_sets[inst_id] != NULL);

  cInstSet* this_instset = (!m_inherited_instset) ? m_inst_sets[inst_id] :
              (parent_instset == NULL) ? new cInheritedInstSet(m_inst_sets[inst_id], m_init_redundancy, *m_allowed_redundancies) :
                                     new cInheritedInstSet(parent_instset, *m_allowed_redundancies);
  
  switch (m_type)
  {
    case HARDWARE_TYPE_CPU_ORIGINAL:
      return new cHardwareCPU(m_world, in_org, this_instset, m_inherited_instset);
    case HARDWARE_TYPE_CPU_SMT:
      return new  cHardwareSMT(m_world, in_org, this_instset, m_inherited_instset);
    case HARDWARE_TYPE_CPU_TRANSSMT:
      return  new cHardwareTransSMT(m_world, in_org, this_instset, m_inherited_instset);
    case HARDWARE_TYPE_CPU_EXPERIMENTAL:
      return new cHardwareExperimental(m_world, in_org, this_instset, m_inherited_instset);
    case HARDWARE_TYPE_CPU_GX:
      return new cHardwareGX(m_world, in_org, this_instset, m_inherited_instset);
    default:
      return NULL;
  }
}


void cHardwareManager::ReplaceDeprecatedInstSet(cOrganism* org, cEventContext& ctx)
{
  int id = (*ctx["id"]).AsInt();
  cInstSet* ptr = (cInstSet*) ( (*ctx["ptr"]).AsInt() );
  if (org->GetInstSetID() == id)
    org->GetHardware().SetInstSet(ptr);
}
