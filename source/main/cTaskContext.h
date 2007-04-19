/*
 *  cTaskContext.h
 *  Avida
 *
 *  Created by David on 3/29/06.
 *  Copyright 2006 Michigan State University. All rights reserved.
 *
 */

#ifndef cTaskContext_h
#define cTaskContext_h

#ifndef tBuffer_h
#include "tBuffer.h"
#endif
#ifndef tList_h
#include "tList.h"
#endif

class cTaskEntry;
class cOrganism;

class cTaskContext
{
  friend class cTaskLib;
private:
  const tBuffer<int>& input_buffer;
  const tBuffer<int>& output_buffer;
  const tList<tBuffer<int> >& other_input_buffers;
  const tList<tBuffer<int> >& other_output_buffers;
  bool net_valid;
  int net_completed;
  tBuffer<int>* received_messages;
  int logic_id;
  int task_success_complete;
  
  cTaskEntry* task_entry;
  cOrganism* organism;

public:

  cTaskContext(const tBuffer<int>& inputs, const tBuffer<int>& outputs, const tList<tBuffer<int> >& other_inputs,
               const tList<tBuffer<int> >& other_outputs, bool in_net_valid, int in_net_completed, 
               tBuffer<int>* in_received_messages = NULL, cOrganism* in_org = NULL)
    : input_buffer(inputs), output_buffer(outputs), other_input_buffers(other_inputs),
    other_output_buffers(other_outputs), net_valid(in_net_valid), net_completed(in_net_completed), 
    received_messages(in_received_messages), organism(in_org), logic_id(0), task_entry(NULL), task_success_complete(0)
  {
  }
 
  void SetTaskEntry(cTaskEntry* in_entry) { task_entry = in_entry; }
};


#ifdef ENABLE_UNIT_TESTS
namespace nTaskContext {
  /**
   * Run unit tests
   *
   * @param full Run full test suite; if false, just the fast tests.
   **/
  void UnitTests(bool full = false);
}
#endif  

#endif
