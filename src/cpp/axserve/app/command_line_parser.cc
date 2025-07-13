// Copyright 2023 Yunseong Hwang
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "command_line_parser.h"

#include <map>
#include <sstream>
#include <string>

#include <QCoreApplication>
#include <QString>
#include <QVariant>

#include "axserve/common/message/show_parser_message.h"

class CustomFormatter : public CLI::Formatter {
private:
  bool enable_footer_formatting__ = false;

public:
  std::string make_help(
      const CLI::App *app, std::string name, CLI::AppFormatMode mode
  ) const {
    // This immediately forwards to the make_expanded method. This is done this
    // way so that subcommands can have overridden formatters
    if (mode == CLI::AppFormatMode::Sub)
      return make_expanded(app, mode);

    std::stringstream out;

    if ((app->get_name().empty()) && (app->get_parent() != nullptr)) {
      if (app->get_group() != "SUBCOMMANDS") {
        out << app->get_group() << ':';
      }
    }

    CLI::detail::streamOutAsParagraph(
        out, make_description(app), description_paragraph_width_, ""
    );

    out << make_usage(app, name);
    out << make_positionals(app);
    out << make_groups(app, mode);
    out << make_subcommands(app, mode);

    if (enable_footer_formatting__) {
      CLI::detail::streamOutAsParagraph(
          out, make_footer(app), footer_paragraph_width_
      );
    } else {
      out << make_footer(app);
    }

    return out.str();
  }
};

const std::map<std::string, ParsedConfig> PRESETS = {
    {"desktop",
     {.gui = true,
      .trayIcon = false,
      .hideOnClose = false,
      .hideOnMinimize = false,
      .startOnLaunch = false,
      .hideOnLaunch = false,
      .minimizeOnLaunch = false}},
    {"service",
     {.gui = true,
      .trayIcon = true,
      .hideOnClose = true,
      .hideOnMinimize = true,
      .startOnLaunch = true,
      .hideOnLaunch = true,
      .minimizeOnLaunch = false}},
    {"headless",
     {.gui = false,
      .trayIcon = false,
      .hideOnClose = false,
      .hideOnMinimize = false,
      .startOnLaunch = true,
      .hideOnLaunch = true,
      .minimizeOnLaunch = false}},
};

CommandLineParser::CommandLineParser()
    : app(appDescription) {

  app.option_defaults()->group("General");
  app.set_help_flag("--help", "Print this help message and exit");
  app.set_version_flag("--version", appVersion, "Print app version and exit");

  app.option_defaults()->group("Server");
  app.add_option("--address-uri", config.addressUri, "Server address URI");

  app.option_defaults()->group("Server SSL/TLS");
  app.add_option("--ssl-root-cert-file", config.sslRootCertFile)
      ->check(CLI::ExistingFile);
  app.add_option("--ssl-private-key-file", config.sslPrivateKeyFile)
      ->check(CLI::ExistingFile);
  app.add_option("--ssl-cert-chain-file", config.sslCertChainFile)
      ->check(CLI::ExistingFile);
  app.add_option(
         "--ssl-client-cert-request-type", config.sslClientCertRequestType
  )
      ->transform(CLI::IsMember(
          {"dont-request", "request-but-dont-verify", "request-and-verify",
           "require-but-dont-verify", "require-and-verify"}
      ));

  app.option_defaults()->group("Server Authentication");
  app.add_option("--auth-api-key-file", config.authApiKeyFile)
      ->check(CLI::ExistingFile);

  app.option_defaults()->group("Logging");
  app.add_option("--logging-level", config.loggingLevel)
      ->transform(CLI::IsMember({"debug", "warn", "info", "critical", "fatal"})
      );
  app.add_option("--logging-format", config.loggingFormat);
  app.add_option("--logging-type", config.loggingType)
      ->transform(CLI::IsMember(
          {"console", "file", "file-basic", "file-rotating", "file-daily"}
      ));
  app.add_option("--logging-file", config.loggingFile);

  app.add_option("--logging-rotating-max-size", config.loggingRotatingMaxSize)
      ->transform(CLI::AsSizeValue(true));
  app.add_option("--logging-rotating-max-files", config.loggingRotatingMaxFiles)
      ->check(CLI::NonNegativeNumber);

  CLI::Validator HourMinute = CLI::Validator(
      [this](const std::string &time) {
        int hour, minute;
        char colon;
        std::istringstream iss(time);
        iss >> hour >> colon >> minute;
        if (!(iss >> hour >> colon >> minute) || colon != ':' || !iss.eof()) {
          return std::string("Invalid time format, use HH:MM");
        }
        if (hour < 0 || hour > 23 || minute < 0 || minute > 59) {
          return std::string("Hour must be 0-23 and minute 0-59");
        }
        config.loggingDailyRotatingHour = hour;
        config.loggingDailyRotatingMinute = minute;
        return std::string{};
      },
      "[HH:MM]"
  );

  app.add_option(
         "--logging-daily-rotating-time", config.loggingDailyRotatingTime
  )
      ->transform(HourMinute);
  app.add_option("--logging-daily-max-files", config.loggingDailyMaxFiles)
      ->check(CLI::NonNegativeNumber);

  app.option_defaults()->group("Flags");
  app.add_flag("--gui,!--no-gui", config.gui, "Enable or disable GUI");
  app.add_flag(
      "--tray-icon,!--no-tray-icon", config.trayIcon,
      "Enable or disable tray icon"
  );
  app.add_flag(
      "--hide-on-close,!--no-hide-on-close", config.hideOnClose,
      "Hide window on close when tray is available"
  );
  app.add_flag(
      "--hide-on-minimize,!--no-hide-on-minimize", config.hideOnMinimize,
      "Hide window on minimize when tray is available"
  );

  app.add_flag(
      "--start-on-launch,!--no-start-on-launch", config.startOnLaunch,
      "Start server automatically on launch"
  );
  app.add_flag(
      "--hide-on-launch,!--no-hide-on-launch", config.hideOnLaunch,
      "Start hidden if tray is available"
  );
  app.add_flag(
      "--minimize-on-launch,!--no-minimize-on-launch", config.minimizeOnLaunch,
      "Start minimized if GUI is available"
  );

  std::string presetsAdditionalHelp = R"(
Presets Explained:

          --preset desktop
            - gui
            - no-tray-icon
            - no-hide-on-close
            - no-hide-on-minimize
            - no-start-on-launch
            - no-hide-on-launch
            - no-minimize-on-launch

          --preset service
            - gui
            - tray-icon
            - hide-on-close
            - hide-on-minimize
            - start-on-launch
            - hide-on-launch
            - no-minimize-on-launch

          --preset headless
            - no-gui
            - no-tray-icon
            - no-hide-on-close
            - no-hide-on-minimize
            - start-on-launch
            - hide-on-launch
            - no-minimize-on-launch
)";
  presetsAdditionalHelp.erase(0, 1);

  app.option_defaults()->group("Presets");
  app.add_option(
         "--preset", config.preset, "Preset profile: desktop, service, headless"
  )
      ->transform(CLI::IsMember(PRESETS));

  app.formatter(std::make_shared<CustomFormatter>());
  app.footer(presetsAdditionalHelp);

  app.allow_extras();
}

ParsedConfig CommandLineParser::parse(int argc, char *argv[]) {
  config = {};

  bool noConsole = QCoreApplication::instance()->property("noConsole").toBool();

  try {
    app.parse(argc, argv);
  } catch (const CLI::CallForHelp &e) {
    config.error = std::current_exception();
    config.returnCode = app.exit(e);
    if (noConsole) {
      showParserUsageMessage(QString::fromStdString(app.help()));
    }
    return config;
  } catch (const CLI::CallForAllHelp &e) {
    config.error = std::current_exception();
    config.returnCode = app.exit(e);
    if (noConsole) {
      showParserUsageMessage(
          QString::fromStdString(app.help("", CLI::AppFormatMode::All))
      );
    }
    return config;
  } catch (const CLI::CallForVersion &e) {
    config.error = std::current_exception();
    config.returnCode = app.exit(e);
    if (noConsole) {
      showParserUsageMessage(QString::fromStdString(app.version()));
    }
    return config;
  } catch (const CLI::ParseError &e) {
    config.error = std::current_exception();
    config.returnCode = app.exit(e);
    if (noConsole) {
      showParserErrorMessage(QString::fromStdString(e.what()));
    }
    return config;
  }

  if (!config.preset.empty()) {
    auto it = PRESETS.find(config.preset);
    const ParsedConfig &presetConfig = it->second;
    config = presetConfig;
    app.parse(argc, argv);
  }

  try {
    if (!config.gui && noConsole) {
      throw CLI::ValidationError(
          "Cannot start app without GUI when no console is attached"
      );
    }
    if (config.trayIcon && !config.gui) {
      throw CLI::ValidationError("Cannot create tray icon without GUI");
    }
    if ((config.hideOnLaunch || config.hideOnClose || config.hideOnMinimize) &&
        !config.trayIcon && noConsole) {
      throw CLI::ValidationError(
          "Cannot hide window without tray icon or console attached"
      );
    }
    if (config.minimizeOnLaunch && !config.gui) {
      throw CLI::ValidationError("Cannot minimize window without GUI");
    }
    if (!config.gui && !config.startOnLaunch) {
      throw CLI::ValidationError(
          "Server should start on launch when running without GUI"
      );
    }
  } catch (const CLI::ParseError &e) {
    config.error = std::current_exception();
    config.returnCode = app.exit(e);
    if (noConsole) {
      showParserErrorMessage(QString::fromStdString(e.what()));
    }
    return config;
  }

  return config;
}

int CommandLineParser::exit(const CLI::ParseError &e) { return app.exit(e); }
