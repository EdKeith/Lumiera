/*
  timeline-ruler.cpp  -  Implementation of the time ruler widget
 
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

#include <cairomm-1.0/cairomm/cairomm.h>

#include "timeline-ruler.hpp"
#include "../../window-manager.hpp"

extern "C" {
#include <gavl/gavltime.h>
#include "../../../lib/time.h"
}

using namespace Gtk;
using namespace std;
using namespace lumiera::gui;
using namespace lumiera::gui::widgets;
using namespace lumiera::gui::widgets::timeline;

namespace lumiera {
namespace gui {
namespace widgets {
namespace timeline {

TimelineRuler::TimelineRuler() :
    Glib::ObjectBase("TimelineRuler")
{
  set_flags(Gtk::NO_WINDOW);  // This widget will not have a window
  set_size_request(-1, 20);
  
  // Install style properties
  timeScale = GAVL_TIME_SCALE / 200;
  timeOffset = 0;
}

bool
TimelineRuler::on_expose_event(GdkEventExpose* event)
{
  // This is where we draw on the window
  Glib::RefPtr<Gdk::Window> window = get_window();
  if(!window)
    return false;
  
  // Makes sure the widget styles have been loaded
  read_styles();
  
  // Prepare to render via cairo      
  Allocation allocation = get_allocation();
  Glib::RefPtr<Style> style = get_style();
  Cairo::RefPtr<Cairo::Context> cairo = window->create_cairo_context();
  Glib::RefPtr<Pango::Layout> pango_layout = create_pango_layout("");

  cairo->translate(allocation.get_x(), allocation.get_y());

  // Render the background
  Gdk::Cairo::set_source_color(cairo, style->get_bg(STATE_NORMAL));
  cairo->rectangle(0, 0, allocation.get_width(), allocation.get_height());
  cairo->fill();
  
  // Render ruler annotations
  gavl_time_t major_spacing = GAVL_TIME_SCALE;
  
  int64_t time_offset = timeOffset;
  while(time_offset / timeScale < allocation.get_width())
    {    
      pango_layout->set_text(lumiera_tmpbuf_print_time(time_offset));
      Pango::Rectangle text_extents = pango_layout->get_logical_extents();     
      Gdk::Cairo::set_source_color(cairo, style->get_fg(STATE_NORMAL));
      
      const int64_t x = time_offset / timeScale;
      cairo->move_to(x, (allocation.get_height() - text_extents.get_height() / Pango::SCALE) / 2);
      
      pango_layout->add_to_cairo_context(cairo);
      
      cairo->fill();
      
      time_offset += major_spacing;
    }

  return true;
}
  
void
TimelineRuler::read_styles()
{
  //background = WindowManager::read_style_colour_property(
  //  *this, "background", 0, 0, 0);
}
  
}   // namespace timeline
}   // namespace widgets
}   // namespace gui
}   // namespace lumiera