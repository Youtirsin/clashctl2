#pragma once

/*
 * Headers
 */

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

/*
 * Declaration
 */

namespace quicky {

inline auto& info() noexcept {
  std::cout << "[INFO] ";
  return std::cout;
}

inline auto& error() noexcept {
  std::cerr << "[ERROR] ";
  return std::cerr;
}

inline void infoln(const char* message) noexcept {
  info() << message << std::endl;
}

inline void errorln(const char* message) noexcept {
  error() << message << std::endl;
}

class ExeInfo {
 public:
  template <class string_type = std::string>
  ExeInfo(string_type&& arg0) noexcept;

  const std::string& arg0() const noexcept { return arg0_; }

  const std::string& name() const noexcept { return name_; }

  const std::string& dir() const noexcept { return dir_; }

 private:
  std::string arg0_, name_, dir_;
};

class Args {
 public:
  Args(int argc, char** argv) noexcept {
    for (size_t i = 1; i < argc; ++i) {
      args_.push_back(argv[i]);
    }
  }

  const std::vector<std::string>& get() const noexcept { return args_; }

 private:
  std::vector<std::string> args_;
};

// process
int run(const std::string& cmd, const std::string& out_filepath = "") noexcept;

int run_background(const std::string& cmd,
                   const std::string& out_filepath = "") noexcept;

bool has_curl() noexcept;

inline int kill(const std::string& name) noexcept {
  return run("pkill -9 -f " + name);
}

// fs
std::string current_path() noexcept;

bool exists(const std::string& filepath) noexcept;

inline std::string current_path() noexcept {
  return std::filesystem::current_path();
}

inline bool exists(const std::string& filepath) noexcept {
  return std::filesystem::exists(filepath);
}

bool rm(const std::string& filepath) noexcept;

bool cp(const std::string& from, const std::string& to) noexcept;

// web
inline int download_file(const std::string& url,
                         const std::string& filepath) noexcept {
  return run("curl -o " + filepath + " \"" + url + "\"");
}

std::string trim_url(const std::string& url) noexcept;

}  // namespace quicky

/*
 * Implementation of template methods.
 */

namespace quicky {

template <class string_type>
ExeInfo::ExeInfo(string_type&& arg0) noexcept
    : arg0_(std::forward<string_type>(arg0)) {
  std::filesystem::path p(arg0);
  name_ = p.filename();
  dir_ = p.parent_path();
}

}  // namespace quicky

/*
 * Implementation that will be part of the .cpp file if split into .hpp and .cpp
 * files.
 */

namespace quicky {

// process
inline int run(const std::string& cmd,
               const std::string& out_filepath) noexcept {
  if (out_filepath.empty())
    return std::system((cmd + " > /dev/null 2>&1").c_str());
  return std::system((cmd + " > " + out_filepath + " 2>&1").c_str());
}

inline int run_background(const std::string& cmd,
                          const std::string& out_filepath) noexcept {
  if (out_filepath.empty())
    return std::system(("nohup " + cmd + " > /dev/null 2>&1 &").c_str());
  return std::system(
      ("nohup " + cmd + " > " + out_filepath + " 2>&1 &").c_str());
}

inline bool has_curl() noexcept { return run("curl --version") == 0; }

// fs
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

inline std::string trim_url(const std::string& url) noexcept {
  if (url.size() < 2) return url;
  if (url.front() == '"' && url.back() == '"') {
    return url.substr(1, url.size() - 2);
  } else {
    return url;
  }
}

}  // namespace quicky
