/*
  ConfigFlags(Test)  -  generating a configuration type defined by flags
 
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


/** @file configflagstest.cpp
 ** \par build a type representing a single configuration defined by a set of flags
 **
 ** The semi-automatic assembly of processing node invocation code utilizes some
 ** metaprogramming to generate a factory, which in turn produces node wiring objects
 ** according to the configuration to be used for the corresponding ProcNode. This relies on
 ** generating a distinct type based on a given set of configuration flags,
 ** which is covered by this test.
 **
 ** @see configflags.hpp
 ** @see typelistmanip.hpp
 ** @see nodewiringconfig.hpp real world usage example
 **
 */


#include "common/test/run.hpp"
#include "common/typelistutil.hpp"
#include "common/util.hpp"
//#include "common/typelist/typelistmanip.hpp"
#include "proc/engine/nodewiringconfig.hpp"  ///////////TODO move to configflags.hpp

#include <boost/format.hpp>
#include <iostream>

using ::test::Test;
using std::string;
using std::cout;
using boost::format;


namespace lumiera {
  namespace typelist {
    namespace test {
      
      
      namespace { // internal definitions
      
        enum Cases
          { ONE = 1
          , TWO
          , THR
          , FOU
          , NUM_Cases = FOU
            
          , NOT_SET   = 0
          };
        
        format fmt ("-<%u>%s");
        
        struct NullP
          {
            static string print () { return "-"; }
          };
        
        /** debugging template, 
         *  printing the "number" used for intantiation on ctor call
         */
        template<class NUM=NullType, class BASE=NullP>
        struct Printer;
        
        template<class BASE>
        struct Printer<NullType, BASE>
          : BASE
          {
            static string print () { return str( fmt % "·" % BASE::print()); }
          };
        
        template<class BASE, char Fl>
        struct Printer<Flag<Fl>, BASE>
          : BASE
          {
            static string print () { return str( fmt % uint(Fl) % BASE::print()); }
          };
        
        
        
        /** call the debug-print for a typelist
         *  utilizing the Printer template */ 
        template<class L>
        string
        printSublist ()
        {
          typedef InstantiateChained<L, Printer, NullP> SubList;
          return SubList::print();
        }
        
        /** Spezialisation for debug-printing of a nested sublist */
        template<class TY, class TYPES, class BASE>
        struct Printer<Node<TY,TYPES>, BASE>
          : BASE
          {
            static string print () 
              {
                typedef Node<TY,TYPES> List;
                return string("\n\t+--") + printSublist<List>()+"+"
                     + BASE::print(); 
              }
          };
        
        template<char f1, char f2, char f3, char f4, char f5, class BASE>
        struct Printer<Config<f1,f2,f3,f4,f5>, BASE>
          : BASE
          {
            static string print () 
              {
                typedef typename Config<f1,f2,f3,f4,f5>::Flags FlagList;
                return string("\n\t+-Conf-[") + printSublist<FlagList>()+"]"
                     + BASE::print(); 
              }
          };
        
        
#define DIAGNOSE(LIST) \
        typedef InstantiateChained<LIST::List, Printer, NullP>  Contents_##LIST;
                     
#define DISPLAY(NAME)  \
        DIAGNOSE(NAME); cout << STRINGIFY(NAME) << "\t" << Contents_##NAME::print() << "\n";
        
        
        
        /* === Test data === */
        typedef Config<> Conf0;
        typedef Config<ONE> Conf1;
        typedef Config<TWO> Conf2;
        typedef Config<THR> Conf3;
        typedef Config<TWO,FOU> Conf4;
        
        typedef Flags<ONE,THR>::Tuple Flags1;
        typedef Flags<TWO,FOU>::Tuple Flags2;
        typedef Types<Flags1,Flags2> SomeFlagsets;
        
        typedef Flags<ONE,TWO,THR,FOU>::Tuple AllFlags;
        typedef CombineFlags<AllFlags>        AllFlagCombinations;
        
        
        
        /** a type which is only partially defined, for some configs.
         *  In ConfigFlags_test::check_filter() we use the metaprogramming machinery
         *  to figure out all possible configs for which \c struct Maybe is defined.
         *  (for this to work, the "defined" specialisations need to provide a
         *  typedef \c is_defined )
         */ 
        template<class CONF> struct Maybe;
        
        struct Indeed { typedef Yes_t is_defined; };
        template<> struct Maybe<Conf1> : Indeed { enum{ CODE = 1 }; };
        template<> struct Maybe<Conf3> : Indeed { enum{ CODE = 3 }; };
        
        template<char Fl> 
        struct Maybe<Config<TWO,Fl> >
        { 
          typedef Yes_t is_defined;
          
          enum{ CODE = 20 + Fl };
        };
        
        
      } // (End) internal defs
      
      
      
      
      
      
      /**************************************************************************
       * @test check the handling of types representing a specific configuration.
       *       Basically, this is a bitset like feature, but working on types
       *       instead of runtime values. The Goal is to detect automatically
       *       all possible defined specialisations of some template based on
       *       such configuration-tuples. This allows us to assemble the glue code
       *       for pulling data from processing nodes out of small building blocks
       *       in all possible configuraions.
       */
      class ConfigFlags_test : public Test
        {
          virtual void run(Arg arg) 
            {
              check_testdata ();
              check_flags();
              check_instantiation ();
              check_filter ();
              check_FlagInfo ();
            }
          
          
          void check_testdata ()
            {
              DISPLAY (Conf0);
              DISPLAY (Conf1);
              DISPLAY (Conf2);
              DISPLAY (Conf3);
              DISPLAY (Conf4);
              
              DISPLAY (AllFlags);
            }
          
          
          /** @test conversion between list-of-flags and a config-type in both directions */
          void check_flags ()
            {
              cout << "\n==== check_flags()\n";
              
              typedef Config<TWO,FOU> Flags1;
              typedef Flags<TWO,FOU>  Flags2;
              DISPLAY (Flags1);
              DISPLAY (Flags2);
              // should denote the same type
              Flags1::Flags flags1 = Flags2::Tuple();
              Flags2::Tuple flags2 = flags1;
              ASSERT (1==sizeof(flags1));   // pure marker-type without content
              ASSERT (1==sizeof(flags2));
              
              typedef DefineConfigByFlags<Node<Flag<ONE>,NullType> >::Config SimpleConfig_defined_by_Typelist;
              DISPLAY (SimpleConfig_defined_by_Typelist);
              
              typedef DefineConfigByFlags<AllFlags>::Config AnotherConfig_defined_by_Typelist;
              DISPLAY (AnotherConfig_defined_by_Typelist);
            }
          
          
          /** @test creates a predicate template (metafunction) returning true
           *  iff the template \c Maybe is defined for the configuration in question
           */
          void check_instantiation ()
            {
              #define CAN_INSTANTIATE(NAME) \
              cout << "defined "             \
                   << STRINGIFY(NAME)         \
                   << "? ---> "                \
                   << Instantiation<Maybe>::Test<NAME>::value << "\n";
              
              cout << "\n==== check_instantiation()\n";
              CAN_INSTANTIATE (Conf0);
              CAN_INSTANTIATE (Conf1);
              CAN_INSTANTIATE (Conf2);
              CAN_INSTANTIATE (Conf3);
              CAN_INSTANTIATE (Conf4);
              
              typedef Config<THR,THR> Trash;
              CAN_INSTANTIATE (Trash);
            }
          
          
          /** @test given a list of flag-tuples, we first create config-types out of them
           *        and then filter out those configs for which \c template Maybe is defined
           */ 
          void check_filter ()
            {
              cout << "\n==== check_filter()\n";
              
              DISPLAY (SomeFlagsets);
              
              typedef Apply<SomeFlagsets::List, DefineConfigByFlags> Configs_defined_by_Flagsets;
              DISPLAY (Configs_defined_by_Flagsets);
              
              typedef Filter<Configs_defined_by_Flagsets::List,Instantiation<Maybe>::Test> Filter_possible_Configs;
              DISPLAY (Filter_possible_Configs);
              
              
              DISPLAY (AllFlagCombinations);
              typedef Apply<AllFlagCombinations::List, DefineConfigByFlags> ListAllConfigs;
              DISPLAY (ListAllConfigs);
              
              typedef Filter<ListAllConfigs::List,Instantiation<Maybe>::Test> Filter_all_possible_Configs;
              DISPLAY (Filter_all_possible_Configs);
            }
          
          
          
            struct TestVisitor
              {
                string result;
                TestVisitor() : result ("TestVisitor application:\n") {}
                
                /* === visitation interface === */
                typedef string Ret;
                
                Ret done()  {return result; }
                
                template<class FLAGS>
                void
                visit (ulong code)
                  {
                    typedef typename DefineConfigByFlags<FLAGS>::Config Config;
                    result += str (format ("visit(code=%u) --> %s\n") 
                                   % code % Printer<Config>::print() );
                  }
              };
          
          /** @test FlagInfo metafunction, which takes as argument a list-of-flags
           *        as well as a list-of-lists-of-flags and especially allows to
           *        apply a visitor object to the latter 
           */
          void check_FlagInfo()
            {
              cout << "\n==== check_FlagInfo()\n";
              
              DISPLAY (Flags1);
              cout << "max bit    : " << FlagInfo<Flags1>::BITS <<"\n";
              cout << "binary code: " << FlagInfo<Flags1>::CODE <<"\n";
              
              DISPLAY (SomeFlagsets);
              cout << "max bit in [SomeFlagsets] : " << FlagInfo<SomeFlagsets::List>::BITS <<"\n";
              
              TestVisitor visitor;
              cout << FlagInfo<SomeFlagsets::List>::accept (visitor);
            }
          
          
          void check_Factory()
            {
            }
          
          
        };
      
      
      /** Register this test class... */
      LAUNCHER (ConfigFlags_test, "unit common");
      
      
      
    } // namespace test
    
  } // namespace typelist

} // namespace lumiera
