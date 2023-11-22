#pragma once

#include <exception>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>

#include "controller.hpp"
#include "utils.hpp"

class commands {
 public:
  static std::unordered_map<std::string, std::function<void()>>&
  make() noexcept {
    static commands cmds;
    return cmds.m_cmds;
  }

  static void update(const std::string& url) {
    auto config = clashctl::Config::Default();
    clashctl::Controller controller(config);
    quicky::infoln("updating config.");
    if (!controller.update(url)) {
      quicky::errorln("failed to update from url: ");
      quicky::errorln(url);
      return;
    }
    quicky::infoln("updated config.");
  }

  static void proxy(int page = 1) {
    auto config = clashctl::Config::Default();
    clashctl::Controller controller(config);

    auto proxy = controller.get_proxy();
    auto proxies = controller.get_proxies();
    if (proxies.size() < (page - 1) * 10) {
      quicky::errorln("invalid page idx.");
      return;
    }
    for (int i = (page - 1) * 10; i < page * 10 && i < proxies.size(); ++i) {
      if (proxies[i] == proxy)
        std::cout << i << ". " << proxies[i] << " (current)" << std::endl;
      else
        std::cout << i << ". " << proxies[i] << std::endl;
    }
    int idx;
    quicky::infoln("select the proxy: ");
    std::cin >> idx;
    if (idx < 0 || idx > proxies.size()) {
      quicky::errorln("invalid idx.");
      return;
    }
    if (!controller.set_proxy(proxies[idx]))
      quicky::errorln("failed to set proxy.");
    quicky::infoln("current proxy: ", controller.get_proxy());
  }

  commands() noexcept {
    m_cmds["help"] = help;
    m_cmds["start"] = start;
    m_cmds["stop"] = stop;
    m_cmds["reload"] = reload;
    m_cmds["ping"] = ping;
    m_cmds["mode"] = mode;
  }

 private:
  static void help() noexcept {
    std::cout << "Usage:\n"
                 "'. set_proxy' to set http(s)_proxy\n"
                 "'. unset_proxy' to unset http(s)_proxy\n"
                 "~/clashctl/clashctl <option> [param]...\n"
                 "Options:\n"
                 "start           start clash\n"
                 "stop            stop clash\n"
                 "reload          reload clash\n"
                 "ping            curl google.com\n"
                 "update <url>    download config from <url> and reload clash\n"
                 "mode            select mode\n"
                 "proxy <page>    select proxy from <page>\n"
                 "help            show usage"
              << std::endl;
  }

  static void start() noexcept {
    auto config = clashctl::Config::Default();
    clashctl::Controller controller(config);

    quicky::infoln("starting clash.");
    if (!controller.reload()) quicky::errorln("failed to start clash.");
    quicky::infoln("clash is now available.");
  }

  static void stop() noexcept {
    auto config = clashctl::Config::Default();
    clashctl::Controller controller(config);

    controller.stop();
    quicky::infoln("stopped clash.");
  }

  static void reload() noexcept {
    auto config = clashctl::Config::Default();
    clashctl::Controller controller(config);

    if (!controller.reload()) quicky::errorln("failed to reload clash.");
    quicky::infoln("reloaded clash.");
  }

  static void ping() noexcept {
    auto config = clashctl::Config::Default();
    clashctl::Controller controller(config);

    if (!controller.ping())
      quicky::infoln("clash is not available.");
    else
      quicky::infoln("clash is available.");
  }

  static void mode() {
    auto config = clashctl::Config::Default();
    clashctl::Controller controller(config);

    auto mode = controller.get_mode();
    auto mode_ = mod_str(mode);
    auto modes = clashctl::mod_strs();
    for (int i = 0; i < modes.size(); ++i) {
      if (modes[i] == mode_)
        std::cout << i << ". " << modes[i] << " (current)" << std::endl;
      else
        std::cout << i << ". " << modes[i] << std::endl;
    }
    int idx;
    quicky::infoln("select the mode: ");
    std::cin >> idx;
    if (idx < 0 || idx > modes.size()) {
      quicky::errorln("invalid idx.");
      return;
    }
    if (!controller.set_mode(clashctl::str_mod(modes[idx])))
      quicky::error("failed to set mode to ", modes[idx]);
    quicky::info("current mode: ", clashctl::mod_str(controller.get_mode()));
  }

 private:
  std::unordered_map<std::string, std::function<void()>> m_cmds;
};