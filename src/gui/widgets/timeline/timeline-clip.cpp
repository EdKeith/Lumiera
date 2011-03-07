/*
  timeline-clip.cpp  -  Implementation of the timeline clip object

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

#include "timeline-clip.hpp"

namespace gui {
namespace widgets {
namespace timeline {

  Clip::Clip(boost::shared_ptr<model::Clip> clip,
             boost::shared_ptr<timeline::DrawStrategy> drawStrategy)
    : Entity(drawStrategy),
      modelClip(clip),
      selected(false)
  {
    REQUIRE(modelClip);

    // TODO: Connect signals
    //modelClip->signalNameChanged().connect(mem_fun(this,
    //  &Clip::onNameChanged);
  }

  gavl_time_t
  Clip::getBegin () const
  {
    REQUIRE (modelClip);
    return modelClip->getBegin();
  }

  gavl_time_t
  Clip::getEnd () const
  {
    REQUIRE (modelClip);
    return modelClip->getEnd();
  }

  std::string
  Clip::getName () const
  {
    REQUIRE (modelClip);
    return modelClip->getName();
  }

  void
  Clip::setSelected(bool selected)
  {
    this->selected = selected;
  }


}   // namespace timeline
}   // namespace widgets
}   // namespace gui

