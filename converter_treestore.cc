#include "converter_treestore.h"
#include <iostream>

ConverterTreeStore::ConverterTreeStore() {
  set_column_types(columns);
}

Glib::RefPtr<ConverterTreeStore> ConverterTreeStore::create() {
  return Glib::RefPtr<ConverterTreeStore>( new ConverterTreeStore() );
}

bool ConverterTreeStore::row_drop_possible_vfunc(const Gtk::TreeModel::Path& dest,
                                                 const Gtk::SelectionData& selection_data) const {
  // dest is the path the dragged row would have when dropped,
  // so comparisons need to be made against it's parent
  
  // get path of row being dragged
  auto ref_this = Glib::RefPtr<Gtk::TreeModel>(const_cast<ConverterTreeStore*>(this));
  ref_this->reference();
  Gtk::TreeModel::Path dragged_row;
  Gtk::TreeModel::Path::get_from_selection_data(selection_data, ref_this, dragged_row);
  
  // compare dragged row to dest parent
  if (dest && dragged_row) {
    auto unconst_this = const_cast<ConverterTreeStore*>(this);
    const_iterator iter_dragged = unconst_this->get_iter(dragged_row);
    bool dragged_is_chapter = iter_dragged->get_value(columns.chapter);

    Gtk::TreeModel::Path dest_parent = dest; // Copy path
    bool dest_not_top_level = dest_parent.up(); // Move path up one level
    bool dest_top_level = !dest_not_top_level || dest_parent.empty();

    if (dragged_is_chapter) {
      // chapters can be dragged to top level only
      return dest_top_level;
    } else {
      // files can be dragged to chapter only
      if (dest_top_level) {
        return false;
      }

      // get dest parent row
      const_iterator iter_dest_parent = unconst_this->get_iter(dest_parent);
      if (iter_dest_parent) {
        bool dest_is_chapter = iter_dest_parent->get_value(columns.chapter);
        return dest_is_chapter;
      }
    }
  }

  return Gtk::TreeStore::row_drop_possible_vfunc(dest, selection_data);
}

// handlers for row change and row delete to keep chapter length up to date