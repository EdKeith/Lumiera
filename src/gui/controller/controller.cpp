/*
  controllerk.cpp  -  Implementation of the timeline track object

  Copyright (C)         Lumiera.org
    2008,               Joel Holdsworth <joel@airwebreathe.org.uk>

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


#include "gui/controller/controller.hpp"

namespace gui {
namespace controller {
  
Controller::Controller(model::Project &modelProject) :
  project(modelProject)
{
  
}

PlaybackController& Controller::get_playback_controller()
{
  return playback;
}

}   // namespace controller
}   // namespace gui

