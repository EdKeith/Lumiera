/*
  viewer-panel.cpp  -  Implementation of the viewer panel
 
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
#include "viewer-panel.hpp"

using namespace gui::widgets;
using namespace Gtk;

namespace gui {
namespace panels {

ViewerPanel::ViewerPanel(model::Project *const owner_project) :
  Panel(owner_project, "viewer", _("Viewer"), "panel_viewer")
{    
  //----- Pack in the Widgets -----//
  pack_start(display, PACK_EXPAND_WIDGET);
}

}   // namespace panels
}   // namespace gui
