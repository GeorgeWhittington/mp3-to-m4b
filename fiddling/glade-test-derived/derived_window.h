#include <gtkmm.h>

class DerivedWindow : public Gtk::Window {
  public:
    // constructor/destructor
    DerivedWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
    virtual ~DerivedWindow();
  
  protected:
    // signal handler
    void on_button_enter();

    Glib::RefPtr<Gtk::Builder> glade;
    Gtk::Button* enterButton;
    Gtk::Entry* entry;
};