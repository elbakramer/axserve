#include "line_edit_with_history.h"

#include <algorithm>

#include <QCache>
#include <QCompleter>
#include <QDateTime>
#include <QFocusEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMap>
#include <QMouseEvent>
#include <QSettings>
#include <QStringListModel>
#include <QVariantHash>
#include <QWidget>

LineEditWithHistory::LineEditWithHistory(const QString &name, QWidget *parent)
    : LineEditWithHistory(name, 10, parent) {}
LineEditWithHistory::LineEditWithHistory(
    const QString &name, qsizetype size, QWidget *parent
)
    : QLineEdit(parent),
      m_name(name),
      m_completeOnFocus(true) {
  m_cache.setMaxCost(size);
  QStringListModel *model = new QStringListModel(this);
  QCompleter *completer = new QCompleter(model, this);
  completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
  setCompleter(completer);
  restoreHistory();
}

void LineEditWithHistory::saveHistory() {
  QVariantHash hash;
  for (const auto &key : m_cache.keys()) {
    hash[key] = m_cache[key]->toMSecsSinceEpoch();
  }
  QJsonDocument document(QJsonObject::fromVariantHash(hash));
  QSettings settings;
  settings.beginGroup("lineedit");
  settings.beginGroup(m_name);
  settings.setValue("history", document.toJson());
  settings.endGroup();
  settings.endGroup();
}

void LineEditWithHistory::updateCompleter() {
  QMap<qint64, QString> map;
  for (const auto &key : m_cache.keys()) {
    map[m_cache[key]->toMSecsSinceEpoch()] = key;
  }
  QStringList stringList = map.values();
  std::reverse(stringList.begin(), stringList.end());
  QCompleter *currentCompleter = completer();
  QStringListModel *completerModel =
      (QStringListModel *)currentCompleter->model();
  completerModel->setStringList(stringList);
}

void LineEditWithHistory::restoreHistory() {
  QSettings settings;
  settings.beginGroup("lineedit");
  settings.beginGroup(m_name);
  QByteArray data = settings.value("history", QByteArray()).toByteArray();
  settings.endGroup();
  settings.endGroup();
  QJsonDocument document = QJsonDocument::fromJson(data);
  QJsonObject o = document.object();
  for (auto it = o.constBegin(); it != o.constEnd(); ++it) {
    m_cache.insert(
        it.key(),
        new QDateTime(QDateTime::fromMSecsSinceEpoch(it.value().toInteger()))
    );
  }
  updateCompleter();
}

QString LineEditWithHistory::name() { return m_name; }

qsizetype LineEditWithHistory::maxSize() { return m_cache.maxCost(); }
void LineEditWithHistory::setMaxSize(qsizetype size) {
  m_cache.setMaxCost(size);
  emit maxSizeChanged(size);
}

bool LineEditWithHistory::completeOnFocus() { return m_completeOnFocus; }
void LineEditWithHistory::setCompleteOnFocus(bool completeOnFocus) {
  m_completeOnFocus = completeOnFocus;
  emit completeOnFocusChanged(completeOnFocus);
}

void LineEditWithHistory::addHistory(const QDateTime &time) {
  addHistory(text(), time);
}

void LineEditWithHistory::addHistory(
    const QString &text, const QDateTime &time
) {
  m_cache.insert(text, new QDateTime(QDateTime::currentDateTime()));
  updateCompleter();
  saveHistory();
}

void LineEditWithHistory::focusInEvent(QFocusEvent *e) {
  QLineEdit::focusInEvent(e);
  switch (e->reason()) {
  case Qt::ActiveWindowFocusReason:
  case Qt::PopupFocusReason:
    break;
  default: {
    if (m_completeOnFocus) {
      this->completer()->complete();
    }
  }
  }
}

void LineEditWithHistory::mousePressEvent(QMouseEvent *e) {
  QLineEdit::mousePressEvent(e);
  if (m_completeOnFocus) {
    this->completer()->complete();
  }
}