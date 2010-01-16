/*
  PLACEMENT-REF.hpp  -  generic reference to an individual placement added to the session
 
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


/** @file placement-ref.hpp
 ** A generic reference mechanism for Placements, as added to the current session.
 ** While this reference itself is not tied to the actual memory layout (it's not a
 ** disguised pointer), the implementation relies on the PlacementIndex within the
 ** current session for tracking and retrieving the actual Placement implementation object.
 ** This approach allows to create active interconnections between placements. We utilise
 ** this to create a system of nested scopes within the high-level-model. But for an (external)
 ** client, this link to an index implementation is kept transparent and implicit.
 ** 
 ** PlacementRef is implemented as a smart-handle, based on an hash-ID (LUID), which is also
 ** used as key within the Index. This hash-ID is incorporated into the Placement instance too.
 ** Each dereferentiation of a placement ref will access the current session to query the
 ** index for the corresponding placement instance in memory. This lookup can fail, causing
 ** \c operator*() to throw. Client code may use the bool conversion to verify the validity
 ** of an PlacementRef before accessing it -- similar to the behaviour of a pointer.
 ** 
 ** A default created PlacementRef is \em bottom (and will throw on any access). A valid
 ** placement ref can be created from any source leading to a Placement-ID. The expected
 ** type of the pointee (MObject) is set by the template parameter used on creation. The
 ** validity of this access type is checked, using the RTTI of the pointee (MObject),
 ** assuming of course the reference to be valid (resolvable by index). This mechanism
 ** allows to pass or re-gain a specifically typed usage context. (e.g. \c Placement<Effect>)
 ** 
 ** @note there is a twist concerning the nominal Placement hierarchy
 **       as generated by the return type of PlacementRef::operator*().
 **       While the original Placement (as added to the session) might been
 **       defined to mimic a more elaborate hierarchy (e.g. Placement<Track> 
 **       inherits from Placement<Meta>), the Placement returned here in
 **       these cases will just be a subclass or Placement<MObject>
 **       (which in the mentioned example would mean it couldn't be
 **       passed to a API function expecting a Placement<Meta>).
 **       This is ugly, but doesn't seem to bear any danger.
 ** @todo better solution for the ID to type connection ///////////////////////TICKET #523
 **
 ** @see Placement
 ** @see PlacementRef_test
 **
 */



#ifndef MOBJECT_PLACEMENT_REF_H
#define MOBJECT_PLACEMENT_REF_H

//#include "pre.hpp"
#include "lib/error.hpp"
#include "lib/bool-checkable.hpp"
#include "proc/mobject/placement.hpp"
#include "proc/mobject/explicitplacement.hpp"  /////////////TODO this is ugly! Why can't placement::resolve() return a reference??
#include "proc/mobject/session/session-service-fetch.hpp"



namespace mobject {
  
  
  class MObject;
  
  
  
  LUMIERA_ERROR_DECLARE (INVALID_PLACEMENTREF);  ///< unresolvable placement reference, or of incompatible type
  LUMIERA_ERROR_DECLARE (BOTTOM_PLACEMENTREF);  ///<  NIL placement-ID marker encountered
  
  
  /**
   * Reference tag denoting a placement attached to the session.
   * Implemented as a smart-handle, transparently accessing the
   * PlacementIndex to resolve the embedded hash-ID into a
   * language reference to the referred placement.
   * @see MObectRef
   * @see placement-index.hpp
   */
  template<class MX =MObject>
  class PlacementRef
    : public lib::BoolCheckable<PlacementRef<MX> >
    {
      typedef Placement<MX>      PlacementMX;
      typedef Placement<MObject>::ID     _ID;
      typedef Placement<MObject>::Id<MX> _Id;
      
      _Id id_;
      
    public:
      /**
       * Creating a PlacementRef from a compatible reference source.
       * Any source allowing to infer a \em compatible mobject::Placement
       * is accepted. Here, compatibility is decided based on the run time
       * type of the pointee, in comparison to the template parameter Y.
       * In any case, for this ctor to succeed, the provided ref or ID 
       * needs to be resolvable to a placement by the implicit PlacementIndex
       * facility used by all PlacementRef instances (typically the Session).
       * @note there is no default ctor, a reference source is mandatory.
       * @param refID reference resolvable to a placement via Index, especially
       *              - an existing Placement
       *              - just an Placement::ID
       *              - a plain LUID
       * @throw error::Invalid on incompatible run time type of the resolved ID  
       */
      template<class Y>
      explicit
      PlacementRef (Y const& refID)
        : id_(recast (refID))
        { 
          validate(id_); 
        }
      
      /** Default is an NIL Placement ref. It throws on any access. */
      PlacementRef ()
        : id_( bottomID() )
        {
          REQUIRE (!isValid(), "hash-ID clash with existing ID");
        }
      
      PlacementRef (PlacementRef const& r)    ///< copy ctor
        : id_(r.id_)
        { 
          validate(id_); 
        }
      
      template<class X>
      PlacementRef (PlacementRef<X> const& r) ///< extended copy ctor, when type X is assignable to MX
        : id_(recast(r))
        {
          validate(id_); 
        }
      
      
      PlacementRef&
      operator= (PlacementRef const& r)
        {
          validate(r.id_);
          id_ = r.id_;
          return *this;
        }
      
      template<class X>
      PlacementRef&
      operator= (PlacementRef<X> const& r)
        {
          validate(recast (r));
          id_ = recast(r);
          return *this;
        }
      
      template<class Y>
      PlacementRef&
      operator= (Y const& refID)
        {
          validate (recast (refID));
          id_ = recast (refID);
          return *this;
        }
      
      
      /* == forwarding smart-ptr operations == */
      
      PlacementMX& operator*()  const { return access(id_); } ///< dereferencing fetches referred Placement from Index
      
      PlacementMX& operator->() const { return access(id_); } ///< provide access to pointee API by smart-ptr chaining 
      
      operator string()         const { return access(id_).operator string(); }
      size_t use_count()        const { return access(id_).use_count(); }
      
      
      /* == accessing the embedded ID == */
      operator _Id const&()     const { return id_; }
      LumieraUid getLUID()      const { return id_.get(); }
      
      template<class O>
      bool operator== (PlacementRef<O> const& o)  const { return id_ == o; }
      template<class O>
      bool operator!= (PlacementRef<O> const& o)  const { return id_ != o; }
      
      
      
      
      
      /* == forwarding part of the Placement-API == */
      
      bool isValid()  const
        { 
          if (checkValidity())
            try
              {
                return access(id_).isValid();
              }
            catch (lumiera::error::Invalid&) {}
            
          return false;
        }
      
      ExplicitPlacement
      resolve() const
        {
          return access(id_).resolve();
        }
      
      
      ////////////////TODO more placement operations to come....
      
    private:
      bool
      checkValidity ()  const
        {
          return session::SessionServiceFetch::isAccessible() // session interface opened?
              && session::SessionServiceFetch::isRegisteredID (this->id_);
        }
      
      static void
      validate (_Id const& rId)
        {
          access (rId); // may throw
          /////////////////////////////TODO more to check on each PlacementRef creation?
        }
      
      static _Id const&
      recast (_ID const& someID)
        {
          return static_cast<_Id const&> (someID);
        }
      
      static _Id const&
      recast (const LumieraUid luid)
        {
          REQUIRE (luid);
          return reinterpret_cast<_Id const&> (*luid);
        }
      
      static _Id const&
      bottomID ()  ///< @return marker for \em invalid reference
        {
          static lumiera_uid invalidLUID;
          return recast (&invalidLUID);
        }
      
      static PlacementMX&
      access (_Id const& placementID)
        {
          if (!placementID)
            throw lumiera::error::Logic ("Attempt to access a NIL PlacementRef"
                                        ,LUMIERA_ERROR_BOTTOM_PLACEMENTREF);
          
          Placement<MObject> & genericPlacement (session::SessionServiceFetch::resolveID (placementID));  // may throw
          REQUIRE (genericPlacement.isValid());
          
          if (!(genericPlacement.template isCompatible<MX>()))
            throw lumiera::error::Invalid("actual type of the resolved placement is incompatible"
                                         , LUMIERA_ERROR_INVALID_PLACEMENTREF);
                  ////////////////////////TODO: 1. better message, including type?
                  ////////////////////////TODO: 2. define a separate error-ID for the type mismatch!
          
          return static_cast<PlacementMX&> (genericPlacement);
        }
    };
  
  
  /** frequently-used shorthand */
  typedef PlacementRef<MObject> RefPlacement;
  
  
  
} // namespace mobject
#endif
