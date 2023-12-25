#pragma once

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

#include "log.hpp"
#include "third-party/httplib.h"
#include "third-party/nlohmann/json.hpp"
#include "utils.hpp"

namespace clashctl {
struct Config {
  static std::shared_ptr<Config> Default() noexcept {
    std::shared_ptr<Config> config(new Config);
    return config;
  }

 private:
  Config() noexcept
      // : clash_path(quicky::current_path() + "/clashctl"),
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

  Config(const Config&) = default;
  Config(Config&&) = default;
  Config& operator=(const Config&) = delete;
  Config& operator=(Config&&) = delete;

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
  static const std::vector<std::string>& modes() noexcept {
    static std::vector<std::string> modes_ = {"DIRECT", "Proxies"};
    return modes_;
  }

  Mode(const std::string& str) {
    const auto& modes_ = modes();
    auto it = std::find(modes_.begin(), modes_.end(), str);
    if (it == modes_.end()) throw std::logic_error("invalid mode string.");

    m_str = *it;
  }

  std::string str() const noexcept { return std::string(m_str); }

 private:
  std::string_view m_str;
};

class Controller {
 public:
  Controller(std::shared_ptr<Config> config) noexcept : m_config(config) {}

  // start clash
  // 1. prepare empty log file for clash
  // 2. run clash background
  // 3. connection test
  bool start() const noexcept {
    if (!rm_log() || !touch_log()) {
      log::get()->errorln("failed to prepare log file.");
      return false;
    }
    log::get()->infoln("starting clash server.");
    if (quicky::run_background(
            m_config->clash_exe + " -d " + m_config->clash_config,
            m_config->clash_log)) {
      log::get()->errorln("failed to start clash server.");
      return false;
    }
    if (!ping()) {
      log::get()->errorln("clash is not available.");
      stop();
      return false;
    }
    return true;
  }

  // stop clash
  // kill clash running background
  void stop() const noexcept { quicky::kill(m_config->clash_exe); }

  // reload clash
  // call stop and start
  bool reload() const noexcept {
    stop();
    return start();
  }

  // connection test by visiting google
  // 1. set http proxy
  // 2. visit google
  // 3. unset http proxy
  bool ping() const noexcept {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    setenv("http_proxy", m_config->proxy_endpoint.c_str(), 1);
    setenv("https_proxy", m_config->proxy_endpoint.c_str(), 1);

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
  bool update(const std::string& url) const noexcept {
    auto& updateFile = m_config->update_temp_file;
    auto& configFile = m_config->clash_config_file;

    if (url.size() < 2) {
      log::get()->errorln("invalid url.");
      return false;
    }

    if (quicky::download_file(quicky::trim_url(url), updateFile)) {
      log::get()->errorln("failed to download config file.");
      return false;
    }

    if (quicky::exists(configFile)) {
      log::get()->infoln("backing up old config file.");
      if (!quicky::cp(configFile, configFile + ".backup")) {
        log::get()->errorln("failed to backup old config file.");
        return false;
      }
    }

    log::get()->infoln("updating config file.");
    if (!quicky::cp(updateFile, configFile)) {
      log::get()->errorln("failed to update config file.");
      return false;
    }

    log::get()->infoln("testing new config file.");
    if (!reload() || !ping()) {
      log::get()->errorln("invalid config file. recovering old config file.");
      if (quicky::exists(configFile + ".backup")) {
        if (!quicky::cp(configFile + ".backup", configFile))
          log::get()->errorln("failed to recover old config file.");
      }
      stop();
      return false;
    }
    stop();
    return true;
  }

  std::string get_proxy() const {
    try {
      httplib::Client cli(m_config->controller_endpoint);
      auto res = cli.Get(m_config->proxy_url);
      if (!res) throw std::logic_error("failed to send request to get proxy.");

      auto j = nlohmann::json::parse(res->body);
      return j["now"].get<std::string>();
    } catch (const std::exception& e) {
      log::get()->errorln(e.what());
      throw std::logic_error("failed to get proxy.");
    }
  }

  std::vector<std::string> get_proxies() const {
    try {
      httplib::Client cli(m_config->controller_endpoint);
      auto res = cli.Get(m_config->proxy_url);
      if (!res)
        throw std::logic_error("failed to send request to get proxies.");

      auto j = nlohmann::json::parse(res->body);
      return j["all"].get<std::vector<std::string>>();
    } catch (const std::exception& e) {
      log::get()->errorln(e.what());
      throw std::logic_error("failed to get proxies.");
    }
  }

  bool set_proxy(const std::string& proxy) const noexcept {
    try {
      const std::string data = "{\"name\": \"" + proxy + "\"}";
      httplib::Client cli(m_config->controller_endpoint);
      auto res = cli.Put(m_config->proxy_url, data, "text/plain");
      if (!res) {
        log::get()->errorln("failed to send request to set proxy.");
        return false;
      }
      if (get_proxy() != proxy) return false;

    } catch (const std::exception& e) {
      log::get()->errorln(e.what());
      log::get()->errorln("failed to set proxy.");
      return false;
    }
    return true;
  }

  std::string get_mode() const {
    try {
      httplib::Client cli(m_config->controller_endpoint);
      auto res = cli.Get(m_config->mode_url);
      if (!res) throw std::logic_error("failed to send request to get mode.");

      auto j = nlohmann::json::parse(res->body);
      return Mode(j["now"].get<std::string>()).str();
    } catch (const std::exception& e) {
      log::get()->errorln(e.what());
      throw std::logic_error("failed to get mode.");
    }
  }

  bool set_mode(const std::string& mode) const noexcept {
    try {
      const std::string data = "{\"name\": \"" + mode + "\"}";
      httplib::Client cli(m_config->controller_endpoint);
      auto res = cli.Put(m_config->mode_url, data, "text/plain");
      if (!res) {
        log::get()->errorln("failed to send request to set mode.");
        return false;
      }
      if (get_mode() != mode) return false;

    } catch (const std::exception& e) {
      log::get()->errorln(e.what());
      log::get()->errorln("failed to set mode");
      return false;
    }
    return true;
  }

 private:
  bool rm_log() const noexcept {
    if (quicky::exists(m_config->clash_log)) {
      if (!quicky::rm(m_config->clash_log)) return false;
    }
    return true;
  }

  bool touch_log() const noexcept {
    return quicky::run("touch " + m_config->clash_log) == 0;
  }

 private:
  std::shared_ptr<Config> m_config;
};
}  // namespace clashctl
