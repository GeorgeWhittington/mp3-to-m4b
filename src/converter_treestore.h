#ifndef CONVERTER_TREESTORE
#define CONVERTER_TREESTORE

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
        Gtk::TreeModelColumn<long long int> length;  // in microseconds
    };

    ModelColumns columns;

    static Glib::RefPtr<ConverterTreeStore> create();
  
  protected:
    void on_row_changed_custom(
      const Gtk::TreeModel::Path& path,
      const Gtk::TreeModel::iterator& iter);
    void on_row_deleted_custom(const Gtk::TreeModel::Path& path);
    long long int get_total_length(const Gtk::TreeModel::iterator& parent);
    
    // overriden function
    bool row_drop_possible_vfunc(
      const Gtk::TreeModel::Path& dest,
      const Gtk::SelectionData& selection_data) const override;
};

#endif