/*
 *  ASAvidaLib.cc
 *  Avida
 *
 *  Created by David on 10/10/08.
 *  Copyright 2008 Michigan State University. All rights reserved.
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

#include "ASAvidaLib.h"

#include "cASCPPParameter_NativeObjectSupport.h"
#include "cASLibrary.h"

#include "cAvidaConfig.h"
#include "cDefaultRunDriver.h"
#include "cWorld.h"

#include <cstring>

#define AS_DECLARE_NATIVE_OBJECT(CLASS, NAME) \
 namespace AvidaScript { template<> inline sASTypeInfo TypeOf<CLASS*>() { return sASTypeInfo(AS_TYPE_OBJECT_REF, #NAME); } }


AS_DECLARE_NATIVE_OBJECT(cAvidaConfig,       Config);
AS_DECLARE_NATIVE_OBJECT(cDefaultRunDriver,  Driver);
AS_DECLARE_NATIVE_OBJECT(cWorld,             World);


static void setupNativeObjects()
{
  tASNativeObject<cAvidaConfig>::InitializeMethodRegistrar();
  tASNativeObject<cAvidaConfig>::
    RegisterMethod(new tASNativeObjectMethod<cAvidaConfig, cString (const cString&)>(&cAvidaConfig::GetAsString), "Get");
  
  
  tASNativeObject<cDefaultRunDriver>::InitializeMethodRegistrar();
  tASNativeObject<cDefaultRunDriver>::
    RegisterMethod(new tASNativeObjectMethod<cDefaultRunDriver, void ()>(&cDefaultRunDriver::Run), "Run");

  
  tASNativeObject<cWorld>::InitializeMethodRegistrar();
};

#undef AS_DECLARE_NATIVE_OBJECT

template<class FunctionType> class tASNativeObjectInstantiate;

template<class NativeClass> 
class tASNativeObjectInstantiate<NativeClass ()> : public cASFunction
{
private:
  sASTypeInfo m_signature;
  
public:
  tASNativeObjectInstantiate() : cASFunction(AvidaScript::TypeOf<NativeClass*>().info)
  {
    m_rtype = AvidaScript::TypeOf<NativeClass*>();
    m_signature = AvidaScript::TypeOf<void>();
  }
  
  int GetArity() const { return 0; }
  const sASTypeInfo& GetArgumentType(int arg) const { return m_signature; }
  
  cASCPPParameter Call(cASCPPParameter args[]) const
  {
    cASCPPParameter rvalue;
    rvalue.Set(new tASNativeObject<NativeClass>(new NativeClass()));
    return rvalue;
  }
};


template<class NativeClass, typename Arg1Type> 
class tASNativeObjectInstantiate<NativeClass (Arg1Type)> : public cASFunction
{
private:
  sASTypeInfo m_signature;
  
public:
  tASNativeObjectInstantiate() : cASFunction(AvidaScript::TypeOf<NativeClass*>().info)
  {
    m_rtype = AvidaScript::TypeOf<NativeClass*>();
    m_signature = AvidaScript::TypeOf<Arg1Type>();
  }
  
  int GetArity() const { return 1; }
  const sASTypeInfo& GetArgumentType(int arg) const { return m_signature; }
  
  cASCPPParameter Call(cASCPPParameter args[]) const
  {
    cASCPPParameter rvalue;
    rvalue.Set(new tASNativeObject<NativeClass>(new NativeClass(args[0].Get<Arg1Type>())));
    return rvalue;
  }
};



void RegisterASAvidaLib(cASLibrary* lib)
{
  setupNativeObjects();
  
  lib->RegisterFunction(new tASNativeObjectInstantiate<cAvidaConfig ()>());
  lib->RegisterFunction(new tASNativeObjectInstantiate<cDefaultRunDriver (cWorld*)>());
  lib->RegisterFunction(new tASNativeObjectInstantiate<cWorld (cAvidaConfig*)>());
    // @TODO - world takes ownership of config, but I don't handle that here... world could delete it without AS knowing
}
