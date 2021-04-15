#include <gtkmm.h>

class ConverterTreeStore : public Gtk::TreeStore {
  protected:
    ConverterTreeStore();

  public:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord {
      public:
        ModelColumns() {
          add(chapter);
          add(title);
          add(file_name);
          add(length);
        }

        // internal
        Gtk::TreeModelColumn<bool> chapter;

        // displayed
        Gtk::TreeModelColumn<Glib::ustring> title;
        Gtk::TreeModelColumn<Glib::ustring> file_name;
        Gtk::TreeModelColumn<int> length;  // in seconds
    };

    ModelColumns columns;

    static Glib::RefPtr<ConverterTreeStore> create();
  
  protected:
    // overriden function
    bool row_drop_possible_vfunc(
      const Gtk::TreeModel::Path& dest,
      const Gtk::SelectionData& selection_data) const override;
};