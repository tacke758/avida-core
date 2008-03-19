//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

/** tasks.hh **
 *
 * A task library is used to keep track of tasks rewarded in a particular
 * environment, and detect which task (if any) a particular output corresponds
 * to.  It is initialized externally in the environment.
 *
 * cTaskEntry contains full information about a single task.
 *
 * cTaskLib::
 *  -- Setup --
 *  cTaskEntry * AddTask(const cString & name);
 *
 *  -- Access Task Info --
 *  int GetSize() const
 *  const cTaskEntry & GetTask(int id);
 *
 *  -- Task Testing Functionality --
 *  void SetupTests(const tBuffer<int> & inputs, const tBuffer<int> & outputs);
 *  double TestOutput(tTaskTest test_task);
 *
 * Notes:
 *  Groups of tasks will often use the same core calculations in order to
 *  determine the task that was performed.  For this reason, mutable internal
 *  states are kept so that such calculations need only be performed a single
 *  time.
 *
 *  Tasks have a double return value in the range of 0 to 1.  This represents
 *  the quality of the task performance, and multipes the bonus earned.
 *  Most tasks will only return a binary 0.0 or 1.0, but other options are
 *  possible.
 *
 **/

#ifndef TASKS_HH
#define TASKS_HH

#include <assert.h>
#include <math.h>

#include "../defs.hh"
#include "../tools/string.hh"
#include "../tools/tArray.hh"
#include "../tools/file.hh"
#include "../tools/tBuffer.hh"

class cTaskLib;
typedef double (cTaskLib::*tTaskTest)() const;

class cTaskEntry {
private:
  cString name;  // Short keyword for task
  cString desc;  // For more human-understandable output...
  int id;
  tTaskTest test_fun;
public:
  cTaskEntry(const cString & _name, const cString & _desc, int _id,
	     tTaskTest _test_fun);
  ~cTaskEntry();

  const cString & GetName()    const { return name; }
  const cString & GetDesc() const { return desc; }
  const int       GetID()      const { return id; }
  const tTaskTest GetTestFun() const { return test_fun; }
};

class cTaskLib {
private:
  tArray<cTaskEntry *> task_array;

  // Active task information...
  mutable tBuffer<int> input_buffer;
  mutable tBuffer<int> output_buffer;
  mutable int logic_id;
public:
  cTaskLib();
  ~cTaskLib();

  int GetSize() const { return task_array.GetSize(); }

  cTaskEntry * AddTask(const cString & name);
  const cTaskEntry & GetTask(int id) const;
  
  void SetupTests(const tBuffer<int> & inputs,
		  const tBuffer<int> & outputs) const;
  double TestOutput(const cTaskEntry & task) const;



private:  // Direct task related methods
  void NewTask(const cString & name, const cString & desc, tTaskTest task_fun);
  void SetupLogicTests(const tBuffer<int> & inputs,
		       const tBuffer<int> & outputs) const;

  double Task_Echo() const;
  double Task_Add()  const;
  double Task_Sub()  const;

  double Task_Not()    const;
  double Task_Nand()   const;
  double Task_And()    const;
  double Task_OrNot()  const;
  double Task_Or()     const;
  double Task_AndNot() const;
  double Task_Nor()    const;
  double Task_Xor()    const;
  double Task_Equ()    const;

  double Task_Logic3in_AA() const;
  double Task_Logic3in_AB() const;
  double Task_Logic3in_AC() const;
  double Task_Logic3in_AD() const;
  double Task_Logic3in_AE() const;
  double Task_Logic3in_AF() const;
  double Task_Logic3in_AG() const;
  double Task_Logic3in_AH() const;
  double Task_Logic3in_AI() const;
  double Task_Logic3in_AJ() const;
  double Task_Logic3in_AK() const;
  double Task_Logic3in_AL() const;
  double Task_Logic3in_AM() const;
  double Task_Logic3in_AN() const;
  double Task_Logic3in_AO() const;
  double Task_Logic3in_AP() const;
  double Task_Logic3in_AQ() const;
  double Task_Logic3in_AR() const;
  double Task_Logic3in_AS() const;
  double Task_Logic3in_AT() const;
  double Task_Logic3in_AU() const;
  double Task_Logic3in_AV() const;
  double Task_Logic3in_AW() const;
  double Task_Logic3in_AX() const;
  double Task_Logic3in_AY() const;
  double Task_Logic3in_AZ() const;
  double Task_Logic3in_BA() const;
  double Task_Logic3in_BB() const;
  double Task_Logic3in_BC() const;
  double Task_Logic3in_BD() const;
  double Task_Logic3in_BE() const;
  double Task_Logic3in_BF() const;
  double Task_Logic3in_BG() const;
  double Task_Logic3in_BH() const;
  double Task_Logic3in_BI() const;
  double Task_Logic3in_BJ() const;
  double Task_Logic3in_BK() const;
  double Task_Logic3in_BL() const;
  double Task_Logic3in_BM() const;
  double Task_Logic3in_BN() const;
  double Task_Logic3in_BO() const;
  double Task_Logic3in_BP() const;
  double Task_Logic3in_BQ() const;
  double Task_Logic3in_BR() const;
  double Task_Logic3in_BS() const;
  double Task_Logic3in_BT() const;
  double Task_Logic3in_BU() const;
  double Task_Logic3in_BV() const;
  double Task_Logic3in_BW() const;
  double Task_Logic3in_BX() const;
  double Task_Logic3in_BY() const;
  double Task_Logic3in_BZ() const;
  double Task_Logic3in_CA() const;
  double Task_Logic3in_CB() const;
  double Task_Logic3in_CC() const;
  double Task_Logic3in_CD() const;
  double Task_Logic3in_CE() const;
  double Task_Logic3in_CF() const;
  double Task_Logic3in_CG() const;
  double Task_Logic3in_CH() const;
  double Task_Logic3in_CI() const;
  double Task_Logic3in_CJ() const;
  double Task_Logic3in_CK() const;
  double Task_Logic3in_CL() const;
  double Task_Logic3in_CM() const;
  double Task_Logic3in_CN() const;
  double Task_Logic3in_CO() const;
  double Task_Logic3in_CP() const;

  double Task_Math1in_AA() const;
  double Task_Math1in_AB() const;
  double Task_Math1in_AC() const;
  double Task_Math1in_AD() const;
  double Task_Math1in_AE() const;
  double Task_Math1in_AF() const;
  double Task_Math1in_AG() const;
  double Task_Math1in_AH() const;
  double Task_Math1in_AI() const;
  double Task_Math1in_AJ() const;
  double Task_Math1in_AK() const;
  double Task_Math1in_AL() const;
  double Task_Math1in_AM() const;
  double Task_Math1in_AN() const;
  double Task_Math1in_AO() const;
  double Task_Math1in_AP() const;

  double Task_Math2in_AA() const;
  double Task_Math2in_AB() const;
  double Task_Math2in_AC() const;
  double Task_Math2in_AD() const;
  double Task_Math2in_AE() const;
  double Task_Math2in_AF() const;
  double Task_Math2in_AG() const;
  double Task_Math2in_AH() const;
  double Task_Math2in_AI() const;
  double Task_Math2in_AJ() const;
  double Task_Math2in_AK() const;
  double Task_Math2in_AL() const;
  double Task_Math2in_AM() const;
  double Task_Math2in_AN() const;
  double Task_Math2in_AO() const;
  double Task_Math2in_AP() const;
  double Task_Math2in_AQ() const;
  double Task_Math2in_AR() const;
  double Task_Math2in_AS() const;
  double Task_Math2in_AT() const;
  double Task_Math2in_AU() const;
  double Task_Math2in_AV() const;

  double Task_Math3in_AA() const;
  double Task_Math3in_AB() const;
  double Task_Math3in_AC() const;
  double Task_Math3in_AD() const;
  double Task_Math3in_AE() const;
  double Task_Math3in_AF() const;
  double Task_Math3in_AG() const;
  double Task_Math3in_AH() const;
  double Task_Math3in_AI() const;
  double Task_Math3in_AJ() const;
  double Task_Math3in_AK() const;
  double Task_Math3in_AL() const;
  double Task_Math3in_AM() const;
  
};

#endif
