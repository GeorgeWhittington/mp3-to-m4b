#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <memory>
#include <stdexcept>

#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/algorithm/string.hpp>
#include <gtkmm.h>

#include "conversion_worker.h"
#include "conversion_dialog.h"

using namespace boost::filesystem;

std::string esc_quotes(std::string path) {
  std::string path_copy = path;
  boost::replace_all(path_copy, "\"", "\\\"");
  return path_copy
}

ConversionWorker::ConversionWorker(std::shared_ptr<ConversionData> convert_data)
: will_stop(false),
  has_stopped(false),
  fraction_done(0.0),
  encountered_error(false),
  completed_work(false),
  message(),
  conversion_data(convert_data)
{
  // Name generation as close to creation as possible, want to reduce
  // possibility of race condition biting us (another process creating 
  // file with same name before we do).
  temp_metadata_path = temp_directory_path() / unique_path();
  std::ofstream metadata_file(temp_metadata_path.string());
  metadata_file.close();

  temp_m4b_path = temp_directory_path() / unique_path("%%%%-%%%%-%%%%-%%%%.m4b");
  std::ofstream m4b_file(temp_m4b_path.string());
  m4b_file.close();
}

ConversionWorker::~ConversionWorker() {
  // remove temp files if still there
  if (exists(temp_metadata_path)) {
    std::remove(temp_metadata_path.c_str());
  }

  if (exists(temp_m4b_path)) {
    std::remove(temp_m4b_path.c_str());
  }
}

void ConversionWorker::get_data(double* frac_done, Glib::ustring* msg) const {
  if (frac_done)
    *frac_done = fraction_done;

  if (msg)
    *msg = message;
}

void ConversionWorker::stop_work() {
  will_stop = true;
}

bool ConversionWorker::get_has_stopped() const {
  return has_stopped;
}

bool ConversionWorker::get_encountered_error() const {
  return encountered_error;
}

bool ConversionWorker::get_has_completed_work() const {
  return completed_work;
}

// NOT thread safe, only use after thread is joined
path ConversionWorker::get_m4b_path() {
  return temp_m4b_path;
}

void ConversionWorker::do_work(ConversionDialog* caller) {
  double steps = 4.0;

  bool stop = update_progress(caller, 0.0 / steps, "Finding chapter lengths...");
  if (stop) {
    set_stop(caller, false);
    return;
  }

  std::vector<std::string> chapter_metadata;
  int err = conversion_data->get_chapter_metadata(&chapter_metadata);
  if (err != 0) {
    set_stop(caller, true);
    return;
  }

  stop = update_progress(caller, 1.0 / steps, "Creating metadata temp file...");
  if (stop) {
    set_stop(caller, false);
    return;
  }

  std::ofstream temp_file(temp_metadata_path.string());
  if (!temp_file.good()) {
    std::cerr << "Metadata file not good to write to" << std::endl;
    set_stop(caller, true);
    return;
  }
  for (const auto &line : chapter_metadata)
    temp_file << line << std::endl;
  temp_file.close();

  stop = update_progress(caller, 2.0 / steps, "Running ffmpeg command...");
  if (stop) {
    set_stop(caller, false);
    return;
  }

  std::string ffmpeg_command;
  err = get_ffmpeg_command(&ffmpeg_command);
  if (err != 0) {
    set_stop(caller, true);
    return;
  }

  std::cout << ffmpeg_command << std::endl;

  std::error_code ec;
  boost::process::system(ffmpeg_command, ec);
  if (ec) {
    std::cerr << "Error running ffmpeg command: " << ec.message() << std::endl;
    set_stop(caller, true);
    return;
  }

  bool atomic_p = conversion_data->year != "" ||
                  conversion_data->author != "" ||
                  conversion_data->cover_image_path != "";

  if (atomic_p) {
    stop = update_progress(caller, 3.0 / steps, "Running AtomicParsley command...");
    if (stop) {
      set_stop(caller, false);
      return;
    }

    std::string atomic_command;
    err = get_atomic_parsley_command(&atomic_command);
    if (err != 0) {
      set_stop(caller, true);
      return;
    }

    std::cout << atomic_command << std::endl;

    boost::process::system(atomic_command, ec);
    if (ec) {
      std::cerr << "Error running AtomicParsley command" << std::endl;
      set_stop(caller, true);
      return;
    }
  } else {
    update_progress(caller, 3.0 / steps, "Skipping AtomicParsley command...");
  }

  caller->attain_lock();
  fraction_done = 4.0 / steps;
  message += "Finished.\n";
  completed_work = true;
  will_stop = false;
  has_stopped = true;
  caller->release_lock();

  caller->notify();

  return;
}

// set variables to prepare for ending the thread
void ConversionWorker::set_stop(ConversionDialog* caller, bool error) {
  caller->attain_lock();
  if (error)
    encountered_error = true;
  will_stop = false;
  has_stopped = true;
  caller->release_lock();
}

// Update progress and return will_stop value
bool ConversionWorker::update_progress(ConversionDialog* caller, double frac_done, std::string msg) {
  caller->attain_lock();
  fraction_done = frac_done;
  message += msg + "\n";
  bool _will_stop = will_stop;
  caller->release_lock();

  caller->notify();

  return _will_stop;
}

int ConversionWorker::get_ffmpeg_command(std::string* ffmpeg_command) {
  std::vector<std::string> all_files;
  for (const auto &chapter : conversion_data->chapters)
    all_files.insert(all_files.end(), chapter.file_names.begin(), chapter.file_names.end());

  std::string inputs;
  for (const auto &file_name : all_files)
    inputs += "-i \"" + esc_quotes(file_name) + "\" ";

  std::string filter;
  for (size_t i = 0; i < all_files.size(); i++) {
    filter += "[" + std::to_string(i + 1) + ":a:0]";
  }
  filter += "concat=n=" + std::to_string(all_files.size()) + ":a=1:v=0[outa]";

  std::stringstream command;

  command << "/usr/local/bin/ffmpeg -i " << temp_metadata_path.string()
          << " " << inputs << " "
          << "-map_metadata 0 "
          << "-filter_complex \"" << filter << "\" "
          << "-map \"[outa]\" "
          << "-ac 2 "
          << "-y "
          << "-codec:a aac "
          << "-b:a 64k "
          << temp_m4b_path.string();

  *ffmpeg_command = command.str();
  
  return 0;
}

int ConversionWorker::get_atomic_parsley_command(std::string* atomic_command) {
  std::stringstream command;

  command << "/usr/local/bin/AtomicParsley " << temp_m4b_path.string() << " "
          << "--overWrite ";
  
  if (conversion_data->cover_image_path != "")
    command << "--artwork \"" << esc_quotes(conversion_data->cover_image_path) << "\" "
  
  if (conversion_data->author != "")
    command << "--artist \"" << esc_quotes(conversion_data->author) << "\" "

  if (conversion_data->year != "") {
    try {
      double check_valid_number = std::stod(conversion_data->year, nullptr);
    } catch (const std::invalid_argument& ia) {
      std::cerr << "Invalid year value" << std::endl;
      return 1;
    }

    command << "--year \"" << conversion_data->year << "\""
  }

  *atomic_command = command.str();

  return 0;
}

