/*
 *  cEventContext.h
 *  Avida
 *
 *  Created by Matthew Rupp on 2/4/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#ifndef cEventContext_h
#define cEventContext_h


#ifndef defs_h
#include "defs.h"
#endif 

#ifndef tDictionary_h
#include "tDictionary.h"
#endif

#ifndef cString_h
#include "cString.h"
#endif

#ifndef cAvidaContext_h
#include "cAvidaContext.h"
#endif

#ifndef cFlexVar_h
#include "cFlexVar.h"
#endif


class cCntxEntry{
private:
  
  cString  m_name;
  cFlexVar m_value;
  cCntxEntry() {;}
public:
  cCntxEntry(const char* name, const cFlexVar& value) : m_name(cString(name)), m_value(value) {;}
  cCntxEntry(const cString& name, const cFlexVar& value) : m_name(name), m_value(value) {;}
  cCntxEntry(const cCntxEntry& in) : m_name(in.m_name), m_value(in.m_value) {;}
  const cString& GetName() const { return m_name;} 
  const cFlexVar& GetValue() const {return m_value;} 
};


class cEventContext : public cAvidaContext
{
  private:
    eEventTrigger m_trigger;
    tDictionary< cCntxEntry* > m_state_info;
  
  public:
    cEventContext(cAvidaContext& ctx, eEventTrigger id = TRIGGER_UNKNOWN) : cAvidaContext(ctx), m_trigger(id) {;}
    ~cEventContext();
    const cFlexVar* operator[](const cString& name);
    cEventContext& operator<<(const cCntxEntry& entry);
    void SetEventTrigger(eEventTrigger id) {m_trigger = id;}
    eEventTrigger GetEventTrigger() const {return m_trigger;}
};

#endif
