#pragma once

#include <QObject>
#include <QPoint>

#include "core/InputEvent.h"

namespace syncmouse {

class InputCapture : public QObject {
  Q_OBJECT
public:
  explicit InputCapture(QObject* parent = nullptr) : QObject(parent) {}
  ~InputCapture() override = default;

  virtual bool start(QString* error = nullptr) = 0;
  virtual void stop() = 0;
  virtual void setSuppressLocal(bool suppress) = 0;
  virtual void warpCursorTo(const QPoint& pos) = 0;

signals:
  void mouseMoved(const QPoint& pos, const QPoint& delta);
  void mouseButton(syncmouse::MouseButton button, bool down);
  void mouseWheel(int deltaX, int deltaY);
  void keyEvent(quint16 keyUsage, quint16 modifiers, bool down);
  void logMessage(const QString& message);
};

} // namespace syncmouse
