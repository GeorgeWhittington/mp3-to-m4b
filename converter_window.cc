#include "converter_window.h"
#include <iostream>

ConverterWindow::ConverterWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
: Gtk::ApplicationWindow(cobject),
  glade(refGlade)
{
  show_all();
}

ConverterWindow::~ConverterWindow() {
}