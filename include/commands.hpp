#pragma once

#include <functional>
#include <iostream>
#include <string>
#include <map>
#include <vector>

#include "controller.hpp"
#include "menu.hpp"
#include "utils.hpp"


class commands {
 private:
  struct Opt {
    std::string name;
    std::string description;
    std::function<void()> fn;
  };

 public:
  static std::map<std::string, Opt>& options() noexcept {
    static auto opts = init_opts();
    return opts;
  }

  static void handle(int argc, char** argv) noexcept {
    quicky::Args::parse(argc, argv);
    quicky::ExeSelfInfo::parse(argv[0]);

    if (argc == 1) {
      commands::main();
      return;
    }

    const std::string option = argv[1];
    auto& cmds = commands::options();
    if(cmds.find(option) == cmds.end())
      cmds["help"].fn();
    cmds[option].fn();
  }

 private:
  static std::map<std::string, Opt> init_opts() noexcept {
    std::map<std::string, Opt> opts;
    opts["help"] = { "help", "show usage", help };
    opts["start"] = { "start", "start clash", start };
    opts["stop"] = { "stop", "stop clash", stop };
    opts["reload"] = { "reload", "reload clash", reload };
    opts["ping"] = { "ping", "curl google.com", ping };
    opts["mode"] = { "mode", "select mode", mode };
    opts["proxy"] = { "proxy", "select proxy", proxy};

    opts["update"] = { "update <url>",
      "download config from <url> and reload clash",
      []() {
        auto& args = quicky::Args::get();
        if (args.size() < 2) {
          quicky::errorln("<url> required for update.");
          return;
        }
        update(args[1]);
      }};

    return opts;
  }

  static void main() {
    std::vector<std::string> opts;
    auto& cmds = options();
    for (auto&& c : cmds) opts.push_back(c.first);

    Menu menu(opts);
    menu.on_opt_enter([&](int, const std::string& opt) {
      cmds[opt].fn();
      return true;
    });
    menu.main();
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

  static void help() noexcept {
    const auto parent_path = quicky::ExeSelfInfo::dir();
    const auto set_proxy_path = parent_path + "/set_proxy";
    const auto unset_proxy_path = parent_path + "/unset_proxy";
    const auto exepath = quicky::ExeSelfInfo::arg0();
    std::cout << "Usage:\n"
                 "'. " << set_proxy_path << "' to set http(s)_proxy\n"
                 "'. " << unset_proxy_path << "' to unset http(s)_proxy\n"
                 << exepath << " <option> [param]...\n"

                 "Options:\n";
    auto& cmds = options();
    for (auto&& c : cmds) {
      std::cout << std::setw(20) << std::left << c.second.name;
      std::cout << c.second.description << std::endl;
    }
  }

  static void start() noexcept {
    auto config = clashctl::Config::Default();
    clashctl::Controller controller(config);

    quicky::infoln("starting clash.");
    if (!controller.reload())
      quicky::errorln("failed to start clash.");
    else
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

    auto mode_ = controller.get_mode();
    auto mode = mod_str(mode_);

    auto modes = clashctl::mod_strs();
    Menu menu(modes);

    menu.on_opt_show([&](int, const std::string& opt) {
      return mode == opt ? opt + " 😎" : opt;
    });

    menu.on_opt_enter([&](int, const std::string& opt) {
      if (!controller.set_mode(clashctl::str_mod(opt)))
        quicky::error("failed to set mode to ", opt);
      quicky::infoln("current mode: ", clashctl::mod_str(controller.get_mode()));
      return true;
    });

    menu.main();
  }

  static void proxy() {
    auto config = clashctl::Config::Default();
    clashctl::Controller controller(config);

    auto proxy = controller.get_proxy();

    auto proxies = controller.get_proxies();
    Menu menu(proxies);

    menu.on_opt_show([&](int, const std::string& opt) {
      return proxy == opt ? opt + " 😎" : opt;
    });

    menu.on_opt_enter([&](int, const std::string& opt) {
      if (!controller.set_proxy(opt))
        quicky::errorln("failed to set proxy to ", opt);
      quicky::infoln("current proxy: ", controller.get_proxy());
      return true;
    });

    menu.main();
  }
};
