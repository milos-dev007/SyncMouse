#include "core/InputShareController.h"

#include "core/InputCapture.h"

#if defined(Q_OS_MAC)
#include "core/InputCaptureMac.h"
#elif defined(Q_OS_WIN)
#include "core/InputCaptureWin.h"
#endif

#include "net/PeerConnection.h"

#include <QGuiApplication>
#include <QScreen>

namespace syncmouse {

namespace {
constexpr quint16 kHotkeyUsage = 0x14; // HID 'Q'
} // namespace

InputShareController::InputShareController(Server* server, QObject* parent)
  : QObject(parent), server_(server) {
#if defined(Q_OS_MAC)
  capture_ = new InputCaptureMac(this);
#elif defined(Q_OS_WIN)
  capture_ = new InputCaptureWin(this);
#else
  capture_ = nullptr;
#endif

  if (capture_) {
    connect(capture_, &InputCapture::mouseMoved, this, &InputShareController::onMouseMoved);
    connect(capture_, &InputCapture::mouseButton, this, &InputShareController::onMouseButton);
    connect(capture_, &InputCapture::mouseWheel, this, &InputShareController::onMouseWheel);
    connect(capture_, &InputCapture::keyEvent, this, &InputShareController::onKeyEvent);
    connect(capture_, &InputCapture::logMessage, this, &InputShareController::logMessage);
  }

  connect(server_, &Server::clientDisconnected, this, &InputShareController::onClientDisconnected);
  edgeCooldown_.invalidate();
}

InputShareController::~InputShareController() {
  if (capture_) {
    capture_->stop();
  }
}

bool InputShareController::setEnabled(bool enabled, QString* error) {
  if (enabled_ == enabled) {
    return true;
  }

  enabled_ = enabled;
  if (!capture_) {
    if (error) {
      *error = QStringLiteral("Input capture is not supported on this platform.");
    }
    enabled_ = false;
    return false;
  }

  if (enabled_) {
    if (!capture_->start(error)) {
      enabled_ = false;
      return false;
    }
    capture_->setSuppressLocal(false);
  } else {
    capture_->setSuppressLocal(false);
    capture_->stop();
    exitRemote();
  }
  return true;
}

void InputShareController::onMouseMoved(const QPoint& pos, const QPoint& delta) {
  if (!enabled_ || !server_->isRunning()) {
    return;
  }

  lastPos_ = pos;
  hasLastPos_ = true;

  if (remoteActive_) {
    if (!activePeer_) {
      exitRemote();
      return;
    }

    InputEvent ev;
    ev.type = InputEventType::MouseMove;
    ev.dx = delta.x();
    ev.dy = delta.y();
    sendInput(ev);

    if (capture_) {
      QPoint clamp = clampPointToEdge(pos, activeEdge_);
      if (clamp != pos) {
        capture_->warpCursorTo(clamp);
      }
    }
    return;
  }

  if (edgeCooldown_.isValid() && edgeCooldown_.elapsed() < edgeCooldownMs_) {
    return;
  }

  Edge edge = edgeForPosition(pos);
  if (edge == Edge::None) {
    return;
  }

  PeerConnection* peer = peerForEdge(edge);
  if (!peer || !peer->isConnected()) {
    return;
  }

  enterRemote(peer, edge);
}

void InputShareController::onMouseButton(MouseButton button, bool down) {
  if (!enabled_ || !remoteActive_ || !activePeer_) {
    return;
  }

  InputEvent ev;
  ev.type = InputEventType::MouseButton;
  ev.button = static_cast<quint8>(button);
  ev.state = down ? 1 : 0;
  sendInput(ev);
}

void InputShareController::onMouseWheel(int deltaX, int deltaY) {
  if (!enabled_ || !remoteActive_ || !activePeer_) {
    return;
  }

  InputEvent ev;
  ev.type = InputEventType::MouseWheel;
  ev.dx = deltaX;
  ev.dy = deltaY;
  sendInput(ev);
}

void InputShareController::onKeyEvent(quint16 keyUsage, quint16 modifiers, bool down) {
  if (!enabled_ || !remoteActive_ || !activePeer_) {
    return;
  }

  if (down && keyUsage == kHotkeyUsage &&
      (modifiers & (ModCtrl | ModShift)) == (ModCtrl | ModShift)) {
    exitRemote();
    emit logMessage(QStringLiteral("Input sharing stopped (hotkey)."));
    return;
  }

  InputEvent ev;
  ev.type = InputEventType::Key;
  ev.key = keyUsage;
  ev.modifiers = modifiers;
  ev.state = down ? 1 : 0;
  sendInput(ev);
}

void InputShareController::onClientDisconnected(PeerConnection* peer) {
  if (peer && peer == activePeer_) {
    exitRemote();
  }
}

void InputShareController::requestReturn(PeerConnection* peer) {
  if (!remoteActive_) {
    return;
  }
  if (peer && peer != activePeer_) {
    return;
  }
  exitRemote();
  emit logMessage(QStringLiteral("Input sharing stopped (client return)."));
}

InputShareController::Edge InputShareController::edgeForPosition(const QPoint& pos) const {
  QScreen* screen = QGuiApplication::primaryScreen();
  if (!screen) {
    return Edge::None;
  }

  const QRect rect = screen->geometry();
  if (pos.x() <= rect.left()) {
    return Edge::Left;
  }
  if (pos.x() >= rect.right()) {
    return Edge::Right;
  }
  if (pos.y() <= rect.top()) {
    return Edge::Up;
  }
  if (pos.y() >= rect.bottom()) {
    return Edge::Down;
  }
  return Edge::None;
}

PeerConnection* InputShareController::peerForEdge(Edge edge) const {
  if (!server_) {
    return nullptr;
  }

  const auto clients = server_->clients();
  for (const auto& info : clients) {
    if (!info.peer) {
      continue;
    }
    if (edgeFromClientPosition(info.position) == edge) {
      return info.peer;
    }
  }
  return nullptr;
}

InputShareController::Edge InputShareController::edgeFromClientPosition(ClientPosition position) const {
  switch (position) {
    case ClientPosition::Left:
      return Edge::Left;
    case ClientPosition::Right:
      return Edge::Right;
    case ClientPosition::Up:
      return Edge::Up;
    case ClientPosition::Down:
      return Edge::Down;
    default:
      return Edge::None;
  }
}

void InputShareController::enterRemote(PeerConnection* peer, Edge edge) {
  activePeer_ = peer;
  activeEdge_ = edge;
  remoteActive_ = true;

  if (capture_) {
    capture_->setSuppressLocal(true);
    QPoint clamp = clampPointToEdge(hasLastPos_ ? lastPos_ : QPoint(), edge);
    capture_->warpCursorTo(clamp);
  }

  emit logMessage(QStringLiteral("Input sharing started. Move to server edge or press Ctrl+Shift+Q to return."));
}

void InputShareController::exitRemote() {
  if (!remoteActive_) {
    return;
  }

  remoteActive_ = false;
  activePeer_ = nullptr;
  activeEdge_ = Edge::None;

  if (capture_) {
    capture_->setSuppressLocal(false);
  }

  edgeCooldown_.restart();
}

void InputShareController::sendInput(const InputEvent& event) {
  if (!activePeer_) {
    return;
  }
  activePeer_->sendInputEvent(event);
}

QPoint InputShareController::clampPointToEdge(const QPoint& pos, Edge edge) const {
  QScreen* screen = QGuiApplication::primaryScreen();
  if (!screen) {
    return pos;
  }

  QRect rect = screen->geometry();
  QPoint result = pos;
  switch (edge) {
    case Edge::Left:
      result.setX(rect.left());
      break;
    case Edge::Right:
      result.setX(rect.right());
      break;
    case Edge::Up:
      result.setY(rect.top());
      break;
    case Edge::Down:
      result.setY(rect.bottom());
      break;
    default:
      break;
  }
  return result;
}

} // namespace syncmouse
