/*
  TEST-HELPER.hpp  -  collection of functions supporting unit testing
 
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
 
*/


#ifndef LIB_TEST_TEST_HELPER_H
#define LIB_TEST_TEST_HELPER_H


#include "lib/symbol.hpp"
#include "lib/lumitime.hpp"

#include <typeinfo>
#include <string>
#include <cstdlib>



namespace lib {
namespace test{
  
  using lib::Literal;
  using lumiera::Time;
  using std::string;
  using std::rand;
  
  
  
  /** get a sensible display for a type or object 
   *  @param obj object of the type in question
   *  @param name optional name to be used literally
   *  @return either the literal name without any further magic,
   *          or the result of compile-time or run time 
   *          type identification as implemented by the compiler.
   */
  template<typename T>
  inline Literal
  showType (T const& obj, Literal name=0)
  {
    return name? name : Literal(typeid(obj).name());
  }
  
  /** get a sensible display for a type 
   *  @param name optional name to be used literally
   *  @return either the literal name without any further magic,
   *          or the result of compile-time or run time 
   *          type identification as implemented by the compiler.
   */
  template<typename T>
  inline Literal
  showType (Literal name=0)
  {
    return name? name : Literal(typeid(T).name());
  }
  
  
  /** for printing sizeof().
   *  prints the given size and name literally, without any further magic */
  string
  showSizeof (size_t siz, const char* name);
  
  /** for printing sizeof(), trying to figure out the type name automatically */
  template<typename T>
  inline string
  showSizeof(const char* name=0)
  {
    return showSizeof (sizeof (T), showType<T> (name));
  }
  
  template<typename T>
  inline string
  showSizeof(T const& obj, const char* name=0)
  {
    return showSizeof (sizeof (obj), showType (obj,name));
  }
  
  template<typename T>
  inline string
  showSizeof(T *obj, const char* name=0)
  {
    return obj? showSizeof (*obj, name)
              : showSizeof<T> (name);
  }
  
  
  
    
  /** create a random but not insane Time value */    
  inline Time
  randTime ()
  {
    return Time (500 * (rand() % 2), (rand() % 600));
  }
  
  /** create garbage string of given length
   *  @return string containing arbitrary lower case letters and numbers
   */
  string randStr (size_t len);
  
  
}} // namespace lib::test




/* === test helper macros === */

/**
 * Macro to verify a statement indeed raises an exception.
 * If no exception is thrown, the #NOTREACHED macro will trigger
 * an assertion failure. In case of an exception, the #lumiera_error
 * state is checked, cleared and verified.
 */
#define VERIFY_ERROR(ERROR_ID, ERRONEOUS_STATEMENT)          \
          try                                                 \
            {                                                  \
              ERRONEOUS_STATEMENT ;                             \
              NOTREACHED("expected '%s' failure in: %s",         \
                          #ERROR_ID, #ERRONEOUS_STATEMENT);       \
            }                                                      \
          catch (...)                                               \
            {                                                        \
              CHECK (lumiera_error_expect (LUMIERA_ERROR_##ERROR_ID));\
            }

#endif
