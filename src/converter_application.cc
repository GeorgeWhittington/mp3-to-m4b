#include "converter_application.h"

ConverterApplication::ConverterApplication()
: Gtk::Application("org.george.mp3_to_m4b"),
  glade(Gtk::Builder::create())
{
  glade->add_from_resource("/org/george/mp3_to_m4b/converter.glade");
  glade->add_from_resource("/org/george/mp3_to_m4b/menu.glade");

  Glib::set_application_name("MP3 to M4B Converter");

  // load css
  Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
  css_provider->load_from_resource("/org/george/mp3_to_m4b/converter.css");

  Glib::RefPtr<Gtk::StyleContext> style_context = Gtk::StyleContext::create();

  Glib::RefPtr<Gdk::Screen> screen = Gdk::Screen::get_default();

  style_context->add_provider_for_screen(screen, css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

Glib::RefPtr<ConverterApplication> ConverterApplication::create() {
  return Glib::RefPtr<ConverterApplication>(new ConverterApplication());
}

void ConverterApplication::on_startup() {
  Gtk::Application::on_startup();

  auto object = glade->get_object("application_menu");
  auto app_menu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);

  if (!app_menu) {
    std::cout << "Application menu not found" << std::endl;
    g_warning("Application menu not found");
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
    glade->get_widget_derived("converter_application_window", window);
    add_window(*window);  // Add window to application so stuff closes correctly
  }
}