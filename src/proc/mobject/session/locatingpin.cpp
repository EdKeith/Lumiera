/*
  LocatingPin  -  Chaining and constraining the Placement of a Media Object

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


#include "proc/mobject/mobject.hpp"
#include "proc/mobject/placement.hpp"
#include "proc/mobject/session/locatingpin.hpp"
#include "proc/mobject/session/fixedlocation.hpp"
#include "proc/mobject/session/relativelocation.hpp"


namespace proc {
namespace mobject {
namespace session {
  
  inline LocatingPin*
  cloneChain (const scoped_ptr<LocatingPin>& chain)
  {
    if (!chain) 
      return 0;
    else        
      return chain->clone(); 
  }
  
  
  /** it's OK to copy a LocainngPin,
   *  causing duplication of any chained lPins
   */
  LocatingPin::LocatingPin (const LocatingPin& other)
    : next_(cloneChain (other.next_))
  { }
  
  
  LocatingPin& 
  LocatingPin::operator= (const LocatingPin& other)
  {
    if (this != &other) 
      this->next_.reset (cloneChain (other.next_));
    return *this;
  }
  
  
  LocatingPin* 
  LocatingPin::clone ()  const
  {
    return new LocatingPin(*this);
  }
  
  
  
  LocatingPin&
  LocatingPin::addChain (LocatingPin* newLp)  ///< @note we take ownership of newLp
  {
    REQUIRE (newLp);
    REQUIRE (!newLp->next_, "can insert only single LocatingPins");
    
    if (next_ && newLp->getPrioLevel() > next_->getPrioLevel())
      return next_->addChain (newLp);
    else
      {
        scoped_ptr<LocatingPin> tmp_next (newLp);
        tmp_next->next_.swap(next_);
        next_.swap(tmp_next);
        return *newLp;
      }
  }
  
  
  /** implementing the core Placement functionality.
   *  By combining all the chained locating pins, try
   *  to get at a definite position (for this chain and
   *  consequently for the MObject handled by the enclosing
   *  Placement object).
   *  @todo this could/should be replaced by a full-blown
   *        constraint solver at some point in the future
   *  @todo we are packing and unpacking the information (time,track)
   *        several times. Ichthyo considers a more elegant solution.
   */
  const LocatingPin::SolutionData
  LocatingPin::resolve ()  const
  {
    LocatingSolution solution;
    resolve (solution);
    return SolutionData (solution.getTime(), solution.getPipe());
  }
  
  bool 
  LocatingPin::isOverdetermined () const
  {
    LocatingSolution solution;
    resolve (solution);
    return solution.is_impossible(); 
  }
  
  
  
  void 
  LocatingPin::resolve (LocatingSolution& solution)  const
  {
    if (!solution.still_to_solve()) 
      return;
    this->intersect (solution);
    if (next_ && solution.still_to_solve())
      next_->resolve(solution);
  }
  
  
  void 
  LocatingPin::intersect (LocatingSolution& solution)  const
  {
    REQUIRE (solution.still_to_solve());
    // base class Implementation is NOP...
  }
  
  
  /** get some time value which could stand in for this
   *  solution. This doesn't imply this value is a solution,
   *  It's just a value we can use. At the moment (10/07),
   *  LocatingSolution is implemented as interval, and
   *  we return the lower bound here. 
   */
  LocatingPin::Time 
  LocatingPin::LocatingSolution::getTime()
  {
    return minTime;   
  }
  
  LocatingPin::Pipe
  LocatingPin::LocatingSolution::getPipe()
  {
    TODO ("implement Placement LocatingSolution");
    return asset::Pipe::query ("pipe(master)");  // yet another idiotic dummy
  }
  
  
  bool 
  LocatingPin::LocatingSolution::is_definite()  ///< found a solution?
  {
    return (minTime == maxTime && minTrack == maxTrack);
  }
  
  bool 
  LocatingPin::LocatingSolution::is_impossible()
  {
    if (minTime > maxTime)  impo = true;
    TODO ("track???");
    return impo;
  }
  
  bool
  LocatingPin::LocatingSolution::still_to_solve () 
  { 
    return !(is_definite() || is_impossible()); 
  }
  
  
  
  
  
  
  
  
  /* === Factory functions for adding LocatingPins === */
  
  FixedLocation&    
  LocatingPin::operator() (Time start, Track track)
  {
    return static_cast<FixedLocation&> 
              (addChain (new FixedLocation (start, track)));
  }
  
  
  RelativeLocation& 
  LocatingPin::operator() (PlaRef& refObj, Offset const& offset)
  {
    return static_cast<RelativeLocation&> 
              (addChain (new RelativeLocation (refObj, offset)));
  }
  
  
  
  
}}} // namespace proc::mobject::session
