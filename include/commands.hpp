#pragma once

/*
 * Headers
 */

#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "controller.hpp"
#include "menu.hpp"
#include "utils.hpp"

/*
 * Declaration
 */

namespace clashctl {

class Commands {
 private:
  struct Opt {
    std::string name;
    std::string description;
    std::function<void()> fn;
  };

 public:
  Commands(int argc, char** argv) noexcept;

  int run() noexcept;

 private:
  void main() noexcept;

  std::map<std::string, Opt> init_opts() noexcept;

  void help() noexcept;

  void start() noexcept;

  void stop() noexcept;

  void reload() noexcept;

  void ping() noexcept;

  void mode() noexcept;

  void proxy() noexcept;

  void update(const std::string& url);

 private:
  clashctl::Config config;
  clashctl::Controller controller_;
  quicky::Args args_;
  quicky::ExeInfo exe_info_;
  std::map<std::string, Opt> option_;
};

};  // namespace clashctl

/*
 * Implementation that will be part of the .cpp file if split into .hpp and .cpp
 * files.
 */

namespace clashctl {

inline Commands::Commands(int argc, char** argv) noexcept
    : controller_(config),
      args_(argc, argv),
      exe_info_(argv[0]),
      option_(init_opts()) {}

inline int Commands::run() noexcept {
  if (args_.get().empty()) {
    main();
    return 0;
  }
  const std::string option = args_.get()[0];
  if (option_.find(option) == option_.end()) {
    option_["help"].fn();
    return 1;
  }
  option_[option].fn();
  return 0;
}

inline void Commands::main() noexcept {
  std::vector<std::string> opts;
  for (auto&& c : option_) opts.push_back(c.first);
  Menu menu(std::move(opts));
  menu.on_opt_enter([&](int, const std::string& opt) {
    option_[opt].fn();
    return true;
  });
  menu.main();
}

inline std::map<std::string, Commands::Opt> Commands::init_opts() noexcept {
  std::map<std::string, Opt> opts;
  opts["help"] = {"help", "show usage", std::bind(&Commands::help, this)};
  opts["start"] = {"start", "start clash", std::bind(&Commands::start, this)};
  opts["stop"] = {"stop", "stop clash", std::bind(&Commands::stop, this)};
  opts["reload"] = {"reload", "reload clash",
                    std::bind(&Commands::reload, this)};
  opts["ping"] = {"ping", "curl google.com", std::bind(&Commands::ping, this)};
  opts["mode"] = {"mode", "select mode", std::bind(&Commands::mode, this)};
  opts["proxy"] = {"proxy", "select proxy", std::bind(&Commands::proxy, this)};
  opts["update"] = {"update <url>",
                    "download config from <url> and reload clash", [this]() {
                      if (args_.get().size() < 2) {
                        quicky::errorln("<url> required for update.");
                        return;
                      }
                      update(args_.get()[1]);
                    }};

  return opts;
}

inline void Commands::help() noexcept {
  const auto& parent_path = exe_info_.dir();
  const auto set_proxy_path = parent_path + "/set_proxy";
  const auto unset_proxy_path = parent_path + "/unset_proxy";
  const auto& exepath = exe_info_.arg0();
  std::cout << "Usage:\n"
               "'. "
            << set_proxy_path
            << "' to set http(s)_proxy\n"
               "'. "
            << unset_proxy_path
            << "' to unset http(s)_proxy\n"
               "`"
            << exepath
            << "` <option> [param]...\n\n"
               "Options:\n";
  for (auto&& c : option_) {
    std::cout << std::setw(20) << std::left << c.second.name;
    std::cout << c.second.description << std::endl;
  }
}

inline void Commands::start() noexcept {
  quicky::infoln("starting clash.");
  if (!controller_.reload()) {
    quicky::errorln("failed to start clash.");
  } else {
    quicky::infoln("clash is now available.");
  }
}

inline void Commands::stop() noexcept {
  controller_.stop();
  quicky::infoln("stopped clash.");
}

inline void Commands::reload() noexcept {
  if (!controller_.reload()) {
    quicky::errorln("failed to reload clash.");
  }
  quicky::infoln("reloaded clash.");
}

inline void Commands::ping() noexcept {
  if (!controller_.ping()) {
    quicky::infoln("clash is not available.");
  } else {
    quicky::infoln("clash is available.");
  }
}

inline void Commands::mode() noexcept {
  auto mode = controller_.get_mode();
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
    if (!controller_.set_mode(opt)) {
      quicky::error() << "failed to set mode to " << opt << std::endl;
    }
    mode = controller_.get_mode();
    if (mode.empty()) {
      quicky::errorln("failed to get current mode.");
      return false;
    }
    quicky::info() << "current mode: " << mode << std::endl;
    return true;
  });

  menu.main();
}

inline void Commands::proxy() noexcept {
  auto proxy = controller_.get_proxy();
  if (proxy.empty()) {
    quicky::errorln("failed to get current proxy.");
    return;
  }
  auto proxies = controller_.get_proxies();
  if (!proxies.has_value()) {
    quicky::errorln("failed to get available proxies.");
    return;
  }
  Menu menu(std::move(proxies.value()));

  menu.on_opt_show([&](int, const std::string& opt) {
    return proxy == opt ? opt + " 😎" : opt;
  });

  menu.on_opt_enter([&](int, const std::string& opt) {
    if (!controller_.set_proxy(opt)) {
      quicky::error() << "failed to set proxy to " << opt << std::endl;
    }
    proxy = controller_.get_proxy();
    if (proxy.empty()) {
      quicky::errorln("failed to get current proxy.");
      return false;
    }
    quicky::info() << "current proxy: " << proxy << std::endl;
    return true;
  });

  menu.main();
}

inline void Commands::update(const std::string& url) {
  quicky::infoln("updating config.");
  if (!controller_.update(url)) {
    quicky::errorln("failed to update from url: ");
    quicky::errorln(url.c_str());
    return;
  }
  quicky::infoln("updated config.");
}

};  // namespace clashctl
