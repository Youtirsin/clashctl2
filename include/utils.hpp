#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>

namespace quicky {
// print
template <class T, class... Rest>
static void error_(T&& val, Rest... rest) noexcept {
  std::cerr << std::forward<T>(val);
  if constexpr (sizeof...(Rest)) error_(rest...);
}

template <class T, class... Rest>
static void info_(T&& val, Rest... rest) noexcept {
  std::cout << std::forward<T>(val);
  if constexpr (sizeof...(Rest)) info_(rest...);
}

template <class T, class... Rest>
void error(T&& val, Rest... rest) noexcept {
  std::cerr << "[ERROR] ";
  error_(std::forward<T>(val), rest...);
}

template <class T, class... Rest>
void info(T&& val, Rest... rest) noexcept {
  std::cout << "[INFO] ";
  info_(std::forward<T>(val), rest...);
}

template <class T, class... Rest>
void errorln(T&& val, Rest... rest) noexcept {
  error(std::forward<T>(val), rest...);
  std::cerr << std::endl;
}

template <class T, class... Rest>
void infoln(T&& val, Rest... rest) noexcept {
  info(std::forward<T>(val), rest...);
  std::cout << std::endl;
}

// process
inline int run(const std::string& cmd,
               const std::string& out_filepath = "") noexcept {
  if (out_filepath.empty())
    return std::system((cmd + " > /dev/null 2>&1").c_str());
  return std::system((cmd + " > " + out_filepath + " 2>&1").c_str());
}

inline int run_background(const std::string& cmd,
                          const std::string& out_filepath = "") noexcept {
  if (out_filepath.empty())
    return std::system(("nohup " + cmd + " > /dev/null 2>&1 &").c_str());
  return std::system(
      ("nohup " + cmd + " > " + out_filepath + " 2>&1 &").c_str());
}

inline bool has_curl() noexcept { return run("curl --version") == 0; }

inline int kill(const std::string& name) noexcept {
  return run("pkill -9 -f " + name);
}

// fs
inline std::string current_path() noexcept {
  return std::filesystem::current_path();
}

inline bool exists(const std::string& filepath) noexcept {
  return std::filesystem::exists(filepath);
}

inline bool rm(const std::string& filepath) noexcept {
  try {
    return std::filesystem::remove(filepath);
  } catch (const std::exception& e) {
    return false;
  }
}

inline bool cp(const std::string& from, const std::string& to) noexcept {
  try {
    return std::filesystem::copy_file(
        from, to, std::filesystem::copy_options::overwrite_existing);
  } catch (const std::exception& e) {
    return false;
  }
}

// web
inline int download_file(const std::string& url,
                         const std::string& filepath) noexcept {
  return run("curl -o " + filepath + " \"" + url + "\"");
}

inline std::string trim_url(const std::string& url) noexcept {
  if (url.size() < 2) return url;
  if (url.front() == '"' && url.back() == '"')
    return url.substr(1, url.size() - 2);
  else
    return url;
}
}
;  // namespace quicky
