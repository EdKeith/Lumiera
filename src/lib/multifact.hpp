/*
  MULTIFACT.hpp  -  flexible family-of-object factory template

  Copyright (C)         Lumiera.org
    2009,               Hermann Vosseler <Ichthyostega@web.de>

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

/** @file multifact.hpp
 ** Framework for building a configurable factory, to generate families of related objects.
 ** These building blocks are targeted towards the "classical" factory situation: obtaining
 ** objects of various kinds, which are related somehow (usually through an common interface).
 ** The creation of these objects might be non-trivial, while the number of flavours to be
 ** produced and the exact parametrisation isn't known beforehand and needs to be figured out
 ** at runtime. As a solution, thus a number of "fabrication lines" is set up, to be selected
 ** on invocation through an ID (which may be symbolic, hashed or structural).
 ** 
 ** Usually, the issue of object and storage management is closely related, while it is
 ** desirable to keep the object production logic clean of these rather technical concerns.
 ** The implementation built here separates the latter into a policy template invoked as a
 ** \em wrapper, accepting the raw product and either registering it, taking ownership, clone
 ** it or use it for more involved wiring. Obviously, the product generated by the installed
 ** "fabrication lines" needs to be delivered in a form acceptable by the concrete wrapper;
 ** mismatch will be spotted by the compiler on registration of the respective fabrication
 ** function.
 ** 
 ** \par Singleton generation
 ** For the very common situation of building a family of singleton objects, accessible by ID,
 ** there is a convenience shortcut: The nested MultiFact::Singleton template can be instantiated
 ** within the context providing the objects (usually a static context). In itself a lib::Singleton
 ** factory, it automatically registers the singleton access function as "fabrication" function
 ** into a suitable MultiFact instance passed in as ctor parameter.
 ** 
 ** @note there is an extension header, multifact-arg.hpp, which provides template specialisations
 **       for the special case when the fabrication functions need additional invocation arguments.
 ** 
 ** @see multifact-test.cpp
 ** @see multifact-argument-test.cpp
 ** @see SingletonFactory
 */


#ifndef LIB_MULTIFACT_H
#define LIB_MULTIFACT_H


#include "lib/error.hpp"
#include "lib/depend.hpp"
#include "util.hpp"

#include <functional>
#include <memory>
#include <map>



namespace lib {
  namespace factory {
    
    // Helpers to wrap the factory's product
    
    /**
     * Dummy "wrapper",
     * just returning a target-ref
     */
    template<typename TAR>
    struct PassReference
      {
        typedef TAR& RType;
        typedef TAR& PType;
        
        PType wrap (RType object) { return object; }
      };
    
    
    /**
     * Wrapper taking ownership,
     * by wrapping into smart-ptr
     */
    template<typename TAR>
    struct BuildRefcountPtr
      {
        typedef TAR*                      RType;
        typedef std::shared_ptr<TAR> PType;
        
        PType wrap (RType ptr) { return PType (ptr); }
      };
    
    
    
    
    
    /**
     * Table of registered production functions for MultiFact.
     * Each stored function can be accessed by ID and is able
     * to fabricate a specific object, which is assignable to
     * the nominal target type in the MultiFact definition.
     */
    template<typename SIG, typename ID>
    struct Fab
      {
        typedef std::function<SIG> FactoryFunc;
        
        
        FactoryFunc&
        select (ID const& id)
          {
            if (!contains (id))
              throw lumiera::error::Invalid("unknown factory product requested.");
            
            return producerTable_[id];
          }
        
        void
        defineProduction (ID const& id, FactoryFunc fun)
          {
            producerTable_[id] = fun;
          }
        
        
        /* === diagnostics === */
        
        bool empty ()         const { return producerTable_.empty(); }
        bool contains (ID id) const { return util::contains (producerTable_,id); }
        
      private:
        std::map<ID, FactoryFunc> producerTable_;
      };
    
    
    
    /** 
     * @internal configuration of the elements
     * to be combined into a MultiFact instance
     */
    template< typename TY
            , template<class> class Wrapper
            >
    struct FabWiring
      : Wrapper<TY>
      {
        typedef typename Wrapper<TY>::PType WrappedProduct;
        typedef typename Wrapper<TY>::RType FabProduct;
        typedef FabProduct SIG_Fab(void);
      };
    
    
    
    /**
     * Factory for creating a family of objects by ID.
     * The actual factory functions are to be installed
     * from the usage site through calls to #defineProduction .
     * Each generated object will be treated by the Wrapper template,
     * allowing for the generation of smart-ptrs. The embedded class
     * Singleton allows to build a family of singleton objects; it is
     * to be instantiated at the call site and acts as singleton factory,
     * accessible through a MultiFact instance as frontend.
     */
    template< typename TY
            , typename ID
            , template<class> class Wrapper
            >
    class MultiFact
      : public FabWiring<TY,Wrapper>
      {
        typedef FabWiring<TY,Wrapper> _Conf;
        typedef typename _Conf::SIG_Fab SIG_Fab;
        typedef Fab<SIG_Fab,ID> _Fab;
        
        _Fab funcTable_;
        
        
      protected:
        typedef typename _Fab::FactoryFunc Creator;
        
        Creator&
        selectProducer (ID const& id)
          {
            return funcTable_.select(id);
          }
        
        
      public:
        typedef typename _Conf::WrappedProduct Product;
        
        Product
        operator() (ID const& id)
          {
            Creator& func = this->selectProducer (id);
            return this->wrap (func());
          }
        
        
        /** to set up a production line,
         *  associated with a specific ID
         */
        template<typename FUNC>
        void
        defineProduction (ID id, FUNC fun)
          {
            funcTable_.defineProduction (id, fun);
          }
        
        
        /**
         * Convenience shortcut for automatically setting up
         * a production line, fabricating a singleton instance
         * of the given implementation target type (IMP)
         */
        template<class IMP>
        class Singleton
          : lib::Depend<IMP>
          {
            typedef lib::Depend<IMP> SingFac;
            
            Creator
            createSingleton_accessFunction()
              {
                return std::bind (&SingFac::operator()
                                      , static_cast<SingFac*>(this));
              }
            
          public:
            Singleton (MultiFact& factory, ID id)
              {
                factory.defineProduction(id, createSingleton_accessFunction());
              }
          };
          
        
        /* === diagnostics === */
        
        bool empty ()         const { return funcTable_.empty();       }
        bool contains (ID id) const { return funcTable_.contains (id); }
      };
    
    
    
  } // namespace factory
  
  
  
  /** 
   * Standard configuration of the family-of-object factory
   * @todo this is rather guesswork... find out what the best and most used configuration could be....
   */
  template< typename TY
          , typename ID
          >
  class MultiFact
    : public factory::MultiFact<TY,ID, factory::PassReference>
    { };
  
  
} // namespace lib
#endif
