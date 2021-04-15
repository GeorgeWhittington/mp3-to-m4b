#ifndef CONVERTER_APPLICATION
#define CONVERTER_APPLICATION

#include <iostream>
#include <string>

#include <gtkmm.h>

#include "converter_window.h"

class ConverterApplication : public Gtk::Application {
  public:
    ConverterApplication(std::string bin_path);
    static Glib::RefPtr<ConverterApplication> create(std::string bin_path);

  protected:
    void on_startup() override;
    void on_activate() override;

  private:
    void create_window();
    void on_hide_window(Gtk::Window* window);

    Glib::RefPtr<Gtk::Builder> glade;
    std::string bin_path;
};

#endif