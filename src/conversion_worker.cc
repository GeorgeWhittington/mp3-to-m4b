#include <mutex>

#include <gtkmm.h>

#include "conversion_dialog.h"
#include "conversion_worker.h"

ConversionWorker::ConversionWorker(ConversionData* convert_data)
: mutex(),
  will_stop(false),
  has_stopped(false),
  fraction_done(0.0),
  message(),
  conversion_data(convert_data)
{
}

void ConversionWorker::get_data(double* frac_done, Glib::ustring* msg) const {
  std::lock_guard<std::mutex> lock(mutex);

  if (frac_done)
    *frac_done = fraction_done;

  if (msg)
    *msg = message;
}

void ConversionWorker::stop_work() {
  std::lock_guard<std::mutex> lock(mutex);
  will_stop = true;
}

bool ConversionWorker::get_has_stopped() const {
  std::lock_guard<std::mutex> lock(mutex);
  return has_stopped;
}

void ConversionWorker::do_work(ConversionDialog* caller) {
  {
    std::lock_guard<std::mutex> lock(mutex);
    has_stopped = false;
    fraction_done = 0.0;
    message = "";
  }

  // for each section of work
    // Do section of work
    // Attain lock
      // update fraction_done and message
      // check if should stop
        // break if should
    // caller->notify();

  // Attain lock, has_stopped = true
  // caller->notify();
}