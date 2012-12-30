/*
  CALC-STREAM.hpp  -  abstraction representing a series of scheduled calculations

  Copyright (C)         Lumiera.org
    2011,               Hermann Vosseler <Ichthyostega@web.de>

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

*/



#ifndef PROC_ENGINE_CALC_STREAM_H
#define PROC_ENGINE_CALC_STREAM_H


#include "lib/error.hpp"
//#include "include/dummy-player-facade.h"
//#include "include/display-facade.h"
//#include "common/instancehandle.hpp"
//#include "lib/singleton-ref.hpp"
//
//#include <boost/noncopyable.hpp>
//#include <boost/scoped_ptr.hpp>
//#include <string>
#include <vector>


namespace proc  {
namespace engine{

//    using std::string;
//    using lumiera::Subsys;
//    using lumiera::Display;
//    using lumiera::DummyPlayer;
  
  
  /**
   * Abstract definition of the environment
   * hosting a given render activity (CalcStream).
   * Exposes all the operations necessary to adjust the
   * runtime behaviour of the render activity, like e.g.
   * re-scheduling with modified playback speed. Since the
   * CalcStream is an conceptual representation of "the rendering",
   * the actual engine implementation is kept opaque this way. 
   */
  class RenderEnvironmentClosure
    {
    public:
      virtual ~RenderEnvironmentClosure() { }   ///< this is an interface 
    };
  
  
  
  
  /*********************************************************
   * A calculation stream groups and abstracts a series of
   * calculation jobs, delivering frames into the configured
   * OutputSlot in a timely fashion. Behind the scenes, this
   * gets translated into several jobs enqueued with the
   * scheduler in the backend layer. The calculation stream
   * implementation cares to create and configure these
   * jobs and to manage the necessary dependencies and
   * callbacks.
   * 
   * Regarding the implementation, a CalcStream is an const
   * value object holding the metadata necessary to manage
   * the underlying jobs. The only way to create a CalcStream
   * properly is to retrieve it from the  factory functions
   * of the EngineService. At that point, the corresponding
   * jobs will be configured and enqueued. 
   */
  class CalcStream
    {
      RenderEnvironmentClosure* env_;
      
    protected:
      CalcStream (RenderEnvironmentClosure& engine)
        : env_(&engine)
        { }
      
      friend class EngineService;
      
      
    public:
      CalcStream()
        : env_(0)
        { }
      
      CalcStream (CalcStream const& o)
        : env_(o.env_)
        { }
      
     ~CalcStream() { }
      
    };
  
  typedef std::vector<CalcStream> CalcStreams;
      

  
  
  
} // namespace engine
} // namespace proc
#endif
