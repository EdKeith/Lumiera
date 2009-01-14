/*
  INTERFACEPROXY  -  definition of forwarding proxies for the facade interfaces
 
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

/** @file interfaceproxy.hpp
 ** Facade Interfaces Lifecycle. Communication between the Layers within Lumiera
 ** usually is routed through <b>Layer Separation Interfaces</b>. These are comprised
 ** of a Facade interface and a equivalent rendering as C Language interface defined
 ** with the help of the Interface/Plugin system. But in order to be able to actually
 ** access a service via this Facade, you need an instance of the interface.
 ** 
 ** lumiera::facade::Proxy and InstanceHandle together are used to create such an concrete
 ** instance of the Facade interface. It is implemented such as to route each call
 ** through the corresponding C Language function defined in the Interface/Plugin system.
 ** Typically there is another subclass of the Facade interfaces sitting "on the other side"
 ** of the interface barrier and actually implementing the functionality. The template
 ** facade::Accessor can be thought of as a factory creating such a proxy instance of the
 ** facade interface for the client code to use. Typically, in instance of the \em factory
 ** is embedded (as a static functor member object) right within the otherwise abstract 
 ** facade interface, this way allowing the client code to write e.g. \c XYZInterface::facade()
 ** to yield a reference to a proxy object implementing \c XYZInterface.
 ** 
 ** Interface Lifecycle
 ** 
 ** Instances of an Interface are either directly provided by some facility within the core,
 ** or they are loaded from a shared module (plugin). In either case this means the interface
 ** isn't accessible all the time, rather it comes up at a defined point in the application
 ** lifecycle and similarly will be shut down deliberately at some point. Beyond this time
 ** window of availability, any access through the proxy factory throws an lumiera::error::State.
 ** Any sort of dependency management is outside the scope of the InstanceHandle (for the core
 ** services, it is handled by the dependency of subsystems, while the plugin loader cares
 ** for dependency issues regarding loadable modules, thereby building on the deployment
 ** descriptors.
 ** 
 ** For the Layer separation interfaces, the process of loading and opening is abstracted as
 ** an InstanceHandle object. When creating such an InstanceHandle using the appropriate
 ** template and ctor parameters, in addition to the registration with the Interface/Plugin 
 ** system, the corresponding facade::Proxy factory is addressed and "opened" by creating
 ** the right proxy object instance. Similarly, when the InstanceHandle object goes out
 ** of scope, prior to detaching from the Interface/Proxy system, the corresponding 
 ** lumiera::facade::Accessor factory is "closed", which additionally means destroying
 ** the proxy object instance and switching any further access to throwing and exception. 
 **
 ** While client code just includes the interface header (including interfaceproxy.hpp
 ** in turn), there needs to be an actual implementation of each proxy object located in
 ** some translation unit. The usual place is interfaceproxy.cpp, which gets linked into 
 ** \c liblumieracommon.so and contains actual specialisations and literal forwarding
 ** code <i>for each individual facade.</i>
 **
 ** @see interface.h
 ** @see plugin.h
 ** @see lumiera::Subsys
 ** @see guinotification.h usage example (facade interface)
 ** @see guinotificationfacade.cpp corresponding implementation within the GUI
 */


#ifndef LUMIERA_INTERFACE_PROXY_H
#define LUMIERA_INTERFACE_PROXY_H


#include "lib/error.hpp"



namespace lumiera {
  namespace facade {
  
    /** error-ID for accessing a (currently) closed facade */
    LUMIERA_ERROR_DECLARE(FACADE_LIFECYCLE);  

  
    /*********************************************************************
     * 
     */
    template<class FA>
    class Accessor
      {
      protected:
        static FA* implProxy_;
        
        
      public:
        FA&
        operator() ()
          {
            if (implProxy_)
              return *implProxy_;
            else
              throw error::State("Facade interface currently closed.");
          }
      };
    
    template<class IHA>
    void openProxy (IHA const&);
    
    template<class IHA>
    void closeProxy ();
    
    template<class IHA>
    class Proxy;
    
    
  } // namespace facade
  
} // namespace lumiera

#endif