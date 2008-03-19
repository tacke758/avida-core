//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "task_buffer.hh"


cIOBuf::cIOBuf(const int _num_tasks, const int _capacity) :
 capacity(_capacity), num(0), offset(0), num_tasks(_num_tasks) {
   assert( num_tasks > 0 );
   assert( _capacity > 0 );
   data = new UINT[capacity];
   last_task_add = new int[num_tasks];
   Clear();
}

cIOBuf::cIOBuf(const cIOBuf & in){
  int i;
  num = in.num;
  offset = in.offset;
  capacity = in.capacity;
  num_tasks = in.num_tasks;
  data = new UINT[capacity];
  for( i=0; i<capacity; ++i ){
    data[i] = in.data[i];
  }
  last_task_add = new int[num_tasks];
  for( i=0; i<num_tasks; ++i ){
    last_task_add[i] = in.last_task_add[i];
  }
}

void cIOBuf::Clear(){
  for( int i=0; i<num_tasks; ++i ){
    last_task_add[i] = 0;
  }
  offset = 0;
  num = 0;
}


cIOBuf & cIOBuf::operator= (const cIOBuf & in){
  int i;
  num = in.num;
  offset = in.offset;
  if( capacity != in.capacity ){
    delete [] data;
    capacity = in.capacity;
    data = new UINT[capacity];
  }
  for( i=0; i<capacity; ++i ){
    data[i] = in[i];
  }
  if( num_tasks != in.num_tasks ){
    delete [] last_task_add;
    num_tasks = in.num_tasks;
    last_task_add = new int[num_tasks];
  }
  for( i=0; i<num_tasks; ++i ){
    last_task_add[i] = in.last_task_add[i];
  }
  return *this;
}




cTaskCount::cTaskCount(const int _num_tasks) : num_tasks(_num_tasks)
{
  assert( num_tasks >= 0 );
  count = new int[num_tasks];
  for( int i=0; i<num_tasks; ++i) {
    count[i] = 0;
  }
}

cTaskCount::cTaskCount(const cTaskCount & _in) : num_tasks(_in.num_tasks)
{
  count = new int[num_tasks];
  for (int i = 0; i < num_tasks; i++) {
    count[i] = _in.count[i];
  }
}

cTaskCount::~cTaskCount()
{
  delete [] count;
}

cTaskCount & cTaskCount::operator=(const cTaskCount & _in)
{
  num_tasks = _in.num_tasks;
  delete [] count;
  count = new int[num_tasks];
  for (int i = 0; i < num_tasks; i++) {
    count[i] = _in.count[i];
  }
  return *this;
}

void cTaskCount::Clear()
{
  for( int i=0; i<num_tasks; ++i ){
    count[i] = 0;
  }
  total_num_tasks_done = 0;
}




void cIOBuf::SaveState(ostream & fp){
  assert(fp.good());
  fp<<"cIOBuf"<<" ";

  fp<<num<<" ";
  fp<<offset<<" ";

  fp<<capacity<<" ";
  for( int i=0; i<capacity; ++i ){
    fp<<data[i]<<" ";
  }

  fp<<num_tasks<<" ";
  for( int i=0; i<num_tasks; ++i ){
    fp<<last_task_add[i]<<" ";
  }
}


void cIOBuf::LoadState(istream & fp){
  assert(fp.good());
  cString foo;
  fp>>foo;
  assert(foo == "cIOBuf");

  fp>>num;
  fp>>offset;

  fp>>capacity;
  if( data != NULL ) delete data;
  data = new UINT[capacity];
  for( int i=0; i<capacity; ++i ){
    fp>>data[i];
  }

  fp>>num_tasks;
  if( last_task_add != NULL ) delete last_task_add;
  last_task_add = new int[num_tasks];
  for( int i=0; i<num_tasks; ++i ){
    fp>>last_task_add[i];
  }
}




