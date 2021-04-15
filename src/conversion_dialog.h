#ifndef CONVERSION_DIALOG
#define CONVERSION_DIALOG

#include <iostream>

#include <gtkmm.h>

#include "conversion_worker.h"
#include "conversion_data.h"

class ConversionDialog : public Gtk::Dialog {
  public:
    ConversionDialog(
      BaseObjectType* c_object,
      const Glib::RefPtr<Gtk::Builder>& ref_glade,
      ConversionData* conversion_data
    );
    virtual ~ConversionDialog();

    // Called from worker thread
    void notify();
  
  protected:
    void begin_conversion();
    void on_cancel_conversion();
    void on_notification();
    void update_widgets();
    void on_quit();

    Glib::RefPtr<Gtk::Builder> glade;

    Gtk::ProgressBar* progress_bar;
    Gtk::TextView* text_view;
    Gtk::Button* cancel_button;

    Glib::Dispatcher dispatcher;
    ConversionWorker worker;
    std::thread* worker_thread;
};

#endif