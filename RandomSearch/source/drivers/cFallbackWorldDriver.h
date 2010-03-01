/*
 *  cFallbackWorldDriver.h
 *  Avida
 *
 *  Created by David on 12/10/05.
 *  Copyright 1999-2007 Michigan State University. All rights reserved.
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

#ifndef cFallbackWorldDriver_h
#define cFallbackWorldDriver_h

#ifndef cDriverManager_h
#include "cDriverManager.h"
#endif
#ifndef cWorldDriver_h
#include "cWorldDriver.h"
#endif

class cFallbackWorldDriver : public cWorldDriver
{
private:
  cFallbackWorldDriver(const cFallbackWorldDriver&); // @not_implemented
  cFallbackWorldDriver& operator=(const cFallbackWorldDriver&); // @not_implemented
  
public:
  cFallbackWorldDriver() { cDriverManager::Register(this); }
  ~cFallbackWorldDriver() { cDriverManager::Unregister(this); }
  
  // Driver Actions
  void SignalBreakpoint() { return; }
  void SetDone() { return; }
  
  void RaiseException(const cString& in_string);
  void RaiseFatalException(int exit_code, const cString& in_string);
  
  // Notifications
  void NotifyComment(const cString& in_string);
  void NotifyWarning(const cString& in_string);
};


#ifdef ENABLE_UNIT_TESTS
namespace nFallbackWorldDriver {
  /**
   * Run unit tests
   *
   * @param full Run full test suite; if false, just the fast tests.
   **/
  void UnitTests(bool full = false);
}
#endif  

#endif