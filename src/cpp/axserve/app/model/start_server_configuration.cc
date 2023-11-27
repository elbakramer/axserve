#include "start_server_configuration.h"

StartServerConfiguration::StartServerConfiguration() {}
StartServerConfiguration::StartServerConfiguration(
    const QString &control, const QString &address, bool createTrayIcon,
    bool startHidden
)
    : m_control(control),
      m_address(address),
      m_createTrayIcon(createTrayIcon),
      m_startHidden(startHidden) {}
StartServerConfiguration::StartServerConfiguration(
    const StartServerConfiguration &other
)
    : StartServerConfiguration(
          other.control(), other.address(), other.createTrayIcon(),
          other.startHidden()
      ) {}

QString StartServerConfiguration::control() const { return m_control; }
QString StartServerConfiguration::address() const { return m_address; }

bool StartServerConfiguration::createTrayIcon() const {
  return m_createTrayIcon;
}
bool StartServerConfiguration::startHidden() const { return m_startHidden; }