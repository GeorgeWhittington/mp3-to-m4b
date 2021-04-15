#include <gtkmm.h>

class ConverterWindow : public Gtk::ApplicationWindow {
  public:
    ConverterWindow(
      BaseObjectType* cobject,
      const Glib::RefPtr<Gtk::Builder>& refGlade
    );
    virtual ~ConverterWindow();
  
  protected:
    Glib::RefPtr<Gtk::Builder> glade;
};