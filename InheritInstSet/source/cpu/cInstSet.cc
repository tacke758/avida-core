/*
 *  cInstSet.cc
 *  Avida
 *
 *  Called "inst_set.cc" prior to 12/5/05.
 *  Copyright 1999-2007 Michigan State University. All rights reserved.
 *  Copyright 1993-2001 California Institute of Technology.
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

#include "cInstSet.h"

#include "cArgContainer.h"
#include "cArgSchema.h"
#include "cAvidaContext.h"
#include "cInitFile.h"
#include "cStringUtil.h"
#include "cWeightedIndex.h"
#include "cWorld.h"
#include "cWorldDriver.h"

using namespace std;

cInstSet::~cInstSet(){
  if (m_weight_ndx != NULL) delete m_weight_ndx; 
}

cInstSet::cInstSet(const cInstSet& is)
: m_world(is.m_world), m_inst_lib(is.m_inst_lib), m_lib_name_map(is.m_lib_name_map)
,m_lib_nopmod_map(is.m_lib_nopmod_map)
{
  m_weight_ndx = new cWeightedIndex(*is.m_weight_ndx);
}

cInstSet::cInstSet(const cInstSet* is)
: m_world(is->m_world)
, m_inst_lib(is->m_inst_lib) 
, m_lib_name_map(is->m_lib_name_map)
, m_lib_nopmod_map(is->m_lib_nopmod_map)
{
  m_weight_ndx = new cWeightedIndex(*(*is).m_weight_ndx);
}


cInstSet& cInstSet::operator=(const cInstSet& _in)
{
  m_inst_lib = _in.m_inst_lib;
  m_lib_name_map = _in.m_lib_name_map;
  m_lib_nopmod_map = _in.m_lib_nopmod_map;
  m_weight_ndx = new cWeightedIndex(*_in.m_weight_ndx);
  return *this;
}



bool cInstSet::OK() const
{
  assert(m_lib_nopmod_map.GetSize() < m_lib_name_map.GetSize());
  return true;
}


cInstruction cInstSet::GetRandomInst(cAvidaContext& ctx) const
{
  int ndx = m_weight_ndx->FindPosition( ctx.GetRandom().GetDouble( m_weight_ndx->GetTotalWeight()) );
  return cInstruction(ndx);
}



cInstruction cInstSet::ActivateNullInst()
{  
  const int inst_id = m_lib_name_map.GetSize();
  const int null_fun_id = m_inst_lib->GetInstNull().GetOp();
  
  assert(inst_id < 255);
  
  // Make sure not to activate again if NULL is already active
  for (int i = 0; i < inst_id; i++) if (m_lib_name_map[i].lib_fun_id == null_fun_id) return cInstruction(i);
  
  
  // Increase the size of the array...
  m_lib_name_map.Resize(inst_id + 1);
  
  // Setup the new function...
  m_lib_name_map[inst_id].lib_fun_id = null_fun_id;
  m_lib_name_map[inst_id].redundancy = 0;
  m_lib_name_map[inst_id].cost = 0;
  m_lib_name_map[inst_id].ft_cost = 0;
  m_lib_name_map[inst_id].energy_cost = 0;
  m_lib_name_map[inst_id].prob_fail = 0.0;
  m_lib_name_map[inst_id].addl_time_cost = 0;
  
  return cInstruction(inst_id);
}


cString cInstSet::FindBestMatch(const cString& in_name) const
{
  int best_dist = 1024;
  cString best_name("");
  
  for (int i = 0; i < m_lib_name_map.GetSize(); i++) {
    const cString & cur_name = m_inst_lib->GetName(m_lib_name_map[i].lib_fun_id);
    const int cur_dist = cStringUtil::EditDistance(cur_name, in_name);
    if (cur_dist < best_dist) {
      best_dist = cur_dist;
      best_name = cur_name;
    }
    if (cur_dist == 0) break;
  }

  return best_name;
}

bool cInstSet::InstInSet(const cString& in_name) const
{
  cString best_name("");
  
  for (int i = 0; i < m_lib_name_map.GetSize(); i++) {
    const cString & cur_name = m_inst_lib->GetName(m_lib_name_map[i].lib_fun_id);
    if (cur_name == in_name) return true;
  }
  return false;
}

void cInstSet::LoadFromFile(const cString& filename)
{
  cInitFile file(filename);
  if (!file.WasOpened()) {
    tConstListIterator<cString> err_it(file.GetErrors());
    const cString* errstr = NULL;
    while ((errstr = err_it.Next())) m_world->GetDriver().RaiseException(*errstr);
    m_world->GetDriver().RaiseFatalException(1, cString("Unable to load instruction set '") + filename + "'.");
  }
   
  cStringList sl;
  for (int line_id = 0; line_id < file.GetNumLines(); line_id++) {
    sl.PushRear(file.GetLine(line_id));
  }
  
  LoadWithStringList(sl);
}


void cInstSet::LoadFromConfig()
{
  LoadWithStringList(m_world->GetConfig().INST_SET_NEW.Get());
}

void cInstSet::LoadWithStringList(const cStringList& sl)
{
  cArgSchema schema;
  
  // Integer
  schema.AddEntry("cost", 1, 0);
  schema.AddEntry("initial_cost", 2, 0);
  schema.AddEntry("energy_cost", 3, 0);
  schema.AddEntry("addl_time_cost", 4, 0);
  
  // Double
  schema.AddEntry("redundancy", 0, 1.0);
  schema.AddEntry("prob_fail", 0, 0.0);
  
  // String  
  schema.AddEntry("inst_code", 0, "");
  
  
  // Ensure that the instruction code length is in the range of bits supported by the int type
  int inst_code_len = m_world->GetConfig().INST_CODE_LENGTH.Get();
  if ((unsigned)inst_code_len > (sizeof(int) * 8)) inst_code_len = sizeof(int) * 8;
  else if (inst_code_len <= 0) inst_code_len = 1;
  
  tList<cString> errors;
  bool success = true;
  for (int line_id = 0; line_id < sl.GetSize(); line_id++) {
    cString cur_line = sl.GetLine(line_id);
    
    // Look for the INST keyword at the beginning of each line, and ignore if not found.
    cString inst_name = cur_line.PopWord();
    if (inst_name != "INST") continue;
    
    // Lookup the instruction name in the library
    inst_name = cur_line.PopWord();
    int fun_id = m_inst_lib->GetIndex(inst_name);
    if (fun_id == -1) {
      // Oh oh!  Didn't find an instruction!
      cString* errorstr = new cString("Unknown instruction '");
      *errorstr += inst_name + "' (Best match = '" + m_inst_lib->GetNearMatch(inst_name) + "').";
      errors.PushRear(errorstr);
      success = false;
      continue;
    }
    
    // Load the arguments for this instruction
    cArgContainer* args = cArgContainer::Load(cur_line, schema, &errors);
    if (!args) {
      success = false;
      continue;
    }
    
    // Check to make sure we are not inserting the special NULL instruction
    if (fun_id == m_inst_lib->GetInstNull().GetOp()) {
      errors.PushRear(new cString("Invalid use of NULL instruction"));
      success = false;
      continue;
    }
    
    double redundancy = args->GetInt(0);
    if (redundancy < 0.0) {
      m_world->GetDriver().NotifyWarning(cString("Instruction '") + inst_name + "' has negative redundancy, ignoring.");
      continue;
    }
    
    
    
    // Get the ID of the new Instruction
    const int inst_id = m_lib_name_map.GetSize();
    assert(inst_id < 255);
    
    // Increase the size of the array...
    m_lib_name_map.Resize(inst_id + 1);
    
    // Setup the new function...
    m_lib_name_map[inst_id].lib_fun_id = fun_id;
    m_lib_name_map[inst_id].redundancy = redundancy;
    m_lib_name_map[inst_id].cost = args->GetInt(1);
    m_lib_name_map[inst_id].ft_cost = args->GetInt(2);
    m_lib_name_map[inst_id].energy_cost = args->GetInt(3);
    m_lib_name_map[inst_id].prob_fail = args->GetDouble(0);
    m_lib_name_map[inst_id].addl_time_cost = args->GetInt(4);
    
    
    // Parse the instruction code
    cString inst_code = args->GetString(0);
    if (inst_code == "") {
      switch (m_world->GetConfig().INST_CODE_DEFAULT_TYPE.Get()) {
        case INST_CODE_ZEROS:
          m_lib_name_map[inst_id].inst_code = 0;
          break;
        case INST_CODE_INSTNUM:
          m_lib_name_map[inst_id].inst_code = ((~0) >> ((sizeof(int) * 8) - inst_code_len)) & inst_id;
          break;
        default:
          errors.PushRear(new cString("Invalid default instruction code type."));
          success = false;
          break;
      }
    } else {
      int inst_code_val = 0;
      for (int i = 0; i < inst_code_len && i < inst_code.GetSize(); i++) {
        inst_code_val <<= 1;
        if (inst_code[i] == '1') inst_code_val |= 1;
        else if (inst_code[i] != '0') {
          errors.PushRear(new cString("Invalid character in instruction code, must be 0 or 1."));
          success = false;
          break;
        }
      }
      
      m_lib_name_map[inst_id].inst_code = inst_code_val;
    }
    

    // If this is a nop, add it to the proper mappings
    if ((*m_inst_lib)[fun_id].IsNop()) {
      // Assert nops are at the _beginning_ of an inst_set.
      if (m_lib_name_map.GetSize() != (m_lib_nopmod_map.GetSize() + 1)) {
        errors.PushRear(new cString("Invalid NOP placement, all NOPs must be listed first."));
        success = false;
      }

      m_lib_nopmod_map.Resize(inst_id + 1);
      m_lib_nopmod_map[inst_id] = fun_id;
    }
    
    
    // Clean up the argument container for this instruction
    delete args;
  }
  
  
  if (!success) {
    cString* errstr = NULL;
    while ((errstr = errors.Pop())) {
      m_world->GetDriver().RaiseException(*errstr);
      delete errstr;
    }
    m_world->GetDriver().RaiseFatalException(1,"Failed to load instruction set due to previous errors.");
  }
  
  //Instruction set loaded okay.
  SynchRedundancyWeights();  
}


 
void cInstSet::LoadFromLegacyFile(const cString& filename)
{
  cInitFile file(filename);
  
  if (file.WasOpened() == false) {
    tConstListIterator<cString> err_it(file.GetErrors());
    const cString* errstr = NULL;
    while ((errstr = err_it.Next())) m_world->GetDriver().RaiseException(*errstr);
    m_world->GetDriver().RaiseFatalException(1, cString("Could not open instruction set '") + filename + "'.");
  }
  
  for (int line_id = 0; line_id < file.GetNumLines(); line_id++) {
    cString cur_line = file.GetLine(line_id);
    cString inst_name = cur_line.PopWord();
    int redundancy = cur_line.PopWord().AsInt();
    int cost = cur_line.PopWord().AsInt();
    int ft_cost = cur_line.PopWord().AsInt();
    int energy_cost = cur_line.PopWord().AsInt();
    double prob_fail = cur_line.PopWord().AsDouble();
    int addl_time_cost = cur_line.PopWord().AsInt();
    
    // If this instruction has 0 redundancy, we don't want it!
    if (redundancy < 0) continue;
    if (redundancy > 256) {
      cString msg("Max redundancy is 256.  Resetting redundancy of \"");
      msg += inst_name; msg += "\" from "; msg += redundancy; msg += " to 256.";
      m_world->GetDriver().NotifyWarning(msg);
      redundancy = 256;
    }
    
    // Otherwise, this instruction will be in the set.
    // First, determine if it is a nop...
    int fun_id = m_inst_lib->GetIndex(inst_name);
    
    if (fun_id == -1) {
      // Oh oh!  Didn't find an instruction!
      cString errorstr("Could not find instruction '");
      errorstr += inst_name + "'\n        (Best match = '" + m_inst_lib->GetNearMatch(inst_name) + "').";
      m_world->GetDriver().RaiseFatalException(1, errorstr);
    }

    
    
    if (fun_id == m_inst_lib->GetInstNull().GetOp())
      m_world->GetDriver().RaiseFatalException(1,"Invalid use of NULL instruction");
    
    const int inst_id = m_lib_name_map.GetSize();
    
    assert(inst_id < 255);
    
    // Increase the size of the array...
    m_lib_name_map.Resize(inst_id + 1);
    
    // Setup the new function...
    m_lib_name_map[inst_id].lib_fun_id = fun_id;
    m_lib_name_map[inst_id].redundancy = redundancy;
    m_lib_name_map[inst_id].cost = cost;
    m_lib_name_map[inst_id].ft_cost = ft_cost;
    m_lib_name_map[inst_id].energy_cost = energy_cost;
    m_lib_name_map[inst_id].prob_fail = prob_fail;
    m_lib_name_map[inst_id].addl_time_cost = addl_time_cost;
    m_lib_name_map[inst_id].inst_code = 0;
    

    if ((*m_inst_lib)[fun_id].IsNop()) {
      // Assert nops are at the _beginning_ of an inst_set.
      assert(m_lib_name_map.GetSize() == (m_lib_nopmod_map.GetSize() + 1));
      
      m_lib_nopmod_map.Resize(inst_id + 1);
      m_lib_nopmod_map[inst_id] = fun_id;
    }
  }
  
  SynchRedundancyWeights();
}


void cInstSet::SynchRedundancyWeights()
{
  if (m_weight_ndx != NULL)
    delete m_weight_ndx;
  m_weight_ndx = new cWeightedIndex(GetSize());
  for (int k = 0; k < GetSize(); k++)
    m_weight_ndx->SetWeight(k, m_lib_name_map[k].redundancy);
}
