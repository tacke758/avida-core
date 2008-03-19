/* task_buffer.hh ************************************************************
 IO-Buffers and Task Count

 Time-stamp: <98/07/20 17:36:07 travc>
*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1998 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////


#ifndef TASK_BUFFER_HH
#define TASK_BUFFER_HH

#include "../defs.hh"
#include "../tools/assert.hh"
#include "../tools/string.hh"

#ifndef UINT
#define UINT unsigned int
#endif


class cIOBuf {
private:
  int capacity;
  int num;
  int offset;
  UINT * data;
  
  int num_tasks;
  int * last_task_add;
  
public:
  cIOBuf(const int _num_tasks, const int _capacity); 
  cIOBuf(const cIOBuf & in); 
  cIOBuf & operator= (const cIOBuf & in);

  ~cIOBuf(){ delete [] data; delete [] last_task_add; }

  int GetCapacity() const { return capacity; }

  void Add(UINT in){
    data[offset] = in;
    ++num;
    ++offset;
    while( offset >= capacity )  offset-=capacity;
  }

  void ZeroNumAdds(const int task_num){   
    assert( task_num>=0  &&  task_num<num_tasks );
    last_task_add[task_num] = num;
  }

  int GetTotal() const { return num; }
  int GetNum(const int task_num) const { 
    assert( task_num>=0  &&  task_num<num_tasks );
    return num - last_task_add[task_num]; }

  void Clear(); 

    
  UINT operator[] (int i) const {
    assert( i>=0  &&  i<capacity );
    int index = offset - i - 1;
    while( index < 0 )  index += capacity;
    assert( index>=0  &&  index<capacity );
    return data[index];
  }

};
    



class cTaskCount {
private:
  const int num_tasks;
  int * count;
  int total_num_tasks_done;

public:
  cTaskCount(const int num_tasks);
  ~cTaskCount();

  // Accessing the actual counts!
  int GetCount(const int i) const {
    assert( i>=0  &&  i<num_tasks ); 
    return count[i];
  }

  int operator[] (const int i) const { return GetCount(i); }

  int GetNumTasks() const { return num_tasks; }
  int GetTotalNumTasksDone() const { return total_num_tasks_done; }

  void Clear();


  // *** Below called by Task_Lib only! *** //
  // Record competed task
  void IncCount(const int i){
    assert( i>=0  &&  i<num_tasks );
    ++total_num_tasks_done;
    ++count[i];
  }
};


#endif