#include <string>

#include <gtkmm.h>

#include "converter_application.h"

ConverterApplication::ConverterApplication(std::string bin_path)
: Gtk::Application("org.george.mp3_to_m4b"),
  bin_path(bin_path),
  glade(Gtk::Builder::create())
{
  glade->add_from_resource("/org/george/mp3_to_m4b/converter.glade");
  glade->add_from_resource("/org/george/mp3_to_m4b/menu.glade");
  glade->add_from_resource("/org/george/mp3_to_m4b/conversion_dialog.glade");

  Glib::set_application_name("MP3 to M4B Converter");

  // load css
  Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
  css_provider->load_from_resource("/org/george/mp3_to_m4b/converter.css");

  Glib::RefPtr<Gtk::StyleContext> style_context = Gtk::StyleContext::create();

  Glib::RefPtr<Gdk::Screen> screen = Gdk::Screen::get_default();

  style_context->add_provider_for_screen(screen, css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

Glib::RefPtr<ConverterApplication> ConverterApplication::create(std::string bin_path) {
  return Glib::RefPtr<ConverterApplication>(new ConverterApplication(bin_path));
}

void ConverterApplication::on_startup() {
  Gtk::Application::on_startup();

  auto object = glade->get_object("application_menu");
  auto app_menu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);

  if (!app_menu) {
    std::cout << "Application menu not found" << std::endl;
  } else {
    set_menubar(app_menu);
  }
}

void ConverterApplication::on_activate() {
  create_window();
}

void ConverterApplication::create_window() {
  if (glade) {
    ConverterWindow* window = nullptr;
    glade->get_widget_derived("converter_application_window", window, bin_path);
    add_window(*window);  // Add window to application so stuff closes correctly

    // delete on hide so destructors get called correctly
    window->signal_hide().connect(sigc::bind<Gtk::Window*>(sigc::mem_fun(*this,
      &ConverterApplication::on_hide_window), window));
  }
}

void ConverterApplication::on_hide_window(Gtk::Window* window) {
  delete window;
}