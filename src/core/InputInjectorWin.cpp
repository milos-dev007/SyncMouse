#include "core/InputInjectorWin.h"

#include "core/KeyMap.h"

namespace syncmouse {

InputInjectorWin::InputInjectorWin(QObject* parent)
  : InputInjector(parent) {}

bool InputInjectorWin::isExtendedKey(quint16 vk) const {
  switch (vk) {
    case VK_INSERT:
    case VK_DELETE:
    case VK_HOME:
    case VK_END:
    case VK_PRIOR:
    case VK_NEXT:
    case VK_RIGHT:
    case VK_LEFT:
    case VK_UP:
    case VK_DOWN:
    case VK_RCONTROL:
    case VK_RMENU:
    case VK_DIVIDE:
    case VK_NUMLOCK:
      return true;
    default:
      return false;
  }
}

void InputInjectorWin::sendMouseButton(MouseButton button, bool down) {
  INPUT input = {};
  input.type = INPUT_MOUSE;

  switch (button) {
    case MouseButton::Left:
      input.mi.dwFlags = down ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
      break;
    case MouseButton::Right:
      input.mi.dwFlags = down ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
      break;
    case MouseButton::Middle:
      input.mi.dwFlags = down ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
      break;
    case MouseButton::X1:
    case MouseButton::X2:
      input.mi.dwFlags = down ? MOUSEEVENTF_XDOWN : MOUSEEVENTF_XUP;
      input.mi.mouseData = (button == MouseButton::X1) ? XBUTTON1 : XBUTTON2;
      break;
  }

  SendInput(1, &input, sizeof(INPUT));
}

void InputInjectorWin::sendKey(quint16 vk, bool down) {
  INPUT input = {};
  input.type = INPUT_KEYBOARD;
  input.ki.wVk = static_cast<WORD>(vk);
  if (!down) {
    input.ki.dwFlags |= KEYEVENTF_KEYUP;
  }
  if (isExtendedKey(vk)) {
    input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
  }

  SendInput(1, &input, sizeof(INPUT));
}

void InputInjectorWin::inject(const InputEvent& event) {
  if (!enabled_) {
    return;
  }

  switch (event.type) {
    case InputEventType::MouseMove: {
      INPUT input = {};
      input.type = INPUT_MOUSE;
      input.mi.dx = event.dx;
      input.mi.dy = event.dy;
      input.mi.dwFlags = MOUSEEVENTF_MOVE;
      SendInput(1, &input, sizeof(INPUT));
      break;
    }
    case InputEventType::MouseButton: {
      sendMouseButton(static_cast<MouseButton>(event.button), event.state == 1);
      break;
    }
    case InputEventType::MouseWheel: {
      if (event.dy != 0) {
        INPUT input = {};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_WHEEL;
        input.mi.mouseData = static_cast<DWORD>(event.dy);
        SendInput(1, &input, sizeof(INPUT));
      }
      if (event.dx != 0) {
        INPUT input = {};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_HWHEEL;
        input.mi.mouseData = static_cast<DWORD>(event.dx);
        SendInput(1, &input, sizeof(INPUT));
      }
      break;
    }
    case InputEventType::Key: {
      quint16 vk = platformKeycodeFromUsage(event.key);
      if (vk == 0) {
        return;
      }
      sendKey(vk, event.state == 1);
      break;
    }
    default:
      break;
  }
}

} // namespace syncmouse
