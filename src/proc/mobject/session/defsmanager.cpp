/*
  DefsManager  -  access to preconfigured default objects and definitions
 
  Copyright (C)         Lumiera.org
    2008,               Hermann Vosseler <Ichthyostega@web.de>
 
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of the
  License, or (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
* *****************************************************/


#include "proc/mobject/session/defsmanager.hpp"
#include "proc/mobject/session/defsregistry.hpp"
#include "common/configrules.hpp"
#include "common/error.hpp"

#include <boost/format.hpp>

using boost::format;

using lumiera::ConfigRules;
using lumiera::query::QueryHandler;
using lumiera::query::LUMIERA_ERROR_CAPABILITY_QUERY;


namespace mobject
  {
  namespace session
    {
    using lumiera::P;
    
    
    
    /** initialize the most basic internal defaults. */
    DefsManager::DefsManager ()  throw()
      : defsRegistry(new DefsRegistry)
    {
      TODO ("setup basic defaults of the session");
    }
    
    
    template<class TAR>
    P<TAR>
    DefsManager::search  (const Query<TAR>& capabilities)
    {
      P<TAR> res;
      QueryHandler<TAR>& typeHandler = ConfigRules::instance();  
      for (DefsRegistry::Iter<TAR> i = defsRegistry->candidates(capabilities); 
           res = *i ; ++i )
        {
          typeHandler.resolve (res, capabilities);
          if (res)
            return res;
        }
      return res; // "no solution found"
    }
    
    
    template<class TAR>
    P<TAR> 
    DefsManager::create  (const Query<TAR>& capabilities)
    {
      P<TAR> res;
      QueryHandler<TAR>& typeHandler = ConfigRules::instance();  
      typeHandler.resolve (res, capabilities);
      if (res)
        defsRegistry->put (res, capabilities);
      return res;
    }
    
    
    template<class TAR>
    bool 
    DefsManager::define  (const P<TAR>& defaultObj, const Query<TAR>& capabilities)
    {
      P<TAR> candidate (defaultObj);
      QueryHandler<TAR>& typeHandler = ConfigRules::instance();  
      typeHandler.resolve (candidate, capabilities);
      if (!candidate)
        return false;
      else
        return defsRegistry->put (candidate, capabilities);
    }
    
    
    template<class TAR>
    bool 
    DefsManager::forget  (const P<TAR>& defaultObj)
    {
      return defsRegistry->forget (defaultObj);
    }

    
    template<class TAR>
    P<TAR> 
    DefsManager::operator() (const Query<TAR>& capabilities)
    {
      P<TAR> res (search (capabilities));
      if (res) 
        return res;
      else
        res = create (capabilities); // not yet known as default, create new
      
      if (!res)
        throw lumiera::error::Config ( str(format("The following Query could not be resolved: %s.") 
                                                 % capabilities.asKey())
                                     , LUMIERA_ERROR_CAPABILITY_QUERY );
      else
        return res;
    }

  } // namespace mobject::session

} // namespace mobject



   /***************************************************************/
   /* explicit template instantiations for querying various Types */
   /***************************************************************/

#include "proc/asset/procpatt.hpp"
#include "proc/asset/pipe.hpp"
#include "proc/asset/track.hpp"
#include "proc/mobject/session/track.hpp"

namespace mobject
  {
  namespace session
    {

    using asset::Pipe;
    using asset::PPipe;
    using asset::ProcPatt;
    using asset::PProcPatt;
    
    using mobject::session::Track;
    using mobject::session::TrackAsset;
    using mobject::session::PTrack;
    using mobject::session::PTrackAsset;
    
    template PPipe       DefsManager::operator() (const Query<Pipe>&); 
    template PProcPatt   DefsManager::operator() (const Query<const ProcPatt>&); 
    template PTrack      DefsManager::operator() (const Query<Track>&); 
    template PTrackAsset DefsManager::operator() (const Query<TrackAsset>&); 
    
    template bool DefsManager::define (const PPipe&, const Query<Pipe>&);
    template bool DefsManager::forget (const PPipe&);

  } // namespace mobject::session

} // namespace mobject
