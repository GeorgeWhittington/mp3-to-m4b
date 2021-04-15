#include "derived_window.h"
#include <iostream>

DerivedWindow::DerivedWindow(BaseObjectType* cObject, const Glib::RefPtr<Gtk::Builder>& refGlade)
: Gtk::Window(cObject),
  m_refGlade(refGlade),
  enterButton(nullptr),
  entry(nullptr)
{
  // get enter button by id, bind it to signal
  m_refGlade->get_widget("enter_button", enterButton);
  if (enterButton) {
    enterButton->signal_clicked().connect( sigc::mem_fun(*this, &DerivedWindow::on_button_enter) );
  }
}

DerivedWindow::~DerivedWindow() {
}

void DerivedWindow::on_button_enter() {
  // get entry box by id, print the text currently inside it
  m_refGlade->get_widget("entry", entry);
  if (entry) {
    std::cout << entry->get_text() << std::endl;
  }
}
