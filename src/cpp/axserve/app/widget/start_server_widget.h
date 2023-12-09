/*
 * Copyright 2023 Yunseong Hwang
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef START_SERVER_WIDGET_H
#define START_SERVER_WIDGET_H

#include <Qt>

#include <QCache>
#include <QCheckBox>
#include <QDateTime>
#include <QLineEdit>
#include <QString>
#include <QWidget>

#include "axserve/app/model/start_server_configuration.h"
#include "axserve/common/widget/line_edit_with_history.h"

class StartServerWidget : public QWidget {
  Q_OBJECT

public:
  StartServerWidget(
      QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags()
  );

private:
  LineEditWithHistory *m_classIdLineEdit;
  LineEditWithHistory *m_addressUriLineEdit;

  QCheckBox *m_createTrayIconCheckBox;
  QCheckBox *m_startHiddenCheckBox;

public slots:
  void onInitialStartRequest(const StartServerConfiguration &conf);
  bool onStartButtonClick();
  void addLineEditHistory(const QString &classId, const QString &addressUri);

signals:
  void startRequested(const StartServerConfiguration &conf);
};

#endif // START_SERVER_WIDGET_H
