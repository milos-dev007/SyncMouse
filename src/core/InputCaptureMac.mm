#include "core/InputCaptureMac.h"

#include "core/KeyMap.h"

namespace syncmouse {

InputCaptureMac::InputCaptureMac(QObject* parent)
  : InputCapture(parent) {
  screenHeight_ = static_cast<int>(CGDisplayPixelsHigh(CGMainDisplayID()));
}

InputCaptureMac::~InputCaptureMac() {
  stop();
}

bool InputCaptureMac::start(QString* error) {
  if (tap_) {
    return true;
  }

  const void* keys[] = { kAXTrustedCheckOptionPrompt };
  const void* values[] = { kCFBooleanTrue };
  CFDictionaryRef options = CFDictionaryCreate(kCFAllocatorDefault,
                                              keys,
                                              values,
                                              1,
                                              &kCFTypeDictionaryKeyCallBacks,
                                              &kCFTypeDictionaryValueCallBacks);
  const bool trusted = AXIsProcessTrustedWithOptions(options);
  CFRelease(options);
  if (!trusted) {
    if (error) {
      *error = QStringLiteral("Accessibility permission required. macOS will show a prompt to enable SyncMouse.");
    }
    return false;
  }

  CGEventMask mask = CGEventMaskBit(kCGEventMouseMoved) |
                     CGEventMaskBit(kCGEventLeftMouseDown) |
                     CGEventMaskBit(kCGEventLeftMouseUp) |
                     CGEventMaskBit(kCGEventRightMouseDown) |
                     CGEventMaskBit(kCGEventRightMouseUp) |
                     CGEventMaskBit(kCGEventOtherMouseDown) |
                     CGEventMaskBit(kCGEventOtherMouseUp) |
                     CGEventMaskBit(kCGEventLeftMouseDragged) |
                     CGEventMaskBit(kCGEventRightMouseDragged) |
                     CGEventMaskBit(kCGEventOtherMouseDragged) |
                     CGEventMaskBit(kCGEventScrollWheel) |
                     CGEventMaskBit(kCGEventKeyDown) |
                     CGEventMaskBit(kCGEventKeyUp);

  tap_ = CGEventTapCreate(kCGSessionEventTap,
                          kCGHeadInsertEventTap,
                          kCGEventTapOptionDefault,
                          mask,
                          &InputCaptureMac::tapCallback,
                          this);
  if (!tap_) {
    if (error) {
      *error = QStringLiteral("Failed to create event tap. Enable Accessibility permissions for SyncMouse.");
    }
    return false;
  }

  source_ = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, tap_, 0);
  CFRunLoopAddSource(CFRunLoopGetMain(), source_, kCFRunLoopCommonModes);
  CGEventTapEnable(tap_, true);

  emit logMessage(QStringLiteral("Input capture started (macOS)."));
  return true;
}

void InputCaptureMac::stop() {
  if (!tap_) {
    return;
  }

  CGEventTapEnable(tap_, false);
  if (source_) {
    CFRunLoopRemoveSource(CFRunLoopGetMain(), source_, kCFRunLoopCommonModes);
    CFRelease(source_);
    source_ = nullptr;
  }
  CFRelease(tap_);
  tap_ = nullptr;
  emit logMessage(QStringLiteral("Input capture stopped (macOS)."));
}

void InputCaptureMac::setSuppressLocal(bool suppress) {
  suppress_ = suppress;
}

void InputCaptureMac::warpCursorTo(const QPoint& pos) {
  ignoreNextMove_ = true;
  const int screenHeight = static_cast<int>(CGDisplayPixelsHigh(CGMainDisplayID()));
  CGPoint cgPoint;
  cgPoint.x = pos.x();
  cgPoint.y = screenHeight - 1 - pos.y();
  CGWarpMouseCursorPosition(cgPoint);
}

CGEventRef InputCaptureMac::tapCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* refcon) {
  if (type == kCGEventTapDisabledByTimeout) {
    auto* self = static_cast<InputCaptureMac*>(refcon);
    if (self && self->tap_) {
      CGEventTapEnable(self->tap_, true);
    }
    return event;
  }

  auto* self = static_cast<InputCaptureMac*>(refcon);
  if (!self) {
    return event;
  }
  return self->handleEvent(type, event);
}

CGEventRef InputCaptureMac::handleEvent(CGEventType type, CGEventRef event) {
  if (type == kCGEventMouseMoved ||
      type == kCGEventLeftMouseDragged ||
      type == kCGEventRightMouseDragged ||
      type == kCGEventOtherMouseDragged) {
    if (ignoreNextMove_) {
      ignoreNextMove_ = false;
      return suppress_ ? nullptr : event;
    }

    CGPoint location = CGEventGetLocation(event);
    QPoint pos(static_cast<int>(location.x), screenHeight_ - 1 - static_cast<int>(location.y));

    int rawDx = static_cast<int>(CGEventGetIntegerValueField(event, kCGMouseEventDeltaX));
    int rawDy = static_cast<int>(CGEventGetIntegerValueField(event, kCGMouseEventDeltaY));
    QPoint delta(rawDx, rawDy);
    if (delta.isNull() && hasLastPos_) {
      delta = pos - lastPos_;
    }
    lastPos_ = pos;
    hasLastPos_ = true;

    emit mouseMoved(pos, delta);
    return suppress_ ? nullptr : event;
  }

  if (type == kCGEventLeftMouseDown || type == kCGEventLeftMouseUp ||
      type == kCGEventRightMouseDown || type == kCGEventRightMouseUp ||
      type == kCGEventOtherMouseDown || type == kCGEventOtherMouseUp) {
    bool down = (type == kCGEventLeftMouseDown || type == kCGEventRightMouseDown || type == kCGEventOtherMouseDown);
    MouseButton button = MouseButton::Left;
    if (type == kCGEventRightMouseDown || type == kCGEventRightMouseUp) {
      button = MouseButton::Right;
    } else if (type == kCGEventOtherMouseDown || type == kCGEventOtherMouseUp) {
      int number = static_cast<int>(CGEventGetIntegerValueField(event, kCGMouseEventButtonNumber));
      if (number == 2) {
        button = MouseButton::Middle;
      } else if (number == 3) {
        button = MouseButton::X1;
      } else if (number == 4) {
        button = MouseButton::X2;
      } else {
        button = MouseButton::X1;
      }
    }

    emit mouseButton(button, down);
    return suppress_ ? nullptr : event;
  }

  if (type == kCGEventScrollWheel) {
    int deltaY = static_cast<int>(CGEventGetIntegerValueField(event, kCGScrollWheelEventDeltaAxis1)) * 120;
    int deltaX = static_cast<int>(CGEventGetIntegerValueField(event, kCGScrollWheelEventDeltaAxis2)) * 120;
    emit mouseWheel(deltaX, deltaY);
    return suppress_ ? nullptr : event;
  }

  if (type == kCGEventKeyDown || type == kCGEventKeyUp) {
    bool down = (type == kCGEventKeyDown);
    const auto keycode = static_cast<quint16>(CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));
    quint16 usage = usageFromPlatformKeycode(keycode);
    if (usage == 0) {
      return suppress_ ? nullptr : event;
    }

    CGEventFlags flags = CGEventGetFlags(event);
    quint16 modifiers = 0;
    if (flags & kCGEventFlagMaskShift) modifiers |= ModShift;
    if (flags & kCGEventFlagMaskControl) modifiers |= ModCtrl;
    if (flags & kCGEventFlagMaskAlternate) modifiers |= ModAlt;
    if (flags & kCGEventFlagMaskCommand) modifiers |= ModMeta;

    emit keyEvent(usage, modifiers, down);
    return suppress_ ? nullptr : event;
  }

  return suppress_ ? nullptr : event;
}

} // namespace syncmouse
