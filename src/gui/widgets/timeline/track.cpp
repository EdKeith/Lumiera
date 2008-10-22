/*
  track.cpp  -  Implementation of the timeline track object
 
  Copyright (C)         Lumiera.org
    2008,               Joel Holdsworth <joel@airwebreathe.org.uk>
 
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

#include "track.hpp"

using namespace Gtk;

namespace gui {
namespace widgets {
namespace timeline {

Track::Track() :
  enableButton(Gtk::StockID("track_enabled")),
  lockButton(Gtk::StockID("track_unlocked"))
{
  buttonBar.append(enableButton);
  buttonBar.append(lockButton);
  
  buttonBar.set_toolbar_style(TOOLBAR_ICONS);
  buttonBar.set_icon_size(ICON_SIZE_MENU);

  headerWidget.pack_start(titleBox, PACK_SHRINK);
  headerWidget.pack_start(buttonBar, PACK_SHRINK);
}

void
Track::add_child_track(timeline::Track* child)
{
  REQUIRE(child != NULL);
  children.push_back(child);
}

const std::vector<timeline::Track*>&
Track::get_child_tracks() const
{
  return children;
}

Gtk::Widget&
Track::get_header_widget()
{
  return headerWidget;
}

int
Track::get_height() const
{
  return 100;
}

Glib::ustring
Track::get_title()
{
  return "Hello";
}

void
Track::draw_track(Cairo::RefPtr<Cairo::Context> cairo) const
{
  
}

}   // namespace timeline
}   // namespace widgets
}   // namespace gui

