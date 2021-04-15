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

    // Tree model columns
    class ModelColumns : public Gtk::TreeModel::ColumnRecord {
      public:
        ModelColumns() {
          add(col_title);
          add(col_file_name);
          add(col_length);
        }

        Gtk::TreeModelColumn<Glib::ustring> col_title;
        Gtk::TreeModelColumn<Glib::ustring> col_file_name;
        Gtk::TreeModelColumn<Glib::ustring> col_length;
    };

    ModelColumns columns;

    // widgets
    Gtk::TreeView* treeView;
    Glib::RefPtr<Gtk::TreeStore> treeModel;
};