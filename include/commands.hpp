#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "controller.hpp"
#include "menu.hpp"
#include "utils.hpp"

namespace clashctl {
class Commands {
 private:
  struct Opt {
    std::string name;
    std::string description;
    std::function<void()> fn;
  };

 public:
  Commands(int argc, char** argv) noexcept
      : m_controller(m_config),
        m_args(argc, argv),
        m_exe_info(argv[0]),
        m_options(init_opts()) {}

  int run() noexcept {
    if (m_args.get().empty()) {
      main();
      return 0;
    }
    const std::string option = m_args.get()[0];
    if (m_options.find(option) == m_options.end()) {
      m_options["help"].fn();
      return 1;
    }
    m_options[option].fn();
    return 0;
  }

 private:
  void main() noexcept {
    std::vector<std::string> opts;
    for (auto&& c : m_options) opts.push_back(c.first);
    Menu menu(std::move(opts));
    menu.on_opt_enter([&](int, const std::string& opt) {
      m_options[opt].fn();
      return true;
    });
    menu.main();
  }

  std::map<std::string, Opt> init_opts() noexcept {
    std::map<std::string, Opt> opts;
    opts["help"] = {"help", "show usage", std::bind(&Commands::help, this)};
    opts["start"] = {"start", "start clash", std::bind(&Commands::start, this)};
    opts["stop"] = {"stop", "stop clash", std::bind(&Commands::stop, this)};
    opts["reload"] = {"reload", "reload clash", std::bind(&Commands::reload, this)};
    opts["ping"] = {"ping", "curl google.com", std::bind(&Commands::ping, this)};
    opts["mode"] = {"mode", "select mode", std::bind(&Commands::mode, this)};
    opts["proxy"] = {"proxy", "select proxy", std::bind(&Commands::proxy, this)};
    opts["update"] = {"update <url>",
                      "download config from <url> and reload clash", [this]() {
                        if (m_args.get().size() < 2) {
                          quicky::errorln("<url> required for update.");
                          return;
                        }
                        update(m_args.get()[1]);
                      }};

    return opts;
  }

  void help() noexcept {
    const auto& parent_path = m_exe_info.dir();
    const auto set_proxy_path = parent_path + "/set_proxy";
    const auto unset_proxy_path = parent_path + "/unset_proxy";
    const auto& exepath = m_exe_info.arg0();
    std::cout << "Usage:\n"
                 "'. " << set_proxy_path << "' to set http(s)_proxy\n"
                 "'. " << unset_proxy_path << "' to unset http(s)_proxy\n"
                 "`" << exepath << "` <option> [param]...\n\n"
                 "Options:\n";
    for (auto&& c : m_options) {
      std::cout << std::setw(20) << std::left << c.second.name;
      std::cout << c.second.description << std::endl;
    }
  }

  void start() noexcept {
    quicky::infoln("starting clash.");
    if (!m_controller.reload()) {
      quicky::errorln("failed to start clash.");
    } else {
      quicky::infoln("clash is now available.");
    }
  }

  void stop() noexcept {
    m_controller.stop();
    quicky::infoln("stopped clash.");
  }

  void reload() noexcept {
    if (!m_controller.reload()) {
      quicky::errorln("failed to reload clash.");
    }
    quicky::infoln("reloaded clash.");
  }

  void ping() noexcept {
    if (!m_controller.ping()) {
      quicky::infoln("clash is not available.");
    } else {
      quicky::infoln("clash is available.");
    }
  }

  void mode() noexcept {
    auto mode = m_controller.get_mode();
    if (mode.empty()) {
      quicky::errorln("failed to get current mode.");
      return;
    }
    auto modes = clashctl::Mode::modes();
    Menu menu(std::move(modes));

    menu.on_opt_show([&](int, const std::string& opt) {
      return mode == opt ? opt + " 😎" : opt;
    });

    menu.on_opt_enter([&](int, const std::string& opt) {
      if (!m_controller.set_mode(opt)) {
        quicky::error() << "failed to set mode to " << opt << std::endl;
      }
      mode = m_controller.get_mode();
      if (mode.empty()) {
        quicky::errorln("failed to get current mode.");
        return false;
      }
      quicky::info() << "current mode: " << mode << std::endl;
      return true;
    });

    menu.main();
  }

  void proxy() noexcept {
    auto proxy = m_controller.get_proxy();
    if (proxy.empty()) {
      quicky::errorln("failed to get current proxy.");
      return;
    }
    auto proxies = m_controller.get_proxies();
    if (!proxies.has_value()) {
      quicky::errorln("failed to get available proxies.");
      return;
    }
    Menu menu(std::move(proxies.value()));

    menu.on_opt_show([&](int, const std::string& opt) {
      return proxy == opt ? opt + " 😎" : opt;
    });

    menu.on_opt_enter([&](int, const std::string& opt) {
      if (!m_controller.set_proxy(opt)) {
        quicky::error() << "failed to set proxy to " << opt << std::endl;
      }
      proxy = m_controller.get_proxy();
      if (proxy.empty()) {
        quicky::errorln("failed to get current proxy.");
        return false;
      }
      quicky::info() << "current proxy: " << proxy << std::endl;
      return true;
    });

    menu.main();
  }

  void update(const std::string& url) {
    quicky::infoln("updating config.");
    if (!m_controller.update(url)) {
      quicky::errorln("failed to update from url: ");
      quicky::errorln(url.c_str());
      return;
    }
    quicky::infoln("updated config.");
  }

 private:
  clashctl::Config m_config;
  clashctl::Controller m_controller;
  quicky::Args m_args;
  quicky::ExeInfo m_exe_info;
  std::map<std::string, Opt> m_options;
};

};  // namespace clashctl
