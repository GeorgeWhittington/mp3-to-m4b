#include "conversion_dialog.h"
#include "conversion_data.h"

ConversionDialog::ConversionDialog(
  BaseObjectType* c_object,
  const Glib::RefPtr<Gtk::Builder>& ref_glade,
  ConversionData* conversion_data
)
: Gtk::Dialog(c_object),
  glade(ref_glade),
  progress_bar(nullptr),
  text_view(nullptr),
  cancel_button(nullptr),
  worker(conversion_data)
{
  glade->get_widget("conversion_progress_bar", progress_bar);
  glade->get_widget("conversion_textview", text_view);
  glade->get_widget("conversion_cancel_button", cancel_button);

  cancel_button->signal_clicked().connect(
    sigc::mem_fun(*this, &ConversionDialog::on_cancel_conversion) );
  
  dispatcher.connect(
    sigc::mem_fun(*this, &ConversionDialog::on_notification) );
  
  show_all_children();
  begin_conversion();
}

void ConversionDialog::begin_conversion() {
  if (worker_thread) {
    std::cerr << "Worker thread already in use" << std::endl;
  } else {
    worker_thread = new std::thread(
      [this]
      {
        worker.do_work(this);
      });
  }
}

void ConversionDialog::on_cancel_conversion() {
  if (!worker_thread) {
    std::cerr << "Can't stop a worker thread, none are running." << std::endl;
  } else {
    worker.stop_work();
    cancel_button->set_sensitive(false);
  }
}

void ConversionDialog::on_quit() {
  if (worker_thread) {
    worker.stop_work();
    if (worker_thread->joinable())
      worker_thread->join();  // blocks until worker is finished
  }
  hide();
}

void ConversionDialog::update_widgets() {
  double fraction_done;
  Glib::ustring message_from_worker;
  worker.get_data(&fraction_done, &message_from_worker);

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
  if (worker_thread && worker.get_has_stopped()) {
    if (worker_thread->joinable())
      worker_thread->join();  // blocks until worker is finished
    delete worker_thread;
    worker_thread = nullptr;
  }

  update_widgets();
}