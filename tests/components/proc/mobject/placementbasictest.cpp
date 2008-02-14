/*
  PlacementBasic(Test)  -  basic Placement and MObject handling
 
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


#include "common/test/run.hpp"
#include "proc/asset/media.hpp"
#include "proc/mobject/session.hpp"
#include "proc/mobject/session/edl.hpp"
#include "proc/mobject/session/testclip.hpp"
#include "proc/mobject/placement.hpp"
#include "proc/mobject/explicitplacement.hpp"
#include "common/util.hpp"
#include "proc/mobject/session/mobjectfactory.hpp"   ////TODO: avoidable?

//#include <boost/format.hpp>
#include <iostream>

//using boost::format;
using cinelerra::Time;
using util::contains;
using std::string;
using std::cout;


namespace mobject
  {
  namespace session
    {
    namespace test
      {
      
      using asset::VIDEO;
      
      
      
      /*******************************************************************
       * @test basic behaviour of Placements and access to MObjects.
       * @see  mobject::Placement
       * @see  mobject::MObject#create
       * @see  mobject::Placement#addPlacement
       * @see  mobject::Placement#resolve
       */
      class PlacementBasic_test : public Test
        {
          typedef shared_ptr<asset::Media> PM;
          typedef shared_ptr<asset::Clip> PCA;
          
          virtual void
          run (Arg arg) 
            {
              // create Clip-MObject, which is wrapped into a placement (smart ptr)
              PM media = asset::Media::create("test-1", VIDEO);
              PCA clipAsset = Media::create(*media);
              Placement<Clip> pc = MObject::create (*clipAsset, *media);

              // use of the Clip-MObject interface by dereferencing the placement
              PM clip_media = pc->getMedia();
              ASSERT (clip_media->ident.category.hasKind (VIDEO));
              
              // using the Placement interface
              // TODO: how to handle unterdetermined Placement? Throw?
              FixedLocation & fixloc = pc.chain(Time(1)); // TODO: the track??
              ExplicitPlacement expla = pc.resolve();
              ASSERT (expla.time == 1);
              ASSERT (!expla.chain.isOverdetermined());
              //ASSERT (*expla == *pc);  ////////////////////////TODO: definie equality on placements
              
              // now overconstraining with another Placement
              pc.chain(Time(2));
              ExplicitPlacement expla2 = pc.resolve();
              ASSERT (expla2.time == 2); // the latest addition wins
              ASSERT (expla2.chain.isOverdetermined()); 
            } 
        };
      
      
      /** Register this test class... */
      LAUNCHER (PlacementBasic_test, "unit session");
      
      
      
    } // namespace test
  
  } // namespace session

} // namespace mobject