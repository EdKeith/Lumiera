/*
  workspace-window.cpp  -  Definition of the main workspace window object
 
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

#include <gtkmm/stock.h>

#ifdef ENABLE_NLS
#  include <libintl.h>
#endif

#include <libgdl-1.0/gdl/gdl-tools.h>
#include <libgdl-1.0/gdl/gdl-dock.h>
#include <libgdl-1.0/gdl/gdl-dock-item.h>
#include <libgdl-1.0/gdl/gdl-dock-placeholder.h>
#include <libgdl-1.0/gdl/gdl-dock-bar.h>

#include "gtk-lumiera.hpp"
#include "workspace-window.hpp"

using namespace Gtk;
using namespace lumiera::gui::model;

namespace lumiera {
namespace gui {
namespace workspace {
	
  WorkspaceWindow::WorkspaceWindow(Project *source_project) :
    project(source_project),
    actions(*this)
  {
    layout = NULL;

    create_ui();
  }

  WorkspaceWindow::~WorkspaceWindow()
  {
    if(layout != NULL) g_object_unref(layout);
  }

  void
  WorkspaceWindow::create_ui()
  {
    //----- Configure the Window -----//
    set_title(AppTitle);
    set_default_size(1024, 768);

    //----- Set up the UI Manager -----//
    // The UI will be nested within a VBox
    add(base_container);

    uiManager = Gtk::UIManager::create();
    uiManager->insert_action_group(actions.actionGroup);

    add_accel_group(uiManager->get_accel_group());

    //Layout the actions in a menubar and toolbar:
    Glib::ustring ui_info = 
        "<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu action='FileMenu'>"
        "      <menuitem action='FileNewProject'/>"
        "      <menuitem action='FileOpenProject'/>"
        "      <separator/>"
        "      <menuitem action='FileRender'/>"
        "      <separator/>"
        "      <menuitem action='FileQuit'/>"
        "    </menu>"
        "    <menu action='EditMenu'>"
        "      <menuitem action='EditCopy'/>"
        "      <menuitem action='EditPaste'/>"
        "    </menu>"
        "    <menu action='HelpMenu'>"
        "      <menuitem action='HelpAbout'/>"
        "    </menu>"
        "  </menubar>"
        "  <toolbar  name='ToolBar'>"
        "    <toolitem action='FileNewProject'/>"
        "    <toolitem action='FileOpenProject'/>"
        "  </toolbar>"
        "</ui>";

    try
    {
      uiManager->add_ui_from_string(ui_info);
    }
    catch(const Glib::Error& ex)
    {
      g_error("Building menus failed: %s", ex.what().data());
      return;
    }

    //----- Set up the Menu Bar -----//
    Gtk::Widget* menu_bar = uiManager->get_widget("/MenuBar");
    g_assert(menu_bar != NULL);
    base_container.pack_start(*menu_bar, Gtk::PACK_SHRINK);
    
    //----- Set up the Tool Bar -----//
    Gtk::Toolbar* toolbar = dynamic_cast<Gtk::Toolbar*>(uiManager->get_widget("/ToolBar"));
    g_assert(toolbar != NULL);
    toolbar->set_toolbar_style(TOOLBAR_ICONS);
    base_container.pack_start(*toolbar, Gtk::PACK_SHRINK);

    //----- Create the dock -----//
    dock = Glib::wrap(gdl_dock_new());
    
    layout = gdl_dock_layout_new((GdlDock*)dock->gobj());
	  
    dockbar = Glib::wrap(gdl_dock_bar_new ((GdlDock*)dock->gobj()));

    dock_container.pack_start(*dockbar, PACK_SHRINK);
    dock_container.pack_end(*dock, PACK_EXPAND_WIDGET);
    base_container.pack_start(dock_container, PACK_EXPAND_WIDGET);

	  gdl_dock_add_item ((GdlDock*)dock->gobj(), assets_panel.get_dock_item(), GDL_DOCK_LEFT);
    gdl_dock_add_item ((GdlDock*)dock->gobj(), viewer_panel.get_dock_item(), GDL_DOCK_RIGHT);
    gdl_dock_add_item ((GdlDock*)dock->gobj(), timeline_panel.get_dock_item(), GDL_DOCK_BOTTOM);

    // Manually dock and move around some of the items
	  gdl_dock_item_dock_to (timeline_panel.get_dock_item(), assets_panel.get_dock_item(),
			       GDL_DOCK_BOTTOM, -1);
	  gdl_dock_item_dock_to (viewer_panel.get_dock_item(), assets_panel.get_dock_item(),
			       GDL_DOCK_RIGHT, -1);
    show_all_children();

	  gdl_dock_placeholder_new ("ph1", (GdlDockObject*)dock->gobj(), GDL_DOCK_TOP, FALSE);
	  gdl_dock_placeholder_new ("ph2", (GdlDockObject*)dock->gobj(), GDL_DOCK_BOTTOM, FALSE);
	  gdl_dock_placeholder_new ("ph3", (GdlDockObject*)dock->gobj(), GDL_DOCK_LEFT, FALSE);
	  gdl_dock_placeholder_new ("ph4", (GdlDockObject*)dock->gobj(), GDL_DOCK_RIGHT, FALSE);
  }

}   // namespace workspace
}   // namespace gui
}   // namespace lumiera

