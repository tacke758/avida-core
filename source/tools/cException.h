#ifndef cException_h
#define cException_h

#include <exception>

class cRandomDebugException : std::exception
{
  const char *m_msg;
public:
  cRandomDebugException(const char* msg = 0):m_msg(msg){}
  virtual ~cRandomDebugException() throw() {}
  virtual char const* what() const throw() {
    return (m_msg)?(m_msg):("cRandomDebugException occurred.");
  }
};

#endif /* cException_h */
