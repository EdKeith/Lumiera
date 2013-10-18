/*
  DEPENDENCY.hpp  -  access point to singletons and dependencies

  Copyright (C)         Lumiera.org
    2013,               Hermann Vosseler <Ichthyostega@web.de>

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

====================================================================
This code is heavily inspired by
 The Loki Library (loki-lib/trunk/include/loki/Singleton.h)
    Copyright (c) 2001 by Andrei Alexandrescu
    Loki code accompanies the book:
    Alexandrescu, Andrei. "Modern C++ Design: Generic Programming
        and Design Patterns Applied".
        Copyright (c) 2001. Addison-Wesley. ISBN 0201704315

*/



#ifndef LIB_DEPEND_H
#define LIB_DEPEND_H


#include "lib/sync-classlock.hpp"
#include "lib/dependency-factory.hpp"

#include "lib/meta/duck-detector.hpp"   ////TODO move in separate header

#include <boost/noncopyable.hpp>        ////TODO move in separate header
#include <boost/static_assert.hpp>      ////TODO move in separate header
#include <boost/utility/enable_if.hpp>  ////TODO move in separate header


namespace lib {
  
  /**
   * Access point to singletons and other kinds of dependencies.
   * Actually this is a Factory object, which is typically placed into a static field
   * of the Singleton (target) class or some otherwise suitable interface.
   * @note uses static fields internally, so all factory instances share pInstance_
   * @remark there is an ongoing discussion regarding the viability of the
   *   Double Checked Locking pattern, which requires either the context of a clearly defined
   *   language memory model (as in Java), or needs to be supplemented by memory barriers.
   *   In our case, this debate boils down to the question: does \c pthread_mutex_lock/unlock
   *   constitute a memory barrier, such as to force any memory writes happening \em within
   *   the singleton ctor to be flushed and visible to other threads when releasing the lock?
   *   To my understanding, the answer is yes. See
   *   [POSIX](http://www.opengroup.org/onlinepubs/000095399/basedefs/xbd_chap04.html#tag_04_10)
   * @param SI the class of the Singleton instance
   * @param Create policy defining how to create/destroy the instance
   * @param Life policy defining how to manage Singleton Lifecycle
   */
  template<class SI>
  class Depend
    {
      typedef ClassLock<SI> SyncLock;
      
      static SI* volatile instance;
      static DependencyFactory factory;
      
      
    public:
      /** Interface to be used by SingletonFactory's clients.
       *  Manages internally the instance creation, lifecycle
       *  and access handling in a multithreaded context.
       *  @return "the" single instance of class S
       */
      SI&
      operator() ()
        {
          if (!instance)
            {
              SyncLock guard;
              
              if (!instance)
                instance = static_cast<SI*> (factory.buildInstance());
            }
          ENSURE (instance);
          return *instance;
        }
      
      
      typedef DependencyFactory::InstanceConstructor Constructor;
      
      Depend (Constructor ctor = buildSingleton<SI>())
        {
          factory.installConstructorFunction (ctor);
        }
      
      // standard copy operations applicable
      
      
      /* === Management / Test support interface === */
      
      static void
      shutdown()
        {
          SyncLock guard;
          
          factory.deconfigure (instance);
          instance = NULL;
        }
      
      static void
      injectReplacement (SI* mock)
        {
          REQUIRE (mock);
          factory.takeOwnership (mock);   // EX_SANE
          
          SyncLock guard;
          factory.shaddow (instance);     // EX_FREE
          instance = mock;
        }
      
      static void
      dropReplacement()
        {
          SyncLock guard;
          factory.restore (instance);     // EX_FREE
        }
    };
  
  
  // Storage for SingletonFactory's static fields...
  template<class SI>
  SI* volatile Depend<SI>::instance;
  
  template<class SI>
  DependencyFactory Depend<SI>::factory;
  
  
  
  
  namespace { ///< details: inject a mock automatically in place of a singleton
    
    using boost::enable_if;
    using lib::meta::Yes_t;
    using lib::meta::No_t;
    
    /**
     * Metafunction: does the Type in question
     * give us a clue about what service interface to use?
     */
    template<class MOCK>
    class defines_ServiceInterface
      {
        META_DETECT_NESTED  (ServiceInterface);
        
      public:
        enum{ value = HasNested_ServiceInterface<MOCK>::value
            };
      };
    
    
    /**
     * Policy-Trait: determine the access point to install the mock.
     * @note either the mock service implementation needs to provide
     *       explicitly a typedef for the ServiceInterface, or we're
     *       just creating a separate new instance of the singleton service,
     *       while shadowing (but not destroying) the original instance.
     */
    template<class I, class YES =void>
    struct ServiceInterface
      {
        typedef I Type;
      };
    
    
    template<class MOCK>
    struct ServiceInterface<MOCK, typename enable_if< defines_ServiceInterface<MOCK> >::type>
      {
        typedef typename MOCK::ServiceInterface Type;
      };
    
  }//(End) mock injection details
  
  
  /**
   * Scoped object for installing/deinstalling a mocked service automatically.
   * Placing a suitably specialised instance of this template into a local scope
   * will inject the corresponding mock installation and remove it when the
   * control flow leaves this scope.
   * @param TYPE the concrete mock implementation type to inject. It needs to
   *        be default constructible. If TYPE is a subclass of the service interface,
   *        it needs to expose a typedef \c ServiceInterface
   */
  template<class TYPE>
  struct Use4Test
    : boost::noncopyable
    {
      typedef typename ServiceInterface<TYPE>::Type Interface;
      
      Use4Test()
        {
          Depend<Interface>::injectReplacement (new TYPE);
        }
      
     ~Use4Test()
        {
          Depend<Interface>::dropReplacement();
        }
    };
  
  
} // namespace lib
#endif
