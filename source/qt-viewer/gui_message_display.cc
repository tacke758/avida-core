
#ifndef STRING_HH
#include "string.hh"
#endif
#ifndef MESSAGE_DISPLAY_HH
#include "message_display.hh"
#endif

#include <qstring.h>


using namespace std;


cMessageClosure &
cMessageClosure::operator<<(const QString &s){
  prefix();
  if(QString::null == s){
    _msg += "(null)";
  } else {
    _msg += s.latin1();
  }
  return *this;
}
