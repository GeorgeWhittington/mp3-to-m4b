#include <gtkmm.h>

class ConverterApplication : public Gtk::Application {
  public:
    ConverterApplication();
    static Glib::RefPtr<ConverterApplication> create();

  protected:
    void on_startup() override;
    void on_activate() override;

  private:
    void create_window();

    Glib::RefPtr<Gtk::Builder> glade;
};