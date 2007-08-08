/*
  BUILDERFACADE.hpp  -  Facade and service access point for the Builder Subsystem
 
  Copyright (C)         CinelerraCV
    2007,               Christian Thaeter <ct@pipapo.org>
 
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


#ifndef PROC_MOBJECT_BUILDERFACADE_H
#define PROC_MOBJECT_BUILDERFACADE_H

#include "proc/engine/renderengine.hpp"



namespace proc
  {
  namespace mobject
    {
    
    
    /**
     * Provides unified access to the builder functionality.
     * While individual components of the builder subsystem may be called
     * if necessary or suitable, it is usually better to do all extern invocations
     * via the high level methods of this Facade.
     */
    class BuilderFacade
      {
      public:
        /**
         * Main Operation of the Builder:
         * create a render engine for a given part of the timeline
         */
        proc::engine::RenderEngine & buildEngine () ;
        // TODO: allocation, GC??????
      };
      
      
      
  } // namespace proc::mobject

} // namespace proc
#endif
