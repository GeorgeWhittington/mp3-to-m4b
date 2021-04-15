#include <string>

#include "conversion_data.h"

ChapterData::ChapterData(std::string title, std::vector<std::string> file_names) {
  this->title = title;
  this->file_names = file_names;
}

ConversionData::ConversionData()
: chapters()
{
}