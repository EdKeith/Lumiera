/*
  header-container.cpp  -  Implementation of the header container widget
 
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

#include <boost/foreach.hpp>

#include "header-container.hpp"
#include "track.hpp"
#include "../timeline-widget.hpp"

using namespace Gtk;
using namespace std;

namespace lumiera {
namespace gui {
namespace widgets {
namespace timeline {

HeaderContainer::HeaderContainer(lumiera::gui::widgets::TimelineWidget *timeline_widget) :
    timelineWidget(timeline_widget)
  {
    REQUIRE(timeline_widget != NULL);
  
    set_flags(Gtk::NO_WINDOW);  // This widget will not have a window at first
    set_redraw_on_allocate(false);
    
    // Connect to the timeline widget's vertical scroll event,
    // so that we get notified when the view shifts
    timelineWidget->verticalAdjustment.signal_value_changed().connect(
      sigc::mem_fun(this, &HeaderContainer::on_scroll) );
  }
  
void
HeaderContainer::update_headers()
  {
    ASSERT(timelineWidget != NULL);
    
    BOOST_FOREACH( Track* track, timelineWidget->tracks )
      {
        ASSERT(track != NULL);
                
        Glib::RefPtr<Gtk::Frame> headerFrame(new Gtk::Frame());
        headerFrame->add(track->get_header_widget());
        headerFrame->set_shadow_type (Gtk::SHADOW_ETCHED_OUT);
        headerFrame->set_name ("TimelineHeaderBaseUnselected");
        headerFrame->set_parent(*this);
        headerFrame->show();
        
        const RootHeader header = { headerFrame, track };
        rootHeaders.push_back(header);
      }
      
    layout_headers();
  }
  
void
HeaderContainer::on_realize()
  {
    set_flags(Gtk::NO_WINDOW);
    
    ensure_style();

    // Call base class:
    Gtk::Widget::on_realize();

    // Create the GdkWindow:
    GdkWindowAttr attributes;
    memset(&attributes, 0, sizeof(attributes));

    Allocation allocation = get_allocation();

    // Set initial position and size of the Gdk::Window:
    attributes.x = allocation.get_x();
    attributes.y = allocation.get_y();
    attributes.width = allocation.get_width();
    attributes.height = allocation.get_height();

    attributes.event_mask = get_events () | Gdk::EXPOSURE_MASK; 
    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.wclass = GDK_INPUT_OUTPUT;

    gdkWindow = Gdk::Window::create(get_window(), &attributes,
            GDK_WA_X | GDK_WA_Y);
    unset_flags(Gtk::NO_WINDOW);
    set_window(gdkWindow);
    
    // Unset the background so as to make the colour match the parent window
    unset_bg(STATE_NORMAL);

    // Make the widget receive expose events
    gdkWindow->set_user_data(gobj());
  }
  
void
HeaderContainer::on_unrealize()
  {
    // Unreference any window we may have created
    gdkWindow.clear();

    // Call base class:
    Gtk::Widget::on_unrealize();
  }

void
HeaderContainer::on_size_request (Requisition* requisition)
  {   
    // Initialize the output parameter:
    *requisition = Gtk::Requisition();

    requisition->width = TimelineWidget::HeaderWidth; 
    requisition->height = 0;
  }
  
void
HeaderContainer::on_size_allocate (Allocation& allocation)
  { 
    // Use the offered allocation for this container:
    set_allocation(allocation);
    
    // Resize the widget's window
    if(gdkWindow)
      gdkWindow->resize(allocation.get_width(), allocation.get_height());
    
    // Relayout the child widgets of the headers
    layout_headers();
  }
  
void
HeaderContainer::forall_vfunc(gboolean /* include_internals */,
        GtkCallback callback, gpointer callback_data)
  {      
    BOOST_FOREACH( RootHeader &header, rootHeaders )
      {
        ASSERT(header.widget);
        callback(header.widget->gobj(), callback_data);
      }
  }
  
void
HeaderContainer::on_scroll()
  {
    // If the scroll has changed, we will have to shift all the
    // header widgets
    layout_headers();
  }
  
void
HeaderContainer::layout_headers()
  {
    ASSERT(timelineWidget != NULL);  
  
    int offset = 0;
    const int y_scroll_offset = timelineWidget->get_y_scroll_offset();
    
    const Allocation container_allocation = get_allocation();    
    
    BOOST_FOREACH( RootHeader &header, rootHeaders )
      {
        ASSERT(header.widget);
        ASSERT(header.track != NULL);
        
        const int height = header.track->get_height();
        ASSERT(height >= 0);
             
        Gtk::Allocation header_allocation;
        header_allocation.set_x (0);
        header_allocation.set_y (offset - y_scroll_offset);
        header_allocation.set_width (container_allocation.get_width ());
        header_allocation.set_height (height);
        header.widget->size_allocate(header_allocation);
        
        offset += height + TimelineWidget::TrackPadding;
      }
  }

}   // namespace timeline
}   // namespace widgets
}   // namespace gui
}   // namespace lumiera