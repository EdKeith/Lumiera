/*
  timeline-arrow-tool.cpp  -  Implementation of the ArrowTool class

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

#include "timeline-arrow-tool.hpp"

namespace gui {
namespace widgets {
namespace timeline {

  ArrowTool::ArrowTool(TimelineBody &timelineBody) :
    Tool(timelineBody)
  {

  }

  ToolType
  ArrowTool::get_type() const
  {
    return Arrow;
  }

  Gdk::Cursor
  ArrowTool::get_cursor() const
  {
    return Gdk::Cursor(Gdk::LEFT_PTR);
  }

  void
  ArrowTool::on_button_press_event(GdkEventButton* event)
  {
    REQUIRE (event != NULL);
    Tool::on_button_press_event(event);

    // Convert the mouse click position to a Time
    boost::shared_ptr<TimelineState> state = timelineBody.getTimelineWidget().get_state();
    REQUIRE(state);
    const TimelineViewWindow &window = state->get_view_window();
    lumiera::Time tpoint = window.x_to_time(mousePoint.get_x());

    // Get the clip, if any
    boost::shared_ptr<timeline::Track> track = getHoveringTrack();
    boost::shared_ptr<Clip> clip = track->getClipAt(tpoint);

    // Nothing to do if there is no clip
    if (clip == boost::shared_ptr<Clip>())
      return;

    clip->setSelected(true);
  }

  void
  ArrowTool::on_button_release_event(GdkEventButton* event)
  {
    REQUIRE (event != NULL);
    Tool::on_button_release_event(event);

    boost::shared_ptr<timeline::Track> track =
      getHoveringTrack();
  }

  void
  ArrowTool::on_motion_notify_event(GdkEventMotion *event)
  {
    REQUIRE (event != NULL);
    Tool::on_motion_notify_event(event);

    // We do not need to do anything if we are not dragging
    if (!isDragging)
      return;
  }

  boost::shared_ptr<timeline::Track>
  ArrowTool::getHoveringTrack ()
  {
    boost::shared_ptr<timeline::Track> track(
      timelineBody.getTimelineWidget().get_hovering_track());
    return track;
  }

}   // namespace timeline
}   // namespace widgets
}   // namespace gui
