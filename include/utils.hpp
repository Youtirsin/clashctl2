#pragma once

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

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
  ExeInfo(std::string&& arg0) noexcept : m_arg0(std::move(arg0)) {
    std::filesystem::path p(arg0);
    m_name = p.filename();
    m_dir = p.parent_path();
  }

  const std::string& arg0() const noexcept { return m_arg0; }

  const std::string& name() const noexcept { return m_name; }

  const std::string& dir() const noexcept { return m_dir; }

 private:
  std::string m_arg0, m_name, m_dir;
};

class Args {
 public:
  Args(int argc, char** argv) noexcept {
    for (size_t i = 1; i < argc; ++i) {
      m_args.push_back(argv[i]);
    }
  }

  const std::vector<std::string>& get() const noexcept { return m_args; }

 private:
  std::vector<std::string> m_args;
};

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

};  // namespace quicky
