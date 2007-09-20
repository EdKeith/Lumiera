/*
  MULTITHREAD.hpp  -  generic interface for multithreading primitives
 
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



#ifndef CINELERRA_MULTITHREAD_H
#define CINELERRA_MULTITHREAD_H



namespace cinelerra
  {
      
    /**
     * Interface/Policy for managing parallelism issues.
     * Basically everything is forwarded to the corresponding backend functions,
     * because managing threads and locking belongs to the cinelerra backend layer.
     * 
     * @todo actually implement this policy using the cinelerra databackend.
     */
    struct Thread
      {
        template<class X>
        class Lock
          {
          public:
            Lock()    { TODO ("aquire Thread Lock for Class"); }
            Lock(X*)  { TODO ("aquire Thread Lock for Instance"); }
            ~Lock()   { TODO ("release Thread Lock"); }
          };
      };
    
  
} // namespace cinelerra
#endif