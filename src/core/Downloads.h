#pragma once

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDateTime>

namespace syncmouse::downloads {

inline QString downloadsDir() {
  QString path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
  if (path.isEmpty()) {
    path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
  }
  return path;
}

inline QString uniquePath(const QString& fileName) {
  QDir dir(downloadsDir());
  QString base = QFileInfo(fileName).completeBaseName();
  QString ext = QFileInfo(fileName).suffix();

  QString candidate = dir.filePath(fileName);
  if (!QFileInfo::exists(candidate)) {
    return candidate;
  }

  for (int i = 1; i < 1000; ++i) {
    QString numbered = ext.isEmpty()
      ? QString("%1 (%2)").arg(base).arg(i)
      : QString("%1 (%2).%3").arg(base).arg(i).arg(ext);
    candidate = dir.filePath(numbered);
    if (!QFileInfo::exists(candidate)) {
      return candidate;
    }
  }

  return dir.filePath(QString("%1 (%2)").arg(base).arg(QDateTime::currentDateTimeUtc().toSecsSinceEpoch()));
}

} // namespace syncmouse::downloads
