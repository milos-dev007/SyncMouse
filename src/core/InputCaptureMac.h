#pragma once

#include "core/InputCapture.h"

#include <ApplicationServices/ApplicationServices.h>

namespace syncmouse {

class InputCaptureMac : public InputCapture {
  Q_OBJECT
public:
  explicit InputCaptureMac(QObject* parent = nullptr);
  ~InputCaptureMac() override;

  bool start(QString* error = nullptr) override;
  void stop() override;
  void setSuppressLocal(bool suppress) override;
  void warpCursorTo(const QPoint& pos) override;

private:
  static CGEventRef tapCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* refcon);
  CGEventRef handleEvent(CGEventType type, CGEventRef event);

  bool suppress_ = false;
  bool ignoreNextMove_ = false;
  bool hasLastPos_ = false;
  QPoint lastPos_;
  int screenHeight_ = 0;

  CFMachPortRef tap_ = nullptr;
  CFRunLoopSourceRef source_ = nullptr;
};

} // namespace syncmouse
