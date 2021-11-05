// This file is part of loop192
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#include "mainwindow.h"
#include "style.h"

#include "../xpm/loop192.xpm"
#include "../xpm/loop192_32.xpm"


MainWindow::MainWindow(Engine * engine)
{
    m_engine = engine;

    Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
    css_provider->load_from_data(c_mainwindow_css);
    this->get_style_context()->add_provider_for_screen(Gdk::Screen::get_default(), css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    add(m_vbox);

    m_toolbar_play_state = false;
    m_vbox.pack_start(m_toolbar, false, false);
    m_toolbar.set_size_request(0, 55);
    m_toolbar.get_style_context()->add_class("toolbar");
    m_toolbar.set_spacing(c_toolbar_spacing);


    m_toolbar_panic.set_size_request(36, 0);
    m_toolbar_panic.set_can_focus(false);
    m_toolbar_panic.set_label("◭");
    m_toolbar_panic.set_tooltip_text("Mute all loops");
    m_toolbar_panic.get_style_context()->add_class("panic");
    m_toolbar_panic.signal_clicked().connect([&]{
        Engine::osc_cmd_handler("/panic", "", NULL, 0, NULL, m_engine);
    });
    m_toolbar.pack_start(m_toolbar_panic, false, false);

    m_toolbar_stop.set_size_request(36, 0);
    m_toolbar_stop.set_can_focus(false);
    m_toolbar_stop.set_label("◼");
    m_toolbar_stop.set_tooltip_text("Stop transport");
    m_toolbar_stop.get_style_context()->add_class("stop");
    m_toolbar_stop.signal_clicked().connect([&]{
        Engine::osc_cmd_handler("/stop", "", NULL, 0, NULL, m_engine);
    });
    m_toolbar.pack_start(m_toolbar_stop, false, false);

    m_toolbar_play.set_size_request(36, 0);
    m_toolbar_play.set_can_focus(false);
    m_toolbar_play.set_label("▶");
    m_toolbar_play.set_tooltip_text("Start transport");
    m_toolbar_play.get_style_context()->add_class("play");
    m_toolbar_play.signal_clicked().connect([&]{
        Engine::osc_cmd_handler("/play", "", NULL, 0, NULL, m_engine);
    });
    m_toolbar.pack_start(m_toolbar_play, false, false);

    m_toolbar_logo.set(Gdk::Pixbuf::create_from_xpm_data(loop192_xpm));
    m_toolbar.pack_end(m_toolbar_logo, false, false);

    m_vbox.add(m_scroll);
    m_scroll.add(m_loops);
    m_scroll.set_overlay_scrolling(false);
    m_loops.set_spacing(1);
    for (std::list <Loop>::iterator i = engine->m_loops.begin(); i != engine->m_loops.end(); i++) {
        LoopWidget * w = new LoopWidget(&(*i));
        m_loops.pack_start(*w, false, false);
    }

    // timer callback (25 fps)
    Glib::signal_timeout().connect(mem_fun(*this, &MainWindow::timer_callback), 20);


    set_icon(Gdk::Pixbuf::create_from_xpm_data(loop192_32_xpm));

    resize(600, 200);
    show_all();
}

MainWindow::~MainWindow()
{

}

bool
MainWindow::timer_callback()
{

    if (m_engine->m_playing != m_toolbar_play_state) {
        m_toolbar_play_state = m_engine->m_playing;
        if (m_toolbar_play_state) m_toolbar_play.get_style_context()->add_class("on");
        else m_toolbar_play.get_style_context()->remove_class("on");
    }

    auto children = m_loops.get_children();
    for (auto i = children.begin(); i != children.end(); i++) {
        LoopWidget *w = (LoopWidget *)(*i);
        w->timer_callback();
    }

    return true;
}
