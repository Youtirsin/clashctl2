#pragma once

#include <fstream>
#include <iostream>

class log {
 public:
  static void on(bool enable = true) noexcept { enabled() = enable; }

  static bool with_log_file(const std::string& filename) noexcept {
    try {
      file().open(filename);
      use_file() = true;
    } catch (const std::exception& e) {
      return false;
    }
    return true;
  }

  template <class T, class... Rest>
  static void errorln(T&& val, Rest... rest) noexcept {
    error(std::forward<T>(val), rest...);
    endl();
  }

  template <class T, class... Rest>
  static void infoln(T&& val, Rest... rest) noexcept {
    info(std::forward<T>(val), rest...);
    endl();
  }

  template <class T, class... Rest>
  static void error(T&& val, Rest... rest) noexcept {
    if (!enabled()) return;
    if (use_file()) {
      file() << "[ERROR] ";
      file_out(std::forward<T>(val), rest...);
    } else {
      std::cerr << "[ERROR] ";
      error_(std::forward<T>(val), rest...);
    }
  }

  template <class T, class... Rest>
  static void info(T&& val, Rest... rest) noexcept {
    if (!enabled()) return;
    if (use_file()) {
      file() << "[INFO] ";
      file_out(std::forward<T>(val), rest...);
    } else {
      std::cout << "[INFO] ";
      info_(std::forward<T>(val), rest...);
    }
  }

 private:
  template <class T, class... Rest>
  static void error_(T&& val, Rest... rest) noexcept {
    std::cerr << std::forward<T>(val);
    if constexpr (sizeof...(Rest)) error_(rest...);
  }

  template <class T, class... Rest>
  static void info_(T&& val, Rest... rest) noexcept {
    std::cerr << std::forward<T>(val);
    if constexpr (sizeof...(Rest)) info_(rest...);
  }

  template <class T, class... Rest>
  static void file_out(T&& val, Rest... rest) noexcept {
    file() << std::forward<T>(val);
    if constexpr (sizeof...(Rest)) file_out(rest...);
  }

  static void endl() noexcept {
    if (!enabled()) return;
    if (use_file())
      file() << std::endl;
    else
      std::cerr << std::endl;
  }

  static bool& enabled() noexcept {
    static bool e = true;
    return e;
  }

  static bool& use_file() noexcept {
    static bool u = false;
    return u;
  }

  static std::ofstream& file() noexcept {
    static std::ofstream f;
    return f;
  }
};