/*
  Notification(Proxy)  -  public service allowing to push information into the GUI

  Copyright (C)         Lumiera.org
    2008,               Hermann Vosseler <Ichthyostega@web.de>

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

* *****************************************************/



/** @file notification-interface-proxy.hpp
 ** This is an implementation fragment, intended to be included into common/interfaceproxy.cpp
 ** 
 ** The purpose is to define a proxy implementation of gui::GuiNotification, in order to
 ** redirect any calls through the associated C Language Interface "lumieraorg_GuiNotification"
 **
 ** @see guinotification-facade.hpp
 ** @see notification-service.hpp actual implementation within the GUI 
 */





    /* ==================== GuiNotification =================================== */
    
#include "include/guinotification-facade.h"

namespace gui {
  
  /** storage for the facade proxy factory
   *  used by client code to invoke through the interface */
  lumiera::facade::Accessor<GuiNotification> GuiNotification::facade;

} // namespace gui



namespace lumiera {
  namespace facade {
    
    typedef InstanceHandle< LUMIERA_INTERFACE_INAME(lumieraorg_GuiNotification, 0)
                          , gui::GuiNotification
                          > IHandle_GuiNotification;
    
    
    template<>
    class Proxy<IHandle_GuiNotification>
      : public Holder<IHandle_GuiNotification>
      {
        //----Proxy-Implementation-of-GuiNotification--------
        
        void displayInfo (string const& text)           { _i_.displayInfo (cStr(text)); }
        void triggerGuiShutdown (string const& cause)   { _i_.triggerGuiShutdown (cStr(cause)); }
        
        
      public:
        Proxy (IHandle const& iha) : THolder(iha) {}
      };
    
    
    template  void openProxy<IHandle_GuiNotification>  (IHandle_GuiNotification const&);
    template  void closeProxy<IHandle_GuiNotification> (void);
    
   } // namespace facade
  
} // namespace lumiera
