/*
 *  avida.h
 *  Avida
 *
 *  Called "avida.hh" prior to 12/2/05.
 *  Copyright 1999-2008 Michigan State University. All rights reserved.
 *  Copyright 1993-2001 California Institute of Technology.
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

#ifndef avida_h
#define avida_h

class cAvidaConfig;
class cString;
class cStringList;

namespace Avida
{
  cString GetVersion();
  void PrintVersionBanner();
  
  void ProcessArgs(cStringList &argv, cAvidaConfig* cfg);
  void ProcessCmdLineArgs(int argc, char* argv[], cAvidaConfig* cfg);

  //! This function properly shuts down the Avida program.
  void Exit(int exit_code);
};

#endif