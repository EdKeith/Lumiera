/*
  RenderInvocation  -  Interface for creating processing nodes of various kinds

  Copyright (C)         Lumiera.org
    2008,               Hermann Vosseler <Ichthyostega@web.de>

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


#include "proc/engine/render-invocation.hpp"
#include "proc/engine/stateproxy.hpp"


namespace proc {
namespace engine {
  
  namespace { // Details...

    
  } // (END) Details...
  
  
//  using mobject::Placement;
//  using mobject::session::Effect;
  
  
  /** @note this builds a one-way off invocation state context
   *        and then forwards the call; this may or may not cause
   *        actual calculations, depending on the cache.
   *  @todo for real use within the engine, the pull()-call should be
   *        dispatched through the scheduler; of course then the call semantics
   *        would be completely different. Maybe this invocation will be test only?  
   */
  BuffHandle
  RenderInvocation::operator[] (size_t channel)
  {
    REQUIRE (theNode_);
    REQUIRE (channel < size());
    
    StateProxy invocationState;
    return theNode_->pull(invocationState, channel);
  }
  
  
}} // namespace engine
