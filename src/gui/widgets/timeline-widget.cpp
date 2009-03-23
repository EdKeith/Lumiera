/*
  timeline-widget.cpp  -  Implementation of the timeline widget
 
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

#include "timeline-widget.hpp"

#include <boost/foreach.hpp>
#include <typeinfo>

using namespace Gtk;
using namespace std;
using namespace boost;
using namespace util;
using namespace gui::widgets::timeline;

namespace gui {
namespace widgets {

const int TimelineWidget::TrackPadding = 1;
const int TimelineWidget::HeaderWidth = 150;
const int TimelineWidget::HeaderIndentWidth = 10;
const double TimelineWidget::ZoomIncrement = 1.25;
const int64_t TimelineWidget::MaxScale = 30000000;

TimelineWidget::TimelineWidget(
  boost::shared_ptr<timeline::TimelineState> source_state) :
  Table(2, 2),
  layoutHelper(*this),
  headerContainer(NULL),
  body(NULL),
  ruler(NULL),
  horizontalAdjustment(0, 0, 0),
  verticalAdjustment(0, 0, 0),
  horizontalScroll(horizontalAdjustment),
  verticalScroll(verticalAdjustment),
  update_tracks_frozen(true)
{
  set_state(source_state);
  thaw_update_tracks();
  
  body = new TimelineBody(*this);
  ENSURE(body != NULL);
  headerContainer = new TimelineHeaderContainer(*this);
  ENSURE(headerContainer != NULL);
  ruler = new TimelineRuler(*this);
  ENSURE(ruler != NULL);
    
  horizontalAdjustment.signal_value_changed().connect( sigc::mem_fun(
    this, &TimelineWidget::on_scroll) );
  verticalAdjustment.signal_value_changed().connect( sigc::mem_fun(
    this, &TimelineWidget::on_scroll) );
  body->signal_motion_notify_event().connect( sigc::mem_fun(
    this, &TimelineWidget::on_motion_in_body_notify_event) );
    
  update_tracks();
  
  attach(*body, 1, 2, 1, 2, FILL|EXPAND, FILL|EXPAND);
  attach(*ruler, 1, 2, 0, 1, FILL|EXPAND, SHRINK);
  attach(*headerContainer, 0, 1, 1, 2, SHRINK, FILL|EXPAND);
  attach(horizontalScroll, 1, 2, 2, 3, FILL|EXPAND, SHRINK);
  attach(verticalScroll, 2, 3, 1, 2, SHRINK, FILL|EXPAND);
  
  set_tool(timeline::Arrow);
}

TimelineWidget::~TimelineWidget()
{
  // Destroy child widgets
  REQUIRE(body != NULL);
  if(body != NULL)
    body->unreference();
    
  REQUIRE(headerContainer != NULL);
  if(headerContainer != NULL)
    headerContainer->unreference();
    
  REQUIRE(ruler != NULL);
  if(ruler != NULL)
    ruler->unreference();
}

/* ===== Data Access ===== */

boost::shared_ptr<timeline::TimelineState>
TimelineWidget::get_state()
{
  ENSURE(state);
  return state;
}

void
TimelineWidget::set_state(shared_ptr<timeline::TimelineState> new_state)
{
  state = new_state;
  REQUIRE(state);

  // Hook up event handlers
  state->get_view_window().changed_signal().connect( sigc::mem_fun(
    this, &TimelineWidget::on_view_window_changed) );
  state->get_sequence()->get_child_track_list().signal_changed().
    connect(sigc::mem_fun(
      this, &TimelineWidget::on_track_list_changed ) );
  
  state->selection_changed_signal().connect(mem_fun(*this,
    &TimelineWidget::on_body_changed));
  state->playback_changed_signal().connect(mem_fun(*this,
    &TimelineWidget::on_body_changed));

  update_tracks();
}

void
TimelineWidget::zoom_view(int zoom_size)
{
  REQUIRE(state);
  
  const int view_width = body->get_allocation().get_width();
  state->get_view_window().zoom_view(view_width / 2, zoom_size);
}

ToolType
TimelineWidget::get_tool() const
{
  REQUIRE(body != NULL);
  return body->get_tool();
}
  
void
TimelineWidget::set_tool(ToolType tool_type)
{
  REQUIRE(body != NULL);
  body->set_tool(tool_type);
}

shared_ptr<timeline::Track>
TimelineWidget::get_hovering_track() const
{
  return hoveringTrack;
}

/* ===== Signals ===== */

sigc::signal<void, gavl_time_t>
TimelineWidget::mouse_hover_signal() const
{
  return mouseHoverSignal;
}

sigc::signal<void>
TimelineWidget::playback_period_drag_released_signal() const
{
  return playbackPeriodDragReleasedSignal;
}

sigc::signal<void, shared_ptr<timeline::Track> >
TimelineWidget::hovering_track_changed_signal() const
{
  return hoveringTrackChangedSignal;
}

/* ===== Events ===== */

void
TimelineWidget::on_scroll()
{
  REQUIRE(state);
  state->get_view_window().set_time_offset(
    horizontalAdjustment.get_value());
}
  
void
TimelineWidget::on_size_allocate(Allocation& allocation)
{
  Widget::on_size_allocate(allocation);
  
  update_scroll();
}

void
TimelineWidget::on_view_window_changed()
{
  REQUIRE(ruler != NULL);
  REQUIRE(state);
   
  timeline::TimelineViewWindow &window = state->get_view_window();
  const int view_width = body->get_allocation().get_width();
  
  horizontalAdjustment.set_page_size(
    window.get_time_scale() * view_width);
  horizontalAdjustment.set_value(window.get_time_offset());
}

void
TimelineWidget::on_body_changed()
{
  REQUIRE(ruler != NULL);
  REQUIRE(body != NULL);
  ruler->queue_draw();
  body->queue_draw();
}

void
TimelineWidget::on_add_track_command()
{  
  // # TEST CODE
  sequence()->get_child_track_list().push_back(
    shared_ptr<model::Track>(new model::ClipTrack()));
}

/* ===== Internals ===== */

void
TimelineWidget::update_tracks()
{ 
  if(update_tracks_frozen)
    return;
  
  // Remove any tracks which are no longer present in the model
  remove_orphaned_tracks();
  
  // Create timeline tracks from all the model tracks
  create_timeline_tracks();
  
  // Update the layout helper
  layoutHelper.clone_tree_from_sequence();
  layoutHelper.update_layout();
}

void
TimelineWidget::freeze_update_tracks()
{
  update_tracks_frozen = true;
}
  
void
TimelineWidget::thaw_update_tracks()
{
  update_tracks_frozen = false;
}

void
TimelineWidget::create_timeline_tracks()
{
  BOOST_FOREACH(shared_ptr<model::Track> child,
    sequence()->get_child_tracks())
    create_timeline_tracks_from_branch(child);
    
  // Update the header container
  REQUIRE(headerContainer != NULL);
  headerContainer->update_headers();
}

void
TimelineWidget::create_timeline_tracks_from_branch(
  shared_ptr<model::Track> model_track)
{
  REQUIRE(model_track);
  
  // Is a timeline UI track present in the map already?
  if(!contains(trackMap, model_track))
    {
      // The timeline UI track is not present
      // We will need to create one
      trackMap[model_track] = 
        create_timeline_track_from_model_track(model_track);
    }
  
  // Recurse to child tracks
  BOOST_FOREACH(shared_ptr<model::Track> child,
    model_track->get_child_tracks())
    create_timeline_tracks_from_branch(child);
}

shared_ptr<timeline::Track>
TimelineWidget::create_timeline_track_from_model_track(
  shared_ptr<model::Track> model_track)
{
  REQUIRE(model_track);
  
  // Choose a corresponding timeline track class from the model track's
  // class
  if(typeid(*model_track) == typeid(model::ClipTrack))
    return shared_ptr<timeline::Track>(new timeline::ClipTrack(
      *this, model_track));
  else if(typeid(*model_track) == typeid(model::GroupTrack))
    return shared_ptr<timeline::Track>(new timeline::GroupTrack(
      *this, dynamic_pointer_cast<model::GroupTrack>(model_track)));
  
  ASSERT(NULL); // Unknown track type;
  return shared_ptr<timeline::Track>();
}

void
TimelineWidget::remove_orphaned_tracks()
{  
  std::map<boost::shared_ptr<model::Track>,
    boost::shared_ptr<timeline::Track> >
    orphan_track_map(trackMap);
  
  // Remove all tracks which are still present in the sequence
  BOOST_FOREACH(shared_ptr<model::Track> child,
    sequence()->get_child_tracks())
    search_orphaned_tracks_in_branch(child, orphan_track_map);
  
  // orphan_track_map now contains all the orphaned tracks
  // Remove them
  std::pair<shared_ptr<model::Track>, shared_ptr<timeline::Track> >
    pair; 
  BOOST_FOREACH( pair, orphan_track_map )
    {
      ENSURE(pair.first);
      trackMap.erase(pair.first);
    }
}

void
TimelineWidget::search_orphaned_tracks_in_branch(
    boost::shared_ptr<model::Track> model_track,
    std::map<boost::shared_ptr<model::Track>,
    boost::shared_ptr<timeline::Track> > &orphan_track_map)
{
  REQUIRE(model_track);
  
  // Is the timeline UI still present?
  if(contains(orphan_track_map, model_track))
    orphan_track_map.erase(model_track);
  
  // Recurse to child tracks
  BOOST_FOREACH(shared_ptr<model::Track> child,
    model_track->get_child_tracks())
    search_orphaned_tracks_in_branch(child, orphan_track_map);
}

shared_ptr<timeline::Track>
TimelineWidget::lookup_timeline_track(
  shared_ptr<model::Track> model_track) const
{
  REQUIRE(model_track);
  REQUIRE(model_track != sequence()); // The sequence isn't
                                      // really a track

  std::map<shared_ptr<model::Track>, shared_ptr<timeline::Track> >::
    const_iterator iterator = trackMap.find(model_track);
  if(iterator == trackMap.end())
    {
      // The track is not present in the map
      // We are in an error condition if the timeline track is not found
      // - the timeline tracks must always be synchronous with the model
      // tracks.
      ENSURE(0);
      return shared_ptr<timeline::Track>();
    }
  ENSURE(iterator->second != NULL);
  return iterator->second;
}

void
TimelineWidget::on_layout_changed()
{
  REQUIRE(headerContainer != NULL);
  REQUIRE(body != NULL);
  
  headerContainer->on_layout_changed();
  body->queue_draw();
  update_scroll();
}

void
TimelineWidget::update_scroll()
{
  REQUIRE(body != NULL);
  const Allocation body_allocation = body->get_allocation();
  
  REQUIRE(state);
  timeline::TimelineViewWindow &window = state->get_view_window();
  
  //----- Horizontal Scroll ------//
  
  // TEST CODE
  horizontalAdjustment.set_upper(1000 * GAVL_TIME_SCALE / 200);
  horizontalAdjustment.set_lower(-1000 * GAVL_TIME_SCALE / 200);
  
  // Set the page size
  horizontalAdjustment.set_page_size(
    window.get_time_scale() * body_allocation.get_width());
  
  //----- Vertical Scroll -----//
  
  // Calculate the vertical length that can be scrolled:
  // the total height of all the tracks minus one screenful 
  int y_scroll_length = layoutHelper.get_total_height() -
    body_allocation.get_height();
  if(y_scroll_length < 0) y_scroll_length = 0;    
  
  // If by resizing we're now over-scrolled, scroll back to
  // maximum distance
  if((int)verticalAdjustment.get_value() > y_scroll_length)
      verticalAdjustment.set_value(y_scroll_length);
  
  verticalAdjustment.set_upper(y_scroll_length);
  
  // Hide the scrollbar if no scrolling is possible
#if 0
  // Having this code included seems to cause a layout loop as the
  // window is shrunk
  if(y_scroll_length <= 0 && verticalScroll.is_visible())
    verticalScroll.hide();
  else if(y_scroll_length > 0 && !verticalScroll.is_visible())
    verticalScroll.show();
#endif

}

int
TimelineWidget::get_y_scroll_offset() const
{
  return (int)verticalAdjustment.get_value();
}

void
TimelineWidget::set_y_scroll_offset(const int offset)
{
  verticalAdjustment.set_value(offset);
}

bool
TimelineWidget::on_motion_in_body_notify_event(GdkEventMotion *event)
{
  REQUIRE(event != NULL);
  ruler->set_mouse_chevron_offset(event->x);
  
  REQUIRE(state);
  timeline::TimelineViewWindow &window = state->get_view_window();
  mouseHoverSignal.emit(window.x_to_time(event->x));
  
  return true;
}

boost::shared_ptr<model::Sequence>
TimelineWidget::sequence() const
{
  REQUIRE(state);
  boost::shared_ptr<model::Sequence> sequence = state->get_sequence();
  ENSURE(sequence);
  return sequence;
}

void
TimelineWidget::on_track_list_changed()
{
  update_tracks();
}

void
TimelineWidget::on_playback_period_drag_released()
{
  playbackPeriodDragReleasedSignal.emit();
}

void
TimelineWidget::set_hovering_track(
  shared_ptr<timeline::Track> hovering_track)
{
  hoveringTrack = hovering_track;
  hoveringTrackChangedSignal.emit(hovering_track);
}

}   // namespace widgets
}   // namespace gui
