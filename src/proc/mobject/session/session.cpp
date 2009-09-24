/*
  Session  -  holds the complete session to be edited by the user
 
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
 
* *****************************************************/


/** @file session.cpp
 ** Actual connection between the Session interface and its Implementation.
 ** Holds the storage for the SessionManager implementation (singleton)
 ** 
 ** @see session::SessionImpl
 ** @see session::SessionManagerImpl
 **
 */


#include "proc/mobject/session.hpp"
#include "proc/mobject/session/defsmanager.hpp"
#include "proc/mobject/session/session-impl.hpp"

#include "lib/symbol.hpp"
#include "lib/singleton.hpp"


using lib::Symbol;
using lumiera::Singleton;
using mobject::session::SessManager;
using mobject::session::SessManagerImpl;

namespace mobject {
  
  /** the sole access point for all client code to the system-wide
   *  "current session". Implemented as smart pointer to singleton
   *  implementation object, where the smart pointer is actually
   *  the SessionManager (which is singleton as well...).
   * 
   *  Consequently, if you want to talk to the <i>session manager,</i>
   *  you use dot-notation, while you access the <i>session object</i>
   *  via arrow notaion (e.g. \code Session::current->getFixture() )
   */
  SessManager& Session::current = Singleton<SessManagerImpl>()();
  
  
  /** \par
   *  LifecycleHook, to perform all the basic setup for a new session,
   *  prior to adding any specific data, configuration or content. Any subsystems
   *  requiring to (re)-initialise for a new session should register here. When this
   *  hook is activated, the session implementation facilities are available and the
   *  corresponding interfaces are already opened and accessible, but the session itself
   *  is completely pristine and empty.
   *  @note plugins providing additional facilities to be used by content of a (persisted)
   *        session should register their basic setup functions using this hook, which can be
   *        done via the C interface functions defined in lifecycle.h
   */
  const char* ON_SESSION_START ("ON_SESSION_START");
  
  /** \par
   *  LifecycleHook, to perform any initialisation, wiring and registrations necessary
   *  to get the session into a usable state. When activated, the specific session content
   *  and configuration has already be loaded. Any subsystems requiring to build some indices
   *  or wiring to keep track of the session's content should register here.
   */
  const char* ON_SESSION_INIT ("ON_SESSION_INIT");
  
  /** \par
   *  LifecycleHook, to perform any state saving, deregistration or de-activation necessary 
   *  before bringing down an existing session. When invoked, the session is still fully valid
   *  and functional, but the GUI/external access has already been closed.
   *  @todo specify how this is related to "saving". Is there a way for subsystems to add
   *        specific/internal information into the persisted state, besides actually attaching
   *        data to objects within the session?
   */
  const char* ON_SESSION_END ("ON_SESSION_END");
  
  
  
  
  Session::~Session () 
  { }
  
  Session::Session (session::DefsManager& def)  throw()
    : defaults(def)
  { }



} // namespace mobject
