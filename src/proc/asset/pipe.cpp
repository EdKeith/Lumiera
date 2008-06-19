/*
  Pipe  -  structural asset denoting a processing pipe generating media output
 
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


#include "proc/asset/pipe.hpp"
#include "common/util.hpp"

using util::isnil;

namespace asset
  {
  
  
  /** */
  Pipe::Pipe ( const Asset::Ident& idi
             , PProcPatt& wiring
             , const string& pipeID
             , wstring shortName
             , wstring longName
             ) 
    : Struct (idi),
      pipeID_ (pipeID),
      wiringTemplate(wiring),
      shortDesc (shortName),
      longDesc (longName)
  {
    REQUIRE (!isnil (pipeID));
    if (isnil (shortDesc))
      shortDesc = wstring (pipeID.begin(), pipeID.end());
  }

  
  
  PPipe 
  Pipe::query (string properties)
    { 
      return Struct::create (Query<Pipe> (properties)); 
    }
  
  void 
  Pipe::switchProcPatt (PProcPatt& another)
  {
    wiringTemplate = another;
    TODO ("trigger rebuild fixture");
  }




} // namespace asset