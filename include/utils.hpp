#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>

class log {
 private:
  class Endl {};

 public:
  constexpr static Endl endl = {};

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

  static log& info() noexcept {
    auto& ins = make_info();
    if (enabled()) {
      ins << "[INFO] ";
    }
    return ins;
  }

  static log& error() noexcept {
    auto& ins = make_error();
    if (enabled()) {
      ins << "[ERROR] ";
    }
    return ins;
  }

  template <class T>
  log& operator<<(const T& t) noexcept {
    if (!enabled()) {
      return *this;
    }
    if (use_file()) {
      if (file().is_open()) {
        file() << t;
      }
    } else {
      if (as_info) {
        std::cout << t;
      } else {
        std::cerr << t;
      }
    }
    return *this;
  }

  log& operator<<(Endl endl) noexcept {
    if (!enabled()) {
      return *this;
    }
    if (use_file()) {
      if (file().is_open()) {
        file() << std::endl;
      }
    } else {
      if (as_info) {
        std::cout << std::endl;
      } else {
        std::cerr << std::endl;
      }
    }
    return *this;
  }

 private:
  log(bool as_info_) noexcept : as_info(as_info_) {}

  static log& make_info() noexcept {
    static log l(true);
    return l;
  }

  static log& make_error() noexcept {
    static log l(false);
    return l;
  }

  static std::ofstream& file() noexcept {
    static std::ofstream output;
    return output;
  }

  static bool& use_file() noexcept {
    static bool use_file_ = false;
    return use_file_;
  }

  static bool& enabled() noexcept {
    static bool enabled_ = true;
    return enabled_;
  }

  bool as_info;
};

namespace quicky {
// print
inline auto& info() noexcept {
  std::cout << "[INFO] ";
  return std::cout;
}

inline auto& error() noexcept {
  std::cerr << "[ERROR] ";
  return std::cerr;
}

// process
inline int run(const std::string& cmd, const std::string& out_filepath = "") noexcept {
  if (out_filepath.empty()) {
    return std::system((cmd + " > /dev/null 2>&1").c_str());
  }
  return std::system((cmd + " > " + out_filepath + " 2>&1").c_str());
}

inline int run_background(const std::string& cmd,
                          const std::string& out_filepath = "") noexcept {
  if (out_filepath.empty()) {
    return std::system(("nohup " + cmd + " > /dev/null 2>&1 &").c_str());
  }
  return std::system(
      ("nohup " + cmd + " > " + out_filepath + " 2>&1 &").c_str());
}

inline bool has_curl() noexcept {
  return run("curl --version") == 0;
}

inline int kill(const std::string& name) noexcept { return run("pkill -9 -f " + name); }

// fs
inline std::string current_path() noexcept { return std::filesystem::current_path(); }

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
inline int download_file(const std::string& url, const std::string& filepath) noexcept {
  return run("curl -o " + filepath + " \"" + url + "\"");
}

inline std::string trim_url(const std::string& url) noexcept {
  if (url.size() < 2) {
    return url;
  }
  if (url.front() == '"' && url.back() == '"') {
    return url.substr(1, url.size() - 2);
  } else {
    return url;
  }
}
};  // namespace quicky
