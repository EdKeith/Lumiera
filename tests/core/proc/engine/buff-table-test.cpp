/*
  BuffTable(Test)  -  check consistency of buffer table chunk allocation

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


#include "lib/test/run.hpp"
#include "lib/error.hpp"

#include "proc/engine/procnode.hpp"
#include "proc/engine/bufftable-obsolete.hpp"
#include "lib/ref-array.hpp"

#include <boost/scoped_ptr.hpp>
#include <iostream>

using test::Test;
using std::cout;
using std::rand;


namespace proc {
namespace engine{
namespace test  {

  namespace { // used internally
    
    const uint TABLE_SIZ = 100000;
    const uint CHUNK_MAX = 8000;
    const uint WIDTH_MAX = 3;
    
    
    /** just some crap to pass in as ctor argument... */
    template<class E>
    struct DummyArray : lib::RefArray<E>
      {
        E decoy;
        E const& operator[] (size_t)  const  { return decoy; }
        size_t size()                 const  { return CHUNK_MAX;}
      };
    DummyArray<ChannelDescriptor> dummy1;
    DummyArray<InChanDescriptor>  dummy2;
    
    
    /** a "hijacked" WiringDescriptor requesting
     *  a random number of inputs and outputs */
    struct MockSizeRequest
      : WiringDescriptor
      {
        uint ii,oo;
        
        MockSizeRequest()
          : WiringDescriptor(dummy1,dummy2,0,NodeID()),
            ii(rand() % CHUNK_MAX),
            oo(rand() % CHUNK_MAX)
          { }
        
        virtual uint getNrI()  const { return ii; }
        virtual uint getNrO()  const { return oo; }
        
        virtual BuffHandle callDown (State&, uint) const
        { throw lumiera::Error("not intended to be called"); }
      };
    
    
    
#if false /////////////////////////////////////////////////////////////////////////////////////////////////////////////UNIMPLEMENTED :: TICKET #852
    void*
    detect_start_level (BuffTableStorage& sto)
    {
      return BuffTableChunk(MockSizeRequest(), sto ).outHandle;
    }       // address of first available storage element
    
    
    inline void*
    first_behind (BuffTable const& thisChunk, const uint nrI)
    {
      return &thisChunk.inHandle[nrI];
    }
    
    
    inline bool
    not_within(void* candidate, void* lower, void* upper)
    {
      return (candidate < lower) || (upper <= candidate);
    }
    
    
    bool
    consistencyCheck (BuffTable const& b, WiringDescriptor const& num, void* lastLevel)
    {
      return (b.outHandle == lastLevel )  // storage is allocated continuously
          && (b.outBuff   <= b.inBuff  )  // input slots are behind the output slots
          && (b.outHandle <= b.inHandle)
          && (b.inBuff   == &b.outBuff  [num.nrO])
          && (b.inHandle == &b.outHandle[num.nrO])
          && (not_within(b.outBuff, b.outHandle, &b.inHandle[num.nrO])) // storage doesn't overlap
          && (not_within(b.inBuff,  b.outHandle, &b.inHandle[num.nrO]))
          && (not_within(b.outHandle, b.outBuff, &b.inBuff[num.nrO]))
          && (not_within(b.inHandle,  b.outBuff, &b.inBuff[num.nrO]))
           ;
    }
#endif    /////////////////////////////////////////////////////////////////////////////////////////////////////////////UNIMPLEMENTED :: TICKET #852
  } // (End) internal defs
  
  
  
  /***************************************************************//**
   * @test create a random pattern of recursive invocations, each
   *       allocating a chunk out of a global buffer table storage.
   *       After returning, each allocation should be cleanly
   *       deallocated and the internal level in the storage vector
   *       should have doped to zero again.
   */
  class BuffTable_test : public Test
    {
      typedef boost::scoped_ptr<BuffTableStorage> PSto;
      
      PSto pStorage;
      ulong counter;
      
      virtual void run(Arg) 
        {
           counter = 0;
           
           // allocate storage block to be used chunk wise
           pStorage.reset (new BuffTableStorage (TABLE_SIZ));
           
#if false /////////////////////////////////////////////////////////////////////////////////////////////////////////////UNIMPLEMENTED :: TICKET #852
           invocation (0, detect_start_level(*pStorage));
#endif    /////////////////////////////////////////////////////////////////////////////////////////////////////////////UNIMPLEMENTED :: TICKET #852
           
           pStorage.reset(0);  // dtor throws assertion error if corrupted
           
           cout << "BuffTable chunks allocated: "<<counter<< "\n";
        }
      
      
      /** recurse down randomly
       *  until exhausting storage
       */
      void invocation (uint consumed, void* lastLevel)
        {
          MockSizeRequest numbers;
          consumed += numbers.getNrI()+numbers.getNrO();
          if (TABLE_SIZ <= consumed)
            return; // end recursion
          
          ++counter;
#if false /////////////////////////////////////////////////////////////////////////////////////////////////////////////UNIMPLEMENTED :: TICKET #833
          BuffTableChunk thisChunk (numbers, *pStorage);
          CHECK (consistencyCheck (thisChunk, numbers, lastLevel));
          
          uint nrBranches ( 1 + (rand() % WIDTH_MAX));
          while (nrBranches--)
            invocation (consumed, first_behind (thisChunk,numbers.getNrI()));
#endif    /////////////////////////////////////////////////////////////////////////////////////////////////////////////UNIMPLEMENTED :: TICKET #833
        }
    };
  
  
  /** Register this test class... */
  LAUNCHER (BuffTable_test, "unit engine");
  
  
  
}}} // namespace proc::engine::test
