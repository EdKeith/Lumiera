/*
  GuiFacade  -  access point for communicating with the Lumiera GTK GUI
 
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


#include "gui/guifacade.hpp"
#include "common/singleton.hpp"

#include <string>


namespace gui {
  
  using std::string;
  
  class GuiSubsysDescriptor
    : public lumiera::Subsys
    {
      operator string ()  const { return "Lumiera GTK GUI"; }
      
      bool 
      shouldStart (lumiera::Option&)
        {
          UNIMPLEMENTED ("determine, if a GUI is needed");
          return false;
        }
      
      bool
      start (lumiera::Option&, Subsys::SigTerm termination)
        {
          UNIMPLEMENTED ("load and start the GUI and register shutdown hook");
          return false;
        }
      
      void
      triggerShutdown ()  throw()
        {
          UNIMPLEMENTED ("initiate closing the GUI");
        }
      
    };
  
  namespace {
    lumiera::Singleton<GuiSubsysDescriptor> theDescriptor;
  }
  
  
  
  
  /** @internal intended for use by main(). */
  lumiera::Subsys&
  GuiFacade::getDescriptor()
  {
    return theDescriptor();
  }



} // namespace gui
