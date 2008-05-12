/*
  video-display-widget.hpp  -  Declaration of the video viewer widget
 
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
/** @file viewer-display-widget.hpp
 ** This file contains the definition of video viewer widget
 */

#ifndef VIDEO_DISPLAY_WIDGET_HPP
#define VIDEO_DISPLAY_WIDGET_HPP

#include <gtkmm.h>

#include "../output/displayer.hpp"

using namespace lumiera::gui::output;

namespace lumiera {
namespace gui {
namespace widgets {

  class VideoDisplayWidget : public Gtk::Widget
    {
    public:
      VideoDisplayWidget();

      ~VideoDisplayWidget();

      /* ===== Overrides ===== */
    private:
      virtual void on_realize();
  
      virtual void on_unrealize();
      
      virtual bool on_expose_event(GdkEventExpose* event);

      virtual bool on_button_press_event (GdkEventButton* event);

      /* ===== Internals ===== */
    private:
      static Displayer*
        createDisplayer( Gtk::Widget *drawingArea, int width, int height );

    private:
      Glib::RefPtr<Gdk::Window> gdkWindow;

      Displayer *displayer;
    };

}   // namespace widgets
}   // namespace gui
}   // namespace lumiera

#endif // VIDEO_DISPLAY_WIDGET_HPP
