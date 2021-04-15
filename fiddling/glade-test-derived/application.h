#include <gtkmm.h>

class MyApplication : public Gtk::Application {
  public:
    // constructor/destructor
    MyApplication();
    static Glib::RefPtr<MyApplication> create();
  
  protected:
    void on_startup() override;
    void on_activate() override;
  
  private:
    void create_window();
    void on_menu_file_new();
    void on_menu_file_quit();
    void on_menu_help_help();

    Glib::RefPtr<Gtk::Builder> glade;
};