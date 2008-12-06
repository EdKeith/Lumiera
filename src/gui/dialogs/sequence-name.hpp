/*
  sequence-ane.hpp  -  Definition of the sequence name dialog
 
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
/** @file sequence-name.hpp
 ** This file contains the definition of the sequence name dialog
 **
 */

#ifndef SEQUENCE_NAME_H
#define SEQUENCE_NAME_H

#include "../gtk-lumiera.hpp"

namespace gui {
namespace dialogs {

/** 
 * The defintion of sequence name dialog class
 */
class SequenceName : public Gtk::Dialog
{
public:

  enum Action
  {
    AddSequence,
    RenameSequence
  };
  
public:
  SequenceName(Gtk::Window &parent, SequenceName::Action action,
    Glib::ustring default_title);
    
  const Glib::ustring get_name() const;

private:
  Gtk::HBox hBox;
  Gtk::Label caption;
  Gtk::Entry name;
};

}   // namespace dialogs
}   // namespace gui

#endif // SEQUENCE_NAME_H