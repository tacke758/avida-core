#ifndef MESSAGE_DISPLAY_HH
#define MESSAGE_DISPLAY_HH

#ifndef STRING_HH
#warning include "string.hh" (provides cString) to use me.
#endif
#ifndef _CPP_IOSTREAM
#include <iostream>
#endif

class cString;


/*
The message display classes dependency hierarchy:
  class cMessageClosure
    class cMessageType
      class cMessages
        struct cMessage
      class cMessageClass
        class cMessageDisplay
*/


/*
cMessageDisplay displays the message "final_msg" to the user/programmer.
cMessageDisplay is meant to be subclassed, where
  out(string final_msg)
is reimplemented to display "final_msg" in the desired manner. 

Instances of the subclass may then be passed to
  setStandardOutDisplay(cMessageDisplay &)
and
  setStandardErrDisplay(cMessageDisplay &)
with expected results.
*/
class cMessageDisplay{
public:
  virtual void out(cString &final_msg){}
  virtual void abort(){ abort(); }
};

/*
This subclass of cMessageDisplay just prints to stdout and stderr.

cMessageDisplay displays the message "final_msg" to the user/programmer.
cMessageDisplay is meant to be subclassed, where
  out(string final_msg)
is reimplemented to display "final_msg" in the desired manner. 

Instances of the subclass may then be passed to
  setStandardOutDisplay(cMessageDisplay &)
and
  setStandardErrDisplay(cMessageDisplay &)
with expected results.
*/
class cDefaultMessageDisplay : public cMessageDisplay{
private:
  std::ostream *_out;
public:
  cDefaultMessageDisplay(std::ostream *stream):_out(stream){}
  void out(cString &final_msg);
};

extern cDefaultMessageDisplay s_info_msg_cout;
extern cDefaultMessageDisplay s_debug_msg_cerr;
extern cDefaultMessageDisplay s_error_msg_cerr;
extern cDefaultMessageDisplay s_fatal_msg_cerr;
extern cDefaultMessageDisplay s_noprefix_msg_cout;

void setInfoDisplay(cMessageDisplay &info);
void setDebugDisplay(cMessageDisplay &debug);
void setErrorDisplay(cMessageDisplay &error);
void setFatalDisplay(cMessageDisplay &fatal);
void setNoPrefixDisplay(cMessageDisplay &noprefix);


class cMessageType;
class cMessageClass{
public:
  cMessageClass(
    const char *class_name,
    cMessageDisplay **msg_display,
    bool is_fatal,
    bool is_prefix,
    bool no_prefix
  );
public:
  void configure(cMessageType *message_type);
public:
  const char *const m_class_name;
  cMessageDisplay **m_msg_display;
  bool const m_is_fatal;
  bool const m_is_prefix;
  bool const m_no_prefix;
private:
  bool _configured;
};


/*
Declaration of the five message classes.
*/
extern cMessageClass MCInfo;
extern cMessageClass MCDebug;
extern cMessageClass MCError;
extern cMessageClass MCFatal;
extern cMessageClass MCNoPrefix;



class cMessageType{
public:
  cMessageType(
    const char *type_name,
    cMessageClass &message_class
  );
public:
  const char *m_type_name;
  const cMessageClass &m_message_class;
  bool m_is_active;
};


class QString;
class cMessageClosure{
private:
  cMessageType &_type;
  const char *_function;
  const char *_file;
  int _line;
  int _error;
  int _op_count;
  cString _msg;
public:
  cMessageClosure(
    cMessageType &type,
    const char *function_name,
    const char *file_name,
    int line_number
  );
  ~cMessageClosure(void);
public:
  /*
  operator<< and operator() definitions for cMessageClosure.

  Any kind of input can be placed behind the operator<<, really, but try
  to not (further) overcrowd the class...

  The operator is meant to convert its input into a string for display.
  If for example you implement
    cMessageClosure &operator<<(const myClass &instance);
  then you can write a message thus:
    DemoDebug << "this is a myClass instance: " << my_instance;
  with the result:
    "Debug [Demo] this is a myClass instance: stuff in my_instance."
  */
  cMessageClosure &operator<<(char c);
  cMessageClosure &operator<<(unsigned char c);
  cMessageClosure &operator<<(int i);
  cMessageClosure &operator<<(unsigned int i);
  cMessageClosure &operator<<(long i);
  cMessageClosure &operator<<(unsigned long i);
  cMessageClosure &operator<<(float f);
  cMessageClosure &operator<<(double f);
  cMessageClosure &operator<<(const void *p);
  cMessageClosure &operator<<(const char *s);
  cMessageClosure &operator<<(const QString &s);
  /*
  QString support is implemented outside of core of avida, in qt gui, so
  that QString can remain undefined in gui-less builds.  -- kgn
  */

  cMessageClosure &operator()(char c){ return operator<<(c); }
  cMessageClosure &operator()(unsigned char c){ return operator<<(c); }
  cMessageClosure &operator()(int i){ return operator<<(i); }
  cMessageClosure &operator()(unsigned int i){ return operator<<(i); }
  cMessageClosure &operator()(long i){ return operator<<(i); }
  cMessageClosure &operator()(unsigned long i){ return operator<<(i); }
  cMessageClosure &operator()(float f){ return operator<<(f); }
  cMessageClosure &operator()(double f){ return operator<<(f); }
  cMessageClosure &operator()(const void *p){ return operator<<(p); }
  cMessageClosure &operator()(const char *s){ return operator<<(s); }
  cMessageClosure &operator()(const QString &s){ return operator<<(s); }

  /*
  cMessageClosure::va():
  
  Support for cString-based operator<< and operator() definitions in
  cMessageClosure.  Variable argument lists are formatted and printed, a
  la printf, into a temporary buffer; the contents of this buffer are
  then appended to the cString _msg.
  */
  cMessageClosure &va(const char *fmt, ...);
private:
  void prefix(void);
};


/*
you probably don't need to change these macros...
*/

// window compatibility
#ifdef WIN32
# ifndef __PRETTY_FUNCTION__
#  define __PRETTY_FUNCTION__ "<unknown on win32>"
# endif
#endif

#define _MSG_CLOSURE_FL(x)\
  cMessageClosure(\
    x,\
    __PRETTY_FUNCTION__,\
    __FILE__,\
    __LINE__\
  )

#define _INFO_MSG(type)\
  if (Info_ ## type ## _Msg.m_is_active)\
    _MSG_CLOSURE_FL(Info_ ## type ## _Msg)

#define _DEBUG_MSG(type)\
  if (Debug_ ## type ## _Msg.m_is_active)\
    _MSG_CLOSURE_FL(Debug_ ## type ## _Msg)

#define _ERROR_MSG(type)\
  if (Error_ ## type ## _Msg.m_is_active)\
    _MSG_CLOSURE_FL(Error_ ## type ## _Msg)

#define _FATAL_MSG(type)\
  if (Fatal_ ## type ## _Msg.m_is_active)\
    _MSG_CLOSURE_FL(Fatal_ ## type ## _Msg)

#define _PLAIN_MSG(type)\
  if (type ## _Msg.m_is_active)\
    _MSG_CLOSURE_FL(type ## _Msg)



/*
I've made the one message type "NoPrefix_Msg" permanently available, and
the macro to trigger such a message is "Message".

* added general message types GenInfo, GenDebug, GenError, GenFatal.

Each message type that you define should be accompanied by a macro.
message types come in sets of four -- the Info, Debug, Error, and Fatal
types.  For example, to define a "Demo" message type set, declare the
message types like so:
  extern cMessageType Info_Demo_Msg;
  extern cMessageType Debug_Demo_Msg;
  extern cMessageType Error_Demo_Msg;
  extern cMessageType Fatal_Demo_Msg;
with corresponding macros
  #define DemoInfo _INFO_MSG(Demo)
  #define DemoDebug _DEBUG_MSG(Demo)
  #define DemoError _ERROR_MSG(Demo)
  #define DemoFatal _FATAL_MSG(Demo)
and definitions
  cMessageType Info_Demo_Msg("Demo", MCInfo);
  cMessageType Debug_Demo_Msg("Demo", MCDebug);
  cMessageType Error_Demo_Msg("Demo", MCError);
  cMessageType Fatal_Demo_Msg("Demo", MCFatal);
*/
extern cMessageType NoPrefix_Msg;
extern cMessageType Info_GEN_Msg;
extern cMessageType Debug_GEN_Msg;
extern cMessageType Error_GEN_Msg;
extern cMessageType Fatal_GEN_Msg;
#define Message _PLAIN_MSG(NoPrefix)
#define GenInfo _INFO_MSG(GEN)
#define GenDebug _DEBUG_MSG(GEN)
#define GenError _ERROR_MSG(GEN)
#define GenFatal _FATAL_MSG(GEN)



#endif /* !MESSAGE_DISPLAY_HH */
