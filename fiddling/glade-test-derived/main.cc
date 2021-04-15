#include "application.h"
#include <gtkmm.h>

int main (int argc, char *argv[]) {
  auto app = MyApplication::create();
  const int status = app->run(argc, argv);
  return status;
}