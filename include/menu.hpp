#pragma once

/*
 * Headers
 */

#include <termios.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <vector>

/*
 * Declaration
 */

class RawMode {
 public:
  static void enable() noexcept;

 private:
  static void disable() noexcept {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &backup());
  }

 private:
  static termios& backup() noexcept;

  static constexpr int mode = ECHO      // no output
                              | ICANON  // no need to press enter
      ;
};

class Menu {
 public:
  Menu(std::vector<std::string>&& opts) noexcept;

  void on_opt_show(
      std::function<std::string(int, const std::string&)>&& fn) noexcept {
    opt_fn_ = std::move(fn);
  }

  void on_opt_enter(std::function<bool(int, const std::string&)>&& fn) noexcept {
    opt_enter_fn_ = std::move(fn);
  }

  bool main();

  void show() noexcept;

  bool up() noexcept;

  bool down() noexcept;

  bool left() noexcept;

  bool right() noexcept;

 private:
  static constexpr int MAX = 10;

  int idx_;
  std::vector<std::string> opts_;
  std::function<bool(int, const std::string&)> opt_enter_fn_;
  std::function<std::string(int, const std::string&)> opt_fn_;
};

/*
 * Implementation that will be part of the .cpp file if split into .hpp and .cpp
 * files.
 */

inline void RawMode::enable() noexcept {
  termios raw;
  tcgetattr(STDIN_FILENO, &raw);
  backup() = raw;
  raw.c_lflag &= ~(mode);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

  // disable raw mode when exits
  std::atexit(disable);
}

inline termios& RawMode::backup() noexcept {
  static termios b;
  return b;
}

inline Menu::Menu(std::vector<std::string>&& opts) noexcept
    : opts_(std::move(opts)),
      idx_(0),
      opt_fn_([](int, const std::string& opt) { return opt; }),
      opt_enter_fn_([](int, const std::string&) { return true; }) {}

inline bool Menu::main() {
  RawMode::enable();
  char c;
  show();
  while ((c = std::getchar())) {
    if (c == 'q')
      break;
    else if (c == 'w')
      up();
    else if (c == 's')
      down();
    else if (c == 'a')
      left();
    else if (c == 'd')
      right();
    else if (c == '\r' || c == '\n') {
      std::system("clear");
      return opt_enter_fn_(idx_, opts_[idx_]);
    }
  }
  return true;
}

inline void Menu::show() noexcept {
  std::system("clear");
  const int start = idx_ / MAX * MAX;
  const int end = idx_ / MAX * MAX + MAX;
  for (int i = start; i < opts_.size() && i < end; ++i) {
    if (i == idx_) {
      std::cout << opt_fn_(i, opts_[i]) << " 👈" << std::endl;
    } else {
      std::cout << opt_fn_(i, opts_[i]) << std::endl;
    }
  }

  std::cout << "\n\n\n\nuse `w`, `s`, `a`, `d` to navigate, `enter` to "
               "enter, `q` to quit.";
}

inline bool Menu::up() noexcept {
  if (idx_ <= 0) return false;
  --idx_;
  show();
  return true;
}

inline bool Menu::down() noexcept {
  if (idx_ >= opts_.size() - 1) return false;
  ++idx_;
  show();
  return true;
}

inline bool Menu::left() noexcept {
  if (idx_ - MAX < 0) return false;
  idx_ -= MAX;
  show();
  return true;
}

inline bool Menu::right() noexcept {
  if (idx_ + MAX >= opts_.size() - 1) return false;
  idx_ += MAX;
  show();
  return true;
}
