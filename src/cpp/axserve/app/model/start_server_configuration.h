#ifndef START_SERVER_CONFIGURATION_H
#define START_SERVER_CONFIGURATION_H

#include <QString>

class StartServerConfiguration {

private:
  QString m_control;
  QString m_address;

  bool m_createTrayIcon;
  bool m_startHidden;

public:
  StartServerConfiguration();
  StartServerConfiguration(
      const QString &control, const QString &address,
      bool createTrayIcon = false, bool startHidden = false
  );
  StartServerConfiguration(const StartServerConfiguration &other);

  QString control() const;
  QString address() const;

  bool createTrayIcon() const;
  bool startMinimized() const;
  bool startHidden() const;
};

#endif // START_SERVER_CONFIGURATION_H