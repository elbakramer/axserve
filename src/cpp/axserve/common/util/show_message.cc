#include "show_message.h"

#include <qcoreapplication.h>
#include <qlatin1stringview.h>
#include <qstring.h>
#include <qsystemdetection.h>
#include <qtconfigmacros.h>
#include <qvariant.h>
#include <stdio.h>
#include <stdlib.h>

#include <QMessageBox>
#include <QtLogging>

QT_BEGIN_NAMESPACE

void showMessage(const QString &message, MessageType type) {
  bool noGui = false;
  bool noConsole = false;
  QCoreApplication *app = QCoreApplication::instance();
  if (app) {
    noGui = app->property("noGui").toBool();
    noConsole = app->property("noConsole").toBool();
  }
  if (!noGui) {
    QString title;
    if (app)
      title = app->property("applicationDisplayName").toString();
    if (title.isEmpty())
      title = QCoreApplication::applicationName();
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    msgBox.exec();
  }
  if (!noConsole) {
    fputs(qPrintable(message), stderr);
    qDebug();
  }
}

QT_END_NAMESPACE
