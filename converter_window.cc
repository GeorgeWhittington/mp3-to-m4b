#include "converter_window.h"
#include "get_duration.h"

ConverterWindow::ConverterWindow(BaseObjectType* c_object, const Glib::RefPtr<Gtk::Builder>& ref_glade)
: Gtk::ApplicationWindow(c_object),
  glade(ref_glade),
  tree_view(nullptr),
  title_entry(nullptr),
  author_entry(nullptr),
  year_entry(nullptr),
  cover_image_button(nullptr),
  cover_image_display(nullptr),
  comment_text_view(nullptr)
{
  // fetch all widgets
  glade->get_widget("tree_view", tree_view);
  glade->get_widget("title_entry", title_entry);
  glade->get_widget("author_entry", author_entry);
  glade->get_widget("year_entry", year_entry);
  glade->get_widget("cover_image_button", cover_image_button);
  glade->get_widget("cover_image_display", cover_image_display);
  glade->get_widget("comment_text_view", comment_text_view);

  // bind image picker button
  cover_image_button->signal_clicked().connect(
    sigc::mem_fun(*this, &ConverterWindow::on_cover_image_button_clicked) );

  // Set up actions
  add_action("add_chapter", sigc::mem_fun(*this, &ConverterWindow::on_add_chapter));
  add_action("add_file", sigc::mem_fun(*this, &ConverterWindow::on_add_file));
  add_action("remove_row", sigc::mem_fun(*this, &ConverterWindow::on_remove_row));
  add_action("convert", sigc::mem_fun(*this, &ConverterWindow::on_convert));

  // Set up tree model and view
  tree_model = ConverterTreeStore::create();
  tree_view->set_model(tree_model);
  tree_view->set_reorderable();

  int title_index = tree_view->append_column_editable("Chapter Title", tree_model->columns.title);
  tree_view->append_column("File Name", tree_model->columns.file_name);
  int length_index = tree_view->append_column("Length", tree_model->columns.length);

  // ensure only rows that are chapters can have title edited
  title_index -= 1;
  Gtk::TreeViewColumn* title_column = tree_view->get_column(title_index);
  Gtk::CellRenderer* title_cell_renderer = tree_view->get_column_cell_renderer(title_index);
  title_column->add_attribute(*title_cell_renderer, "editable", tree_model->columns.chapter);

  // Ensure n.o. seconds in length renders as a time
  length_index -= 1;
  Gtk::TreeViewColumn* length_column = tree_view->get_column(length_index);
  Gtk::CellRenderer* length_cell_renderer = tree_view->get_column_cell_renderer(length_index);
  length_column->set_cell_data_func(
    *length_cell_renderer,
    sigc::mem_fun(*this, &ConverterWindow::on_set_cell_length) );

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
  std::string filename;
  int length;
  
  Gtk::FileChooserDialog dialog("Please choose an mp3", Gtk::FILE_CHOOSER_ACTION_OPEN);
  dialog.set_transient_for(*this);

  dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
  dialog.add_button("_Open", Gtk::RESPONSE_OK);

  auto filter_audio = Gtk::FileFilter::create();
  filter_audio->set_name("MP3 files");
  filter_audio->add_pattern("*.mp3");
  dialog.add_filter(filter_audio);

  int result = dialog.run();

  switch (result) {
    case (Gtk::RESPONSE_OK): {
      filename = dialog.get_filename();
      // std::cout << "File selected: " << filename << std::endl;
      const char *filename_c = filename.c_str();
      length = get_audio_duration(filename_c);
      // std::cout << "File duration (in seconds): " << length << std::endl;
      break;
    }
    default: {
      return;
    }
  }

  auto selection = tree_view->get_selection();
  auto row = selection->get_selected();

  // If chapter selected, add under
  if (row) {
    bool chapter = row->get_value(tree_model->columns.chapter);
    if (chapter) {

      Gtk::TreeModel::Row file_row = *(tree_model->append(row->children()));
      file_row[tree_model->columns.chapter] = false;
      file_row[tree_model->columns.file_name] = filename;
      file_row[tree_model->columns.length] = length;
      return;
    }
  }

  // Otherwise create and add under
  auto chapter = add_chapter();
  Gtk::TreeModel::Row file_row = *(tree_model->append(chapter.children()));
  file_row[tree_model->columns.chapter] = false;
  file_row[tree_model->columns.file_name] = filename;
  file_row[tree_model->columns.length] = length;
}

void ConverterWindow::on_remove_row() {
  auto selection = tree_view->get_selection();
  auto row = selection->get_selected();
  if (row) {
    tree_model->erase(row);
  } 
}

void ConverterWindow::on_convert() {
  std::cout << "--- Metadata ---" << std::endl;
  std::cout << "Title: " << title_entry->get_text() << std::endl;
  std::cout << "Author: " << author_entry->get_text() << std::endl;
  std::cout << "Year: " << year_entry->get_text() << std::endl;
  std::cout << "Cover Image: " << cover_image_path << std::endl;
  std::cout << "Comment: " << comment_text_view->get_buffer()->get_text() << std::endl;

  std::cout << "--- Chapters ---" << std::endl;
  auto chapters = tree_model->children();

  for (uint i = 0; i < chapters.size(); i++) {
    std::cout << i << ": " << chapters[i].get_value(tree_model->columns.title) << std::endl;

    auto files = chapters[i].children();

    for (uint j = 0; j < files.size(); j++) {
      std::cout << "    " << j << ": " << files[j].get_value(tree_model->columns.file_name);
      std::cout << " " << files[j].get_value(tree_model->columns.length) << std::endl;
    }
  }
}

void ConverterWindow::on_set_cell_length(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter) {
  if (iter) {
    Glib::ustring view_text;
    int seconds, hours, minutes;
    seconds = iter->get_value(tree_model->columns.length);

    if (seconds == -1) {
      view_text = "N/A";
    } else {
      minutes = seconds / 60;
      hours = minutes / 60;

      // TODO: could stand to store ms or ns instead of s

      // max int is 2147483647, so max time is 596523:14:07
      char buffer[15];
      sprintf(buffer, "%02d:%02d:%02d",
            hours, int(minutes % 60), int(seconds % 60));
      view_text = buffer;
    }

    Gtk::CellRendererText* text_renderer = static_cast<Gtk::CellRendererText*>(renderer);
    text_renderer->property_text() = view_text;
  }
}

bool hasEnding(std::string const &fullString, std::string const &ending) {
  if (fullString.length() >= ending.length()) {
    return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
  } else {
    return false;
  }
}

void ConverterWindow::on_cover_image_button_clicked() {
  std::string filename;
  bool animated;

  Gtk::FileChooserDialog dialog("Please choose a cover image", Gtk::FILE_CHOOSER_ACTION_OPEN);
  dialog.set_transient_for(*this);

  dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
  dialog.add_button("_Open", Gtk::RESPONSE_OK);

  auto filter_image = Gtk::FileFilter::create();
  filter_image->set_name("Images");
  filter_image->add_mime_type("image/jpeg");
  filter_image->add_mime_type("image/png");
  filter_image->add_mime_type("image/gif");
  dialog.add_filter(filter_image);

  int result = dialog.run();

  switch (result) {
    case (Gtk::RESPONSE_OK): {
      filename = dialog.get_filename();
      std::cout << "File selected: " << filename << std::endl;

      animated = hasEnding(filename, ".gif");
      break;
    }
    default: {
      return;
    }
  }

  if (animated) {
    Glib::RefPtr<Gdk::PixbufAnimation> pixbuf_animated;
    try {
      pixbuf_animated = Gdk::PixbufAnimation::create_from_file(filename);
    } catch (const Glib::FileError& ex) {
      std::cerr << "FileError: " << ex.what() << std::endl;
      return;
    }

    cover_image_display->set(pixbuf_animated);
  } else {
    Glib::RefPtr<Gdk::Pixbuf> pixbuf;
    try {
      pixbuf = Gdk::Pixbuf::create_from_file(filename);
    } catch (const Glib::FileError& ex) {
      std::cerr << "FileError: " << ex.what() << std::endl;
      return;
    } catch (const Gdk::PixbufError& ex) {
      std::cerr << "PixbufError: " << ex.what() << std::endl;
      return;
    }

    cover_image_display->set(pixbuf);
  }

  cover_image_path = filename;
}