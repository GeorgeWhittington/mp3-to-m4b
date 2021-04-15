#include "converter_treestore.h"
#include <iostream>

ConverterTreeStore::ConverterTreeStore() {
  set_column_types(columns);

  // bind signals to keep chapter length up to date
  signal_row_changed().connect(sigc::mem_fun(*this, &ConverterTreeStore::on_row_changed_custom));
  signal_row_deleted().connect(sigc::mem_fun(*this, &ConverterTreeStore::on_row_deleted_custom));
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

int ConverterTreeStore::get_total_length(const Gtk::TreeModel::iterator& parent) {
  int total_duration = 0;

  typedef Gtk::TreeModel::Children type_children;
  type_children children = parent->children();
  for (type_children::iterator iter = children.begin(); iter != children.end(); ++iter) {
    Gtk::TreeModel::Row child = *iter;
    total_duration += child.get_value(columns.length);
  }

  return total_duration;
}

void ConverterTreeStore::on_row_changed_custom(const Gtk::TreeModel::Path& path, const Gtk::TreeModel::iterator& iter) {
  // chapters changing doesn't affect chapter length
  bool is_chapter = iter->get_value(columns.chapter);
  if (is_chapter) {
    return;
  }

  Gtk::TreeModel::Path parent_path = path;
  parent_path.up();

  Gtk::TreeModel::iterator parent = get_iter(parent_path);
  parent->set_value(columns.length, get_total_length(parent));
}

void ConverterTreeStore::on_row_deleted_custom(const Gtk::TreeModel::Path& path) {
  Gtk::TreeModel::Path parent_path = path; // Copy path
  bool path_not_top_level = parent_path.up(); // Move path up one level
  bool path_top_level = !path_not_top_level || parent_path.empty();

  // if a chapter is deleted, no updates are needed
  if (path_top_level) {
    return;
  }

  // not using the const stuff the example code I used did, might break
  Gtk::TreeModel::iterator parent = get_iter(parent_path);
  parent->set_value(columns.length, get_total_length(parent));
}