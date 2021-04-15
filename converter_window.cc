#include "converter_window.h"
#include <iostream>

// TODO: Fix all variables in camelcase! Just use underscores.

ConverterWindow::ConverterWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
: Gtk::ApplicationWindow(cobject),
  glade(refGlade),
  treeView(nullptr)
{
  // Set up actions
  add_action("add_chapter", sigc::mem_fun(*this, &ConverterWindow::on_add_chapter));
  add_action("add_file", sigc::mem_fun(*this, &ConverterWindow::on_add_file));
  add_action("remove_row", sigc::mem_fun(*this, &ConverterWindow::on_remove_row));
  add_action("convert", sigc::mem_fun(*this, &ConverterWindow::on_convert));

  // Set up tree model and view
  glade->get_widget("tree_view", treeView);

  treeModel = ConverterTreeStore::create();
  treeView->set_model(treeModel);
  treeView->set_reorderable();

  int title_index = treeView->append_column_editable("Chapter Title", treeModel->columns.title);
  treeView->append_column("File Name", treeModel->columns.file_name);
  treeView->append_column("Length", treeModel->columns.length);

  // ensure only rows that are chapters can have title edited
  title_index -= 1;
  Gtk::TreeViewColumn* title_column = treeView->get_column(title_index);
  Gtk::CellRenderer* title_cell_renderer = treeView->get_column_cell_renderer(title_index);
  title_column->add_attribute(*title_cell_renderer, "editable", treeModel->columns.chapter);

  show_all();
}

ConverterWindow::~ConverterWindow() {
}

void ConverterWindow::on_add_chapter() {
  add_chapter();
}

Gtk::TreeModel::Row ConverterWindow::add_chapter() {
  Gtk::TreeModel::Row row = *(treeModel->append());
  auto children = treeModel->children();
  row[treeModel->columns.chapter] = true;
  row[treeModel->columns.title] = "Chapter " + std::to_string(children.size());
  return row;
}

void ConverterWindow::on_add_file() {
  // filepicker, analyse mp3's for length

  auto selection = treeView->get_selection();
  auto row = selection->get_selected();

  // If chapter selected, add under
  if (row) {
    bool chapter = row->get_value(treeModel->columns.chapter);
    if (chapter) {
      Gtk::TreeModel::Row fileRow = *(treeModel->append(row->children()));
      fileRow[treeModel->columns.chapter] = false;
      fileRow[treeModel->columns.file_name] = "chapter_one.mp3";
      fileRow[treeModel->columns.length] = 130;
      return;
    }
  }

  // Otherwise create and add under
  auto chapter = add_chapter();
  Gtk::TreeModel::Row fileRow = *(treeModel->append(chapter.children()));
  fileRow[treeModel->columns.chapter] = false;
  fileRow[treeModel->columns.file_name] = "chapter_one.mp3";
  fileRow[treeModel->columns.length] = 130;
}

void ConverterWindow::on_remove_row() {
  auto selection = treeView->get_selection();
  auto row = selection->get_selected();
  if (row) {
    treeModel->erase(row);
  } 
}

void ConverterWindow::on_convert() {
  auto chapters = treeModel->children();

  for (uint i = 0; i < chapters.size(); i++) {
    std::cout << i << ": " << chapters[i].get_value(treeModel->columns.chapter);
    std::cout << " " << chapters[i].get_value(treeModel->columns.title) << std::endl;

    auto files = chapters[i].children();

    for (uint j = 0; j < files.size(); j++) {
      std::cout << " " << j << ": " << files[j].get_value(treeModel->columns.chapter);
      std::cout << " " << files[j].get_value(treeModel->columns.file_name);
      std::cout << " " << files[j].get_value(treeModel->columns.length) << std::endl;
    }
  }
}