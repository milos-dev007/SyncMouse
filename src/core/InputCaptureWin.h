#pragma once

#include "core/InputCapture.h"

#include <atomic>
#include <windows.h>

namespace syncmouse {

class InputCaptureWin : public InputCapture {
  Q_OBJECT
public:
  explicit InputCaptureWin(QObject* parent = nullptr);
  ~InputCaptureWin() override;

  bool start(QString* error = nullptr) override;
  void stop() override;
  void setSuppressLocal(bool suppress) override;
  void warpCursorTo(const QPoint& pos) override;

private:
  void runLoop();
  LRESULT handleMouse(int nCode, WPARAM wParam, LPARAM lParam);
  LRESULT handleKeyboard(int nCode, WPARAM wParam, LPARAM lParam);

  static LRESULT CALLBACK mouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);
  static LRESULT CALLBACK keyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);

  std::atomic<bool> suppress_{false};
  std::atomic<bool> running_{false};
  std::atomic<bool> ignoreNextMove_{false};

  HHOOK mouseHook_ = nullptr;
  HHOOK keyHook_ = nullptr;
  DWORD threadId_ = 0;
  QPoint lastPos_;
  bool hasLastPos_ = false;
};

} // namespace syncmouse
