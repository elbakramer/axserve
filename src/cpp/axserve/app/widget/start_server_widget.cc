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

#include "start_server_widget.h"

#include <Qt>
#include <QtDebug>
#include <QtLogging>

#include <QCheckBox>
#include <QCoreApplication>
#include <QFontMetrics>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMargins>
#include <QMessageBox>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "axserve/common/util/address_uri_validator.h"
#include "axserve/common/util/clsid_validator.h"

StartServerWidget::StartServerWidget(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f) {

  // create server group box
  QGroupBox *serverGroupBox = new QGroupBox(this);
  QFormLayout *serverFormLayout = new QFormLayout(serverGroupBox);
  QString serverGroupBoxTitle = tr("Server");
  serverGroupBox->setTitle(serverGroupBoxTitle);
  serverGroupBox->setLayout(serverFormLayout);

  // create class id line edit
  QLabel *classIdLabel = new QLabel(serverGroupBox);
  LineEditWithHistory *classIdLineEdit =
      new LineEditWithHistory("control", serverGroupBox);
  QString classIdLabelText = tr("&CLSID:");
  QString classIdPlaceholderText = "{01234567-89AB-CDEF-0123-456789ABCDEF}";
  QFontMetrics classIdFontMetrics = classIdLineEdit->fontMetrics();
  QMargins classIdTextMargins = classIdLineEdit->textMargins();
  int classIdMinimumWidth = classIdTextMargins.left() +
      classIdFontMetrics.horizontalAdvance(classIdPlaceholderText) +
      classIdTextMargins.right() + 16;
  classIdLabel->setText(classIdLabelText);
  classIdLabel->setBuddy(classIdLineEdit);
  classIdLineEdit->setPlaceholderText(classIdPlaceholderText);
  classIdLineEdit->setMinimumWidth(classIdMinimumWidth);
  QString classIdToolTip = tr(R"TOOLTIP(
This string may contain the string form of a CLSID, contained in braces, e.g., "{9DBAFCCF-592F-101B-85CE-00608CEC297B}".
Alternatively, the string may contain the COM/OCX "short name" (ProgID) for the class, e.g., "CIRC3.Circ3Ctrl.1".
The name needs to match the same name registered by the control.
)TOOLTIP");
  classIdToolTip = classIdToolTip.trimmed();
  classIdLineEdit->setToolTip(classIdToolTip);
  QValidator *classIdValidator = new CLSIDValidator(classIdLineEdit);
  classIdLineEdit->setValidator(classIdValidator);

  // create address uri line edit
  QLabel *addressUriLabel = new QLabel(serverGroupBox);
  LineEditWithHistory *addressUriLineEdit =
      new LineEditWithHistory("address", serverGroupBox);
  QString addressUriLabelText = tr("&Address URI:");
  QString addressUriPlacehodlerText = "127.0.0.1:8080";
  QFontMetrics addressUriFontMetrics = addressUriLineEdit->fontMetrics();
  QMargins addressUriTextMargins = addressUriLineEdit->textMargins();
  int addressUriMinimumWidth = addressUriTextMargins.left() +
      addressUriFontMetrics.horizontalAdvance(addressUriPlacehodlerText) +
      addressUriTextMargins.right() + 16;
  addressUriLabel->setText(addressUriLabelText);
  addressUriLabel->setBuddy(addressUriLineEdit);
  addressUriLineEdit->setPlaceholderText(addressUriPlacehodlerText);
  addressUriLineEdit->setMinimumWidth(addressUriMinimumWidth);
  QString addressUriToolTip = tr(R"TOOLTIP(
The address to try to bind to the server in URI form.
If the scheme name is omitted, "dns:///" is assumed.
To bind to any address, please use IPv6 any, i.e., [::]:<port>, which also accepts IPv4 connections.
Valid values include dns:///localhost:1234, 192.168.1.1:31416, dns:///[::1]:27182, etc.
)TOOLTIP");
  addressUriToolTip = addressUriToolTip.trimmed();
  addressUriLineEdit->setToolTip(addressUriToolTip);
  QValidator *addressUriValidator = new AddressURIValidator(addressUriLineEdit);
  addressUriLineEdit->setValidator(addressUriValidator);

  // add rows to server group box
  serverFormLayout->addRow(classIdLabel, classIdLineEdit);
  serverFormLayout->addRow(addressUriLabel, addressUriLineEdit);

  // create window group box
  QGroupBox *windowGroupBox = new QGroupBox(this);
  QVBoxLayout *windowVboxLayout = new QVBoxLayout(windowGroupBox);
  QString windowGroupBoxTitle = tr("Window");
  windowGroupBox->setTitle(windowGroupBoxTitle);
  windowGroupBox->setLayout(windowVboxLayout);

  // create create-tray-icon check box
  QCheckBox *createTrayIconCheckBox = new QCheckBox(windowGroupBox);
  QString createTrayIconText = tr("Create system &tray icon");
  createTrayIconCheckBox->setText(createTrayIconText);
  QString createTrayIconToolTip = tr(R"TOOLTIP(
Create a system tray icon for managing this process.
The process will stay alive until the "Exit" button is clicked via the system tray icon.
Pressing close button of the existing dialog will only hide the dialog, not stopping the process running.
)TOOLTIP");
  createTrayIconToolTip = createTrayIconToolTip.trimmed();
  createTrayIconCheckBox->setToolTip(createTrayIconToolTip);

  // create start-hidden check box
  QCheckBox *startHiddenCheckBox = new QCheckBox(windowGroupBox);
  QString startHiddenText = tr("Start &hidden");
  startHiddenCheckBox->setText(startHiddenText);
  QString startHiddenToolTip = tr(R"TOOLTIP(
Start with the dialog hidden if created with a system tray icon.
)TOOLTIP");
  startHiddenToolTip = startHiddenToolTip.trimmed();
  startHiddenCheckBox->setToolTip(startHiddenToolTip);
  startHiddenCheckBox->setEnabled(createTrayIconCheckBox->isChecked());
  connect(
      createTrayIconCheckBox, &QCheckBox::stateChanged, startHiddenCheckBox,
      &QCheckBox::setEnabled
  );

  // add rows to window group box
  windowVboxLayout->addWidget(createTrayIconCheckBox);
  windowVboxLayout->addWidget(startHiddenCheckBox);

  // create start button
  QPushButton *startPushButton = new QPushButton(this);
  QString startPushButtonText = tr("&Start");
  startPushButton->setText(startPushButtonText);
  connect(
      startPushButton, &QPushButton::clicked, this,
      &StartServerWidget::onStartButtonClick
  );

  // create main layout and add boxes and buttons
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->addWidget(serverGroupBox);
  mainLayout->addWidget(windowGroupBox);
  mainLayout->addWidget(startPushButton);
  setLayout(mainLayout);

  // initialize members
  m_classIdLineEdit = classIdLineEdit;
  m_addressUriLineEdit = addressUriLineEdit;
  m_createTrayIconCheckBox = createTrayIconCheckBox;
  m_startHiddenCheckBox = startHiddenCheckBox;
}

void StartServerWidget::onInitialStartRequest(
    const StartServerConfiguration &conf
) {
  m_classIdLineEdit->setText(conf.control());
  m_addressUriLineEdit->setText(conf.address());
  m_createTrayIconCheckBox->setChecked(conf.createTrayIcon());
  m_startHiddenCheckBox->setChecked(conf.startHidden());
}

bool StartServerWidget::onStartButtonClick() {
  {
    // check gui availability and app title for printing errors
    QCoreApplication *app = QCoreApplication::instance();
    bool noGui = false;
    QString title;
    if (app)
      noGui = app->property("noGui").toBool();
    if (!noGui) {
      if (app)
        title = app->property("applicationDisplayName").toString();
      if (title.isEmpty()) {
        title = QCoreApplication::applicationName();
      }
    }

    // print error if given clsid is invalid
    if (!m_classIdLineEdit->hasAcceptableInput()) {
      QString message = tr("Given CLSID is invalid.");
      qWarning() << qPrintable(message);
      if (!noGui) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle(title);
        msgBox.setText(message);
        msgBox.exec();
      }
      return false;
    }

    // print error if given address uri is invalid
    if (!m_addressUriLineEdit->hasAcceptableInput()) {
      QString message = tr("Given address URI is invalid.");
      qWarning() << qPrintable(message);
      if (!noGui) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle(title);
        msgBox.setText(message);
        msgBox.exec();
      }
      return false;
    }
  }

  // get values from edits and checkboxes
  // and send start signal to main window
  QString control = m_classIdLineEdit->text();
  QString address = m_addressUriLineEdit->text();
  bool createTrayIcon = m_createTrayIconCheckBox->isChecked();
  bool startHidden =
      m_startHiddenCheckBox->isEnabled() && m_startHiddenCheckBox->isChecked();
  StartServerConfiguration conf(control, address, createTrayIcon, startHidden);
  emit startRequested(conf);
  return true;
}

void StartServerWidget::addLineEditHistory(
    const QString &classId, const QString &addressUri
) {
  QDateTime current = QDateTime::currentDateTime();
  m_classIdLineEdit->addHistory(classId, current);
  m_addressUriLineEdit->addHistory(addressUri, current);
}
