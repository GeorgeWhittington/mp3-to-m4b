#include "converter_window.h"
#include "get_duration.h"

typedef unsigned int uint;

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
  add_action("add-chapter", sigc::mem_fun(*this, &ConverterWindow::on_add_chapter));
  add_action("add-file", sigc::mem_fun(*this, &ConverterWindow::on_add_file));
  add_action("move-up", sigc::mem_fun(*this, &ConverterWindow::on_move_up));
  add_action("move-down", sigc::mem_fun(*this, &ConverterWindow::on_move_down));
  add_action("remove-row", sigc::mem_fun(*this, &ConverterWindow::on_remove_row));
  add_action("convert", sigc::mem_fun(*this, &ConverterWindow::on_convert));
  add_action("about", sigc::mem_fun(*this, &ConverterWindow::on_about));

  // Set up tree model and view
  tree_model = ConverterTreeStore::create();
  tree_view->set_model(tree_model);
  tree_view->set_reorderable();

  int title_index = tree_view->append_column_editable("Chapter Title", tree_model->columns.title);
  int filename_index = tree_view->append_column("File Name", tree_model->columns.file_name);
  int length_index = tree_view->append_column("Length", tree_model->columns.length);

  // ensure only rows that are chapters can have title edited
  title_index -= 1;
  Gtk::TreeViewColumn* title_column = tree_view->get_column(title_index);
  Gtk::CellRenderer* title_cell_renderer = tree_view->get_column_cell_renderer(title_index);
  title_column->add_attribute(*title_cell_renderer, "editable", tree_model->columns.chapter);

  // Adjust chapter title text wrapping
  title_column->set_sizing(Gtk::TreeViewColumnSizing::TREE_VIEW_COLUMN_AUTOSIZE);
  Gtk::CellRendererText* title_cell_renderer_text = dynamic_cast<Gtk::CellRendererText*>(title_cell_renderer);
  title_cell_renderer_text->property_wrap_mode().set_value(Pango::WRAP_WORD_CHAR);
  title_cell_renderer_text->property_wrap_width().set_value(240);

  // Adjust file name text wrapping
  filename_index -= 1;
  Gtk::TreeViewColumn* filename_column = tree_view->get_column(filename_index);
  filename_column->set_sizing(Gtk::TreeViewColumnSizing::TREE_VIEW_COLUMN_AUTOSIZE);
  Gtk::CellRendererText* filename_cell_renderer = dynamic_cast<Gtk::CellRendererText*>(
    tree_view->get_column_cell_renderer(filename_index) );
  filename_cell_renderer->property_wrap_mode().set_value(Pango::WRAP_WORD_CHAR);
  filename_cell_renderer->property_wrap_width().set_value(240);

  // Ensure n.o. microseconds in length renders as a time
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
  std::string filename;
  long long int length;
  
  Glib::RefPtr<Gtk::FileChooserNative> dialog = Gtk::FileChooserNative::create(
    "Please choose an mp3", *this,
    Gtk::FILE_CHOOSER_ACTION_OPEN, "", "");

  auto filter_audio = Gtk::FileFilter::create();
  filter_audio->set_name("MP3 files");
  filter_audio->add_pattern("*.mp3"); // Can only pick files with mp3 extension
  dialog->add_filter(filter_audio);

  int result = dialog->run();

  switch (result) {
    case (Gtk::ResponseType::RESPONSE_ACCEPT): {
      filename = dialog->get_filename();
      length = get_audio_duration(filename);
      break;
    }
    default: {
      return; // Bad response from filepicker dialog, stop
    }
  }

  auto selection = tree_view->get_selection();
  auto row = selection->get_selected();

  // If chapter selected, add the file under it
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

  // Otherwise create a chapter to add under
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
    int microseconds, seconds, minutes, hours;
    // Length is in microseconds
    long long int length = iter->get_value(tree_model->columns.length);

    if (length == -1) {
      view_text = "N/A";
    } else {
      seconds = length / 1000000;
      microseconds = length % 1000000;
      minutes = seconds / 60;
      seconds %= 60;
      hours = minutes / 60;
      minutes %= 60;

      // max long long int is 9,223,372,036,854,775,807
      // so max time is 2562047788:00:54.775807
      char buffer[25];
      sprintf(buffer, "%02d:%02d:%02d.%02d", hours, minutes, seconds,
              (100 * microseconds) / 1000000);
      view_text = buffer;
    }

    Gtk::CellRendererText* text_renderer = static_cast<Gtk::CellRendererText*>(renderer);
    text_renderer->property_text() = view_text;
  }
}

// https://stackoverflow.com/a/874160
bool has_ending(std::string const &fullString, std::string const &ending) {
  if (fullString.length() >= ending.length()) {
    return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
  } else {
    return false;
  }
}

void ConverterWindow::on_cover_image_button_clicked() {
  std::string filename;
  bool animated;

  Glib::RefPtr<Gtk::FileChooserNative> dialog = Gtk::FileChooserNative::create(
    "Please choose a cover image", *this,
    Gtk::FILE_CHOOSER_ACTION_OPEN, "", "");

  // Only allow picking jpeg, png or gif images
  auto filter_image = Gtk::FileFilter::create();
  filter_image->set_name("Images");
  filter_image->add_pattern("*.jpeg");
  filter_image->add_pattern("*.jpg");
  filter_image->add_pattern("*.png");
  filter_image->add_pattern("*.gif");
  dialog->add_filter(filter_image);

  int result = dialog->run();

  switch (result) {
    case (Gtk::RESPONSE_ACCEPT): {
      filename = dialog->get_filename();
      animated = has_ending(filename, ".gif");
      break;
    }
    default: {
      return; // Bad response from filepicker dialog, stop
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

      int width = pixbuf->get_width();
      if (width > 500) {
        std::cout << "scaling down image" << std::endl;
        // Scale to fit inside 500x500 dimensions, while preserving aspect ratio
        pixbuf = Gdk::Pixbuf::create_from_file(filename, 500, 500);
      }

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

void ConverterWindow::on_about() {
  Gtk::AboutDialog dialog;
  dialog.set_transient_for(*this);

  dialog.set_program_name("MP3 to M4B Converter");
  dialog.set_version("v1.0");
  dialog.set_copyright("Copyright 2021, George Whittington");
  dialog.set_license_type(Gtk::License::LICENSE_GPL_3_0);

  Glib::RefPtr<Gdk::Pixbuf> logo = Gdk::Pixbuf::create_from_resource("/org/george/mp3_to_m4b/logo.svg", 200, 200);
  dialog.set_logo(logo);

  std::vector<Glib::ustring> list_authors;
  list_authors.push_back("George Whittington");
  dialog.set_authors(list_authors);

  std::vector<Glib::ustring> list_logo_artists;
  list_logo_artists.push_back("<a href='https://www.freepik.com' title='Freepik'>Freepik</a> from <a href='https://www.flaticon.com/' title='Flaticon'>www.flaticon.com</a>");
  dialog.add_credit_section("Logo creator", list_logo_artists);

  dialog.run();
}

// TODO: Implement these two

void ConverterWindow::on_move_up() {
  auto selection = tree_view->get_selection();
  auto row = selection->get_selected();
  if (row) {
  }
}

void ConverterWindow::on_move_down() {
  auto selection = tree_view->get_selection();
  auto row = selection->get_selected();
  if (row) {
  }
}
