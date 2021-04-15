#include "application.h"
#include <iostream>

MyApplication::MyApplication()
: Gtk::Application("org.george.example.main_menu")
{
  Glib::set_application_name("Main Menu Example");
}

Glib::RefPtr<MyApplication> MyApplication::create()
{
  return Glib::RefPtr<MyApplication>(new MyApplication());
}

void MyApplication::on_startup() {
  // Call the base class's implementation:
  Gtk::Application::on_startup();

  // add actions
  add_action("new", sigc::mem_fun(*this, &MyApplication::on_menu_file_new));
  add_action("quit", sigc::mem_fun(*this, &MyApplication::on_menu_file_quit));
  add_action("help", sigc::mem_fun(*this, &MyApplication::on_menu_help_help));

  // Create builder
  // (catching these but not fatally erroring seems bad. fix later.)
  glade = Gtk::Builder::create();
  try {
    // glade->add_from_file("window.glade");
    glade->add_from_file("menu2.glade");
  } catch(const Glib::FileError& ex) {
    std::cerr << "FileError: " << ex.what() << std::endl;
  } catch(const Glib::MarkupError& ex) {
    std::cerr << "MarkupError: " << ex.what() << std::endl;
  } catch(const Gtk::BuilderError& ex) {
    std::cerr << "BuilderError: " << ex.what() << std::endl;
  } catch(const Glib::Error& ex) {
    std::cerr << "Building menu failed: " << ex.what() << std::endl;
  }

  // Add menu to application
  auto object = glade->get_object("menu-example");
  auto gmenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
  object = glade->get_object("appmenu");
  auto appMenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
  if (!(gmenu && appMenu)) {
    g_warning("menus not found!");
  } else {
    set_app_menu(appMenu);
    set_menubar(gmenu);
  }
}

void MyApplication::on_activate()
{
  // The application has been started, so let's show a window.
  // A real application might want to reuse this window in on_open(),
  // when asked to open a file, if no changes have been made yet.
  create_window();
}

void MyApplication::create_window()
{
  Gtk::Window win;
  Gtk::Button button;
  win.add(button);
  win.show_all();

  //Make sure that the application runs for as long this window is still open:
  add_window(win);  
}

void MyApplication::on_menu_file_new() {
  std::cout << "new" << std::endl;
}

void MyApplication::on_menu_file_quit() {
  std::cout << "quit" << std::endl;
}

void MyApplication::on_menu_help_help() {
  std::cout << "help" << std::endl;
}