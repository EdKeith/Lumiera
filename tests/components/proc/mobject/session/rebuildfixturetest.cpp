/*
  RebuildFixture(Test)  -  (re)building the explicit placements
 
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
 
* *****************************************************/


#include "common/test/run.hpp"
//#include "common/factory.hpp"
//#include "common/util.hpp"

//#include <boost/format.hpp>
#include <iostream>

//using boost::format;
using std::string;
using std::cout;


namespace mobject
  {
  namespace session
    {
    namespace test
      {
      
      
      
      
      /*******************************************************************
       * @test (re)building the ExplicitPlacement objects from the objects
       *       placed into the Session/EDL.
       * @see  mobject::session::Fixture
       * @see  mobject::ExplicitPlacement
       */
      class RebuildFixture_test : public Test
        {
          virtual void run(Arg arg) 
            {
            } 
        };
      
      
      /** Register this test class... */
      LAUNCHER (RebuildFixture_test, "unit session");
      
      
      
    } // namespace test
  
  } // namespace session

} // namespace mobject
