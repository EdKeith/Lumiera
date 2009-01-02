/*
  SyncClasslock(Test)  -  validate the type-based Monitor locking
 
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


#include "lib/test/run.hpp"
#include "lib/error.hpp"

#include "lib/sync-classlock.hpp"

//#include <iostream>

//using std::cout;
using test::Test;


namespace lib {
  namespace test {
  
    namespace { // private test classes and data...
    
      const uint NUM_INSTANCES = 20;   ///< number of probe instances to create
      
      
      /** 
       * instances of this probe class will be created statically.
       * They utilise the class-based locking within ctor and dtor 
       */
      struct Probe
        {
          Probe() { ClassLock<Probe> ctor_lock; }
         ~Probe() { ClassLock<Probe> dtor_lock; }
        };
  
      
    } // (End) test classes and data....
    
    
    
    
    
    
    
    
    
    
    /**************************************************************************
     * @test check proper handling of class (not instance)-based Monitor locks.
     * Because no instance is available in this case, a hidden storage for the
     * Monitor object needs to be provided in a way safe for use even in the
     * static startup/shutdown phase. This test validates the associated
     * refcounting and object creation works as expected. It does \em not
     * validate the locking functionality as such.
     * 
     * @see sync.hpp
     */
    class SyncClasslock_test : public Test
      {
        
        virtual void
        run (Arg) 
          {
            static Probe objs[NUM_INSTANCES];
            
            ClassLock<Probe> get_class_lock;
            ASSERT ( 1 ==  get_class_lock.use_count()); // ClassLock<Probe> got created exactly once
          }
        
      };
    
      
    
    /** Register this test class... */
    LAUNCHER (SyncClasslock_test, "unit common");
    
    
    
  } // namespace test

} // namespace lumiera