/*
  QueryResolver  -  interface for discovering contents of a scope
 
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


#include "proc/mobject/session/query-resolver.hpp"
//#include "proc/mobject/session/track.hpp"
//#include "proc/mobject/placement.hpp"
//#include "proc/mobject/session/mobjectfactory.hpp"
//#include "proc/asset/track.hpp"

namespace mobject {
namespace session {
  
  
  
  /* generate vtables here */  

  Goal::~Goal() { }

  QueryResolver::~QueryResolver() { }
  
  Resolution::~Resolution() { }
  
  
  struct QueryDispatcher
    {
      
    };
  
  
  QueryResolver::QueryResolver ()
    : dispatcher_(new QueryDispatcher)
    { }
  
  
  /** TODO??? */
  PReso  
  QueryResolver::issue (Goal& query)  const
  {
    if (!canHandleQuery (query.getQID()))
      throw lumiera::error::Invalid ("unable to resolve this kind of query"); ////TICKET #197
    
    UNIMPLEMENTED ("dispatch-mechanism for re-discovering specific type-context");
    
  }
  
  
  
  
}} // namespace mobject::session