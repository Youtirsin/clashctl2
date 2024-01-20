#pragma once

/*
 * Headers
 */

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>

#include "third-party/nlohmann/json.hpp"
#include "third-party/yhirose/httplib.h"
#include "utils.hpp"

/*
 * Declaration
 */

namespace clashctl {

struct Config {
  Config() noexcept;

 public:
  // the path to the clashctl directory
  const std::string clash_path;
  // the path to the clash server
  const std::string clash_exe;
  // the path to the clash server log file
  const std::string clash_log;
  // the path to the clashctl log file
  const std::string clashctl_log;
  // the path to the clash config directory
  const std::string clash_config;
  // the path to the clash config file
  const std::string clash_config_file;
  // the path to the downloaded clash config file
  const std::string update_temp_file;
  // the proxy endpoint
  const std::string proxy_endpoint;
  // the clash server controller endpoint
  const std::string controller_endpoint;
  // the url for getting the final mode
  const std::string mode_url;
  // the url for getting the proxies
  const std::string proxy_url;
};

class Mode {
 public:
  static const std::vector<std::string>& modes() noexcept;

  Mode(const std::string& str);

  std::string str() const noexcept { return std::string(str_); }

 private:
  std::string_view str_;
};

class Controller {
 public:
  Controller(Config& config) noexcept : config_(config) {}

  // start clash
  // 1. prepare empty log file for clash
  // 2. run clash background
  // 3. connection test
  bool start() const noexcept;

  // stop clash
  // kill clash running background
  void stop() const noexcept {
    quicky::kill(config_.clash_exe);
  }

  // reload clash
  // call stop and start
  bool reload() const noexcept;

  // connection test by visiting google
  // 1. set http proxy
  // 2. visit google
  // 3. unset http proxy
  bool ping() const noexcept;

  // update clash subscription
  // 1. download config file
  // 2. backup old config file if exists
  // 3. apply new config file
  // 4. test connection by calling reload and ping
  // 5. stop the clash running for testing
  bool update(const std::string& url) const noexcept;

  std::string get_proxy() const noexcept;

  std::optional<std::vector<std::string>> get_proxies() const;

  bool set_proxy(const std::string& proxy) const noexcept;

  std::string get_mode() const noexcept;

  bool set_mode(const std::string& mode) const noexcept;

 private:
  bool rm_log() const noexcept;

  bool touch_log() const noexcept {
    return quicky::run("touch " + config_.clash_log) == 0;
  }

 private:
  Config& config_;
};
}  // namespace clashctl

/*
 * Implementation that will be part of the .cpp file if split into .hpp and .cpp
 * files.
 */

namespace clashctl {

inline Config::Config() noexcept
    : clash_path(std::string(getenv("HOME")) + "/clashctl"),
      clash_exe(clash_path + "/clashctl-buildin-server"),
      clash_log(clash_path + "/clash.log"),
      clashctl_log(clash_path + "/clashctl.log"),
      clash_config(clash_path + "/config"),
      clash_config_file(clash_config + "/config.yaml"),
      update_temp_file(clash_path + "/update.yaml"),
      proxy_endpoint("127.0.0.1:7890"),
      controller_endpoint("localhost:9090"),
      mode_url("/proxies/Final"),
      proxy_url("/proxies/Proxies") {}

inline const std::vector<std::string>& Mode::modes() noexcept {
  static std::vector<std::string> modes_ = {"DIRECT", "Proxies"};
  return modes_;
}

inline Mode::Mode(const std::string& str) {
  const auto& modes_ = modes();
  auto it = std::find(modes_.begin(), modes_.end(), str);
  if (it == modes_.end()) {
    throw std::logic_error("invalid mode string.");
  }
  str_ = *it;
}

// start clash
// 1. prepare empty log file for clash
// 2. run clash background
// 3. connection test
inline bool Controller::start() const noexcept {
  if (!rm_log() || !touch_log()) {
    quicky::errorln("failed to prepare log file.");
    return false;
  }
  quicky::info() << "starting clash server." << std::endl;
  if (quicky::run_background(config_.clash_exe + " -d " + config_.clash_config,
                             config_.clash_log)) {
    quicky::errorln("failed to start clash server.");
    return false;
  }
  if (!ping()) {
    quicky::errorln("clash is not available.");
    stop();
    return false;
  }
  return true;
}

// reload clash
// call stop and start
inline bool Controller::reload() const noexcept {
  stop();
  return start();
}

// connection test by visiting google
// 1. set http proxy
// 2. visit google
// 3. unset http proxy
inline bool Controller::ping() const noexcept {
  std::this_thread::sleep_for(std::chrono::seconds(1));
  setenv("http_proxy", config_.proxy_endpoint.c_str(), 1);
  setenv("https_proxy", config_.proxy_endpoint.c_str(), 1);

  int res = quicky::run("curl -s --connect-timeout 2 google.com");

  setenv("http_proxy", "", 1);
  setenv("https_proxy", "", 1);
  return res == 0;
}

// update clash subscription
// 1. download config file
// 2. backup old config file if exists
// 3. apply new config file
// 4. test connection by calling reload and ping
// 5. stop the clash running for testing
inline bool Controller::update(const std::string& url) const noexcept {
  auto& updateFile = config_.update_temp_file;
  auto& configFile = config_.clash_config_file;

  if (url.size() < 2) {
    quicky::errorln("invalid url.");
    return false;
  }

  if (quicky::download_file(quicky::trim_url(url), updateFile)) {
    quicky::errorln("failed to download config file.");
    return false;
  }

  if (quicky::exists(configFile)) {
    quicky::infoln("backing up old config file.");
    if (!quicky::cp(configFile, configFile + ".backup")) {
      quicky::errorln("failed to backup old config file.");
      return false;
    }
  }

  quicky::infoln("updating config file.");
  if (!quicky::cp(updateFile, configFile)) {
    quicky::errorln("failed to update config file.");
    return false;
  }

  quicky::infoln("testing new config file.");
  if (!reload() || !ping()) {
    quicky::errorln("invalid config file. recovering old config file.");
    if (quicky::exists(configFile + ".backup")) {
      if (!quicky::cp(configFile + ".backup", configFile)) {
        quicky::errorln("failed to recover old config file.");
      }
    }
    stop();
    return false;
  }
  stop();
  return true;
}

inline std::string Controller::get_proxy() const noexcept {
  try {
    httplib::Client cli(config_.controller_endpoint);
    auto res = cli.Get(config_.proxy_url);
    if (!res) {
      throw std::logic_error("failed to send request to get proxy.");
    }
    auto j = nlohmann::json::parse(res->body);
    return j["now"].get<std::string>();
  } catch (const std::exception& e) {
    quicky::errorln(e.what());
    return "";
  }
}

inline std::optional<std::vector<std::string>> Controller::get_proxies() const {
  try {
    httplib::Client cli(config_.controller_endpoint);
    auto res = cli.Get(config_.proxy_url);
    if (!res) {
      throw std::logic_error("failed to send request to get proxies.");
    }
    auto j = nlohmann::json::parse(res->body);
    return j["all"].get<std::vector<std::string>>();
  } catch (const std::exception& e) {
    quicky::errorln(e.what());
    return std::nullopt;
  }
}

inline bool Controller::set_proxy(const std::string& proxy) const noexcept {
  try {
    const std::string data = "{\"name\": \"" + proxy + "\"}";
    httplib::Client cli(config_.controller_endpoint);
    auto res = cli.Put(config_.proxy_url, data, "text/plain");
    if (!res) {
      quicky::errorln("failed to send request to set proxy.");
      return false;
    }
    if (get_proxy() != proxy) return false;

  } catch (const std::exception& e) {
    quicky::errorln(e.what());
    quicky::errorln("failed to set proxy.");
    return false;
  }
  return true;
}

inline std::string Controller::get_mode() const noexcept {
  try {
    httplib::Client cli(config_.controller_endpoint);
    auto res = cli.Get(config_.mode_url);
    if (!res) {
      throw std::logic_error("failed to send request to get mode.");
    }
    auto j = nlohmann::json::parse(res->body);
    return Mode(j["now"].get<std::string>()).str();
  } catch (const std::exception& e) {
    quicky::errorln(e.what());
    return "";
  }
}

inline bool Controller::set_mode(const std::string& mode) const noexcept {
  try {
    const std::string data = "{\"name\": \"" + mode + "\"}";
    httplib::Client cli(config_.controller_endpoint);
    auto res = cli.Put(config_.mode_url, data, "text/plain");
    if (!res) {
      quicky::errorln("failed to send request to set mode.");
      return false;
    }
    if (get_mode() != mode) return false;

  } catch (const std::exception& e) {
    quicky::errorln(e.what());
    quicky::errorln("failed to set mode");
    return false;
  }
  return true;
}

inline bool Controller::rm_log() const noexcept {
  if (quicky::exists(config_.clash_log)) {
    if (!quicky::rm(config_.clash_log)) return false;
  }
  return true;
}

}  // namespace clashctl
