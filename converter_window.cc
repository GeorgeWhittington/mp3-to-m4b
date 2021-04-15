#include "converter_window.h"
#include <iostream>

// TODO: Fix all variables in camelcase! Just use underscores.

ConverterWindow::ConverterWindow(BaseObjectType* c_object, const Glib::RefPtr<Gtk::Builder>& ref_glade)
: Gtk::ApplicationWindow(c_object),
  glade(ref_glade),
  tree_view(nullptr)
{
  // Set up actions
  add_action("add_chapter", sigc::mem_fun(*this, &ConverterWindow::on_add_chapter));
  add_action("add_file", sigc::mem_fun(*this, &ConverterWindow::on_add_file));
  add_action("remove_row", sigc::mem_fun(*this, &ConverterWindow::on_remove_row));
  add_action("convert", sigc::mem_fun(*this, &ConverterWindow::on_convert));

  // Set up tree model and view
  glade->get_widget("tree_view", tree_view);

  tree_model = ConverterTreeStore::create();
  tree_view->set_model(tree_model);
  tree_view->set_reorderable();

  int title_index = tree_view->append_column_editable("Chapter Title", tree_model->columns.title);
  tree_view->append_column("File Name", tree_model->columns.file_name);
  tree_view->append_column("Length", tree_model->columns.length);

  // ensure only rows that are chapters can have title edited
  title_index -= 1;
  Gtk::TreeViewColumn* title_column = tree_view->get_column(title_index);
  Gtk::CellRenderer* title_cell_renderer = tree_view->get_column_cell_renderer(title_index);
  title_column->add_attribute(*title_cell_renderer, "editable", tree_model->columns.chapter);

  show_all();
}

ConverterWindow::~ConverterWindow() {
}

void ConverterWindow::on_add_chapter() {
  add_chapter();
}

Gtk::TreeModel::Row ConverterWindow::add_chapter() {
  Gtk::TreeModel::Row row = *(tree_model->append());
  auto children = tree_model->children();
  row[tree_model->columns.chapter] = true;
  row[tree_model->columns.title] = "Chapter " + std::to_string(children.size());
  return row;
}

void ConverterWindow::on_add_file() {
  // filepicker, analyse mp3's for length

  auto selection = tree_view->get_selection();
  auto row = selection->get_selected();

  // If chapter selected, add under
  if (row) {
    bool chapter = row->get_value(tree_model->columns.chapter);
    if (chapter) {
      Gtk::TreeModel::Row file_row = *(tree_model->append(row->children()));
      file_row[tree_model->columns.chapter] = false;
      file_row[tree_model->columns.file_name] = "chapter_one.mp3";
      file_row[tree_model->columns.length] = 130;
      return;
    }
  }

  // Otherwise create and add under
  auto chapter = add_chapter();
  Gtk::TreeModel::Row file_row = *(tree_model->append(chapter.children()));
  file_row[tree_model->columns.chapter] = false;
  file_row[tree_model->columns.file_name] = "chapter_one.mp3";
  file_row[tree_model->columns.length] = 130;
}

void ConverterWindow::on_remove_row() {
  auto selection = tree_view->get_selection();
  auto row = selection->get_selected();
  if (row) {
    tree_model->erase(row);
  } 
}

void ConverterWindow::on_convert() {
  auto chapters = tree_model->children();

  for (uint i = 0; i < chapters.size(); i++) {
    std::cout << i << ": " << chapters[i].get_value(tree_model->columns.chapter);
    std::cout << " " << chapters[i].get_value(tree_model->columns.title) << std::endl;

    auto files = chapters[i].children();

    for (uint j = 0; j < files.size(); j++) {
      std::cout << " " << j << ": " << files[j].get_value(tree_model->columns.chapter);
      std::cout << " " << files[j].get_value(tree_model->columns.file_name);
      std::cout << " " << files[j].get_value(tree_model->columns.length) << std::endl;
    }
  }
}