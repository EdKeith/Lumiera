/*
  ThreadWrapperJoin(Test)  -  wait blocking on termination of a thread
 
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

#include "lib/symbol.hpp"
#include "backend/thread-wrapper.hpp"
#include "lib/error.hpp"
#include "lib/sync.hpp"

#include <tr1/functional>

using std::tr1::bind;
using test::Test;



namespace backend {
  namespace test  {
  
    /**************************************************************************
     * @test use the Lumiera backend to create some new threads, additionally
     *       passing an condition variable for waiting on thread termination.
     *       Actually this is implemented as creating and passing a JoinHandle.
     *
     * @see backend::Thread
     * @see threads.h
     */
    class ThreadWrapperJoin_test : public Test
      {
        
        virtual void
        run (Arg)
          {
            simpleUse ();
            wrongUse ();
          }
        
        
        volatile int aValue_;         ///< state to be modified by the other thread
        
        void
        theAction (int secretValue)   ///< to be run in a new thread...
          {
            usleep (100000);          // pause 100ms prior to modifying
            aValue_ =  secretValue+42;
          }
        
        
        void
        simpleUse ()
          {
            aValue_=0;
            int mySecret = (rand() % 1000) - 500;
            
            JoinHandle waitingHandle;
            
            Thread("test Thread joining",
                   bind (&ThreadWrapperJoin_test::theAction, this, mySecret),
                   waitingHandle);
                                      // note binding and thread wrapper already destroyed
            waitingHandle.join();     // blocks until theAction() is done
            
            CHECK (aValue_ == mySecret+42);
          }
        
        
        void
        wrongUse ()
          {
            JoinHandle waitingHandle;
            
            Thread("test Thread joining-1",
                   bind (&ThreadWrapperJoin_test::theAction, this, 111));
                                      // note we "forget" to pass the JoinHandle
            try
              {
                waitingHandle.join(); // protocol error: handle wasn't passed for starting a Thread;
                NOTREACHED();
              }
            catch (lumiera::error::Logic& logo)
              { lumiera_error(); }
            
            
            Thread("test Thread joining-2",
                   bind (&ThreadWrapperJoin_test::theAction, this, 222),
                   waitingHandle);    // this time we pass it....
            
#ifdef DEBUG
            /////////////////////////////////////////////////////////////////////////////////////////////TODO: better way of detecting debug builds
#if false   /////////////////////////////////////////////////////////////////////////////////////////////TODO: re-enable assertions to throw, and make this configurable
            try
              {
                Thread("test Thread joining-3",
                       bind (&ThreadWrapperJoin_test::theAction, this, 333),
                       waitingHandle);    // but then pass it again for another thread....
                NOTREACHED();
              }
            catch (...)
              {
                CHECK (lumiera_error() == lumiera::error::LUMIERA_ERROR_ASSERTION);
              }
#endif
#endif            
            
            // note: the waitingHandle goes out of scope here,
            // which unblocks the second thread. The first thread wasn't blocked,
            // while the third thread wasn't created at all.

            waitingHandle.join();       // just making the above thing pass, JoinHandle thows when not joined and going out of scope
                                       // the semantics herer need to be defined (auto joining? see thread-wrapper.hpp)
          }
        
      public:
        ThreadWrapperJoin_test()
        { lumiera_threadpool_init(); }
        
        ~ThreadWrapperJoin_test()
        { lumiera_threadpool_destroy(); }
        
      };
    
    
    
    /** Register this test class... */
    LAUNCHER (ThreadWrapperJoin_test, "function common");
    
    
    
  } // namespace test

} // namespace backend
