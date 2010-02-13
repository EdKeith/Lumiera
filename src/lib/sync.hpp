/*
  SYNC.hpp  -  generic helper for object based locking and synchronisation
 
  Copyright (C)         Lumiera.org
    2008,               Christian Thaeter <ct@pipapo.org>
                        Hermann Vosseler <Ichthyostega@web.de>
 
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

/** @file sync.hpp
 ** Object Monitor based synchronisation.
 ** Actually, everything is implemented by delegating the raw locking/sync calls
 ** to the Lumiera backend. The purpose is to support and automate the most common
 ** use cases in a way which fits better with scoping and encapsulation; especially
 ** we build upon the monitor object pattern.
 ** 
 ** A class becomes \em lockable by inheriting from lib::Sync with the appropriate
 ** parametrisation. This causes any instance to inherit a monitor member (object),
 ** managing a mutex and (optionally) a condition variable for waiting. The actual
 ** synchronisation is achieved by placing a guard object as local (stack) variable
 ** into a given scope (typically a member function body). This guard object of
 ** class lib::Sync::Lock accesses the enclosing object's monitor and automatically
 ** manages the locking and unlocking; optionally it may also be used for waiting
 ** on a condition.
 ** 
 ** Please note:
 ** - It is important to select a suitable parametrisation of the monitor.
 **   This is done by specifying one of the defined policy classes.
 ** - Be sure to pick the recursive mutex implementation when recursive calls
 **   of synchronised functions can't be avoided. (performance penalty)
 ** - You can't use the Lock#wait and Lock#notify functions unless you pick
 **   a parametrisation including a condition variable.
 ** - The "this" pointer is fed to the ctor of the Lock guard object. Thus
 **   you may use any object's monitor as appropriate, especially in cases 
 **   when adding the monitor to a given class may cause size problems.
 ** - For sake of completeness, this implementation provides the ability for
 **   timed waits. But please consider that in most cases there are better
 **   solutions for running an operation with given timeout by utilising the
 **   Lumiera scheduler. Thus use of timed waits is \b discouraged.
 ** - There is a special variant of the Lock guard called ClassLock, which
 **   can be used to lock based on a type, not an instance. 
 ** - in DEBUG mode, the implementation includes NoBug resource tracking.
 ** 
 ** @see mutex.h
 ** @see sync-locking-test.cpp
 ** @see sync-waiting-test.cpp
 ** @see asset::AssetManager::reg() usage example
 ** @see subsystemrunner.hpp usage example
 */


#ifndef LIB_SYNC_H
#define LIB_SYNC_H

#include "lib/error.hpp"
#include "lib/util.hpp"
#include "lib/sync-nobug-resource-handle.hpp"

extern "C" {
#include "lib/mutex.h"
#include "lib/recmutex.h"
#include "lib/condition.h"
#include "lib/reccondition.h"
}

#include <boost/noncopyable.hpp>
#include <pthread.h>
#include <cerrno>
#include <ctime>

using boost::noncopyable;


namespace lib {
    
    
    /** Helpers and building blocks for Monitor based synchronisation */
    namespace sync {
      
      
      
      /* ========== adaptation layer for accessing the backend code ============== */
      
      struct Wrapped_LumieraExcMutex
        {
          lumiera_mutex self;
          
        protected:
          Wrapped_LumieraExcMutex(const struct nobug_context ctx = NOBUG_CONTEXT_NOFUNC)
          { lumiera_mutex_init    (&self, "Obj.Monitor ExclMutex", &NOBUG_FLAG(sync), ctx); }
         ~Wrapped_LumieraExcMutex()
          { lumiera_mutex_destroy (&self, &NOBUG_FLAG(sync), NOBUG_CONTEXT); }
          
          bool lock (struct nobug_resource_user** handle, const struct nobug_context ctx = NOBUG_CONTEXT_NOFUNC)
          {
            return !!lumiera_mutex_lock (&self, &NOBUG_FLAG(sync), handle, ctx);
          }
          
          void unlock (struct nobug_resource_user** handle, const struct nobug_context ctx = NOBUG_CONTEXT_NOFUNC)
          {
            lumiera_mutex_unlock (&self, &NOBUG_FLAG(sync), handle, ctx);
          }
          
          LumieraReccondition myreccond () {throw "bullshit"; return NULL;} // placeholder, brainstorm
        };
      
      
      struct Wrapped_LumieraRecMutex
        {
          lumiera_recmutex self;
        protected:
          Wrapped_LumieraRecMutex(const struct nobug_context ctx = NOBUG_CONTEXT_NOFUNC)
          { lumiera_recmutex_init    (&self, "Obj.Monitor RecMutex", &NOBUG_FLAG(sync), ctx); }
         ~Wrapped_LumieraRecMutex()
          { lumiera_recmutex_destroy (&self, &NOBUG_FLAG(sync), NOBUG_CONTEXT); }
          
          bool lock (struct nobug_resource_user** handle, const struct nobug_context ctx = NOBUG_CONTEXT_NOFUNC)
          {
            return !!lumiera_recmutex_lock (&self, &NOBUG_FLAG(sync), handle, ctx);
          }
          
          void unlock (struct nobug_resource_user** handle,
                       const struct nobug_context ctx = NOBUG_CONTEXT_NOFUNC)
          {
            lumiera_recmutex_unlock (&self, &NOBUG_FLAG(sync), handle, ctx);
          }
          
          LumieraReccondition myreccond () {throw "bullshit"; return NULL;} // placeholder, brainstorm
        };
      
      
      struct Wrapped_LumieraExcCond
        {
          lumiera_condition self;
        protected:
          Wrapped_LumieraExcCond(const struct nobug_context ctx = NOBUG_CONTEXT_NOFUNC)
          { lumiera_condition_init    (&self, "Obj.Monitor ExclCondition", &NOBUG_FLAG(sync), ctx); }
         ~Wrapped_LumieraExcCond()
          { lumiera_condition_destroy (&self, &NOBUG_FLAG(sync), NOBUG_CONTEXT); }
          
          bool lock (struct nobug_resource_user** handle, const struct nobug_context ctx = NOBUG_CONTEXT_NOFUNC)
          {
            return !!lumiera_condition_lock (&self, &NOBUG_FLAG(sync), handle, ctx);
          }
          
          bool wait (struct nobug_resource_user** handle, const struct nobug_context ctx = NOBUG_CONTEXT_NOFUNC)
          {
            return !!lumiera_condition_wait (&self, &NOBUG_FLAG(sync), handle, ctx);
          }
          
          bool timedwait (const struct timespec* timeout,
                          struct nobug_resource_user** handle,
                          const struct nobug_context ctx = NOBUG_CONTEXT_NOFUNC)
          {
            return !!lumiera_condition_timedwait (&self, timeout, &NOBUG_FLAG(sync), handle, ctx);
          }
          
          void signal(const struct nobug_context ctx = NOBUG_CONTEXT_NOFUNC)
          {
            lumiera_condition_signal (&self, &NOBUG_FLAG(sync), ctx);
          }
          
          void broadcast(const struct nobug_context ctx = NOBUG_CONTEXT_NOFUNC)
          {
            lumiera_condition_broadcast (&self, &NOBUG_FLAG(sync), ctx);
          }
          
          void unlock (struct nobug_resource_user** handle, const struct nobug_context ctx = NOBUG_CONTEXT_NOFUNC)
          {
            lumiera_condition_unlock (&self, &NOBUG_FLAG(sync), handle, ctx);
          }
        };
      
      
      struct Wrapped_LumieraRecCond
        {
          lumiera_reccondition self;
        protected:
          Wrapped_LumieraRecCond(const struct nobug_context ctx = NOBUG_CONTEXT_NOFUNC)
          { lumiera_reccondition_init    (&self, "Obj.Monitor RecCondition", &NOBUG_FLAG(sync), ctx); }
         ~Wrapped_LumieraRecCond()
          { lumiera_reccondition_destroy (&self, &NOBUG_FLAG(sync), NOBUG_CONTEXT); }
          
          bool lock (struct nobug_resource_user** handle, const struct nobug_context ctx = NOBUG_CONTEXT_NOFUNC)
          {
            return !!lumiera_reccondition_lock (&self, &NOBUG_FLAG(sync), handle, ctx);
          }
          
          bool wait (struct nobug_resource_user** handle, const struct nobug_context ctx = NOBUG_CONTEXT_NOFUNC)
          {
            return !!lumiera_reccondition_wait (&self, &NOBUG_FLAG(sync), handle, ctx);
          }
          
          bool timedwait (const struct timespec* timeout,
                          struct nobug_resource_user** handle,
                          const struct nobug_context ctx = NOBUG_CONTEXT_NOFUNC)
          {
            return !!lumiera_reccondition_timedwait (&self, timeout, &NOBUG_FLAG(sync), handle, ctx);
          }
          
          void signal(const struct nobug_context ctx = NOBUG_CONTEXT_NOFUNC)
          {
            lumiera_reccondition_signal (&self, &NOBUG_FLAG(sync), ctx);
          }
          
          void broadcast(const struct nobug_context ctx = NOBUG_CONTEXT_NOFUNC)
          {
            lumiera_reccondition_broadcast (&self, &NOBUG_FLAG(sync), ctx);
          }
          
          void unlock (struct nobug_resource_user** handle, const struct nobug_context ctx = NOBUG_CONTEXT_NOFUNC)
          {
            lumiera_reccondition_unlock (&self, &NOBUG_FLAG(sync), handle, ctx);
          }
        };
      
      
      
      
      /* ========== abstractions defining the usable synchronisation primitives ============== */
      
      template<class MTX>
      class Mutex
        : protected MTX
        {
        protected:
          NobugResourceHandle& 
          _usage()
            {
              return NobugResourceHandle::access();
            }
          
         ~Mutex () { }
          Mutex () { }
          Mutex (const Mutex&); ///< noncopyable...
          const Mutex& operator= (const Mutex&);
          
        public:
            void
            acquire()
              {
                MTX::lock (_usage());
              }
            
            void
            release()
              {
                MTX::unlock (_usage());
              }
        };
      
      
      class Timeout;
      
      
      template<class CDX>
      class Condition
        : public Mutex<CDX>
        {
          typedef Mutex<CDX> _PM;
          
        public:
          void
          signal (bool wakeAll=false)
            {
              if (wakeAll)
                CDX::broadcast ();
              else
                CDX::signal ();
            }
          
          
          template<class BF>
          bool
          wait (BF& predicate, Timeout& waitEndTime)
            {
              bool ok = true;
              while (ok && !predicate())
                if (waitEndTime)
                  ok = CDX::timedwait (&waitEndTime, this->_usage());
                else
                  ok = CDX::wait (this->_usage());
              
              if (!ok && lumiera_error_expect(LUMIERA_ERROR_LOCK_TIMEOUT)) return false;
              lumiera::throwOnError();   // any other error thows
              
              return true;
            }
        };
      
      
      /**
       * helper for specifying an optional timeout for an timed wait.
       * Wrapping a timespec-struct, it allows for easy initialisation
       * by a given relative offset.
       */
      struct Timeout
        : timespec
        {
          Timeout() { tv_sec=tv_nsec=0; }
          
          /** initialise to NOW() + offset (in milliseconds) */
          Timeout&
          setOffset (ulong offs)
            {
              if (offs)
                {
                  clock_gettime(CLOCK_REALTIME, this);
                  tv_sec   += offs / 1000;
                  tv_nsec  += 1000000 * (offs % 1000);
                  if (tv_nsec >= 1000000000)
                    {
                      tv_sec += tv_nsec / 1000000000;
                      tv_nsec %= 1000000000;
                }   }
              return *this;
            }
          
          operator bool() { return 0 != tv_sec; } // allows if (timeout_)....
        };
      
      
      
      /* ==== functor types for defining the waiting condition ==== */
      
      typedef volatile bool& Flag;
      
      struct BoolFlagPredicate
        {
          Flag flag_;
          BoolFlagPredicate (Flag f) : flag_(f) {}
         
          bool operator() () { return flag_; }
        };
      
      
      template<class X>
      struct BoolMethodPredicate
        {
          typedef bool (X::*Method)(void);
          
          X& instance_;
          Method method_;
          BoolMethodPredicate (X& x, Method m) : instance_(x), method_(m) {}
         
          bool operator() () { return (instance_.*method_)(); }
        };
      
      
      
      /**
       * Object Monitor for synchronisation and waiting.
       * Implemented by a (wrapped) set of sync primitives,
       * which are default constructible and noncopyable.
       */
      template<class IMPL>
      class Monitor
        : IMPL
        {
          Timeout timeout_;
          
        public:
          Monitor() {}
          ~Monitor() {}
          
          /** allow copy, without interfering with the identity of IMPL */
          Monitor (Monitor const& ref) : IMPL(), timeout_(ref.timeout_) { }
          const Monitor& operator= (Monitor const& ref) { timeout_ = ref.timeout_; }
          
          
          void acquireLock() { IMPL::acquire(); }
          void releaseLock() { IMPL::release(); }
          
          void signal(bool a){ IMPL::signal(a); }
          
          bool
          wait (Flag flag, ulong timedwait=0)
            {
              BoolFlagPredicate checkFlag(flag);
              return IMPL::wait(checkFlag, timeout_.setOffset(timedwait));
            }
          
          template<class X>
          bool
          wait (X& instance, bool (X::*method)(void), ulong timedwait=0)
            {
              BoolMethodPredicate<X> invokeMethod(instance, method);
              return IMPL::wait(invokeMethod, timeout_.setOffset(timedwait));
            }
          
          void setTimeout(ulong relative) {timeout_.setOffset(relative);}
          bool isTimedWait()              {return (timeout_);}
        };
      
      typedef Mutex<Wrapped_LumieraExcMutex> NonrecursiveLock_NoWait;
      typedef Mutex<Wrapped_LumieraRecMutex> RecursiveLock_NoWait;
      typedef Condition<Wrapped_LumieraExcCond>  NonrecursiveLock_Waitable;
      typedef Condition<Wrapped_LumieraRecCond>  RecursiveLock_Waitable;
      
      
    } // namespace sync (helpers and building blocks)
    
    
    
    
    
    
    
    /* Interface to be used by client code:
     * Inherit from class Sync with a suitable Policy.
     * Then use the embedded Lock class.
     */
    
    /* =======  Policy classes  ======= */
    
    using sync::NonrecursiveLock_NoWait;
    using sync::NonrecursiveLock_Waitable;
    using sync::RecursiveLock_NoWait;
    using sync::RecursiveLock_Waitable;
    
    
    /*************************************************************************
     * Facility for monitor object based locking.
     * To be attached either on a per class base or per object base.
     * Typically, the client class will inherit from this template (but it
     * is possible to use it stand-alone, if inheriting isn't an option).
     * The interface for clients to access the functionality is the embedded
     * Lock template, which should be instantiated as an automatic variable
     * within the scope to be protected.
     *
     */
    template<class CONF = NonrecursiveLock_NoWait>
    class Sync
      {
        typedef sync::Monitor<CONF> Monitor;
        mutable Monitor objectMonitor_;
        
        static Monitor&
        getMonitor(const Sync* forThis)
          {
            REQUIRE (forThis);
            return forThis->objectMonitor_;
          }
        
        
      public:
        class Lock
          : sync::NobugResourceHandle
          {
            Monitor& mon_;
            
          public:
            template<class X>
            Lock(X* it) : mon_(getMonitor(it)){ mon_.acquireLock(); }
            ~Lock()                           { mon_.releaseLock(); }
            
            void notify()                     { mon_.signal(false);}
            void notifyAll()                  { mon_.signal(true); }
            void setTimeout(ulong time)       { mon_.setTimeout(time); }
            bool isTimedWait()                { return mon_.isTimedWait(); }
            
            template<typename C>
            bool
            wait (C& cond, ulong timeout=0)
              {
                return mon_.wait(cond,timeout);
              }
            
            template<typename X>
            bool
            wait  (X& instance, bool (X::*predicate)(void), ulong timeout=0)
              {
                return mon_.wait(instance,predicate,timeout);
              }
            
            /** convenience shortcut:
             *  Locks and immediately enters wait state,
             *  observing a condition defined as member function. */
            template<class X>
            Lock(X* it, bool (X::*method)(void))
              : mon_(getMonitor(it)) 
              { 
                mon_.acquireLock();
                mon_.wait(*it,method);
              }
            
          protected:
            /** for creating a ClassLock */
            Lock(Monitor& m) : mon_(m)
              { mon_.acquireLock(); }
            
            /** for controlled access to the
             * underlying sync primitives */
            Monitor&
            accessMonitor() { return mon_; }
          };
        
        
      };
  
  
  
  
} // namespace lumiera
#endif
