#pragma once

#include "core/InputInjector.h"

#include <windows.h>

namespace syncmouse {

class InputInjectorWin : public InputInjector {
  Q_OBJECT
public:
  explicit InputInjectorWin(QObject* parent = nullptr);
  void inject(const InputEvent& event) override;

private:
  void sendMouseButton(MouseButton button, bool down);
  void sendKey(quint16 vk, bool down);
  bool isExtendedKey(quint16 vk) const;
  void sendAbsoluteMove(int dx, int dy);

  bool hasPos_ = false;
  double cursorX_ = 0.0;
  double cursorY_ = 0.0;
};

} // namespace syncmouse
