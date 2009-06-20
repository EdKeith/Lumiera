/*
  FunctionErasure(Test)  -  verify the wrapping of functor object with type erasure
 
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


#include "lib/test/run.hpp"
#include "lib/test/test-helper.hpp"
#include "lib/meta/function-erasure.hpp"
#include "lib/error.hpp"
#include "meta/dummy-functions.hpp"

#include <tr1/functional>

using ::test::Test;
using lumiera::error::LUMIERA_ERROR_ASSERTION;



namespace lumiera {
namespace typelist{
namespace test {
  
  using std::tr1::function;
  using std::tr1::bind;
  
  
  typedef FunErasure<StoreFunction> Efun;
  typedef FunErasure<StoreFunPtr>   Efp;
  typedef FunErasure<StoreUncheckedFunPtr> Evoid;
  
  
  /***********************************************************************
   * @test Create specifically typed functor objects and then wrap them
   *       into common holder objects, thereby discarding the specific
   *       signature type information. Later on, the concrete functor
   *       can be re-accessed, given the exact and specific type.
   * 
   * @see control::FunErasure
   * @see command-mutation.hpp real world usage example
   */
  class FunctionErasure_test : public Test
    {
      virtual void
      run (Arg) 
        {
          function<void(int,char)> bindFunc = bind (testFunc,_1,_2);
          function<void(int     )> pAplFunc = bind (testFunc,_1,'x');
          function<void(    char)> membFunc = bind (&FunctionErasure_test::testMemberFunction,this, _1);
          
          function<int(void)> getterFunc  = &returnIt;
          
          check_FunctorContainer( Efun (testFunc)
                                , Efun (bindFunc)
                                , Efun (pAplFunc)
                                , Efun (membFunc)
                                , Efun (getterFunc)
                                );
          
          check_FunctPtrHolder(Efp(testFunc),Efp(&testFunc), Efp(returnIt));
          check_VoidPtrHolder(Evoid(testFunc),Evoid(&testFunc),Evoid(returnIt));
        }
      
      void
      testMemberFunction (char c) ///< for checking bind-to member function
        {
          return testFunc('a'-'A', c);
        }
      
      
      void
      check_FunctorContainer (Efun f1, Efun f2, Efun f3, Efun f4, Efun f5)
        {
          typedef void (Sig1) (int,char);
          typedef void (Sig2) (int);
          typedef void (Sig3) (char);
          typedef int  (Sig4) ();
          
          _sum_ = 0;
          f1.getFun<Sig1>() (-11,'M');                // invoke stored tr1::function...
          ASSERT (_sum_ == 'M'-11);
          
          _sum_ = 0;
          f2.getFun<Sig1>() (-22,'M');
          ASSERT (_sum_ == 'M'-22);
          
          _sum_ = 0;
          f3.getFun<Sig2>() (-33);
          ASSERT (_sum_ == 'x'-33);
          
          _sum_ = 0;
          f4.getFun<Sig3>() ('U');
          ASSERT (_sum_ == 'u');
          
          ASSERT ( 'u' == f5.getFun<Sig4>() () );
          ASSERT (INSTANCEOF (function<Sig4>, &f5.getFun<Sig4>()));
          
          
#if false ////////////////////////////////////////////////////////TODO: restore throwing ASSERT
          VERIFY_ERROR (ASSERTION, f1.getFun<Sig2>() );
          VERIFY_ERROR (ASSERTION, f1.getFun<Sig3>() );
          VERIFY_ERROR (ASSERTION, f1.getFun<Sig4>() );
          
          VERIFY_ERROR (ASSERTION, f2.getFun<Sig2>() );
          VERIFY_ERROR (ASSERTION, f3.getFun<Sig3>() );
          VERIFY_ERROR (ASSERTION, f2.getFun<Sig4>() );
          
          VERIFY_ERROR (ASSERTION, f3.getFun<Sig1>() );
          VERIFY_ERROR (ASSERTION, f3.getFun<Sig3>() );
          VERIFY_ERROR (ASSERTION, f3.getFun<Sig4>() );
          
          VERIFY_ERROR (ASSERTION, f4.getFun<Sig1>() );
          VERIFY_ERROR (ASSERTION, f4.getFun<Sig2>() );
          VERIFY_ERROR (ASSERTION, f4.getFun<Sig4>() );
          
          VERIFY_ERROR (ASSERTION, f5.getFun<Sig1>() );
          VERIFY_ERROR (ASSERTION, f5.getFun<Sig2>() );
          VERIFY_ERROR (ASSERTION, f5.getFun<Sig3>() );
#endif    ////////////////////////////////////////////////////////
        }
      
      
      void
      check_FunctPtrHolder (Efp f1, Efp f2, Efp f3)
        {
          typedef void (*FP)(int,char);
          typedef void (&FR)(int,char);
          
          FP fun1 = &f1.getFun<void(int,char)>();
          FP fun2 = &f2.getFun<void(int,char)>();
          FR fun2r = f2.getFun<void(int,char)>();
          
          _sum_ = 0;
          (*fun1) (10, 'a');                          // invoke retrieved function pointer
          ASSERT (_sum_ == 10+'a');
          
          (*fun2) (20, 'b');
          ASSERT (_sum_ == 10+'a'+20+'b');
          
          fun2r (30, 'c');
          ASSERT (_sum_ == 10+'a'+20+'b'+30+'c');
          
          ASSERT (_sum_ == (f3.getFun<int(void)>()) () );
          
#if false ////////////////////////////////////////////////////////TODO: restore throwing ASSERT
          VERIFY_ERROR (ASSERTION, f1.getFun<int(int)>() );
#endif////////////////////////////////////////////////////////////
          
        }
      
      
      void
      check_VoidPtrHolder (Evoid f1, Evoid f2, Evoid f3)
        {
          typedef void (*FP)(int,char);
          typedef void (&FR)(int,char);
          
          FP fun1 = &f1.getFun<void(int,char)>();
          FP fun2 = &f2.getFun<void(int,char)>();
          FR fun2r = f2.getFun<void(int,char)>();
          
          _sum_ = 0;
          (*fun1) (10, 'a');
          ASSERT (_sum_ == 10+'a');
          
          (*fun2) (20, 'b');
          ASSERT (_sum_ == 10+'a'+20+'b');
          
          fun2r (30, 'c');
          ASSERT (_sum_ == 10+'a'+20+'b'+30+'c');
          
          ASSERT (_sum_ == (f3.getFun<int(void)>()) () );
          
          FP bad_fun = &f3.getFun<void(int,char)>();
          ASSERT ((void*)bad_fun == &returnIt);   // got wrong function!
          
        //(*bad_fun) (11, 'x');  // The compiler would accept this line!
        }                       //  likely to result in heap corruption or SEGV
    };
  
  
  /** Register this test class... */
  LAUNCHER (FunctionErasure_test, "unit common");
  
  
  
}}} // namespace lumiera::typelist::test