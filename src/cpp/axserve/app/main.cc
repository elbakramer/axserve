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

#include <Qt>
#include <QtDebug>
#include <QtLogging>

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QProcessEnvironment>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QTranslator>

#include "axserve/app/model/start_server_configuration.h"
#include "axserve/app/widget/main_window.h"
#include "axserve/common/logging/message_handlers_manager.h"
#include "axserve/common/util/show_parser_message.h"

#include "axserve/app/config.h"

class CommandLineParser {
  Q_DECLARE_TR_FUNCTIONS(CommandLineParser)

private:
  // parser
  QScopedPointer<QCommandLineParser> m_parser;

  // builtin parser options
  QCommandLineOption m_helpOption;
  QCommandLineOption m_versionOption;

  // application gui options
  QCommandLineOption m_guiOption;
  QCommandLineOption m_noGuiOption;

  // translation options
  QCommandLineOption m_translateOption;
  QCommandLineOption m_noTranslateOption;

  // logging level options
  QCommandLineOption m_logLevelOption;
  QCommandLineOption m_debugOption;
  QCommandLineOption m_infoOption;
  QCommandLineOption m_warningOption;
  QCommandLineOption m_criticalOption;
  QCommandLineOption m_fatalOption;

  // main options
  QCommandLineOption m_classIdOption;
  QCommandLineOption m_addressUriOption;

  // other flag options
  QCommandLineOption m_createTrayIconOption;
  QCommandLineOption m_noTrayIconOption;
  QCommandLineOption m_startMinimizedOption;
  QCommandLineOption m_startHiddenOption;

public:
  CommandLineParser()
      : m_helpOption("help"),
        m_versionOption("version"),
        m_guiOption("gui"),
        m_noGuiOption("no-gui"),
        m_translateOption("translate"),
        m_noTranslateOption("no-translate"),
        m_logLevelOption("log-level"),
        m_debugOption("debug"),
        m_infoOption("info"),
        m_warningOption("warning"),
        m_criticalOption("critical"),
        m_fatalOption("fatal"),
        m_classIdOption("clsid"),
        m_addressUriOption("address"),
        m_createTrayIconOption("tray-icon"),
        m_noTrayIconOption("no-tray-icon"),
        m_startMinimizedOption("minimized"),
        m_startHiddenOption("hidden") {
    initialize();
  }

  // initialize members
  void initialize() {
    // create parser
    m_parser.reset(new QCommandLineParser());

    // set application description
    m_parser->setApplicationDescription(
        tr("Description:") + " " +
        tr("gRPC server process for an Active-X or COM support.")
    );

    // set parser builtin options
    m_helpOption = m_parser->addHelpOption();
    m_versionOption = m_parser->addVersionOption();

    // setup core options
    m_classIdOption.setDescription(tr("CLSID for Active-X or COM."));
    m_classIdOption.setValueName(tr("CLSID"));
    m_addressUriOption.setDescription(tr("Address URI for gRPC server to bind.")
    );
    m_addressUriOption.setValueName(tr("ADDRESS"));

    // setup flag options
    m_createTrayIconOption.setDescription(
        tr("Create system tray icon for process management.")
    );
    m_startHiddenOption.setDescription(
        tr("Hide the starting window on start. Valid only when the tray "
           "icon is created.")
    );

    // setup common options
    m_noGuiOption.setDescription(
        tr("Disable GUI components. Valid only when console is attached.")
    );
    m_translateOption.setDescription(
        tr("Translate to current locale if available.")
    );
    m_logLevelOption.setDescription(
        tr("Mininmum log level or type to print (debug, info, warning, "
           "critical, fatal).")
    );
    m_logLevelOption.setValueName(tr("TYPE"));

    // add user defined options to parser
    m_parser->addOption(m_classIdOption);
    m_parser->addOption(m_addressUriOption);
    m_parser->addOption(m_createTrayIconOption);
    m_parser->addOption(m_startHiddenOption);
    m_parser->addOption(m_noGuiOption);
    m_parser->addOption(m_translateOption);
    m_parser->addOption(m_logLevelOption);
  }

  // create args list
  QStringList createArguments(int &argc, char *argv[]) {
    QStringList list;
    for (int i = 0; i < argc; ++i) {
      list << QString::fromLocal8Bit(argv[i]);
    }
    return list;
  }

  // slice args before double dash
  QStringList sliceBeforeDoubleDash(const QStringList &args) {
    QStringList argsBeforeDoubleDash = args;
    int doubleDashPos = args.indexOf("--");
    if (doubleDashPos >= 0) {
      argsBeforeDoubleDash = argsBeforeDoubleDash.sliced(0, doubleDashPos);
    }
    return argsBeforeDoubleDash;
  }

  // check console availability
  bool checkNoConsole() {
    if (GetConsoleWindow())
      return false;
    STARTUPINFO startupInfo;
    startupInfo.cb = sizeof(STARTUPINFO);
    GetStartupInfo(&startupInfo);
    return !(startupInfo.dwFlags & STARTF_USESTDHANDLES);
  }

  // check gui option without parsing
  bool checkNoGuiOption(const QStringList &args) {
    QStringList guiOptionNames = m_guiOption.names();
    QStringList guiOptionArgs;
    for (const QString &name : guiOptionNames) {
      guiOptionArgs << "-" + name;
      guiOptionArgs << "--" + name;
    }
    QStringList noGuiOptionNames = m_noGuiOption.names();
    QStringList noGuiOptionArgs;
    for (const QString &name : noGuiOptionNames) {
      noGuiOptionArgs << "-" + name;
      noGuiOptionArgs << "--" + name;
    }
    for (auto it = args.crbegin(); it != args.crend(); it++) {
      const QString &arg = *it;
      for (const QString &oarg : noGuiOptionArgs) {
        if (arg == oarg) {
          return true;
        }
      }
      for (const QString &oarg : guiOptionNames) {
        if (arg == oarg) {
          return false;
        }
      }
    }
    return false;
  }

  // check translate option without parsing
  bool checkTranslateOption(const QStringList &args) {
    QStringList translateOptionNames = m_translateOption.names();
    QStringList translateOptionArgs;
    for (const QString &name : translateOptionNames) {
      translateOptionArgs << "-" + name;
      translateOptionArgs << "--" + name;
    }
    QStringList noTranslateOptionNames = m_noTranslateOption.names();
    QStringList noTranslateOptionArgs;
    for (const QString &name : noTranslateOptionNames) {
      noTranslateOptionArgs << "-" + name;
      noTranslateOptionArgs << "--" + name;
    }
    for (auto it = args.crbegin(); it != args.crend(); it++) {
      const QString &arg = *it;
      for (const QString &oarg : translateOptionArgs) {
        if (arg == oarg) {
          return true;
        }
      }
      for (const QString &oarg : noTranslateOptionArgs) {
        if (arg == oarg) {
          return false;
        }
      }
    }
    return false;
  }

  // create app instance and set properties based on some flags
  QCoreApplication *createApplication(int &argc, char *argv[]) {
    QStringList args = createArguments(argc, argv);
    QStringList nonPosArgs = sliceBeforeDoubleDash(args);
    QCoreApplication *app = nullptr;
    bool noGui = checkNoGuiOption(nonPosArgs);
    bool noConsole = checkNoConsole();
    if (noGui && noConsole) {
      QString errorText = tr(
          "'%1' option is set but no console is attached, ignoring the option"
      );
      errorText = errorText.arg(m_noGuiOption.names().last());
      qWarning() << qPrintable(errorText);
      noGui = false;
    }
    if (noGui) {
      QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
      env.insert("QT_COMMAND_LINE_PARSER_NO_GUI_MESSAGE_BOXES", "1");
    }
    if (noGui) {
      app = new QApplication(argc, argv);
    } else {
      app = new QApplication(argc, argv);
    }
    app->setProperty("noGui", noGui);
    app->setProperty("noConsole", noConsole);
    return app;
  }

  // process arguments from app
  void process(QCoreApplication *app) {
    // prepare args
    QStringList args = app->arguments();
    QStringList nonPosArgs = sliceBeforeDoubleDash(args);
    // check args and install translator if needed
    if (checkTranslateOption(nonPosArgs)) {
      // create translator
      QTranslator *translator = new QTranslator(app);
      // check if installation was successful
      bool successful = false;
      // try loading translator for the current locale
      QLocale locale = QLocale::system();
      QString directory =
          ":/i18n" + QLibraryInfo::path(QLibraryInfo::TranslationsPath);
      if (translator->load(locale, "", "", directory)) {
        // try install translator
        if (QCoreApplication::installTranslator(translator)) {
          // reload parser descriptions after the installation
          initialize();
          // check as successful
          successful = true;
        }
      }
      // delete translator if installation was not successful
      if (!successful) {
        translator->deleteLater();
      }
    }
    // parse and process app arguments using inner parser
    m_parser->process(*app);
    // process logging level
    MessageHandlersManager *manager = MessageHandlersManager::instance();
    QString logLevel = m_parser->value(m_logLevelOption).toLower();
    if (!logLevel.isEmpty()) {
      if (logLevel == "debug") {
        manager->setMinimumMessageType(QtDebugMsg);
      } else if (logLevel == "info") {
        manager->setMinimumMessageType(QtInfoMsg);
      } else if (logLevel == "warning") {
        manager->setMinimumMessageType(QtWarningMsg);
      } else if (logLevel == "critical") {
        manager->setMinimumMessageType(QtCriticalMsg);
      } else if (logLevel == "fatal") {
        manager->setMinimumMessageType(QtFatalMsg);
      } else {
        QString errorText = tr("Unexpected log level value after '%1': %2.");
        errorText = errorText.arg("--" + m_logLevelOption.names().last());
        errorText = errorText.arg(m_parser->value(m_logLevelOption));
        QString helpText = m_parser->helpText();
        ::showParserErrorMessage(errorText + "\n\n" + helpText);
        ::exit(EXIT_FAILURE);
      }
    } else if (false) {
      if (m_parser->isSet(m_debugOption)) {
        manager->setMinimumMessageType(QtDebugMsg);
      } else if (m_parser->isSet(m_infoOption)) {
        manager->setMinimumMessageType(QtInfoMsg);
      } else if (m_parser->isSet(m_warningOption)) {
        manager->setMinimumMessageType(QtWarningMsg);
      } else if (m_parser->isSet(m_criticalOption)) {
        manager->setMinimumMessageType(QtCriticalMsg);
      } else if (m_parser->isSet(m_fatalOption)) {
        manager->setMinimumMessageType(QtFatalMsg);
      }
    }
    // check options and exit if required option values are not given
    bool noGui = QCoreApplication::instance()
        ? QCoreApplication::instance()->property("noGui").toBool()
        : false;
    QString classId = m_parser->value(m_classIdOption);
    QString addressUri = m_parser->value(m_addressUriOption);
    bool createTrayIcon = m_parser->isSet(m_createTrayIconOption);
    bool startHidden = m_parser->isSet(m_startHiddenOption);
    if ((noGui || startHidden) && (classId.isEmpty() || addressUri.isEmpty())) {
      QString errorText = tr("Both values of '%1' and '%2' should be given.");
      errorText = errorText.arg("--" + m_classIdOption.names().last());
      errorText = errorText.arg("--" + m_addressUriOption.names().last());
      QString helpText = m_parser->helpText();
      ::showParserErrorMessage(errorText + "\n\n" + helpText);
      ::exit(EXIT_FAILURE);
    }
  }

  // get parsed values as conf
  StartServerConfiguration conf() {
    QString classId = m_parser->value(m_classIdOption);
    QString addressUri = m_parser->value(m_addressUriOption);
    bool createTrayIcon = m_parser->isSet(m_createTrayIconOption);
    bool startHidden = m_parser->isSet(m_startHiddenOption);
    bool noGui = QCoreApplication::instance()
        ? QCoreApplication::instance()->property("noGui").toBool()
        : false;
    if (startHidden && !createTrayIcon) {
      QString errorText =
          tr("'%1' option is set without '%2', ignoring '%1' option");
      errorText = errorText.arg("--" + m_startHiddenOption.names().last());
      errorText = errorText.arg("--" + m_createTrayIconOption.names().last());
      qWarning() << qPrintable(errorText);
      startHidden = false;
    }
    if (noGui) {
      if (createTrayIcon) {
        QString errorText =
            tr("'%1' option is set with '%2', ignoring '%1' option");
        errorText = errorText.arg("--" + m_createTrayIconOption.names().last());
        errorText = errorText.arg("--" + m_noGuiOption.names().last());
        qWarning() << qPrintable(errorText);
        createTrayIcon = false;
      }
      startHidden = true;
    }
    return StartServerConfiguration(
        classId, addressUri, createTrayIcon, startHidden
    );
  }
};

int main(int argc, char *argv[]) {
  // initialize message handlers manager instance for logging
  MessageHandlersManager::instance();
  // set application properties
  QApplication::setOrganizationName(AXSERVE_ORG_NAME);
  QApplication::setOrganizationDomain(AXSERVE_ORG_DOMAIN);
  QApplication::setApplicationName(AXSERVE_APP_NAME);
  QApplication::setApplicationDisplayName(AXSERVE_APP_DISPLAY_NAME);
  QApplication::setApplicationVersion(AXSERVE_APP_VERSION);
  // create command line parser for parsing args
  CommandLineParser parser;
  // create application instance
  QScopedPointer<QCoreApplication> app(parser.createApplication(argc, argv));
  // process args
  parser.process(app.data());
  // get parser output
  StartServerConfiguration conf = parser.conf();
  // create main window
  MainWindow window;
  // send initial configuration
  window.onInitialStartRequest(conf);
  // exec app
  return app->exec();
}
