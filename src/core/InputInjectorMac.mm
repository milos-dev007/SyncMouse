#include "core/InputInjectorMac.h"

#include "core/KeyMap.h"

#include <cmath>

namespace syncmouse {

InputInjectorMac::InputInjectorMac(QObject* parent)
  : InputInjector(parent) {}

CGPoint InputInjectorMac::currentCursorPosition() const {
  CGEventRef event = CGEventCreate(nullptr);
  CGPoint pos = CGEventGetLocation(event);
  CFRelease(event);
  return pos;
}

void InputInjectorMac::postMouseButton(MouseButton button, bool down, const CGPoint& pos) {
  CGMouseButton cgButton = kCGMouseButtonLeft;
  CGEventType type = down ? kCGEventLeftMouseDown : kCGEventLeftMouseUp;

  switch (button) {
    case MouseButton::Right:
      cgButton = kCGMouseButtonRight;
      type = down ? kCGEventRightMouseDown : kCGEventRightMouseUp;
      break;
    case MouseButton::Middle:
      cgButton = kCGMouseButtonCenter;
      type = down ? kCGEventOtherMouseDown : kCGEventOtherMouseUp;
      break;
    default:
      cgButton = kCGMouseButtonLeft;
      type = down ? kCGEventLeftMouseDown : kCGEventLeftMouseUp;
      break;
  }

  CGEventRef ev = CGEventCreateMouseEvent(nullptr, type, pos, cgButton);
  if (button == MouseButton::X1 || button == MouseButton::X2) {
    CGEventSetIntegerValueField(ev, kCGMouseEventButtonNumber, button == MouseButton::X1 ? 3 : 4);
  }
  CGEventPost(kCGHIDEventTap, ev);
  CFRelease(ev);
}

void InputInjectorMac::inject(const InputEvent& event) {
  if (!enabled_) {
    return;
  }

  switch (event.type) {
    case InputEventType::MouseMove: {
      CGPoint current = currentCursorPosition();
      const int screenWidth = static_cast<int>(CGDisplayPixelsWide(CGMainDisplayID()));
      const int screenHeight = static_cast<int>(CGDisplayPixelsHigh(CGMainDisplayID()));

      CGPoint target;
      const double dx = event.dx * mouseScale_;
      const double dy = event.dy * mouseScale_;
      target.x = current.x + std::lround(dx);
      target.y = current.y - std::lround(dy); // convert Qt (down) to CG (up)

      if (target.x < 0) target.x = 0;
      if (target.y < 0) target.y = 0;
      if (screenWidth > 0 && target.x > screenWidth - 1) {
        target.x = screenWidth - 1;
      }
      if (screenHeight > 0 && target.y > screenHeight - 1) {
        target.y = screenHeight - 1;
      }

      if (screenWidth > 0 && screenHeight > 0) {
        maybeRequestReturn(static_cast<int>(target.x), static_cast<int>(target.y),
                           0, 0, screenWidth - 1, screenHeight - 1, false);
      }

      CGEventRef move = CGEventCreateMouseEvent(nullptr, kCGEventMouseMoved, target, kCGMouseButtonLeft);
      CGEventPost(kCGHIDEventTap, move);
      CFRelease(move);
      break;
    }
    case InputEventType::MouseButton: {
      CGPoint pos = currentCursorPosition();
      postMouseButton(static_cast<MouseButton>(event.button), event.state == 1, pos);
      break;
    }
    case InputEventType::MouseWheel: {
      int linesY = event.dy / 120;
      int linesX = event.dx / 120;
      if (linesY == 0 && event.dy != 0) linesY = event.dy > 0 ? 1 : -1;
      if (linesX == 0 && event.dx != 0) linesX = event.dx > 0 ? 1 : -1;
      CGEventRef scroll = CGEventCreateScrollWheelEvent(nullptr, kCGScrollEventUnitLine, 2, linesY, linesX);
      CGEventPost(kCGHIDEventTap, scroll);
      CFRelease(scroll);
      break;
    }
    case InputEventType::Key: {
      quint16 keycode = platformKeycodeFromUsage(event.key);
      if (keycode == 0) {
        return;
      }
      CGEventRef key = CGEventCreateKeyboardEvent(nullptr, static_cast<CGKeyCode>(keycode), event.state == 1);
      CGEventPost(kCGHIDEventTap, key);
      CFRelease(key);
      break;
    }
    default:
      break;
  }
}

} // namespace syncmouse
