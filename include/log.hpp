#pragma once

#include <fstream>
#include <iostream>
#include <memory>

class log {
public:
  static std::shared_ptr<log> get() noexcept {
    static auto l = std::shared_ptr<log>(new log);
    return l;
  }

  void on(bool enable = true) noexcept { m_enabled = enable; }

  bool with_log_file(const std::string &filename) noexcept {
    try {
      m_writer.open(filename);
      m_use_file = true;
    } catch (const std::exception &e) {
      return false;
    }
    return true;
  }

  template <class T, class... Rest>
  void errorln(T &&val, Rest... rest) noexcept {
    error(std::forward<T>(val), rest...);
    endl();
  }

  template <class T, class... Rest>
  void infoln(T &&val, Rest... rest) noexcept {
    info(std::forward<T>(val), rest...);
    endl();
  }

  template <class T, class... Rest> void error(T &&val, Rest... rest) noexcept {
    if (!m_enabled) return;
    if (m_use_file) {
      m_writer << "[ERROR] ";
      file_out(std::forward<T>(val), rest...);
    } else {
      std::cerr << "[ERROR] ";
      error_(std::forward<T>(val), rest...);
    }
  }

  template <class T, class... Rest> void info(T &&val, Rest... rest) noexcept {
    if (!m_enabled) return;
    if (m_use_file) {
      m_writer << "[INFO] ";
      file_out(std::forward<T>(val), rest...);
    } else {
      std::cout << "[INFO] ";
      info_(std::forward<T>(val), rest...);
    }
  }

private:
  log() noexcept : m_enabled(true), m_use_file(false) {}

  template <class T, class... Rest>
  void error_(T &&val, Rest... rest) noexcept {
    std::cerr << std::forward<T>(val);
    if constexpr (sizeof...(Rest))
      error_(rest...);
  }

  template <class T, class... Rest> void info_(T &&val, Rest... rest) noexcept {
    std::cerr << std::forward<T>(val);
    if constexpr (sizeof...(Rest))
      info_(rest...);
  }

  template <class T, class... Rest>
  void file_out(T &&val, Rest... rest) noexcept {
    m_writer << std::forward<T>(val);
    if constexpr (sizeof...(Rest))
      file_out(rest...);
  }

  void endl() noexcept {
    if (!m_enabled) return;
    if (m_use_file)
      m_writer << std::endl;
    else
      std::cerr << std::endl;
  }

private:
  bool m_enabled, m_use_file;
  std::ofstream m_writer;
};

