/*
  util.cpp  -  helper functions implementation

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


#include "lib/util.hpp"

#include <boost/algorithm/string.hpp>
#include <functional>
#include <boost/bind.hpp>          // we need operator! for bind-expressions

using boost::algorithm::trim_right_copy_if;
using boost::algorithm::is_any_of;
using boost::algorithm::is_alnum;
using boost::algorithm::is_space;


namespace util {

  using std::function;
  
  typedef function<bool(string::value_type)> ChPredicate;
  ChPredicate operator! (ChPredicate p) { return ! bind(p,_1); }

  // character classes used for sanitising a string
  ChPredicate isValid (is_alnum() || is_any_of("-_.:+$'()@"));           ///< characters to be retained
  ChPredicate isPunct (is_space() || is_any_of(",;#*~´`?\\=/&%![]{}")); ///<  punctuation to be replaced by '_'

  
  string
  sanitise (const string& org)
  {
    string res (trim_right_copy_if(org, !isValid ));
    string::iterator       j = res.begin();
    string::const_iterator i = org.begin();
    string::const_iterator e = i + (res.length());
    while ( i != e )
      {
        while ( i != e && !isValid (*i) )   ++i;
        while ( i != e && isValid  (*i) )   *(j++) = *(i++); 
        if    ( i != e && isPunct  (*i) ) 
          {
            *j++ = '_';
            do  ++i;
            while ( i != e && isPunct (*i));  
          }
      }
    res.erase(j,res.end());
    return res;
  }
  

  
} // namespace util

