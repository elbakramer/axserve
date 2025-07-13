#include "server_log_widget.h"

ServerLogWidget::ServerLogWidget(QTextEdit *log, QWidget *parent)
    : QWidget(parent),
      m_logEdit(log) {
  if (m_logEdit == nullptr) {
    m_logEdit = new QTextEdit(this);
  }
  m_logEdit->setReadOnly(true);
  m_clearButton = new QPushButton(tr("Clear"), this);
  connect(m_clearButton, &QPushButton::clicked, m_logEdit, &QTextEdit::clear);
  m_layout = new QVBoxLayout(this);
  m_layout->addWidget(m_logEdit);
  m_layout->addWidget(m_clearButton);
}

void ServerLogWidget::setLogEdit(QTextEdit *edit) {
  m_layout->removeWidget(m_logEdit);
  m_logEdit->deleteLater();
  m_logEdit = edit;
  m_layout->insertWidget(0, m_logEdit);
}
