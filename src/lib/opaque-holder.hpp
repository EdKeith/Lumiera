/*
  OPAQUE-HOLDER.hpp  -  buffer holding an object inline while hiding the concrete type 
 
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

/** @file opaque-holder.hpp
 ** Helper allowing type erasure while holding the actual object inline.
 ** Controlling the actual storage of objects usually binds us to commit
 ** to a specific type, thus ruling out polymorphism. But sometimes, when
 ** we are able to control the maximum storage for a family of classes, we
 ** can escape this dilemma by using the type erasure pattern combined with
 ** an inline buffer holding an object of the concrete subclass. Typically,
 ** this situation arises when dealing with functor objects.
 ** 
 ** These templates help with building custom objects and wrappers based on
 ** this pattern: InPlaceAnyHolder provides an buffer for the target objects
 ** and controls access through a two-layer capsule; while the outer container
 ** exposes a neutral interface, the inner container keeps track of the actual
 ** type by means of a vtable. OpaqueHolder is built on top of InPlaceAnyHolder
 ** additionally to support a "common base interface" and re-access of the
 ** embedded object through this interface. For this to work, all of the 
 ** stored types need to be derived from this common base interface.
 ** OpaqueHolder then may be even used like a smart-ptr, exposing this
 ** base interface. To the contrary, InPlaceAnyHolder has lesser requirements
 ** on the types to be stored within. It can be configured with policy classes
 ** to control the re-access; when using InPlaceAnyHolder_unrelatedTypes
 ** the individual types to be stored need not be related in any way, but
 ** of course this rules out anything beyond re-accessing the embedded object
 ** by knowing it's exact type. Generally speaking, re-accessing the concrete
 ** object requires knowledge of the actual type, similar to boost::any
 ** (but contrary to OpaqueHolder the latter uses heap storage). 
 ** 
 ** Using this approach is bound to specific stipulations regarding the
 ** properties of the contained object and the kind of access needed.
 ** When, to the contrary, the contained types are \em not related
 ** and you need to re-discover their concrete type, then maybe
 ** a visitor or variant record might be a better solution.
 ** 
 ** @see opaque-holder-test.cpp
 ** @see function-erasure.hpp usage example
 ** @see variant.hpp 
 */


#ifndef LIB_OPAQUE_HOLDER_H
#define LIB_OPAQUE_HOLDER_H


#include "lib/error.hpp"
#include "lib/bool-checkable.hpp"
#include "lib/access-casted.hpp"
#include "lib/util.hpp"


namespace lib {
  
  using lumiera::error::LUMIERA_ERROR_WRONG_TYPE;
  using util::isSameObject;
  using util::unConst;
  
  
  namespace { // implementation helpers...
    
    using boost::disable_if;
    using boost::is_convertible;
    
    bool
    validitySelfCheck (bool boolConvertible)
      {
        return boolConvertible;
      }
    
    template<typename X>
        typename disable_if< is_convertible<X,bool>, 
    bool >::type
    validitySelfCheck (X const&)
      {
        return true; // just pass if this type doesn't provide a validity check...
      }
    
  }
  
  
  
  /* ==== Policy classes controlling re-Access ==== */
  
  /**
   * Standard policy for accessing the contents via 
   * a common base class interface. Using this policy
   * causes static or dynamic casts or direct conversion
   * to be employed as appropriate.
   */
  template<class BA>
  struct InPlaceAnyHolder_useCommonBase
    {
      typedef BA Base;
      
      template<class SUB>
      static Base*
      convert2base (SUB& obj)
        {
          SUB* oPtr = &obj;
          BA* asBase = util::AccessCasted<BA*>::access (oPtr);
          if (asBase) 
            return asBase;
          
          throw lumiera::error::Logic ("Unable to convert concrete object to Base interface"
                                      , LUMIERA_ERROR_WRONG_TYPE
                                      );
        }
      
      template<class SUB>
      static SUB*
      access (Base* asBase)
        {
          // Because we don't know anything about the involved types,
          // we need to exclude a brute force static cast
          // (which might slice or reinterpret or even cause SEGV)
          if (!util::use_static_downcast<Base*,SUB*>::value)
            {
              SUB* content = util::AccessCasted<SUB*>::access (asBase);
              return content;
              // might be NULL
            }
          else
            return 0;
        }
    };
  
  /**
   * Alternative policy for accessing the contents without
   * a common interface; use this policy if the intention is
   * to use OpaqueHolder with a family of similar classes, 
   * \em without requiring all of them to be derived from
   * a common base class. (E.g. tr1::function objects).
   * In this case, the "Base" type will be defined to void*
   * As a consequence, we loose all type information and
   * no conversions are possible on re-access. You need
   * to know the \em exact type to get back at the object.
   */
  struct InPlaceAnyHolder_unrelatedTypes
    {
      typedef void Base;
      
      template<class SUB>
      static void*
      convert2base (SUB& obj)
        {
          return static_cast<void*> (&obj);
        }
      
      template<class SUB>
      static SUB*
      access (Base*)
        {
          return 0;
        }
    };
  
  
  
  
  
  
  
  /**
   * Inline buffer holding and owning an object while concealing the
   * concrete type. The object is given either as ctor parameter or
   * by direct assignment; it is copy-constructed into the buffer.
   * It is necessary to specify the required buffer storage space
   * as a template parameter. InPlaceAnyHolder may be created empty
   * or cleared afterwards, and this #empty() state may be detected
   * at runtime. In a similar vein, when the stored object has a
   * \c bool validity check, this can be accessed though #isValid().
   * Moreover \code !empty() && isValid() \endcode may be tested
   * as by \bool conversion of the Holder object. The whole compound
   * is copyable if and only if the contained object is copyable.
   * 
   * @note assertion failure when trying to place an instance not
   *       fitting into given size.
   * @note \em not threadsafe!
   */
  template
    < size_t siz           ///< maximum storage required for the targets to be held inline
    , class AccessPolicy = InPlaceAnyHolder_unrelatedTypes
                           ///< how to access the contents via a common interface?
    >
  class InPlaceAnyHolder
    : public BoolCheckable<InPlaceAnyHolder<siz, AccessPolicy> >
    {
      typedef typename AccessPolicy::Base * BaseP;
      
      /** Inner capsule managing the contained object (interface) */
      struct Buffer
        {
          char content_[siz];
          void* ptr() { return &content_; }
          
          virtual ~Buffer() {}
          virtual bool  isValid()  const =0;
          virtual bool  empty()    const =0;
          virtual BaseP getBase()  const =0;
          
          virtual void  clone (void* targetStorage)  const =0;
        };
      
      /** special case: no stored object */
      struct EmptyBuff : Buffer
        {
          virtual bool isValid()  const { return false; }
          virtual bool empty()    const { return true; }
          
          BaseP
          getBase()  const
            {
              throw lumiera::error::Invalid("accessing empty holder");
            }
          
          virtual void
          clone (void* targetStorage)  const
            {
              new(targetStorage) EmptyBuff();
            }
        };
      
      
      /** concrete subclass managing a specific kind of contained object.
       *  @note invariant: #content_ always contains a valid SUB object */
      template<typename SUB>
      struct Buff : Buffer
        {
          SUB&
          get()  const  ///< core operation: target is contained within the inline buffer
            {
              return *reinterpret_cast<SUB*> (unConst(this)->ptr());
            }
          
          ~Buff()
            {
              get().~SUB();
            }
          
          explicit
          Buff (SUB const& obj)
            {
              REQUIRE (siz >= sizeof(SUB));
              new(Buffer::ptr()) SUB (obj);
            }
          
          Buff (Buff const& oBuff)
            {
              new(Buffer::ptr()) SUB (oBuff.get());
            }
          
          Buff&
          operator= (Buff const& ref) ///< currently not used
            {
              if (&ref != this)
                get() = ref.get();
              return *this;
            }
          
          /* == virtual access functions == */
          
          virtual void
          clone (void* targetStorage)  const
            {
              new(targetStorage) Buff(get());
            }
          
          virtual BaseP
          getBase()  const
            {
              return AccessPolicy::convert2base (get());
            }
          
          virtual bool
          empty()  const
            {
              return false;
            }
          
          virtual bool
          isValid()  const
            {
              return validitySelfCheck (this->get());
            }
        };
      
      
      
      enum{ BUFFSIZE = sizeof(Buffer) };
      
      /** embedded buffer actually holding the concrete Buff object,
       *  which in turn holds and manages the target object.
       *  @note Invariant: always contains a valid Buffer subclass */
      char storage_[BUFFSIZE];
      
      
      
      
      
      
    protected: /* === internal interface for managing the storage === */
      
      Buffer&
      buff()
        {
          return *reinterpret_cast<Buffer*> (&storage_);
        }
      const Buffer&
      buff()  const
        {
          return *reinterpret_cast<const Buffer *> (&storage_);
        }
      
      
      void
      killBuffer()                          
        {
          buff().~Buffer();
        }
      
      void
      make_emptyBuff()
        {
          new(&storage_) EmptyBuff();
        }
      
      template<class SUB>
      void
      place_inBuff (SUB const& obj)
        {
          new(&storage_) Buff<SUB> (obj);
        }
      
      void
      clone_inBuff (InPlaceAnyHolder const& ref)
        {
          ref.buff().clone (storage_);
        }
      
      
      
    public:
      
      ~InPlaceAnyHolder()
        {
          killBuffer();
        }
      
      void
      clear ()
        {
          killBuffer();
          make_emptyBuff();
        }
      
      
      InPlaceAnyHolder()
        { 
          make_emptyBuff();
        }
      
      template<class SUB>
      InPlaceAnyHolder(SUB const& obj)
        {
          place_inBuff (obj);
        }
      
      InPlaceAnyHolder (InPlaceAnyHolder const& ref)
        {
          clone_inBuff (ref);
        }
      
      InPlaceAnyHolder&
      operator= (InPlaceAnyHolder const& ref)
        {
          if (!isSameObject (*this, ref))
            {
              killBuffer();
              try
                {
                  clone_inBuff (ref);
                }
              catch (...)
                {
                  make_emptyBuff();
                  throw;
                }
            }
          return *this;
        }
      
      template<class SUB>
      InPlaceAnyHolder&
      operator= (SUB const& newContent)
        {
          if (  empty() 
             || !isSameObject (*buff().getBase(), newContent)
             )
            {
              killBuffer();
              try
                {
                  place_inBuff (newContent);
                }
              catch (...)
                {
                  make_emptyBuff();
                  throw;
                }
            }
          return *this;
        }
      
      
      
      /** re-accessing the concrete contained object.
       *  When the specified type is exactly the same
       *  as used when storing the object, we can directly
       *  re-access the buffer. Otherwise, a conversion might
       *  be possible going through the Base type, depending
       *  on the actual types involved and the AccessPolicy.
       *  But, as we don't "know" the actual type of the object
       *  in storage, a \em static upcast to any type \em between
       *  the concrete object type and the base type has to be
       *  ruled out for safety reasons. When the contained object
       *  has RTTI, a \em dynamic cast can be performed in this
       *  situation. You might consider using visitor.hpp instead
       *  if this imposes a serious limitation.
       *  @throws lumiera::error::Logic when conversion/access fails
       */
      template<class SUB>
      SUB& get()  const
        {
          typedef const Buffer* Iface;
          typedef const Buff<SUB> * Actual;
          Iface interface = &buff();
          Actual actual = dynamic_cast<Actual> (interface);
          if (actual)
            return actual->get();
          
          // second try: maybe we can perform a dynamic downcast
          // or direct conversion to the actual target type.
          BaseP asBase = buff().getBase(); 
          ASSERT (asBase);
          SUB* content = AccessPolicy::template access<SUB> (asBase);
          if (content) 
            return *content;
          
          throw lumiera::error::Logic ("Attempt to access OpaqueHolder's contents "
                                       "specifying incompatible target type"
                                      , LUMIERA_ERROR_WRONG_TYPE
                                      );
        }
      
      
      bool
      empty() const
        {
          return buff().empty();
        }
      
      
      bool
      isValid() const
        {
          return buff().isValid();
        }
    };
  
  
  
  
  
  
  /**
   * Inline buffer holding and owning an object while concealing the
   * concrete type. Access to the contained object is similar to a
   * smart-pointer, but the object isn't heap allocated. OpaqueHolder
   * may be created empty, which can be checked by a bool test.
   * The whole compound is copyable if and only if the contained
   * object is copyable.
   *
   * \par using OpaqueHolder
   * OpaqueHolder instances are copyable value objects. They are created
   * either empty, by copy from an existing OpaqueHolder, or by directly
   * specifying the concrete object to embed. This target object will be
   * \em copy-constructed into the internal buffer. Additionally, you
   * may assign a new value, which causes the old value object to be
   * destroyed and a new one to be copy-constructed into the buffer.
   * Later on, the embedded value might be accessed
   * - using the smart-ptr-like access through the common base interface BA
   * - when knowing the exact type to access, the templated #get might be an option
   * - the empty state of the container and a \c isValid() on the target may be checked
   * - a combination of both is available as a \c bool check on the OpaqueHolder instance.
   *  
   * For using OpaqueHolder, several \b assumptions need to be fulfilled
   * - any instance placed into OpaqueHolder is below the specified maximum size
   * - the caller cares for thread safety. No concurrent get calls while in mutation!
   */
  template
    < class BA                   ///< the nominal Base/Interface class for a family of types
    , size_t siz = sizeof(BA)    ///< maximum storage required for the targets to be held inline
    >
  class OpaqueHolder
    : public InPlaceAnyHolder<siz, InPlaceAnyHolder_useCommonBase<BA> >
    {
      typedef InPlaceAnyHolder<siz, InPlaceAnyHolder_useCommonBase<BA> > InPlaceHolder;
      
    public:
      OpaqueHolder() : InPlaceHolder() {}
      
      template<class SUB>
      OpaqueHolder(SUB const& obj) : InPlaceHolder(obj) {}
      
      template<class SUB>
      OpaqueHolder&
      operator= (SUB const& newContent)
        {
          static_cast<InPlaceHolder&>(*this) = newContent;
          return *this;
        }
      
      // note: using standard copy operations 
      
      
      
      /* === smart-ptr style access === */
      
      BA&
      operator* ()  const
        {
          ASSERT (!InPlaceHolder::empty());
          return *InPlaceHolder::buff().getBase();
        }
      
      BA* 
      operator-> ()  const
        {
          ASSERT (!InPlaceHolder::empty());
          return InPlaceHolder::buff().getBase();
        }
      
    };
  
  
  
  
  
} // namespace lib
#endif