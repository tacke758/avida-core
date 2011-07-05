/*
 *  data/Manager.h
 *  avida-core
 *
 *  Created by David on 5/16/11.
 *  Copyright 2011 Michigan State University. All rights reserved.
 *  http://avida.devosoft.org/
 *
 *
 *  This file is part of Avida.
 *
 *  Avida is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 *  Avida is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License along with Avida.
 *  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Authors: David M. Bryson <david@programerror.com>
 *
 */

#ifndef AvidaDataManager_h
#define AvidaDataManager_h

#include "apto/platform.h"
#include "avida/core/Types.h"
#include "avida/core/World.h"
#include "avida/data/Types.h"


namespace Avida {
  namespace Data {
    
    // Data::Manager - Manages available and active data providers for a given world
    // --------------------------------------------------------------------------------------------------------------
    
    class Manager : public WorldFacet
    {
    private:
      World* m_world;
      Apto::Map<DataID, ProviderActivateFunctor> m_provider_map;
      mutable DataSetPtr m_available;
      
      Apto::Set<RecorderPtr> m_recorders;
      
      Apto::Array<ProviderPtr> m_active_providers;
      Apto::Map<DataID, ProviderPtr> m_active_map;
      
      mutable Apto::Map<DataID, PackagePtr> m_current_values;
      
    public:
      LIB_EXPORT Manager();
      LIB_EXPORT ~Manager();
      
      LIB_EXPORT ConstDataSetPtr GetAvailable() const;
      LIB_EXPORT bool IsAvailable(const DataID& data_id) const;
      LIB_EXPORT bool IsActive(const DataID& data_id) const;
      
      LIB_EXPORT bool AttachRecorder(RecorderPtr recorder);
      LIB_EXPORT bool DetachRecorder(RecorderPtr recorder);
      
      LIB_EXPORT bool Register(const DataID& data_id, ProviderActivateFunctor functor);
      
      LIB_EXPORT static bool Attach(ManagerPtr ptr, World* world);
      LIB_EXPORT static ManagerPtr Of(World* world);
      
    public:
      LIB_LOCAL void PerformUpdate(Avida::Context& ctx, Update current_update);
      LIB_LOCAL WorldFacetID UpdateBefore() const;
      LIB_LOCAL WorldFacetID UpdateAfter() const;
      
      LIB_LOCAL PackagePtr GetCurrentValue(const DataID& data_id) const;
    };
    
  };
};

#endif