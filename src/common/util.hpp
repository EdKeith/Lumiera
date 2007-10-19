/*
  UTIL.hpp  -  collection of small helper functions used "everywhere"
 
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


#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <set>
#include <string>
#include <cstring>
#include <algorithm>

#include "nobugcfg.h"      ///////////////////TODO: just temporarily!!!!


namespace util
  {
  using std::string;

  
  template <class NUM>
  inline int
  sgn (NUM n)
  {
    return (n==0)? 0 :((n<0)? -1:+1 );
  }
  

  /** a family of util functions providing a "no value whatsoever" test.
      Works on strings and all STL containers, includes NULL test for pointers */
  template <class CONT>
  inline bool
  isnil (const CONT& container)
  {
    return container.empty();
  }
  
  template <class CONT>
  inline bool
  isnil (const CONT* pContainer)
  {
    return !pContainer || pContainer->empty();
  }
  
  inline bool
  isnil (const char* pCStr)
  {
    return !pCStr || 0 == std::strlen(pCStr);
  }

  
  /** cut a numeric value to be >=0 */
  template <typename NUM>
  inline NUM 
  noneg (NUM val)
  {
    return (0<val? val : 0);
  }
  
  /** shortcut for containment test on a map */
  template <typename MAP>
  inline bool 
  contains (MAP& map, typename MAP::key_type& key)
  {
    return map.find(key) != map.end();
  }
  
  /** shortcut for set value containment test */
  template <typename T>
  inline bool 
  contains (std::set<T>& set, const T& val)
  {
    return set.end() != set.find (val);
  }
  
  /** shortcut for brute-force containment test
   *  in any sequencial container */
  template <typename SEQ>
  inline bool 
  contains (SEQ& cont, typename SEQ::value_type& val)
  {
    typename SEQ::iterator end = cont.end();
    return end != std::find(cont.begin(),end, val);
  }
  
  
  /** shortcut for operating on all elements of a container.
   *  Isn't this already defined somewhere? It's so obvious..
   */
  template <typename Container, typename Oper>
  inline Oper
  for_each (Container& c, Oper doIt)
  {
    return std::for_each (c.begin(),c.end(), doIt);
  }
   
  
  
  /** produce an identifier based on the given string.
   *  remove non-standard-chars, reduce sequences of punctuation 
   *  and whitespace to single underscores. The sanitized string
   *  will start with an alphanumeric character.
   * 
   * @par Example Conversions
\verbatim 
   "Word"                             --> 'Word'
   "a Sentence"                       --> 'a_Sentence'
   "trailing Withespace  \t \n"       --> 'trailing_Withespace'
   "with    a   lot  \nof Whitespace" --> 'with_a_lot_of_Whitespace'
   "with\"much (punctuation)[]!"      --> 'withmuch_(punctuation)'
   "§&Ω%€  leading garbarge"          --> 'leading_garbarge'
   "mixed    Ω   garbarge"            --> 'mixed_garbarge'
   "Bääääh!!"                         --> 'Bh'
\endverbatim
   */
  string sanitize (const string& org);
  
  
  
  
  /** convienience shortcut: conversion to c-String via string.
   *  usable for printf with objects providing to-string conversion.
   */
  inline const char* 
  cStr (const string& org)
  {
    return org.c_str();
  }

  
} // namespace util



 /* === some common macro definitions === */

/** supress "warning: unused variable" on vars, which
 *  are introduced into a scope because of some sideeffect, i.e. Locking
 */
#define SIDEEFFECT __attribute__ ((unused));

/** this macro wraps its parameter into a cstring literal */
#define STRINGIFY(TOKEN) __STRNGFY(TOKEN)
#define __STRNGFY(TOKEN) #TOKEN

/** shortcut for subclass test, intended for assertions */
#define INSTANCEOF(CLASS, EXPR) (dynamic_cast<CLASS*> (EXPR)) 

#endif /*UTIL_HPP_*/
