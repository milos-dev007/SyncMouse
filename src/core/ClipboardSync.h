#pragma once

#include <QObject>
#include <QString>

class QClipboard;

namespace syncmouse {

class ClipboardSync : public QObject {
  Q_OBJECT
public:
  explicit ClipboardSync(QObject* parent = nullptr);

  void setEnabled(bool enabled);
  bool isEnabled() const;

  void applyRemoteText(const QString& text);

signals:
  void localClipboardChanged(const QString& text);

private slots:
  void onClipboardDataChanged();

private:
  QClipboard* clipboard_ = nullptr;
  bool enabled_ = true;
  bool suppressNext_ = false;
};

} // namespace syncmouse
