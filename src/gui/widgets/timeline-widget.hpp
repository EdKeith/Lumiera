/*
  timeline-widget.hpp  -  Declaration of the timeline widget
 
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
/** @file timeline-widget.hpp
 ** This file contains the definition of timeline widget
 */

#ifndef TIMELINE_WIDGET_HPP
#define TIMELINE_WIDGET_HPP

#include "../gtk-lumiera.hpp"
#include "timeline/header-container.hpp"
#include "timeline/timeline-body.hpp"
#include "timeline/timeline-ruler.hpp"
#include "timeline/timeline-tool.hpp"
#include "timeline/timeline-arrow-tool.hpp"
#include "timeline/timeline-ibeam-tool.hpp"
#include "timeline/track.hpp"

namespace lumiera {
namespace gui {
namespace widgets {

class TimelineWidget : public Gtk::Table
{
public:
  TimelineWidget();

  ~TimelineWidget();
  
  /* ===== Data Access ===== */
public:
  /**
   * Gets the time offset. This is the time value displaid at the
   * left-hand edge of the timeline body area.
   */
  gavl_time_t get_time_offset() const;

  /**
   * Sets the time offset. This is the time value displaid at the
   * left-hand edge of the timeline body area.
   */
  void set_time_offset(gavl_time_t time_offset);
  
  /**
   * Gets the time scale value.
   * @return The scale factor, which is the number of microseconds per
   * screen pixel.
   */
  int64_t get_time_scale() const;
  
  /**
   * Sets the time scale value.
   * @param time_scale The scale factor, which is the number of
   * microseconds per screen pixel. This value must be greater than
   * zero
   */
  void set_time_scale(int64_t time_scale);
  
  /**
   * Zooms the view in or out as by a number of steps while keeping
   * centre of the view still.
   * @param zoom_size The number of steps to zoom by. The scale factor
   * is 1.25^(-zoom_size).
   **/
  void zoom_view(int zoom_size);
  
  /**
   * Zooms the view in or out as by a number of steps while keeping a 
   * given point on the timeline still.
   * @param zoom_size The number of steps to zoom by. The scale factor
   * is 1.25^(-zoom_size).
   **/
  void zoom_view(int point, int zoom_size);
  
  /**
   * Scrolls the view horizontally as a proportion of the view area.
   * @param shift_size The size of the shift in 1/256ths of the view
   * width.
   **/
  void shift_view(int shift_size);
  
  gavl_time_t get_selection_start() const;
  
  gavl_time_t get_selection_end() const;
  
  void set_selection(gavl_time_t start, gavl_time_t end);
  
  timeline::ToolType get_tool() const;
  
  void set_tool(timeline::ToolType tool_type);
    
  /* ===== Events ===== */
protected:
  void on_scroll();
  
  void on_size_allocate(Gtk::Allocation& allocation);
  
  /* ===== Utilities ===== */
protected:
  int time_to_x(gavl_time_t time) const;
  
  gavl_time_t x_to_time(int x) const;

  /* ===== Internals ===== */
private:

  void update_tracks();
  
  void update_scroll();
  
  int get_y_scroll_offset() const;
  
  bool on_motion_in_body_notify_event(GdkEventMotion *event);

protected:

  // View State
  gavl_time_t timeOffset;
  int64_t timeScale;
  
  // Selection State
  gavl_time_t selectionStart;
  gavl_time_t selectionEnd;

  int totalHeight;

  timeline::Track video1;
  timeline::Track video2;
  std::vector<timeline::Track*> tracks;

  timeline::HeaderContainer *headerContainer;
  timeline::TimelineBody *body;
  timeline::TimelineRuler *ruler;

  Gtk::Adjustment horizontalAdjustment, verticalAdjustment;
  Gtk::HScrollbar horizontalScroll;
  Gtk::VScrollbar verticalScroll;
   
  /* ===== Constants ===== */
public:
  static const int64_t MaxScale;
  
protected:
  static const int TrackPadding;
  static const int HeaderWidth;
  static const double ZoomIncrement;

  friend class timeline::TimelineBody;
  friend class timeline::HeaderContainer;
  friend class timeline::TimelineRuler;
  friend class timeline::Tool;
  friend class timeline::ArrowTool;
  friend class timeline::IBeamTool;
};

}   // namespace widgets
}   // namespace gui
}   // namespace lumiera

#endif // TIMELINE_WIDGET_HPP

