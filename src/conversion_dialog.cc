#include <iostream>
#include <memory>

#include <boost/filesystem.hpp>
#include <gtkmm.h>

#include "conversion_dialog.h"
#include "conversion_worker.h"

ConversionDialog::ConversionDialog(
  BaseObjectType* c_object,
  const Glib::RefPtr<Gtk::Builder>& ref_glade,
  std::string bin_path
)
: Gtk::Dialog(c_object),
  working(false),
  conversion_mutex(),
  worker_thread(nullptr),
  worker(nullptr),
  glade(ref_glade),
  progress_bar(nullptr),
  text_view(nullptr),
  cancel_button(nullptr),
  bin_path(bin_path)
{
  glade->get_widget("conversion_progress_bar", progress_bar);
  glade->get_widget("conversion_textview", text_view);
  glade->get_widget("conversion_cancel_button", cancel_button);

  cancel_button->signal_clicked().connect(
    sigc::mem_fun(*this, &ConversionDialog::on_cancel_conversion) );

  this->signal_delete_event().connect(
    sigc::mem_fun(*this, &ConversionDialog::on_quit) );
  
  dispatcher.connect(
    sigc::mem_fun(*this, &ConversionDialog::on_notification) );
}

ConversionDialog::~ConversionDialog() {
}

void ConversionDialog::reset_dialog(std::shared_ptr<ConversionWorker> conversion_worker) {
  worker = conversion_worker;
  
  progress_bar->set_fraction(0.0);

  auto buffer = text_view->get_buffer();
  buffer->set_text("");

  cancel_button->set_sensitive(true);
}

void ConversionDialog::begin_conversion() {
  if (worker_thread && worker_thread->joinable())
    worker_thread->join();

  worker_thread = new std::thread(
    [this] { worker->do_work(this); });  // lambda
}

void ConversionDialog::on_cancel_conversion() {
  cancel_button->set_sensitive(false);
  if (worker_thread) {
    {
      std::lock_guard<std::mutex> lock(conversion_mutex);
      worker->stop_work();
    }
  }
}

bool ConversionDialog::on_quit(GdkEventAny*) {
  cancel_button->set_sensitive(false);
  if (worker_thread) {
    {
      std::lock_guard<std::mutex> lock(conversion_mutex);
      worker->stop_work();
    }
  }
  return true;  // Don't propogate
}

void ConversionDialog::update_widgets() {
  double fraction_done;
  Glib::ustring message_from_worker;
  {
    std::lock_guard<std::mutex> lock(conversion_mutex);
    worker->get_data(&fraction_done, &message_from_worker);
  }

  progress_bar->set_fraction(fraction_done);

  if (message_from_worker != text_view->get_buffer()->get_text()) {
    auto buffer = text_view->get_buffer();
    buffer->set_text(message_from_worker);
  }
}

void ConversionDialog::notify() {
  dispatcher.emit();
}

void ConversionDialog::on_notification() {
  bool has_stopped;
  {
    std::lock_guard<std::mutex> lock(conversion_mutex);
    has_stopped = worker->get_has_stopped();
  }

  if (worker_thread && has_stopped) {
    if (worker_thread->joinable())
      worker_thread->join();

    bool completed_work;
    bool encountered_error;
    {
      std::lock_guard<std::mutex> lock(conversion_mutex);
      completed_work = worker->get_has_completed_work();
      encountered_error = worker->get_encountered_error();
    }

    // Thread has joined, check if it completed work
    if (completed_work) {
      response(Gtk::RESPONSE_OK);
    } else if (encountered_error) {
      response(Gtk::RESPONSE_NONE);
    } else {
      response(Gtk::RESPONSE_CANCEL);
    }

    cancel_button->set_sensitive(false);
    hide();
  }

  update_widgets();
}

// NOT thread safe, only use after thread is joined
boost::filesystem::path ConversionDialog::get_m4b_path() {
  return worker->get_m4b_path();
}

void ConversionDialog::attain_lock() {
  conversion_mutex.lock();
}

void ConversionDialog::release_lock() {
  conversion_mutex.unlock();
}