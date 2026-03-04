#include "core/InputCaptureWin.h"

#include "core/KeyMap.h"

#include <QThread>

namespace syncmouse {

namespace {
InputCaptureWin* g_instance = nullptr;
} // namespace

InputCaptureWin::InputCaptureWin(QObject* parent)
  : InputCapture(parent) {}

InputCaptureWin::~InputCaptureWin() {
  stop();
}

bool InputCaptureWin::start(QString* error) {
  if (running_.load()) {
    return true;
  }

  running_.store(true);
  QThread* thread = QThread::create([this]() { runLoop(); });
  connect(thread, &QThread::finished, thread, &QObject::deleteLater);
  thread->start();

  emit logMessage(QStringLiteral("Input capture started (Windows)."));
  return true;
}

void InputCaptureWin::stop() {
  if (!running_.load()) {
    return;
  }

  running_.store(false);
  if (threadId_ != 0) {
    PostThreadMessage(threadId_, WM_QUIT, 0, 0);
  }

  emit logMessage(QStringLiteral("Input capture stopped (Windows)."));
}

void InputCaptureWin::setSuppressLocal(bool suppress) {
  suppress_.store(suppress);
}

void InputCaptureWin::warpCursorTo(const QPoint& pos) {
  ignoreNextMove_.store(true);
  SetCursorPos(pos.x(), pos.y());
  lastPos_ = pos;
  hasLastPos_ = true;
}

void InputCaptureWin::runLoop() {
  g_instance = this;
  threadId_ = GetCurrentThreadId();

  mouseHook_ = SetWindowsHookEx(WH_MOUSE_LL, &InputCaptureWin::mouseHookProc, nullptr, 0);
  keyHook_ = SetWindowsHookEx(WH_KEYBOARD_LL, &InputCaptureWin::keyboardHookProc, nullptr, 0);

  MSG msg;
  while (GetMessage(&msg, nullptr, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  if (mouseHook_) {
    UnhookWindowsHookEx(mouseHook_);
    mouseHook_ = nullptr;
  }
  if (keyHook_) {
    UnhookWindowsHookEx(keyHook_);
    keyHook_ = nullptr;
  }

  g_instance = nullptr;
  threadId_ = 0;
}

LRESULT CALLBACK InputCaptureWin::mouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (g_instance) {
    return g_instance->handleMouse(nCode, wParam, lParam);
  }
  return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LRESULT CALLBACK InputCaptureWin::keyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (g_instance) {
    return g_instance->handleKeyboard(nCode, wParam, lParam);
  }
  return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LRESULT InputCaptureWin::handleMouse(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode < 0) {
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
  }

  auto* info = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
  QPoint pos(static_cast<int>(info->pt.x), static_cast<int>(info->pt.y));

  if (wParam == WM_MOUSEMOVE) {
    if (ignoreNextMove_.load()) {
      ignoreNextMove_.store(false);
      lastPos_ = pos;
      hasLastPos_ = true;
      return suppress_.load() ? 1 : CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    QPoint delta(0, 0);
    if (hasLastPos_) {
      delta = pos - lastPos_;
    }
    lastPos_ = pos;
    hasLastPos_ = true;
    emit mouseMoved(pos, delta);
  } else if (wParam == WM_LBUTTONDOWN || wParam == WM_LBUTTONUP ||
             wParam == WM_RBUTTONDOWN || wParam == WM_RBUTTONUP ||
             wParam == WM_MBUTTONDOWN || wParam == WM_MBUTTONUP ||
             wParam == WM_XBUTTONDOWN || wParam == WM_XBUTTONUP) {
    bool down = (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN ||
                 wParam == WM_MBUTTONDOWN || wParam == WM_XBUTTONDOWN);
    MouseButton button = MouseButton::Left;
    if (wParam == WM_RBUTTONDOWN || wParam == WM_RBUTTONUP) {
      button = MouseButton::Right;
    } else if (wParam == WM_MBUTTONDOWN || wParam == WM_MBUTTONUP) {
      button = MouseButton::Middle;
    } else if (wParam == WM_XBUTTONDOWN || wParam == WM_XBUTTONUP) {
      const auto xButton = HIWORD(info->mouseData);
      button = (xButton == XBUTTON1) ? MouseButton::X1 : MouseButton::X2;
    }
    emit mouseButton(button, down);
  } else if (wParam == WM_MOUSEWHEEL) {
    const int delta = static_cast<short>(HIWORD(info->mouseData));
    emit mouseWheel(0, delta);
  } else if (wParam == WM_MOUSEHWHEEL) {
    const int delta = static_cast<short>(HIWORD(info->mouseData));
    emit mouseWheel(delta, 0);
  }

  return suppress_.load() ? 1 : CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LRESULT InputCaptureWin::handleKeyboard(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode < 0) {
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
  }

  auto* info = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
  const bool down = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
  const bool up = (wParam == WM_KEYUP || wParam == WM_SYSKEYUP);
  if (!down && !up) {
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
  }

  quint16 usage = usageFromPlatformKeycode(static_cast<quint16>(info->vkCode));
  if (usage != 0) {
    quint16 modifiers = 0;
    if (GetKeyState(VK_SHIFT) & 0x8000) modifiers |= ModShift;
    if (GetKeyState(VK_CONTROL) & 0x8000) modifiers |= ModCtrl;
    if (GetKeyState(VK_MENU) & 0x8000) modifiers |= ModAlt;
    if (GetKeyState(VK_LWIN) & 0x8000 || GetKeyState(VK_RWIN) & 0x8000) modifiers |= ModMeta;

    emit keyEvent(usage, modifiers, down);
  }

  return suppress_.load() ? 1 : CallNextHookEx(nullptr, nCode, wParam, lParam);
}

} // namespace syncmouse
