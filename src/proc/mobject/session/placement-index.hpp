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
 ** Core of the session implementation datastructure.
 ** The PlacementIndex is attached to and controlled by the SessionImpl.
 ** Client code is not intended to interface directly to this API. Even
 ** Proc-Layer internal facilities use the session datastructure through
 ** SessionServices. Embedded within the implementation of PlacementIndex
 ** is a flat table structure holding all the Placement instances \em contained
 ** in the session. Any further structuring exists on the logical level only.
 ** 
 ** \par PlacementIndex, PlacementRef and MObjectRef
 ** Objects are attached to the session by adding (copying) a Placement instance,
 ** and doing so creates  a new Placement-ID, which from then on acts as a shorthand for
 ** "the object instance" within the session. As long as this instance isn't removed from
 ** the session / PlacementIndex, a direct (language) reference can be used to work with
 ** "the object instance"; accessing this way is adequate for implementation code living
 ** within Lumiera's Proc-Layer.
 ** 
 ** To avoid the dangerous dependency on a direct reference, external code would rather
 ** rely on the Placement-ID. Moreover, being a simple value, such an ID can be passed
 ** through plain C APIs. PlacementRef is a smart-ptr like wrapper, containing just
 ** such an ID; dereferentiation transparently causes a lookup operation through the
 ** PlacementIndex of the current session. (accessing an invalid PlacementRef throws)
 ** 
 ** When it comes to ownership and lifecycle management, external client code should
 ** use MObjectRef instances. In addition to containing a PlacementRef, these set up
 ** a smart-ptr managing the MObject instance and sharing ownership with the Placement
 ** contained within the PlacementIndex. Usually, the commands expressing any mutating
 ** operations on the session, bind MObjectRef instances as arguments; similarly, the
 ** public API functions on the Session interface (and similar facade interfaces) are
 ** written in terms of MObectRef.   
 ** 
 ** \par placement scopes
 ** When adding a Placement to the index, it is mandatory to specify a Scope: this is
 ** another Placement already registered within the index; the new Placement can be thought
 ** off as being located "within" or "below" this scope-defining reference Placement. An
 ** typical example would be the addition of a \c Placement<session::Clip>, specifying 
 ** a \c Placement<session::Track> as scope. Thus, all "object instances" within the 
 ** session are arranged in a tree-like fashion. On creation of the PlacementIndex,
 ** a root element needs to be provided. While this root element has a meaning for
 ** the session, within the index it is just a scope-providing element.
 ** Note that a non-empty scope can't be deleted from the Index.
 ** 
 ** \par querying and contents discovery
 ** As "the object instance within the session" is synonymous to the placement instance
 ** managed by PlacementIndex, the (hash)-ID of such a placement can be used as an
 ** object identifier (it is implemented as LUID and stored within the Placement instance).
 ** Thus, a basic operation of the index is to fetch a (language) reference to a Placement,
 ** given this hash-ID. Another basic operation is to retrieve the scope an given object
 ** is living in, represented by the Placement defining this scope (called "scope top").
 ** The reverse operation is also possible: given a scope-defining Placement, we can
 ** \em discover all the other Placements directly contained within this scope:
 ** \c getReferrers(ID) returns an (possibly empty) "Lumiera Forward Iterator",
 ** allowing to enumerate the nested elements. Client code within Lumiera's Proc-Layer
 ** typically uses this functionality through a ScopeQuery passed to the SessionServices,
 ** while external client code would use either QueryFocus and the Scope wrapper objects,
 ** or the specific query functions available on the facade objects accessible through
 ** the public session API.
 ** 
 ** \par type handling
 ** MObjects form a hierarchy and contain RTTI. By special definition trickery, the
 ** various instances of the Placement template mirror this hierarchy to some extent.
 ** By using the vtable of the referred MObject, a given \c Placement<MObject> can
 ** be casted into a more specifically typed Placement, thus allowing to re-gain
 ** the fully typed context. This technique plays an important role when it comes
 ** to generic processing of the session contents by a visitor, and especially
 ** within the Builder. This is a fundamental design decision within Proc-Layer:
 ** code should not operate on MObjects and do type/capability queries -- rather
 ** any processing is assumed to happen in a suitable typed context. Consequently,
 ** client code will never need to fetch Placements directly from the index. This
 ** allows all type information to be discarded on adding (copying) a Placement
 ** instances into the PlacementIndex.
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
      
      
      /* == query operations == */
      
      PlacementMO& find (ID)                         const;
      
      template<class MO>
      Placement<MO>&  find (PlacementMO::Id<MO>)     const;
      template<class MO>
      Placement<MO>&  find (PlacementRef<MO> const&) const;
      
      PlacementMO& getScope (PlacementMO const&)     const;
      PlacementMO& getScope (ID)                     const;
                                                     
      iterator getReferrers (ID)                     const;
                                                     
                                                     
      /** retrieve the logical root scope */         
      PlacementMO& getRoot()                         const;
                                                     
      size_t size()                                  const;
      bool contains (PlacementMO const&)             const;
      bool contains (ID)                             const;
                                                     
      bool isValid()                                 const;
      
      
      
      
      /* == mutating operations == */
      
      ID   insert (PlacementMO const& newObj, ID targetScope);
      bool remove (PlacementMO&);
      bool remove (ID);
      
      
      
      PlacementIndex(PlacementMO const&);
     ~PlacementIndex() ;
      
      void clear();
      
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
