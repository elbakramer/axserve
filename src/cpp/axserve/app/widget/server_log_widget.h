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

#ifndef SERVER_LOG_WIDGET_H
#define SERVER_LOG_WIDGET_H

#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

class ServerLogWidget : public QWidget {
  Q_OBJECT

public:
  ServerLogWidget(QTextEdit *log = nullptr, QWidget *parent = nullptr);

private:
  QVBoxLayout *m_layout;
  QTextEdit *m_logEdit;
  QPushButton *m_clearButton;

public:
  void setLogEdit(QTextEdit *edit);
};

#endif // SERVER_LOG_WIDGET_H
