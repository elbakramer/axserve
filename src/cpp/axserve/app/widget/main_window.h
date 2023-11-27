#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <Qt>

#include <QCloseEvent>
#include <QMainWindow>
#include <QStackedWidget>
#include <QSystemTrayIcon>
#include <QWidget>

#include "axserve/app/model/start_server_configuration.h"
#include "axserve/app/widget/running_server_widget.h"
#include "axserve/app/widget/start_server_widget.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(
      QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags()
  );

private:
  QStackedWidget *m_central;
  QSystemTrayIcon *m_trayIcon;

  StartServerWidget *m_start;
  RunningServerWidget *m_running;

protected:
  void closeEvent(QCloseEvent *event) override;

public slots:
  void onInitialStartRequest(const StartServerConfiguration &conf);
  void onStartRequest(const StartServerConfiguration &conf);
  void onTrayIconActivate(QSystemTrayIcon::ActivationReason reason);
};

#endif