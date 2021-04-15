#ifndef CONVERSION_DATA
#define CONVERSION_DATA

#include <string>
#include <vector>

class ChapterData {
  public:
    ChapterData(std::string title, std::vector<std::string> file_names);

    std::string title;
    std::vector<std::string> file_names;
};

// Possibly provide with a mutex?
class ConversionData {
  public:
    ConversionData();

    std::string title;
    std::string author;
    std::string year;
    std::string cover_image_path;
    std::string comment;

    std::vector<ChapterData> chapters;
};

#endif