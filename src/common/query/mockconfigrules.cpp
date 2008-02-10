/*
  MockConfigRules  -  mock implementation of the config rules system
 
  Copyright (C)         CinelerraCV
    2007,               Hermann Vosseler <Ichthyostega@web.de>
 
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


#include "common/query/mockconfigrules.hpp"

#include "proc/mobject/session/track.hpp"
#include "proc/asset/procpatt.hpp"
#include "proc/asset/port.hpp"


//#include "common/util.hpp"
#include "nobugcfg.h"



namespace cinelerra
  {
  
  namespace query
    {
    using asset::Struct;
    using asset::Port;
    using asset::PPort;
    
    using asset::ProcPatt;
    using asset::PProcPatt;
    
    namespace
      {
      typedef std::pair<const string, any> AnyPair;
      
      /** helper to simplify creating mock table entries, wrapped correctly */
      template<class TY>
      AnyPair entry (const string& query, typename WrapReturn<TY>::Wrapper& obj)
      {
        return AnyPair ( Query<TY> (query).asKey()
                       , any(obj)); 
      }
      
      /** helper especially for creating structural assets from a capability query */
      template<class STRU>
      AnyPair entry_Struct(Symbol caps)
      {
        typedef typename WrapReturn<STRU>::Wrapper Ptr;
        
        Query<STRU> query(caps); 
        Ptr obj = Struct::create (query);
        return AnyPair(query, obj);
      }
    
    }
    
    
    /** hard coded answers to configuration queries */
    void
    MockTable::fill_mock_table ()
    {
      // for baiscporttest.cpp ---------
      answer_->insert (entry_Struct<const ProcPatt> ("stream(teststream)"));
    }

    
    
    MockConfigRules::MockConfigRules () 
    {
      WARN (config, "using a mock implementation of the ConfigQuery interface");
    }
    
    MockTable::MockTable ()
      : answer_(new Tab())
    {
      fill_mock_table ();
    }

    

    
    /** this is the (preliminary/mock) implementation
     *  handling queries for objects of a specific type
     *  and with capabilities or properties defined by
     *  the query. The real implementation would require
     *  a rule based system (Ichthyo plans to use YAP Prolog),
     *  while this dummy implementation simply relies on a
     *  table of pre-fabricated objects. Never fails.
     *  @return smart ptr (or similar) holding the object,
     *          maybe an empty smart ptr if not found
     */
    const any& 
    MockTable::fetch_from_table_for (const string& queryStr)
    {
      static const any NOTFOUND;
      
      Tab::iterator i = answer_->find (queryStr);
      if (i == answer_->end())
        return NOTFOUND;
      else
        return i->second;
    }
    

  
  } // namespace query
    
  
  /** */


} // namespace cinelerra
