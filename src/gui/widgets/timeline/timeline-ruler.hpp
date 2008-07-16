/*
  timeline-ruler.hpp  -  Declaration of the time ruler widget
 
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
 
*/
/** @file timeline-ruler.hpp
 ** This file contains the declaration of the time ruler widget
 ** widget
 */
 
#ifndef TIMELINE_RULER_HPP
#define TIMELINE_RULER_HPP

#include "../../gtk-lumiera.hpp"

namespace lumiera {
namespace gui {
namespace widgets {
namespace timeline {

class TimelineRuler : public Gtk::DrawingArea
{
public:
  TimelineRuler();
  
  /**
   * Sets the time offset. This is the time value displaid at the
   * left-hand edge of the ruler.
   */
  void set_time_offset(gavl_time_t time_offset);

  /**
   * Sets the time scale value.
   * @param time_scale The scale factor, which is the number of
   * microseconds per screen pixel. This value must be greater than
   * zero.
   */
  void set_time_scale(int64_t time_scale);
  
  /**
   * Sets the offset of the mouse chevron in pixels from the left
   * edge of the widget. If offset is less than 0 or greater than the
   * width, the chevron will not be visible.
   */
  void set_mouse_chevron_offset(int offset);

  /* ===== Events ===== */
protected:

  void on_realize();

  bool on_expose_event(GdkEventExpose *event);
  
  bool on_motion_notify_event(GdkEventMotion *event);
  
  void on_size_request(Gtk::Requisition *requisition);

  void on_size_allocate(Gtk::Allocation& allocation);
  
  /* ===== Internals ===== */
private:
  void draw_ruler(Cairo::RefPtr<Cairo::Context> cairo,
    Gdk::Rectangle ruler_rect);

  void draw_mouse_chevron(Cairo::RefPtr<Cairo::Context> cairo,
    Gdk::Rectangle ruler_rect);

  gavl_time_t calculate_major_spacing() const;
  
  void register_styles() const;
  
  void read_styles();
  
private:
  // View values
  gavl_time_t timeOffset;
  int64_t timeScale;
  
  // Indicated values
  int mouseChevronOffset;
  
  // Style values
  int annotationHorzMargin;
  int annotationVertMargin;
  int majorTickHeight;
  int minorLongTickHeight;
  int minorShortTickHeight;
  int minDivisionWidth;
  int mouseChevronSize;
  
  // Cached ruler image
  Cairo::RefPtr<Cairo::ImageSurface> rulerImage;
};

}   // namespace timeline
}   // namespace widgets
}   // namespace gui
}   // namespace lumiera

#endif // TIMELINE_RULER_HPP

