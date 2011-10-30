/*
  TestFrame(Test)  -  verify proper operation of dummy data frames

  Copyright (C)         Lumiera.org
    2011,               Hermann Vosseler <Ichthyostega@web.de>

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


#include "lib/test/run.hpp"
#include "proc/engine/testframe.hpp"

//#include <iostream>
#include <boost/scoped_ptr.hpp>
#include <cstdlib>
#include <limits.h>

using test::Test;
//using std::cout;
using std::rand;
using boost::scoped_ptr;
//using std::memset;


namespace engine{
namespace test  {

  namespace { // used internally
    
    const uint CHAN_COUNT = 30;     // independent families of test frames to generate
    const uint NUM_FRAMES = 1000;   // number of test frames in each of these families
    
    
    void
    corruptMemory(void* base, uint offset, uint count)
    {
      char* accessor = reinterpret_cast<char*> (base);
      while (count--)
        accessor[offset+count] = rand() % CHAR_MAX;
    }
        
  } // (End) internal defs
  
  
  
  /*******************************************************************
   * @test verify test helper for engine tests: a dummy data frame.
   *       TestFrame instances can be created right away, without any
   *       external library dependencies. A test frame is automatically
   *       filled with random data; multiple frames are arranged in
   *       sequences and channels, causing the random data to be 
   *       reproducible yet different in each frame.
   *       
   *       To ease writing unit tests, TestFrame provides comparison
   *       and assignment and tracks lifecycle automatically. As tests
   *       regarding the engine typically have to deal with buffer
   *       management, an arbitrary memory location can be interpreted
   *       as TestFrame and checked for corruption.
   */
  class TestFrame_test : public Test
    {
      
      virtual void
      run (Arg) 
        {
          verifyBasicProperties();
          verifyFrameLifecycle();
          verifyFrameSeries();
        }
      
      
      void
      verifyBasicProperties()
        {
          CHECK (1024 < sizeof(TestFrame));
          
          TestFrame frameA;
          TestFrame frameB;
          TestFrame frameC(5);
          
          CHECK (frameA == frameB);
          CHECK (frameA != frameC);
          CHECK (frameB != frameC);
          
          CHECK (frameA.isAlive());
          CHECK (frameB.isAlive());
          CHECK (frameC.isAlive());
          
          CHECK (frameA.isSane());
          CHECK (frameB.isSane());
          CHECK (frameC.isSane());
          
          void * frameMem = &frameB;
          
          CHECK (frameA == frameMem);
          corruptMemory(frameMem,20,5);
          CHECK (!frameB.isSane());
          
          frameB = frameC;
          
          CHECK (frameB.isSane());
          CHECK (frameA != frameB);
          CHECK (frameA != frameC);
          CHECK (frameB == frameC);
        }
      
      
      void
      verifyFrameLifecycle()
        {
          CHECK (!TestFrame::isDead  (this));
          CHECK (!TestFrame::isAlive (this));
          
          TestFrame* onHeap = new TestFrame(23);
          CHECK ( TestFrame::isAlive (onHeap));
          CHECK (!onHeap->isDead());
          CHECK (onHeap->isAlive());
          CHECK (onHeap->isSane());
          
          delete onHeap;
          CHECK ( TestFrame::isDead  (onHeap));
          CHECK (!TestFrame::isAlive (onHeap));
        }
      
      
      /** @test build sequences of test frames,
       *        organised into multiple families (channels).
       *        Verify that adjacent frames hold differing data
       */
      void
      verifyFrameSeries()
        {
          scoped_ptr<TestFrame> thisFrames[CHAN_COUNT];
          scoped_ptr<TestFrame> prevFrames[CHAN_COUNT];
          
          for (uint i=0; i<CHAN_COUNT; ++i)
            thisFrames[i].reset (new TestFrame(0, i));
          
          for (uint nr=1; nr<NUM_FRAMES; ++nr)
            for (uint i=0; i<CHAN_COUNT; ++i)
              {
                prevFrames[i].swap (thisFrames[i]);
                thisFrames[i].reset (new TestFrame(nr, i));
                CHECK (thisFrames[i]->isSane());
                CHECK (prevFrames[i]->isSane());
                CHECK (prevFrames[i]->isAlive());
                
                CHECK (*thisFrames[i] != *prevFrames[i]);        // differs from predecessor in the same channel
                
                for (uint j=0; j<CHAN_COUNT; ++j)
                  if (j!=i)
                    {
                      CHECK (*thisFrames[i] != *thisFrames[j]);  // differs from frames in other channels at this point
                      CHECK (*thisFrames[i] != *prevFrames[j]);  // differs cross wise from predecessors in other channels
        }     }     }
    };
  
  
  /** Register this test class... */
  LAUNCHER (TestFrame_test, "unit engine");
  
  
  
}} // namespace engine::test
