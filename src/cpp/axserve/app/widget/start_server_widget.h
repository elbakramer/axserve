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
