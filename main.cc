#include "converter_application.h"
#include <gtkmm.h>
#include <iostream>

int main (int argc, char *argv[]) {
  auto app = ConverterApplication::create();

  const int status = app->run(argc, argv);
  return status;
}