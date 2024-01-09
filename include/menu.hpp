#pragma once

#include <termios.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <vector>

class RawMode {
 public:
  static void enable() noexcept {
    termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    backup() = raw;
    raw.c_lflag &= ~(mode);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    // disable raw mode when exits
    std::atexit(disable);
  }

 private:
  static void disable() noexcept {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &backup());
  }

 private:
  static termios& backup() noexcept {
    static termios b;
    return b;
  }

  static constexpr int mode = ECHO      // no output
                              | ICANON  // no need to press enter
      ;
};

class Menu {
 public:
  Menu(std::vector<std::string>&& opts) noexcept
      : m_opts(std::move(opts)),
        m_idx(0),
        m_opt_fn([](int, const std::string& opt) { return opt; }),
        m_opt_enter_fn([](int, const std::string&) { return true; }) {}

  void on_opt_show(
      std::function<std::string(int, const std::string&)>&& fn) noexcept {
    m_opt_fn = std::move(fn);
  }

  void on_opt_enter(
      std::function<bool(int, const std::string&)>&& fn) noexcept {
    m_opt_enter_fn = std::move(fn);
  }

  bool main() {
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
        return m_opt_enter_fn(m_idx, m_opts[m_idx]);
      }
    }
    return true;
  }

  void show() noexcept {
    std::system("clear");
    const int start = m_idx / MAX * MAX;
    const int end = m_idx / MAX * MAX + MAX;
    for (int i = start; i < m_opts.size() && i < end; ++i) {
      if (i == m_idx) {
        std::cout << m_opt_fn(i, m_opts[i]) << " 👈" << std::endl;
      } else {
        std::cout << m_opt_fn(i, m_opts[i]) << std::endl;
      }
    }

    std::cout << "\n\n\n\nuse `w`, `s`, `a`, `d` to navigate, `enter` to "
                 "enter, `q` to quit.";
  }

  bool up() noexcept {
    if (m_idx <= 0) return false;
    --m_idx;
    show();
    return true;
  }

  bool down() noexcept {
    if (m_idx >= m_opts.size() - 1) return false;
    ++m_idx;
    show();
    return true;
  }

  bool left() noexcept {
    if (m_idx - MAX < 0) return false;
    m_idx -= MAX;
    show();
    return true;
  }

  bool right() noexcept {
    if (m_idx + MAX >= m_opts.size() - 1) return false;
    m_idx += MAX;
    show();
    return true;
  }

 private:
  static constexpr int MAX = 10;

  int m_idx;
  std::vector<std::string> m_opts;
  std::function<bool(int, const std::string&)> m_opt_enter_fn;
  std::function<std::string(int, const std::string&)> m_opt_fn;
};
