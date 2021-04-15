#include "deriveddialog.h"
#include <iostream>

DerivedDialog::DerivedDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
: Gtk::Dialog(cobject),
  m_refGlade(refGlade),
  m_pButton(nullptr)
{
  // Get the Glade-instantiated Button, and connect a signal handler:
  m_refGlade->get_widget("quit_button", m_pButton);
  if (m_pButton)
  {
    // The user-defined properties in DerivedButton are read correctly from the
    // derived.glade file only if you use glibmm 2.62.0 or newer.
    m_pButton->signal_clicked().connect( sigc::mem_fun(*this, &DerivedDialog::on_button_quit) );
  }
}

// The first two parameters are mandatory in a constructor that will be called
// from Gtk::Builder::get_widget_derived().
// Additional parameters, if any, correspond to additional arguments in the call
// to Gtk::Builder::get_widget_derived().
DerivedDialog::DerivedDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade,
  bool is_glad)
: DerivedDialog(cobject, refGlade) // Delegate to the other constructor
{
  // Show an icon.
  auto pImage = Gtk::make_managed<Gtk::Image>();
  pImage->set_from_icon_name(is_glad ? "face-smile" : "face-sad", Gtk::ICON_SIZE_DIALOG);
  pImage->show_all();
  get_content_area()->pack_start(*pImage);
}

DerivedDialog::~DerivedDialog()
{
}

void DerivedDialog::on_button_quit()
{
  hide(); //hide() will cause Gtk::Application::run() to end.
}
