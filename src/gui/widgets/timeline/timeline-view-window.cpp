/*
  timeline-view-window.hpp  -  Implementation of the timeline window object
 
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

#include "timeline-view-window.hpp"
#include "../timeline-widget.hpp"

using namespace Gtk;
using namespace gui::widgets;
using namespace lumiera;

namespace gui {
namespace widgets {
namespace timeline {

TimelineViewWindow::TimelineViewWindow(
  Time offset, int64_t scale) :
  timeOffset(offset),
  timeScale(scale)
{
}

Time
TimelineViewWindow::get_time_offset() const
{
  return timeOffset;
}

void
TimelineViewWindow::set_time_offset(Time offset)
{
  timeOffset = offset;
  changedSignal.emit();
}

int64_t
TimelineViewWindow::get_time_scale() const
{
  return timeScale;
}

void
TimelineViewWindow::set_time_scale(int64_t scale)
{
  timeScale = scale;
  changedSignal.emit();
}

void
TimelineViewWindow::zoom_view(int point, int zoom_size)
{ 
  int64_t new_time_scale = (double)timeScale * pow(1.25, -zoom_size);
  
  // Limit zooming in too close
  if(new_time_scale < 1) new_time_scale = 1;
  
  // Nudge zoom problems caused by integer rounding
  if(new_time_scale == timeScale && zoom_size < 0)
    new_time_scale++;
    
  // Limit zooming out too far
  if(new_time_scale > TimelineWidget::MaxScale)
    new_time_scale = TimelineWidget::MaxScale;
  
  // The view must be shifted so that the zoom is centred on the cursor
  set_time_offset(Time((gavl_time_t)get_time_offset() +
    (timeScale - new_time_scale) * point));
    
  // Apply the new scale
  set_time_scale(new_time_scale);
}

void
TimelineViewWindow::shift_view(int view_width, int shift_size)
{
  set_time_offset(Time((gavl_time_t)get_time_offset() +
    shift_size * timeScale * view_width / 256));
}

int
TimelineViewWindow::time_to_x(gavl_time_t time) const
{
  return (int)((time - timeOffset) / timeScale);
}

Time
TimelineViewWindow::x_to_time(int x) const
{
  return Time((gavl_time_t)((int64_t)x * timeScale + timeOffset));
}

sigc::signal<void>
TimelineViewWindow::changed_signal() const
{
  return changedSignal;
}

}   // namespace timeline
}   // namespace widgets
}   // namespace gui

