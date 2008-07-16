/*
  timeline-panel.cpp  -  Implementation of the timeline panel
 
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

#include "../gtk-lumiera.hpp"
#include "timeline-panel.hpp"

using namespace Gtk;

namespace lumiera {
namespace gui {
namespace panels {

TimelinePanel::TimelinePanel() :
  Panel("timeline", _("Timeline"), "timeline_panel"),
  button(Stock::OK)
  {
    // Setup the toolbar
    toolbar.append(button);
    
    toolbar.set_icon_size(IconSize(ICON_SIZE_MENU));
    toolbar.set_toolbar_style(TOOLBAR_ICONS);
    
    // Add the toolbar
    pack_start(toolbar, PACK_SHRINK);
    pack_start(timeline_widget, PACK_EXPAND_WIDGET);
  }

}   // namespace panels
}   // namespace gui
}   // namespace lumiera
