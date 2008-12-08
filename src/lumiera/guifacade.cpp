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
#include "include/guinotificationfacade.h"
#include "include/error.hpp"
#include "common/singleton.hpp"
#include "lib/functorutil.hpp"
#include "lumiera/instancehandle.hpp"

#include <boost/scoped_ptr.hpp>
#include <tr1/functional>
#include <string>


namespace gui {
  
  using std::string;
  using boost::scoped_ptr;
  using std::tr1::bind;
  using std::tr1::placeholders::_1;
  using lumiera::Subsys;
  using lumiera::InstanceHandle;
  using util::dispatchSequenced;

  
  /** path to the dynamic module to load the GUI from
   *  @todo we need an actual solution. Either find it in the same directory,
   *  or the lib dir or otherwise retrieve the path from the config system */
  const string LUMIERA_GuiStarterPlugin_path ("liblumiera-plugin.so");
  
  inline void
  discover_GuiStarterPlugin ()
    {
      LumieraPlugin GuiP = lumiera_plugin_lookup (LUMIERA_GuiStarterPlugin_path.c_str());
      if (!GuiP || lumiera_error_peek())
        throw lumiera::error::Config("unable to load the GUI module from \""
                                    +LUMIERA_GuiStarterPlugin_path+"\"");
    }
  
  
  
  
  struct GuiRunner
    : public GuiFacade
    {
      typedef InstanceHandle<LUMIERA_INTERFACE_INAME(lumieraorg_Gui, 1)> GuiHandle;
      
      GuiHandle theGUI_;
      
      
      GuiRunner (Subsys::SigTerm terminationHandle)
        : theGUI_("lumieraorg_Gui", 1, 1, "lumieraorg_GuiStarterPlugin") // load GuiStarterPlugin
        {
          ASSERT (theGUI_);
          if (!kickOff (terminationHandle))
            throw lumiera::error::Fatal("failed to bring up GUI",lumiera_error());
        }
      
      ~GuiRunner () {  }
      
      
      bool kickOff (Subsys::SigTerm& terminationHandle) 
        { 
          return theGUI_->kickOff (reinterpret_cast<void*> (&terminationHandle))
              && !lumiera_error_peek();
        }
    };
  
  
  
  
  namespace { // implementation details : start GUI through GuiStarterPlugin
    
    scoped_ptr<GuiRunner> facade (0);
    
    class GuiSubsysDescriptor
      : public lumiera::Subsys
      {
        operator string ()  const { return "Lumiera GTK GUI"; }
        
        bool 
        shouldStart (lumiera::Option& opts)
          {
            if (opts.isHeadless() || 0 < opts.getPort())
              {
                INFO (lumiera, "*not* starting the GUI...");
                return false;
              }
            else
              return true;
          }
        
        bool
        start (lumiera::Option&, Subsys::SigTerm termination)
          {
            //Lock guard (*this);
            if (facade) return false; // already started
            
            discover_GuiStarterPlugin(); ////TODO temporary solution

            facade.reset (
              new GuiRunner (                            // trigger loading load the GuiStarterPlugin...
                dispatchSequenced( closeOnTermination_  //  on termination call this->closeGuiModule(*) first
                                 , termination)));     //...followed by invoking the given termSignal
            return true;
          }
        
        void
        triggerShutdown ()  throw()
          {
            try { GuiNotification::facade().triggerGuiShutdown ("Application shutdown"); }
            
            catch (...){}
          }
        
        bool 
        checkRunningState ()  throw()
          {
            //Lock guard (*this);
            return (facade);
          }
        
        void
        closeGuiModule (lumiera::Error *)
          {
            //Lock guard (*this);
            if (!facade)
              {
                TRACE (operate, "duplicate? call of the termination signal, "
                                "GUI is currently closed.");
              }
            else
              facade.reset (0);
          }
        
        
        Subsys::SigTerm closeOnTermination_;
        
      public:
        GuiSubsysDescriptor()
          : closeOnTermination_ (bind (&GuiSubsysDescriptor::closeGuiModule, this, _1))
          { }
        
      };
    
    lumiera::Singleton<GuiSubsysDescriptor> theDescriptor;
    
  } // (End) impl details
  
  
  
  
  
  
  /** @internal intended for use by main(). */
  lumiera::Subsys&
  GuiFacade::getDescriptor()
  {
    return theDescriptor();
  }
  
  
  bool
  GuiFacade::isUp ()
  {
    return (facade);
  }



} // namespace gui
