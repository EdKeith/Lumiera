/*
  APPSTATE.hpp  -  application initialisation and behaviour 
 
  Copyright (C)         Lumiera.org
    2008,               Hermann Vosseler <Ichthyostega@web.de>
 
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

/** @file appstate.hpp
 ** Registering and managing some application-global services.
 ** Besides encapsulating the logic for starting up the fundamental parts
 ** of the application, there is a mechanism for registering and firing off
 ** application lifecycle event callbacks. 
 **
 ** @see LifecycleHook
 ** @see main.cpp
 ** @see nobugcfg.h
 */


#ifndef LUMIERA_APPSTATE_H
#define LUMIERA_APPSTATE_H

#include "include/symbol.hpp"
#include "lumiera/option.hpp"
#include "lumiera/subsys.hpp"

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <string>
#include <map>



namespace lumiera {
  
  using std::string;
  using boost::scoped_ptr;
  using boost::noncopyable;
  
  class LifecycleRegistry;
  class SubsystemRunner;
  
  
  /**
   * Singleton to hold global flags directing the overall application behaviour,
   * for triggering lifecycle events and performing early initialisation tasks.
   * AppState services are available already from static initialisation code.
   * @warning don't use AppState in destructors.
   */
  class AppState
    : private noncopyable
    {
    private:
      AppState ();
      
      ~AppState ();
      friend void boost::checked_delete<AppState>(AppState*);
      
      
    public:
      /** get the (single) AppState instance. 
       *  @warning don't use it after the end of main()! */
      static AppState& instance();
      
      
      /** fire off all lifecycle callbacks
       *  registered under the given label  */
      static void lifecycle (Symbol eventLabel);
      
      
      /** evaluate the result of option parsing and maybe additional configuration
       *  such as to be able to determine the further behaviour of the application.
       *  Set the internal state within this object accordingly. */
      void init (lumiera::Option& options);
      
      
      /** building on the state determined by #evaluate, decide if the given Subsys
       *  needs to be pulled up and, if necessary, register the Subsys and its
       *  prerequisites to be maintained throughout the application's lifetime. */
      void maybeStart (lumiera::Subsys&);
      
      
      enum ExitCode {
        NORMAL_EXIT,
        CLEAN_EXIT_AFTER_ERROR,
        CLEAN_EMERGENCY_EXIT,
        FAILED_EMERGENCY_EXIT
      };
      
      /** put the main thread of the application into a wait state, if some subsystem(s)
       *  registered with #maybeStart still need to be maintained. On termination of
       *  one of those components, tear down the remaining components and initiate
       *  a normal or emergency shutdown of the application, depending on the
       *  triggering component's mode of termination (exit or exception). 
       *  @return global application exit code */
      ExitCode maybeWait();
      
      
      /** initiate the controlled error shutdown sequence
       *  @param problem causing exception */
      ExitCode abort (lumiera::Error& problem);
      
      
      /** initiate an fatal emergency shutdown,
       *  caused by an unforeseen error condition */
      ExitCode abort ()  throw();
      
      
    
    private:
      typedef scoped_ptr<LifecycleRegistry> PLife;
      typedef scoped_ptr<SubsystemRunner>   PSub;
      
      PLife lifecycleHooks_;
      PSub  subsystems_;
      
      bool emergency_;
      bool core_up_;
      
      friend class LifecycleHook;
      
    };
  
  
} // namespace lumiera

#endif