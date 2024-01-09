#include "commands.hpp"
#include "utils.hpp"

int main(int argc, char** argv) {
  if (!quicky::has_curl()) {
    quicky::errorln("please install curl.");
    return 1;
  }

  clashctl::Commands commands(argc, argv);
  return commands.run();
}
