#include "converter_application.h"

ConverterApplication::ConverterApplication()
: Gtk::Application("org.george.mp3_to_m4b"),
  glade(Gtk::Builder::create())
{
  // If this was going to be built as an installable app, the .glade
  // files would get smushed into the binary, most of this error
  // checking could disappear, and it'd therefore be fine to open them here :)
  try {
    glade->add_from_file("converter.glade");
    glade->add_from_file("menu.glade");
  } catch(const Glib::FileError& ex) {
    std::cerr << "FileError: " << ex.what() << std::endl;
  } catch(const Glib::MarkupError& ex) {
    std::cerr << "MarkupError: " << ex.what() << std::endl;
  } catch(const Gtk::BuilderError& ex) {
    std::cerr << "BuilderError: " << ex.what() << std::endl;
  }

  Glib::set_application_name("MP3 to M4B Converter");
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
    add_window(*window);  // Add window to application
  }
}