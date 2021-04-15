#include <gtkmm.h>
#include <iostream>

Gtk::Window* pWindow = nullptr;

static void on_button_clicked() {
  std::cout << "Button clicked" << std::endl;
}

int main (int argc, char *argv[]) {
  auto app = Gtk::Application::create(argc, argv, "org.gtkmm.example");

  auto refBuilder = Gtk::Builder::create();
  try {
    refBuilder->add_from_file("window.glade");
  } catch(const Glib::FileError& ex) {
    std::cerr << "FileError: " << ex.what() << std::endl;
    return 1;
  } catch(const Glib::MarkupError& ex) {
    std::cerr << "MarkupError: " << ex.what() << std::endl;
    return 1;
  } catch(const Gtk::BuilderError& ex) {
    std::cerr << "BuilderError: " << ex.what() << std::endl;
    return 1;
  }

  refBuilder->get_widget("main_window", pWindow);
  if(pWindow) {
    Gtk::Button* pButton = nullptr;
    refBuilder->get_widget("enter_button", pButton);
    
    if(pButton) {
      pButton->signal_clicked().connect( sigc::ptr_fun(on_button_clicked) );
    }

    return app->run(*pWindow);
  }

  delete pWindow;
  return 0;
}