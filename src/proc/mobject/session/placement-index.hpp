/*
  PLACEMENT-INDEX.hpp  -  tracking individual Placements and their relations
 
  Copyright (C)         Lumiera.org
    2009,               Hermann Vosseler <Ichthyostega@web.de>
 
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
 
*/


/** @file placement-index.hpp
 ** Key interface of the session implementation datastructure.
 ** The PlacementIndex is attached to and controlled by the SessionImpl.
 ** Client code is not intended to interface directly to this API. Even
 ** Proc-Layer internal facilities use the session datastructure through
 ** SessionServices. Embedded within the implementation of PlacementIndex
 ** is a flat table structure holding all the Placement instances \em contained
 ** in the session. Any further structuring exists on the logical level only.
 ** 
 ** \par PlacementIndex, PlacementRef and MObjectRef
 ** TODO
 ** 
 ** \par Querying and contents discovery
 ** TODO
 ** 
 ** @note PlacementIndex is <b>not threadsafe</b>.
 **
 ** @see PlacementRef
 ** @see PlacementIndex_test
 **
 */



#ifndef MOBJECT_PLACEMENT_INDEX_H
#define MOBJECT_PLACEMENT_INDEX_H

//#include "pre.hpp"
//#include "proc/mobject/session/locatingpin.hpp"
//#include "proc/asset/pipe.hpp"
#include "lib/util.hpp"
#include "lib/error.hpp"
#include "lib/itertools.hpp"
#include "proc/mobject/placement.hpp"
#include "proc/mobject/placement-ref.hpp"

#include <tr1/unordered_map>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>


namespace mobject {

  class MObject;
  
namespace session {
  
  LUMIERA_ERROR_DECLARE (NOT_IN_SESSION);   ///< referring to a Placement not known to the current session
  LUMIERA_ERROR_DECLARE (PLACEMENT_TYPE);   ///< requested Placement (pointee) type not compatible with data or context
  LUMIERA_ERROR_DECLARE (NONEMPTY_SCOPE);   ///< Placement scope (still) contains other elements
  
  
  using boost::scoped_ptr;
  
  
  
  /**
   * Structured compound of Placement instances
   * with lookup capabilities. Core of the session datastructure.
   * Adding a Placement creates a separate instance within this network,
   * owned and managed by the backing implementation. All placements are
   * related in a tree-like hierarchy of scopes, where each Placement is
   * within the scope of a parent Placement. There is an additional
   * reverse index, allowing to find the immediate children of any
   * given Placement efficiently. All lookup is based on the
   * Placement's hash-IDs.
   */
  class PlacementIndex
    : boost::noncopyable
    {
      class Table;
      
      scoped_ptr<Table> pTab_;
      
      
      typedef PlacementMO::ID _PID;
      typedef std::tr1::unordered_multimap<_PID,_PID>::const_iterator ScopeIter;
      typedef lib::RangeIter<ScopeIter> ScopeRangeIter;
      
      
      
    public:
      typedef Placement<MObject> PlacementMO;
      typedef PlacementRef<MObject> PRef;
      typedef PlacementMO::ID const& ID;
      
      typedef lib::TransformIter<ScopeRangeIter, PlacementMO&> iterator;
      
      
      PlacementMO& find (ID)  const;
      
      template<class MO>
      Placement<MO>&  find (PlacementMO::Id<MO>)  const;
      template<class MO>
      Placement<MO>&  find (PlacementRef<MO> const&) const;
      
      PlacementMO& getScope (PlacementMO const&)  const;
      PlacementMO& getScope (ID)                  const;
      
      iterator getReferrers (ID)                  const;
      
      
      /** retrieve the logical root scope */
      PlacementMO& getRoot()                      const;
      
      size_t size()                               const;
      bool contains (PlacementMO const&)          const;
      bool contains (ID)                          const;
      
      bool isValid()                              const;
      
      
      
      
      /* == mutating operations == */
      
      ID   insert (PlacementMO const& newObj, ID targetScope);
      bool remove (PlacementMO&);
      bool remove (ID);
      
      
      
      PlacementIndex(PlacementMO const&);
     ~PlacementIndex() ;
      
      void clear();
      
    protected:
    };
  
  
  
  
  
  
  
  
  /* === forwarding implementations of the templated API === */
  
    
  namespace { // shortcuts...
    
    template<class MOX>
    inline void
    ___check_compatibleType(PlacementMO& questionable)
    {
      if (!questionable.isCompatible<MOX>())
        throw lumiera::error::Logic ("Attempt to retrieve a Placement of specific type, "
                                     "while the actual type of the pointee (MObject) "
                                     "registered within the index isn't compatible with the "
                                     "requested specific MObject subclass"
                                    ,LUMIERA_ERROR_PLACEMENT_TYPE);
    }
    
    inline void
    __check_knownID(PlacementIndex const& idx, PlacementMO::ID id)
    {
    if (!id)
      throw lumiera::error::Logic ("Encountered a NIL Placement-ID marker"
                                  ,LUMIERA_ERROR_BOTTOM_PLACEMENTREF);
    if (!idx.contains (id))
      throw lumiera::error::Invalid ("Accessing Placement not registered within the index"
                                    ,LUMIERA_ERROR_NOT_IN_SESSION);              ///////////////////////TICKET #197
    }
  }//(End) shortcuts
      

  
  
  template<class MO>
  inline Placement<MO>&
  PlacementIndex::find (PlacementMO::Id<MO> id)  const
  {
    PlacementMO& result (find (id));
    
    ___check_compatibleType<MO> (result);
    return static_cast<Placement<MO>&> (result);
  }
  
    
  template<class MO>
  inline Placement<MO>&
  PlacementIndex::find (PlacementRef<MO> const& pRef)  const
  {
    PlacementMO::Id<MO> id (pRef);
    return find (id);
  }
  
  
  inline Placement<MObject>&
  PlacementIndex::getScope (PlacementMO const& p)  const
  {
    return getScope(p.getID()); 
  }
  
  inline bool
  PlacementIndex::contains (PlacementMO const& p)  const
  {
    return contains (p.getID());
  }
  
  inline bool
  PlacementIndex::remove (PlacementMO& p)
  {
    return remove (p.getID());
  }
  
  
  
}} // namespace mobject::session
#endif
