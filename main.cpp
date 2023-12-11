#include "commands.hpp"
#include "utils.hpp"

int main(int argc, char** argv) {
  if (!quicky::has_curl()) {
    quicky::errorln("please install curl.");
    return 1;
  }

  commands::handle(argc, argv);

  return 1;
}

