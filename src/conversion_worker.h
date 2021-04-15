#ifndef CONVERSION_WORKER
#define CONVERSION_WORKER

#include <thread>
#include <mutex>

#include <gtkmm.h>

#include "conversion_data.h"
#include "conversion_dialog.h"

class ConversionWorker {
  public:
    ConversionWorker(ConversionData* convert_data);

    void do_work(ConversionDialog* caller);

    void get_data(double* frac_done, Glib::ustring* msg) const;
    void stop_work();
    bool get_has_stopped() const;

  private:
    mutable std::mutex mutex;

    bool will_stop;
    bool has_stopped;
    double fraction_done;
    Glib::ustring message;

    ConversionData* conversion_data;
};

#endif