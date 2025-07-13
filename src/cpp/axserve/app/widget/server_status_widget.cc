#include "server_status_widget.h"

#include <QFormLayout>

ServerStatusWidget::ServerStatusWidget(QWidget *parent)
    : QWidget(parent) {
  QFormLayout *layout = new QFormLayout(this);
  m_statusLabel = new QLabel("Stopped", this);
  layout->addRow("Status:", m_statusLabel);
}

void ServerStatusWidget::setStatus(ServerStatus status) {
  switch (status) {
  case ServerStatus::Stopped:
    m_statusLabel->setText("Stopped");
    break;
  case ServerStatus::Starting:
    m_statusLabel->setText("Starting...");
    break;
  case ServerStatus::Running:
    m_statusLabel->setText("Running");
    break;
  case ServerStatus::Stopping:
    m_statusLabel->setText("Stopping...");
    break;
  case ServerStatus::Error:
    m_statusLabel->setText("Error!");
    break;
  }
}
