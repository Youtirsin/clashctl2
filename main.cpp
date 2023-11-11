#include <iostream>

#include "commands.hpp"
#include "utils.hpp"

int main(int argc, char** argv) {
  try {
    if (!quicky::has_curl()) {
      quicky::error() << "please install curl." << std::endl;
      return 1;
    }

    auto cmds = commands::make();
    if (argc == 1) {
      cmds["help"]();
      return 1;
    }
    const std::string option = argv[1];

    if (option == "update") {
      if (argc != 3) {
        cmds["help"]();
        return 1;
      }
      commands::update(argv[2]);
      return 0;
    }

    if (option == "proxy") {
      if (argc != 3) {
        commands::proxy();
      } else {
        commands::proxy(std::stoi(argv[2]));
      }
      return 0;
    }

    if (cmds.find(option) != cmds.end()) {
      cmds[argv[1]]();
      return 0;
    }

    cmds["help"]();
  } catch (const std::exception& e) {
    quicky::error() << e.what() << std::endl;
    return 1;
  }
  return 1;
}
