/*
  TYPELISTDIAGNOSTICS  -  helper for testing the typelist based utilities
 
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


/** @file typelistdiagnostics.hpp
 ** a Printer template usable for debugging the structure of a typelist built
 ** upon some simple debugging-style types. Examples being a Num<int> template,
 ** or the Flag type. A Printer type generated from this template provides
 ** a static \c print() function returing a string visualizing the structure
 ** of the typelist provided as parameter to the Printer template.
 **
 ** @see typelistmaniptest.cpp
 ** @see configflagstest.cpp
 **
 */
#ifndef META_TYPELISTDIAGNOSTICS_H
#define META_TYPELISTDIAGNOSTICS_H


#include "common/meta/generator.hpp"
//#include "common/meta/typelistutil.hpp"
//#include "common/meta/util.hpp"
//#include "common/util.hpp"

#include <boost/format.hpp>

using std::string;
using boost::format;


namespace lumiera {
  namespace typelist {
    namespace test {
      
      
      namespace { // internal definitions
      
        
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
        
        
        
      } // (End) internal defs
      
      
      
    } // namespace test
    
  } // namespace typelist

} // namespace lumiera
#endif
