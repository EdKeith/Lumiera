/*
  TimeGrid  -  reference scale for quantised time

  Copyright (C)         Lumiera.org
    2010,               Hermann Vosseler <Ichthyostega@web.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of
  the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

* *****************************************************/


#include "proc/asset/meta/time-grid.hpp"
#include "proc/asset/entry-id.hpp"
#include "proc/assetmanager.hpp"
//#include "lib/time/quantiser.hpp"
#include "lib/time/timevalue.hpp"
#include "lib/time/display.hpp"
//#include "lib/util.hpp"
//#include "include/logging.h"

#include <boost/format.hpp>
#include <string>

using boost::format;
using boost::str;
using std::string;


namespace asset {
namespace meta {
  
  namespace error = lumiera::error;
  
  namespace {
    
    // Implementation details (still required???)
  }
  
 
  /** */
  TimeGrid::TimeGrid (EntryID<TimeGrid> const& nameID)
    : Meta (nameID.getIdent())
    { }
  
  
  using lib::time::Time;
  using lib::time::TimeValue;
  using lib::time::TimeSpan;
  using lib::time::Duration;
  using lib::time::Offset;
  
  /** 
   * TimeGrid implementation: a trivial time grid,
   * starting at a given point in time and using a
   * constant grid spacing
   */
  class SimpleTimeGrid
    : public TimeGrid
    {
      TimeSpan timeBase_;
      
      // TODO implement the TimeGrid API here
      
    public:
      SimpleTimeGrid (Time start, Duration gridSpacing, EntryID<TimeGrid> const& name)
        : TimeGrid (name)
        , timeBase_(start,gridSpacing)
        { }
    };
  
    
  
  /** Setup of a TimeGrid: validate the settings configured into this builder instance,
   *  then decide on the implementation strategy for the time grid. Convert the given
   *  frames per second into an appropriate gridSpacing time and build a suitable
   *  name-ID to denote the TimeGrid-meta-Asset to be built. 
   * @return shared_ptr holding onto the new asset::Meta, which has already been
   *         registered with the AssetManager.
   * @throw  error::Config in case of invalid frames-per-second. The AssetManager
   *         might raise further exception when asset registration fails.
   * @todo currently (12/2010) the AssetManager is unable to detect duplicate assets.
   *       Later on the intention is that in such cases, instead of creating a new grid
   *       we'll silently return the already registered exisiting and equivalent grid.
   */
  P<TimeGrid>
  Builder<TimeGrid>::commit()
  {
    if (predecessor_)
      throw error::Invalid("compound and variable time grids are a planned feature"
                          , error::LUMIERA_ERROR_UNIMPLEMENTED);
    if (!fps_)
      throw error::Config ("attempt to build a TimeGrid with 0 frames per second");
    
    Time spacing = 1 / fps_;  // seconds per frame
    
    format gridIdFormat("grid_%f_%s");
    string name = str(gridIdFormat % fps_ % origin_);
    EntryID<TimeGrid> nameID (name);
    TimeGrid& newGrid (*new SimpleTimeGrid(origin_, Offset(spacing), nameID));
    
    return AssetManager::instance().wrap (newGrid);
  }
  
}} // namespace asset::meta
