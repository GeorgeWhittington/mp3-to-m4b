#ifndef CONVERSION_DIALOG
#define CONVERSION_DIALOG

#include <iostream>
#include <memory>
#include <mutex>

#include <boost/filesystem.hpp>
#include <gtkmm.h>

// Forward declaration to break loop
class ConversionWorker;

class ConversionDialog : public Gtk::Dialog {
  public:
    ConversionDialog(
      BaseObjectType* c_object,
      const Glib::RefPtr<Gtk::Builder>& ref_glade
    );
    virtual ~ConversionDialog();

    void reset_dialog(std::shared_ptr<ConversionWorker> conversion_worker);
    void begin_conversion();
    boost::filesystem::path get_m4b_path();

    // Called from worker thread
    void notify();
    void attain_lock();
    void release_lock();

    bool working;
    mutable std::mutex conversion_mutex;
    std::thread* worker_thread;
    std::shared_ptr<ConversionWorker> worker;
  
  protected:
    void on_cancel_conversion();
    void on_notification();
    void update_widgets();
    bool on_quit(GdkEventAny*);

    Glib::RefPtr<Gtk::Builder> glade;

    Gtk::ProgressBar* progress_bar;
    Gtk::TextView* text_view;
    Gtk::Button* cancel_button;

    Glib::Dispatcher dispatcher;
};

#endif