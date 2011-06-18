/*
  OUTPUT-MANAGER.hpp  -  handling all the real external output connections

  Copyright (C)         Lumiera.org
    2011,               Hermann Vosseler <Ichthyostega@web.de>

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

/** @file output-manager.hpp
 ** A global service to handle all external output connections.
 **
 ** @see output-manager-test.cpp  ////TODO
 */


#ifndef PROC_PLAY_OUTPUT_MANAGER_H
#define PROC_PLAY_OUTPUT_MANAGER_H


#include "lib/error.hpp"
#include "proc/play/output-slot.hpp"

#include <boost/noncopyable.hpp>
//#include <string>
//#include <vector>
#include <tr1/memory>


namespace proc {
namespace play {
  
//using std::string;
//using std::vector;
  using std::tr1::shared_ptr;
  
  
  
  
  
  
  
  /******************************************************
   * Management of external Output connections.
   * 
   * @todo write Type comment
   */
  class OutputManager
    : boost::noncopyable
    {
    public:
      OutputManager() {}
    };
  
  typedef shared_ptr<OutputManager> POutputManager;
  
  
}} // namespace proc::play
#endif
