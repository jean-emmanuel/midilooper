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

#include "../config.h"

#include "../xpm/loop192.xpm"
#include "../xpm/loop192_32.xpm"

MainWindow::MainWindow(Engine * engine, Glib::RefPtr<Gtk::Application> app, nsm_client_t * nsm)
{
    m_engine = engine;
    m_app = app;
    m_nsm = nsm;
    m_nsm_visible = true;

    Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
    css_provider->load_from_data(c_mainwindow_css);
    this->get_style_context()->add_provider_for_screen(Gdk::Screen::get_default(), css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    add(m_vbox);

    // menu bar
    m_vbox.pack_start(m_menu, false, false);
    m_menu_file.set_label("_File");
    m_menu_file.set_use_underline(true);
    m_menu_file.set_submenu(m_submenu_file);
    m_menu.append(m_menu_file);

    m_menu_file_quit.set_label(m_nsm && global_nsm_optional_gui_support ? "_Hide" : "_Quit");
    m_menu_file_quit.set_use_underline(true);
    m_menu_file_quit.add_accelerator("activate", get_accel_group(), 'q', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_file_quit.signal_activate().connect([this]{
        close();
    });
    m_submenu_file.append(m_menu_file_quit);

    m_menu_transport.set_label("_Transport");
    m_menu_transport.set_use_underline(true);
    m_menu_transport.set_submenu(m_submenu_transport);
    m_menu.append(m_menu_transport);

    m_menu_transport_start_label.set_label("Start");
    m_menu_transport_start_label.set_alignment(0);
    m_menu_transport_start_label.set_accel(GDK_KEY_space, (Gdk::ModifierType)0);
    m_menu_transport_start.add_accelerator("activate", get_accel_group(), GDK_KEY_space, (Gdk::ModifierType)0, Gtk::ACCEL_VISIBLE);
    m_menu_transport_start.add(m_menu_transport_start_label);
    m_menu_transport_start.signal_activate().connect([&]{
        Engine::osc_cmd_handler("/play", "", NULL, 0, NULL, m_engine);
        clear_focus();
    });
    m_submenu_transport.append(m_menu_transport_start);

    m_menu_transport_stop_label.set_label("Stop");
    m_menu_transport_stop_label.set_alignment(0);
    m_menu_transport_stop_label.set_accel(GDK_KEY_Escape, (Gdk::ModifierType)0);
    m_menu_transport_stop.add_accelerator("activate", get_accel_group(), GDK_KEY_Escape, (Gdk::ModifierType)0, Gtk::ACCEL_VISIBLE);
    m_menu_transport_stop.add(m_menu_transport_stop_label);
    m_menu_transport_stop.signal_activate().connect([&]{
        Engine::osc_cmd_handler("/stop", "", NULL, 0, NULL, m_engine);
        clear_focus();
    });
    m_submenu_transport.append(m_menu_transport_stop);


    // Toolbar
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
        clear_focus();
    });
    m_toolbar.pack_start(m_toolbar_panic, false, false);

    m_toolbar_stop.set_size_request(36, 0);
    m_toolbar_stop.set_can_focus(false);
    m_toolbar_stop.set_label("◼");
    m_toolbar_stop.set_tooltip_text("Stop transport");
    m_toolbar_stop.get_style_context()->add_class("stop");
    m_toolbar_stop.signal_clicked().connect([&]{
        Engine::osc_cmd_handler("/stop", "", NULL, 0, NULL, m_engine);
        clear_focus();
    });
    m_toolbar.pack_start(m_toolbar_stop, false, false);

    m_toolbar_play.set_size_request(36, 0);
    m_toolbar_play.set_can_focus(false);
    m_toolbar_play.set_label("▶");
    m_toolbar_play.set_tooltip_text("Start transport");
    m_toolbar_play.get_style_context()->add_class("play");
    m_toolbar_play.signal_clicked().connect([&]{
        Engine::osc_cmd_handler("/play", "", NULL, 0, NULL, m_engine);
        clear_focus();
    });
    m_toolbar.pack_start(m_toolbar_play, false, false);

    m_toolbar_bpm_label.set_label("Tempo");
    m_toolbar_bpm_label.set_sensitive(false);
    m_toolbar_bpm_label.get_style_context()->add_class("nomargin");
    m_toolbar.pack_start(m_toolbar_bpm_label, false, false);

    m_toolbar_bpm_adj = Gtk::Adjustment::create(m_engine->get_bpm(), Config::MIN_BPM, Config::MAX_BPM, 1, 10, 1);
    m_toolbar_bpm.set_tooltip_text("Beats per minute");
    m_toolbar_bpm.set_width_chars(6);
    m_toolbar_bpm.set_digits(2);
    m_toolbar_bpm.set_numeric(true);
    m_toolbar_bpm.set_alignment(0.5);
    m_toolbar_bpm.set_adjustment(m_toolbar_bpm_adj);
    m_toolbar_bpm.signal_activate().connect([&]{clear_focus();});
    m_toolbar_bpm.signal_value_changed().connect([&]{
        m_engine->set_bpm(m_toolbar_bpm.get_value());
    });
    m_toolbar.pack_start(m_toolbar_bpm, false, false);

    m_toolbar_length_label.set_label("Length");
    m_toolbar_length_label.set_sensitive(false);
    m_toolbar_length_label.get_style_context()->add_class("nomargin");
    m_toolbar.pack_start(m_toolbar_length_label, false, false);
    m_toolbar_length_adj = Gtk::Adjustment::create(m_engine->get_length() / Config::PPQN * 2, Config::MIN_8TH_PER_CYCLE, Config::MAX_8TH_PER_CYCLE, 1, 10, 1);
    m_toolbar_length.set_tooltip_text("Eighth notes per cycle");
    m_toolbar_length.signal_activate().connect([&]{clear_focus();});
    m_toolbar_length.set_width_chars(4);
    m_toolbar_length.set_digits(1);
    m_toolbar_length.set_alignment(0.5);
    m_toolbar_length.set_adjustment(m_toolbar_length_adj);
    m_toolbar_length.signal_value_changed().connect([&]{
        m_engine->set_measure_length(m_toolbar_length.get_value());
    });
    m_toolbar.pack_start(m_toolbar_length, false, false);

    m_toolbar_logo.set(Gdk::Pixbuf::create_from_xpm_data(loop192_xpm));
    m_toolbar.pack_end(m_toolbar_logo, false, false);

    // Loops
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

    // Window icon
    set_icon(Gdk::Pixbuf::create_from_xpm_data(loop192_32_xpm));

    resize(800, 400);
    clear_focus();
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

    double bpm = m_engine->get_bpm();
    if (m_toolbar_bpm.get_value() != bpm) {
        m_toolbar_bpm.set_value(bpm);
    }

    auto children = m_loops.get_children();
    for (auto i = children.begin(); i != children.end(); i++) {
        LoopWidget *w = (LoopWidget *)(*i);
        w->timer_callback();
    }

    if (m_nsm) {
        nsm_check_nowait(m_nsm);
        if (global_nsm_optional_gui_support && m_nsm_visible != global_nsm_gui) {
            m_nsm_visible = global_nsm_gui;
            if (m_nsm_visible)
            {
                show();
                nsm_send_is_shown(m_nsm);
            }
            else
            {
                m_app->hold();
                hide();
                nsm_send_is_hidden(m_nsm);
            }
        }
    }

    return true;
}

void
MainWindow::clear_focus()
{
    m_toolbar_bpm.select_region(0, 0);
    m_toolbar_length.select_region(0, 0);
    m_scroll.set_can_focus(true);
    m_scroll.grab_focus();
    m_scroll.set_can_focus(false);
}


bool
MainWindow::on_delete_event(GdkEventAny *event)
{
    if (m_nsm && global_nsm_optional_gui_support) {
        // nsm : hide gui instead of closing
        global_nsm_gui = false;
        return true;
    }

    return false;
}
