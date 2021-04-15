#include "converter_application.h"
#include <gtkmm.h>
#include <iostream>

int main (int argc, char *argv[]) {
  std::string bin_path = "/usr/local/bin";
  bool found_bin_path = false;
  for (int i = 0; i < argc; i++) {
    if (found_bin_path)
      bin_path = argv[i];
      break;
    
    if (argv[i] == "--bin_path")
      found_bin_path = true;
  }

  auto app = ConverterApplication::create(bin_path);

  const int status = app->run(argc, argv);
  return status;
}