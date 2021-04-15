#ifndef CONVERTER_WINDOW
#define CONVERTER_WINDOW

#include <gtkmm.h>

#include "converter_treestore.h"
#include "conversion_dialog.h"

class ConverterWindow : public Gtk::ApplicationWindow {
  public:
    ConverterWindow(
      BaseObjectType* c_object,
      const Glib::RefPtr<Gtk::Builder>& ref_glade,
      std::string bin_path
    );
    virtual ~ConverterWindow();
  
  protected:
    // Signal handlers
    void on_add_chapter();
    void on_add_file();
    void on_move_up();
    void on_move_down();
    void on_remove_row();
    void on_convert();
    void on_about();
    void on_set_cell_length(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);
    void on_cover_image_button_clicked();

    Gtk::TreeModel::Row add_chapter();

    Glib::RefPtr<Gtk::Builder> glade;

    Gtk::Entry* title_entry;
    Gtk::Entry* author_entry;
    Gtk::Entry* year_entry;
    Gtk::Button* cover_image_button;
    Gtk::Image* cover_image_display;
    Gtk::TextView* comment_text_view;

    Gtk::TreeView* tree_view;
    Glib::RefPtr<ConverterTreeStore> tree_model;

    ConversionDialog* conversion_dialog;
    std::string cover_image_path;
};

#endif