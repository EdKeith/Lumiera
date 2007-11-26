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
#include "proc/mobject/session.hpp"
#include "proc/mobject/session/edl.hpp"
#include "proc/mobject/session/testsession1.hpp"
#include "common/util.hpp"

//#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <iostream>

//using boost::format;
using boost::bind;
using util::contains; 
using util::for_each; 
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
          virtual void
          run (Arg arg) 
            {
              PSess sess = Session::current;
              sess.clear();
              buildTestseesion1();
              ASSERT (sess->isValid());
              sess->rebuildFixture();
              TODO ("check the fixture has been touched. e.g. by hash.");
              TODO ("query all Placements of all Clips (via AssetManager). Verify explicit plac contained in Fixture.");

              UNIMPLEMENTED ("iterate over fixture");
// TODO              
//            for_each (sess->getFixture(), 
//                      bind (&check_is_from_EDL, _1, sess.getEDL()));
              
              TODO ("can we check the other direction, from EDL to Fixture??");
            } 
          
          static void 
          check_is_from_EDL (PMO explicitPlacement, EDL& edl)
            {
              
////TODO do we still support this? can it be replaced by a directly checking predicate on ExplicitPlacement??
              
//              PMO originalPlacement = explicitPlacement->subject->getPlacement();
//              ASSERT (edl.contains(originalPlacement));
            }
        };
      
      
      /** Register this test class... */
      LAUNCHER (RebuildFixture_test, "unit session");
      
      
      
    } // namespace test
  
  } // namespace session

} // namespace mobject
