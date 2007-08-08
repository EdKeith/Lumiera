/*
  SESSION.hpp  -  holds the complete session to be edited by the user
 
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


#ifndef PROC_MOBJECT_SESSION_SESSION_H
#define PROC_MOBJECT_SESSION_SESSION_H



namespace proc
  {
  namespace mobject
    {
    namespace session
      {
      
      class EDL;
      class Fixture;
      
      /**
       * The (current) Session holds all the user
       * visible content to be edited and manipulated
       * within the Cinelerra Application. From a users
       * perspective, it is a collection of Media Objects
       * (--> MObject) placed (--> Placement) onto virtual
       * Tracks.
       */
      class Session
        {
        protected:
          EDL& edl;
          Fixture& fixture;

        };
        
        
        
    } // namespace proc::mobject::session

  } // namespace proc::mobject

} // namespace proc
#endif
