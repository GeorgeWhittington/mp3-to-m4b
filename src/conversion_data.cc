#include <string>
#include <sstream>
#include <vector>

#include <gtkmm.h>

#include "conversion_data.h"
#include "get_duration.h"

/* FFMPEG Metadata files have special characters '=', ';', '#', '\\' and '\n'.
Any existing in user inputted strings need to be escaped. */
const std::vector<char> illegal_chars = {'=', ';', '#', '\\', '\n'};

std::string esc_metadata(const std::string in_string) {
  std::stringstream out_stream;

  bool illegal_found = false;

  for (const char letter : in_string) {
    for (const char illegal_letter : illegal_chars) {
      if (letter == illegal_letter) {
        out_stream << '\\' << letter;
        illegal_found = true;
        break;
      }
    }
    if (!illegal_found)
      out_stream << letter;
    illegal_found = false;
  }

  return out_stream.str();
}

ChapterData::ChapterData(std::string title, std::vector<std::string> file_names) {
  this->title = title;
  this->file_names = file_names;
}

// returning less that 0 means error
double ChapterData::get_chapter_length() {
  double length = 0.0;

  for (auto file = file_names.begin(); file != file_names.end(); ++file) {
    double file_length = get_audio_duration_exact(*file);
    if (file_length < 0) {
      return -1;
    }
    length += file_length;
  }

  return length;
}

ConversionData::ConversionData()
: chapters()
{
}

// Fills vector provided with lines for a metadata file
// check return value for error
int ConversionData::get_chapter_metadata(std::vector<std::string>* chapter_metadata) {
  chapter_metadata->push_back(";FFMETADATA1");

  if (title != "")
    chapter_metadata->push_back("title=" + esc_metadata(title));
  if (comment != "")
    chapter_metadata->push_back("comment=" + esc_metadata(comment));

  int start = 0;

  if (chapters.size() <= 0)
    return 1;

  for (auto chapter = chapters.begin(); chapter != chapters.end(); ++chapter) {
    if (chapter->title == "")
      return 1;

    double length = chapter->get_chapter_length();
    if (length <= 0)
      return 1;

    int end = (int)(length * 1000.0) + start;

    chapter_metadata->push_back("[CHAPTER]");
    chapter_metadata->push_back("TIMEBASE=1/1000");
    chapter_metadata->push_back("START=" + std::to_string(start));
    chapter_metadata->push_back("END=" + std::to_string(end));
    chapter_metadata->push_back("title=" + esc_metadata(chapter->title));

    start = end;
  }

  return 0;
}