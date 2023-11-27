#ifndef LINE_EDIT_WITH_HISTORY_H
#define LINE_EDIT_WITH_HISTORY_H

#include <QCache>
#include <QDateTime>
#include <QFocusEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QString>
#include <QWidget>

class LineEditWithHistory : public QLineEdit {
  Q_OBJECT

  Q_PROPERTY(QString name READ name CONSTANT)
  Q_PROPERTY(qsizetype maxSize READ maxSize WRITE setMaxSize NOTIFY
                 maxSizeChanged)
  Q_PROPERTY(bool completeOnFocus READ completeOnFocus WRITE setCompleteOnFocus
                 NOTIFY completeOnFocusChanged)

private:
  QString m_name;
  QCache<QString, QDateTime> m_cache;
  bool m_completeOnFocus;

public:
  LineEditWithHistory(const QString &name, QWidget *parent = nullptr);
  LineEditWithHistory(
      const QString &name, qsizetype size, QWidget *parent = nullptr
  );

private:
  void updateCompleter();
  void saveHistory();
  void restoreHistory();

public:
  QString name();
  qsizetype maxSize();
  void setMaxSize(qsizetype size);
  bool completeOnFocus();
  void setCompleteOnFocus(bool completeOnFocus);
  void addHistory(const QDateTime &time = QDateTime());
  void addHistory(const QString &text, const QDateTime &time = QDateTime());

protected:
  void focusInEvent(QFocusEvent *e) override;
  void mousePressEvent(QMouseEvent *e) override;

signals:
  void maxSizeChanged(qsizetype size);
  void completeOnFocusChanged(bool completeOnFocus);
};

#endif // LINE_EDIT_WITH_HISTORY_H