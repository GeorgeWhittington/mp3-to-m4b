#ifndef CONVERSION_DATA
#define CONVERSION_DATA

#include <string>
#include <vector>

class ChapterData {
  public:
    ChapterData(std::string title, std::vector<std::string> file_names);

    double get_chapter_length();

    std::string title;
    std::vector<std::string> file_names;
};

class ConversionData {
  public:
    ConversionData();

    int get_chapter_metadata(std::vector<std::string>* chapter_metadata);

    std::string title;
    std::string author;
    std::string year;
    std::string cover_image_path;
    std::string comment;

    std::vector<ChapterData> chapters;
};

#endif