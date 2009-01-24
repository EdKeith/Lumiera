/*
  playback-controller.hpp  -  Declaration of the playback controller object
 
  Copyright (C)         Lumiera.org
    2009,               Joel Holdsworth <joel@airwebreathe.org.uk>
 
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
/** @file controller/playback-controller.hpp
 ** This file contains the definition of the playback controller object
 */

#include <sigc++/sigc++.h>
#include <glibmm.h>

#ifndef PLAYBACK_CONTROLLER_HPP
#define PLAYBACK_CONTROLLER_HPP

namespace gui {
namespace controller { 

class PlaybackController
{
public:

  PlaybackController();
  
  ~PlaybackController();

  void play();
  
  void pause();
  
  void stop();
  
  bool is_playing();

  void attach_viewer(const sigc::slot<void, void*>& on_frame);
  
private:

  void start_playback_thread();

  void playback_thread();

  void pull_frame();
  
  void on_frame();
  
private:

  Glib::Thread *thread;
  
  Glib::StaticMutex mutex;
  
  Glib::Dispatcher dispatcher;
  
  volatile bool finish_playback_thread;
  
  volatile bool playing;
  
  unsigned char buffer[320 * 240 * 4];
  
  sigc::signal<void, void*> frame_signal;
};

}   // namespace controller
}   // namespace gui

#endif // PLAYBACK_CONTROLLER_HPP
