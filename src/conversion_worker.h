#ifndef CONVERSION_WORKER
#define CONVERSION_WORKER

#include <thread>
#include <memory>
#include <mutex>

#include <gtkmm.h>
#include <boost/filesystem.hpp>

#include "conversion_data.h"

// Forward declaration to break loop
class ConversionDialog;

class ConversionWorker {
  public:
    ConversionWorker(std::shared_ptr<ConversionData> convert_data);
    virtual ~ConversionWorker();

    void do_work(ConversionDialog* caller);

    void get_data(double* frac_done, Glib::ustring* msg) const;
    void stop_work();
    bool get_has_stopped() const;
    bool get_encountered_error() const;
    bool get_has_completed_work() const;
    boost::filesystem::path get_m4b_path();
    void clear_temp_files();

  private:
    void set_stop(ConversionDialog* caller, bool error);
    bool update_progress(ConversionDialog* caller, double frac_done, std::string msg);
    int get_ffmpeg_command(std::string* ffmpeg_command, std::string bin_path);
    int get_atomic_parsley_command(std::string* atomic_command, std::string bin_path);

    bool will_stop;
    bool has_stopped;
    double fraction_done;
    bool encountered_error;
    bool completed_work;
    Glib::ustring message;
    boost::filesystem::path temp_metadata_path;
    boost::filesystem::path temp_m4b_path;

    std::shared_ptr<ConversionData> conversion_data;
};

#endif