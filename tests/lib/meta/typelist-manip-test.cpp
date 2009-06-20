/*
  TypeListManip(Test)  -  appending, mixing and filtering typelists
 
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


/** @file typelist-manip-test.cpp
 ** \par checking the correct working of simple list manipulation metafunctions
 **
 ** The semi-automatic assembly of processing node invocation code utilises some
 ** metaprogramming magic built upon simple list manipulation. As template metaprogramming
 ** is kind-of functional programming, most of this is done by recursion.
 ** To check the correct working, this test uses some constant-wrapper types and a debugging
 ** template which will print these constant numbers, thus allowing to verify in the output
 ** if various lists of such constant-wrapper types were manipulated as expected.
 **
 ** @see typelist-test.cpp
 ** @see typelistutil.hpp
 ** @see nodewiringconfig.hpp real world usage example
 **
 */


#include "lib/test/run.hpp"
#include "lib/meta/generator.hpp"
#include "lib/meta/typelistutil.hpp"
#include "meta/typelist-diagnostics.hpp"
#include "lib/util.hpp"

#include <boost/format.hpp>
#include <iostream>

using ::test::Test;
using std::string;
using std::cout;


namespace lumiera {
namespace typelist{
namespace test {
      
      
      namespace { // test data
        
        
        
        typedef Types< Num<1>
                     , Num<2>
                     , Num<3>
                     >::List List1;
        typedef Types< Num<5>
                     , Num<6>
                     , Num<7>
                     >::List List2;
        
        
        template<class X> struct CountDown          { typedef NullType List; };
        template<>        struct CountDown<Num<0> > { typedef Node<Num<0>, NullType> List; };
        template<int I>   struct CountDown<Num<I> > { typedef Node<Num<I>, typename CountDown<Num<I-1> >::List> List; };
        
        
        
      } // (End) test data
      
      
      
      
      
      
      /*************************************************************************
       * @test check utilities for manipulating lists-of-types.
       *       - build an list of constant-wrapper-types and
       *         print them for debugging purpose.
       *       - append lists, single elements and NullType
       *         in various combinations
       *       - filtering out some types from a typelist by
       *         using a "predicate template" (metafunction)
       *       - building combinations and permutations
       */
      class TypeListManipl_test : public Test
        {
          virtual void run(Arg arg) 
            {
              check_diagnostics ();
              check_apply  ();
              check_append ();
              check_filter ();
              check_prefix ();
              check_distribute();
              check_combine();
            }
          
          
          void check_diagnostics ()
            {
              // Explanation: the DISPLAY macro expands as follows....
              typedef InstantiateChained<List1::List, Printer, NullP >  Contents_List1;
              cout << "List1" << "\t:" << Contents_List1::print() << "\n";
              
              // That is: we instantiate the "Printer" template for each of the types in List1,
              // forming an inheritance chain. I.e. the defined Type "Contents_List1" inherits
              // from each instantiation (single inheritance).
              // The print() function is defined to create a string showing each.
              
              DISPLAY (List2);
            }
          
          
          void check_append ()
            {
              typedef Append<NullType, NullType> Append1;
              DISPLAY (Append1);
              
              typedef Append<Num<11>,Num<22> >   Append2;
              DISPLAY (Append2);
              
              typedef Append<Num<111>,NullType>  Append3;
              DISPLAY (Append3);
              
              typedef Append<NullType,Num<222> > Append4;
              DISPLAY (Append4);
              
              typedef Append<List1,NullType>     Append5;
              DISPLAY (Append5);
              
              typedef Append<NullType,List2>     Append6;
              DISPLAY (Append6);
              
              typedef Append<Num<111>,List2>     Append7;
              DISPLAY (Append7);
              
              typedef Append<List1,Num<222> >    Append8;
              DISPLAY (Append8);
              
              typedef Append<List1,List2>        Append9;
              DISPLAY (Append9);
            }
          
          template<class X> struct AddConst2          { typedef X        Type; };
          template<int I>   struct AddConst2<Num<I> > { typedef Num<I+2> Type; };
          
          void check_apply ()
            {
              typedef Apply<List1, AddConst2> Added2;
              DISPLAY (Added2);
            }
          
          
          template<class X> struct IsEven          { enum {value = false };        };
          template<int I>   struct IsEven<Num<I> > { enum {value = (0 == I % 2) }; };
          
          void check_filter ()
            {
              typedef Filter<Append<List1,List2>::List, IsEven > FilterEven; 
              DISPLAY (FilterEven);
            }
          
          
          void check_prefix ()
            {
              typedef PrefixAll<Num<11>,Num<22> > Prefix1;
              DISPLAY (Prefix1);
              
              typedef PrefixAll<Num<101>,List1>   Prefix2;
              DISPLAY (Prefix2);
              
              typedef PrefixAll<NullType,List1>   Prefix3;
              DISPLAY (Prefix3);
              
              typedef Types<List1::List,Num<0>,List2::List>::List  List_of_Lists;
              typedef PrefixAll<Num<111>,List_of_Lists> Prefix4;
              DISPLAY (Prefix4);
              
              typedef PrefixAll<List1,List2>   Prefix5;
              DISPLAY (Prefix5);
              
              typedef PrefixAll<List1,List_of_Lists> Prefix6;
              DISPLAY (Prefix6);
            }
          
          
          void check_distribute()
            {
              typedef Distribute<Num<11>, List1> Dist1;
              DISPLAY (Dist1);
              
              typedef Types<Num<11>,Num<22>,Num<33> >::List Prefixes;
              typedef Distribute<Prefixes, Num<0> > Dist2;
              DISPLAY (Dist2);
              
              typedef Distribute<Prefixes, List1> Dist3;
              DISPLAY (Dist3);
              
              typedef Distribute<Prefixes, Types<List1::List,List2::List>::List> Dist4;
              DISPLAY (Dist4);
            }
          
          
          void check_combine()
            {
              typedef CountDown<Num<11> > Down;
              DISPLAY (Down);
              
              typedef Combine<List1::List, CountDown> Combi;
              DISPLAY (Combi);
              
              typedef CombineFlags<List1::List> OnOff;
              DISPLAY (OnOff);
            }
          
          
        };
      
      
      /** Register this test class... */
      LAUNCHER (TypeListManipl_test, "unit common");
      
      
      
}}} // namespace lumiera::typelist::test