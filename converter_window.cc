#include "converter_window.h"
#include <iostream>

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

  treeModel = Gtk::TreeStore::create(columns);
  treeView->set_model(treeModel);
  treeView->set_reorderable();

  treeView->append_column_editable("Chapter Title", columns.col_title);
  treeView->append_column("File Name", columns.col_file_name);
  treeView->append_column("Length", columns.col_length);

  show_all();
}

ConverterWindow::~ConverterWindow() {
}

// Implement custom tree model to enforce chapter > file heirarchy and content

void ConverterWindow::on_add_chapter() {
  add_chapter();
}

Gtk::TreeModel::Row ConverterWindow::add_chapter() {
  Gtk::TreeModel::Row row = *(treeModel->append());
  auto children = treeModel->children();
  row[columns.col_title] = "Chapter " + std::to_string(children.size());
  return row;
}

void ConverterWindow::on_add_file() {
  // filepicker, analyse mp3's for length

  auto selection = treeView->get_selection();
  auto row = selection->get_selected();

  // If chapter selected, add under
  if (row) {
    auto chapter_title = row->get_value(columns.col_title);
    if (!chapter_title.empty()) {
      Gtk::TreeModel::Row fileRow = *(treeModel->append(row->children()));
      fileRow[columns.col_file_name] = "chapter_one.mp3";
      fileRow[columns.col_length] = "130";
      return;
    }
  }

  // Otherwise create and add under
  auto chapter = add_chapter();
  Gtk::TreeModel::Row fileRow = *(treeModel->append(chapter.children()));
  fileRow[columns.col_file_name] = "chapter_one.mp3";
  fileRow[columns.col_length] = "130";
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
    std::cout << i << ": " << chapters[i].get_value(columns.col_title) << std::endl;

    auto files = chapters[i].children();

    for (uint j = 0; j < files.size(); j++) {
      std::cout << " " << j << ": " << files[j].get_value(columns.col_file_name);
      std::cout << " " << files[j].get_value(columns.col_length) << std::endl;
    }
  }
}