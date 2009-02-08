/*
  DisplayService  -  service providing access to a display for outputting frames
 
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
 
* *****************************************************/


#include "gui/display-service.hpp"
//#include "lib/singleton.hpp"

extern "C" {
#include "common/interfacedescriptor.h"
}

#include <string>
//#include <memory>
//#include <boost/scoped_ptr.hpp>


namespace gui {
  
  using std::string;
//    using boost::scoped_ptr;
  
  
  namespace { // hidden local details of the service implementation....
    
    
    
    /* ================== define an lumieraorg_GuiNotification instance ======================= */
    
    LUMIERA_INTERFACE_INSTANCE (lumieraorg_interfacedescriptor, 0
                               ,lumieraorg_DisplayFacade_descriptor
                               , NULL, NULL, NULL
                               , LUMIERA_INTERFACE_INLINE (name, "\323\343\324\023\064\216\120\201\073\056\366\020\110\263\060\023",
                                                           const char*, (LumieraInterface ifa),
                                                             { (void)ifa;  return "Display"; }
                                                          )
                               , LUMIERA_INTERFACE_INLINE (brief, "\305\026\070\133\033\357\014\202\203\270\174\072\341\256\226\235",
                                                           const char*, (LumieraInterface ifa),
                                                             { (void)ifa;  return "UI Interface: service for outputting frames to a viewer or display"; }
                                                          )
                               , LUMIERA_INTERFACE_INLINE (homepage, "\170\104\246\175\123\144\332\312\315\263\071\170\164\213\024\275",
                                                           const char*, (LumieraInterface ifa),
                                                             { (void)ifa;  return "http://www.lumiera.org/develompent.html" ;}
                                                          )
                               , LUMIERA_INTERFACE_INLINE (version, "\265\343\045\346\110\241\276\111\217\120\155\246\230\341\344\124",
                                                           const char*, (LumieraInterface ifa),
                                                             { (void)ifa;  return "0.1~pre"; }
                                                          )
                               , LUMIERA_INTERFACE_INLINE (author, "\302\027\122\045\301\166\046\236\257\253\144\035\105\166\070\103",
                                                           const char*, (LumieraInterface ifa),
                                                             { (void)ifa;  return "Hermann Vosseler"; }
                                                          )
                               , LUMIERA_INTERFACE_INLINE (email, "\074\013\020\161\075\135\302\265\260\000\301\147\116\355\035\261",
                                                           const char*, (LumieraInterface ifa),
                                                             { (void)ifa;  return "Ichthyostega@web.de"; }
                                                          )
                               , LUMIERA_INTERFACE_INLINE (copyright, "\037\232\153\100\114\103\074\342\164\132\370\210\372\164\115\275",
                                                           const char*, (LumieraInterface ifa),
                                                             {
                                                               (void)ifa;
                                                               return
                                                                 "Copyright (C)        Lumiera.org\n"
                                                                 "  2009               Hermann Vosseler <Ichthyostega@web.de>";
                                                             }
                                                          )
                               , LUMIERA_INTERFACE_INLINE (license, "\026\243\334\056\125\245\315\311\155\375\262\344\007\076\341\254",
                                                           const char*, (LumieraInterface ifa),
                                                             {
                                                               (void)ifa;
                                                               return
                                                                 "This program is free software; you can redistribute it and/or modify\n"
                                                                 "it under the terms of the GNU General Public License as published by\n"
                                                                 "the Free Software Foundation; either version 2 of the License, or\n"
                                                                 "(at your option) any later version.\n"
                                                                 "\n"
                                                                 "This program is distributed in the hope that it will be useful,\n"
                                                                 "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                                                                 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
                                                                 "GNU General Public License for more details.\n"
                                                                 "\n"
                                                                 "You should have received a copy of the GNU General Public License\n"
                                                                 "along with this program; if not, write to the Free Software\n"
                                                                 "Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA";
                                                             }
                                                          )
                               , LUMIERA_INTERFACE_INLINE (state, "\243\302\332\160\060\272\155\334\212\256\303\141\160\063\164\154",
                                                           int, (LumieraInterface ifa),
                                                             {(void)ifa;  return LUMIERA_INTERFACE_EXPERIMENTAL; }
                                                          )
                               , LUMIERA_INTERFACE_INLINE (versioncmp, "\363\125\123\060\231\147\053\017\131\341\105\157\231\273\334\136",
                                                           int, (const char* a, const char* b),
                                                             {return 0;}  ////////////////////////////////////////////TODO define version ordering
                                                          )
                               );
    
    
    
    
    
    using lumiera::facade::LUMIERA_ERROR_FACADE_LIFECYCLE;
    typedef lib::SingletonRef<DisplayService>::Accessor InstanceRef;
    
    InstanceRef _instance; ///< a backdoor for the C Language impl to access the actual DummyPlayer implementation...
    
    
    
    LUMIERA_INTERFACE_INSTANCE (lumieraorg_Display, 0
                               ,lumieraorg_DisplayService
                               , LUMIERA_INTERFACE_REF(lumieraorg_interfacedescriptor, 0, lumieraorg_DisplayFacade_descriptor)
                               , NULL /* on  open  */
                               , NULL /* on  close */
                               , LUMIERA_INTERFACE_INLINE (allocate, "\177\221\146\253\255\161\160\137\015\005\263\362\307\022\243\365",
                                                           void, (LumieraDisplaySlot slotHandle),
                                                             { 
                                                               if (!_instance)
                                                                 { 
                                                                   lumiera_error_set(LUMIERA_ERROR_FACADE_LIFECYCLE, 0);
                                                                   return;
                                                                 }
                                                               
                                                               REQUIRE (slotHandle);
                                                               try
                                                                 {
                                                                 _instance->allocate (slotHandle,true); 
                                                                 }
                                                               catch (lumiera::Error&){ /* error state remains set */ }
                                                             }
                                                          )
                               , LUMIERA_INTERFACE_INLINE (release, "\166\374\106\313\011\142\115\161\111\110\376\016\346\115\240\364",
                                                           void, (LumieraDisplaySlot slotHandle),
                                                             { 
                                                               if (!_instance)
                                                                 { 
                                                                   lumiera_error_set(LUMIERA_ERROR_FACADE_LIFECYCLE, 0);
                                                                   return;
                                                                 }
                                                               
                                                               REQUIRE (slotHandle);
                                                               _instance->allocate (slotHandle,false);
                                                             }
                                                          )
                               , LUMIERA_INTERFACE_INLINE (put, "\340\062\234\227\152\131\370\272\146\207\224\015\361\070\252\135",
                                                           void, (LumieraDisplaySlot slotHandle, LumieraDisplayFrame frame),
                                                             { 
                                                               //skipping full checks for performance reasons
                                                               REQUIRE (_instance && !lumiera_error_peek());
                                                               
                                                               REQUIRE (slotHandle);
                                                               DisplayerSlot& slot = _instance->resolve (slotHandle);
                                                               slot.put (frame);
                                                             }
                                                          )
                               );
    
    
    
    
  } // (End) hidden service impl details
  
  
  
  
  DisplayService::DisplayService()
    : error_("")
    , implInstance_(this,_instance)
    , serviceInstance_( LUMIERA_INTERFACE_REF (lumieraorg_Display, 0, lumieraorg_DisplayService))
  {
    INFO (progress, "Display Facade opened.");
  }
  
  
  
  LumieraDisplaySlot
  DisplayService::setUp (FrameDestination const& outputDestination)
  {
    DisplayerTab& slots (_instance->slots_);
    return &slots.manage (new DisplayerSlot (outputDestination));
  }
  
  
  
  void
  DisplayService::allocate (LumieraDisplaySlot handle, bool doAllocate)
  {
    REQUIRE (handle);
    if (doAllocate)
      {
        if (handle->put_)
          throw lumiera::error::Logic("slot already allocated for output");
        else
          // Mark the handle as "allocated" and ready for output: 
          // Place the function pointer from the C interface into the handle struct.
          // calling it will invoke the implementing instance's "put" function
          // (see the LUMIERA_INTERFACE_INLINE above in this file!)
          handle->put_ = serviceInstance_.get().put;
      }
    else
      handle->put_ = 0;
  }
  
  
  
  DisplayerSlot& 
  DisplayService::resolve (LumieraDisplaySlot handle)
  {
    REQUIRE (handle);
    REQUIRE (handle->put_, "accessing a DisplayerSlot, which hasn't been locked for output");
    
    return *static_cast<DisplayerSlot*> (handle);
  }
  
  
  
  
  
  /* === DisplayerSlot Implementation === */
  
  
  DisplayerSlot::DisplayerSlot (FrameDestination const& outputDestination)
    : currBuffer_(0)
  { 
    put_ = 0; // mark as not allocated
    hasFrame_.connect (outputDestination);
    dispatcher_.connect (sigc::mem_fun (this, &DisplayerSlot::displayCurrentFrame));
  }
  
  
  DisplayerSlot::~DisplayerSlot() 
  {
    TRACE (gui_dbg, "Displayer Slot closing...");
  }
  
  
  void
  DisplayerSlot::displayCurrentFrame()
  {
    hasFrame_.emit (currBuffer_);
  }
  
  
} // namespace proc
