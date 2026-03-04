#pragma once

#include "core/InputInjector.h"

#include <ApplicationServices/ApplicationServices.h>

namespace syncmouse {

class InputInjectorMac : public InputInjector {
  Q_OBJECT
public:
  explicit InputInjectorMac(QObject* parent = nullptr);
  void inject(const InputEvent& event) override;

private:
  CGPoint currentCursorPosition() const;
  void postMouseButton(MouseButton button, bool down, const CGPoint& pos);
};

} // namespace syncmouse
