/*
  JOB-TICKET.hpp  -  execution plan for render jobs

  Copyright (C)         Lumiera.org
    2012,               Hermann Vosseler <Ichthyostega@web.de>

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


#ifndef PROC_ENGINE_JOB_TICKET_H
#define PROC_ENGINE_JOB_TICKET_H

#include "proc/common.hpp"
//#include "proc/state.hpp"
//#include "lib/time/timevalue.hpp"
//#include "lib/time/timequant.hpp"

#include <boost/noncopyable.hpp>


namespace proc {
namespace engine {
  
//using lib::time::TimeSpan;
//using lib::time::Duration;
//using lib::time::FSecs;
//using lib::time::Time;
//  
//class ExitNode;
  
  /**
   * execution plan for pulling a specific exit node.
   * Usable as blue print for generating actual render jobs.
   * Job tickets are created on demand, specialised for each segment
   * of the low-level model, and for each individual feed (corresponding
   * to a single model port). Once created, they are final for this segment,
   * stored together with the other descriptor objects (ProcNode and WiringDescriptor)
   * and finally discarded in bulk, in case that segment of the low-level model becomes
   * obsolete and is replaced by a newly built new version of.
   * 
   * Job tickets are created by a classical recursive descent call on the exit node,
   * which figures out everything to be done for generating data from this node.
   * To turn a JobTicket into an actual job, we need the additional information
   * regarding the precise frame number (=nominal time) and the channel number
   * to calculate (in case the actual feed is multichannel, which is the default). 
   * 
   * @todo 1/12 WIP-WIP-WIP defining the invocation sequence and render jobs
   */
  class JobTicket
    : boost::noncopyable
    {
      
    public:
      
      JobTicket()
        {
          UNIMPLEMENTED ("job representation, planning and scheduling");
        }
      
      
      bool
      isValid()  const
        {
          UNIMPLEMENTED ("validity self check");
        }
    };
  
  
  
}} // namespace proc::engine
#endif
