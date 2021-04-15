#include "converter_treestore.h"
#include <gtkmm.h>

class ConverterWindow : public Gtk::ApplicationWindow {
  public:
    ConverterWindow(
      BaseObjectType* cobject,
      const Glib::RefPtr<Gtk::Builder>& refGlade
    );
    virtual ~ConverterWindow();
  
  protected:
    // Signal handlers
    void on_add_chapter();
    void on_add_file();
    void on_remove_row();
    void on_convert();

    Gtk::TreeModel::Row add_chapter();

    Glib::RefPtr<Gtk::Builder> glade;

    Gtk::TreeView* treeView;
    Glib::RefPtr<ConverterTreeStore> treeModel;
};