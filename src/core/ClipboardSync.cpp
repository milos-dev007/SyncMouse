#include "core/ClipboardSync.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QTimer>

namespace syncmouse {

ClipboardSync::ClipboardSync(QObject* parent)
  : QObject(parent) {
  clipboard_ = QGuiApplication::clipboard();
  if (clipboard_) {
    connect(clipboard_, &QClipboard::dataChanged, this, &ClipboardSync::onClipboardDataChanged);
  }
}

void ClipboardSync::setEnabled(bool enabled) {
  enabled_ = enabled;
}

bool ClipboardSync::isEnabled() const {
  return enabled_;
}

void ClipboardSync::applyRemoteText(const QString& text) {
  if (!clipboard_ || !enabled_) {
    return;
  }
  suppressNext_ = true;
  clipboard_->setText(text);
  QTimer::singleShot(100, this, [this]() { suppressNext_ = false; });
}

void ClipboardSync::onClipboardDataChanged() {
  if (!clipboard_ || !enabled_ || suppressNext_) {
    return;
  }

  const QString text = clipboard_->text();
  if (!text.isEmpty()) {
    emit localClipboardChanged(text);
  }
}

} // namespace syncmouse
