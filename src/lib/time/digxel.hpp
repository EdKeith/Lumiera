/*
  TIMECODE.hpp  -  grid aligned and fixed format time specifications

  Copyright (C)         Lumiera.org
    2010,               Hermann Vosseler <Ichthyostega@web.de>

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

*/


/** @file digxel.hpp
 ** A self-contained numeric element for building structured numeric displays.
 ** Within the GUI of an editing application, we got to build several display widgets
 ** to show numeric values in a structured fashion, like Colours or Timecode. While the
 ** actual formats of such display elements vary largely, the common property is that
 ** they use an underlying \em format to build the numbers out of individual numeric elements.
 ** For a timecode display these are for example the sexagesimal (base 60) "digits" of the
 ** common human readable time representation. A "Digxel" is an abstract element with specific
 ** properties to support building such display widgets. It doesn't contain any GUI code, but
 ** can be wrapped up to build a custom widget.
 ** 
 ** \par properties of a "Digxel"
 ** 
 ** Semantically, it's a number or number component. It holds an internal numeric representation
 ** and is implicitly convertible both to integrals and floating point numbers. This implicit
 ** conversion is a compromise to support generic processing.
 ** 
 ** But at the same time, a Digxel has a definite textual format and the ability to present
 ** its numeric value formatted accordingly. To this end, the contract \em requires that
 ** numeric data pushed to the Digxel be kept within such limits to prevent exceeding the
 ** embedded formatting buffer. There is an assertion in debug mode, and a range check,
 ** but the result will be just truncated, so this is clearly the caller's responsibility.
 ** Digxel might be considered an implementation support class, and performance is important
 ** to some limited degree; especially, formatted values will be cached.
 ** 
 ** To support in-place modification, the digxel stores a mutation signal (functor) and exposes
 ** a special \c mutate(newVal) function, which invokes this stored functor, if defined. Usually
 ** this should invoke some internal recalculations, resulting in a new value being pushed to
 ** the Digxel for display.
 ** 
 ** \par configuration
 ** the Digxel template can be configured to some degree to adjust the stored numeric data
 ** and the actual format to be applied
 ** 
 ** @see timecode.hpp typical usage example
 **
 */

#ifndef LIB_TIME_DIGXEL_H
#define LIB_TIME_DIGXEL_H

#include "lib/error.hpp"
#include "lib/symbol.hpp"

#include <boost/operators.hpp>
#include <boost/lexical_cast.hpp>
#include <cstdlib>  ///////////TODO
#include <cmath>
#include <string>


namespace lib {
namespace time {
  
//  using std::string;
  
  /**
   * A number element for building structured numeric displays.
   * The purpose is to represent parts of a numeric format, like
   * e.g. the sexagesimal "digits" of a timecode display. Digxel
   * - is customised by template parameters to a specific number format
   * - requires that any number set must not overflow the format buffer
   * - can receive new numbers by assignment
   * - will then format these numbers and cache the formatted representation
   * - can store and invoke a mutation functor
   *  
   * @note comparisons are assumed to be not performance relevant
   * @see lib::time::TCode
   * @todo WIP-WIP-WIP
   */
  class Digxel
    : public boost::totally_ordered<Digxel,
             boost::totally_ordered<Digxel, int,
             boost::totally_ordered<Digxel, double
             > > >
    {
      typedef const char* CBuf;
      
    public:
      virtual ~Digxel ();  ///< this is an ABC
      
      operator int()     const { return getIntValue(); }
      operator double()  const { return getDoubleValue(); }
      
      CBuf     show()          { return getFormatted(); }
      void operator= (int i)   { return changeTo(i); }
      void operator= (double d){ return changeTo(d); }
      
      
      // Supporting totally_ordered
      bool operator<  (Digxel const& o)  const { return double(*this) <  double(o); }
      bool operator== (Digxel const& o)  const { return double(*this) == double(o); }
      bool operator== (int    i)         const { return    int(*this) ==        i ; }
      bool operator<  (int    i)         const { return    int(*this) <         i ; }
      bool operator>  (int    i)         const { return    int(*this) >         i ; }
      bool operator== (double d)         const { return double(*this) ==        d ; }
      bool operator<  (double d)         const { return double(*this) <         d ; }
      bool operator>  (double d)         const { return double(*this) >         d ; }
      
    protected:
      virtual int    getIntValue()    const   =0;
      virtual double getDoubleValue() const   =0;
      virtual CBuf   getFormatted()           =0;
      virtual void   changeTo (int i)         =0;
      virtual void   changeTo (double d)      =0;
    };
  
  namespace digxel {
    
    using std::string;
    using lib::Literal;
    using boost::lexical_cast;
    
    
    
    template<typename NUM>
    struct ValTrait;
    
    template<>
    struct ValTrait<int>
      {
        static int    asInt    (int val) { return val; }
        static double asDouble (int val) { return val; }
      };
    
    template<>
    struct ValTrait<double>
      {
        static int    asInt    (double val) { return std::floor(0.5+val); }  ///< in accordance with Lumiera's time handling RfC
        static double asDouble (double val) { return val; }
      };
    
    
    
    template<typename NUM, size_t len>
    class PrintfFormatter
      {
        enum{ bufsiz = len+1 };
        
        char printbuffer_[bufsiz];
        Literal formatSpec_;
        
      public:
        PrintfFormatter (Literal fmt)
          : printbuffer_()
          , formatSpec_(fmt)
          { 
            printbuffer_[0] = '\0';
          }
        
        void
        show (NUM val)
          {
            size_t space = std::snprintf (printbuffer_, bufsiz, "%5d", val);
            REQUIRE (space <= bufsiz, "Digxel value exceeded available buffer size. "
                                      "For showing %s, %d chars instead of just %d would be required."
                                    , lexical_cast<string>(val), space, bufsiz);
          }
      };
    
    template<typename NUM>
    struct Formatter;
    
    template<>
    struct Formatter<int>
      : PrintfFormatter<int, 6>
      {
        Formatter() : PrintfFormatter<int,6>("%5d") { }
        
      };
    
    template<>
    struct Formatter<double>
      : PrintfFormatter<double, 7>
      {
        Formatter() : PrintfFormatter<double,7>("%06.3f") { }
        
      };
  
    /**
     * The outward hull of a concrete Digxel implementation.
     * Inheriting from the Digxel interface, it embodies a concrete
     * Formatter specialised to yield the desired behaviour.
     *  
     * @param TODO
     * @todo WIP-WIP-WIP
     */
    template< typename NUM
            , class FMT  = digxel::Formatter<NUM>
            >
    class Holder
      : public Digxel
      {
        FMT buffer_;
        NUM value_;
        
        
        /* === Digxel implementation === */
        int
        getIntValue()  const
          {
            return ValTrait<NUM>::asInt (value_);
          }
        
        double
        getDoubleValue()  const
          {
            return ValTrait<NUM>::asDouble (value_);
          }
        
        CBuf
        getFormatted()
          {
            UNIMPLEMENTED("call formatting or cache");
          }
        
        void
        changeTo (int i)
          {
            UNIMPLEMENTED("mutate INT");
          }
        
        void
        changeTo (double d)
          {
            UNIMPLEMENTED("mutate FLOAT");
          }

        
      public:
        Holder ()
          : buffer_()
          , value_()
          { }
        
        using Digxel::operator=;
      };
    
  }
  
  
  
}} // lib::time
#endif
