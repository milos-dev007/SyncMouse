#pragma once

#include <QObject>
#include <QPoint>
#include <QElapsedTimer>

#include "core/InputEvent.h"
#include "net/Server.h"

namespace syncmouse {

class InputCapture;
class PeerConnection;

class InputShareController : public QObject {
  Q_OBJECT
public:
  explicit InputShareController(syncmouse::Server* server, QObject* parent = nullptr);
  ~InputShareController() override;

  bool setEnabled(bool enabled, QString* error = nullptr);
  bool isEnabled() const { return enabled_; }

signals:
  void logMessage(const QString& message);

public slots:
  void requestReturn(syncmouse::PeerConnection* peer);

private slots:
  void onMouseMoved(const QPoint& pos, const QPoint& delta);
  void onMouseButton(syncmouse::MouseButton button, bool down);
  void onMouseWheel(int deltaX, int deltaY);
  void onKeyEvent(quint16 keyUsage, quint16 modifiers, bool down);
  void onClientDisconnected(syncmouse::PeerConnection* peer);

private:
  enum class Edge { None, Left, Right, Up, Down };

  Edge edgeForPosition(const QPoint& pos) const;
  syncmouse::PeerConnection* peerForEdge(Edge edge) const;
  Edge edgeFromClientPosition(syncmouse::ClientPosition position) const;
  void enterRemote(syncmouse::PeerConnection* peer, Edge edge);
  void exitRemote();
  void sendInput(const syncmouse::InputEvent& event);
  QPoint clampPointToEdge(const QPoint& pos, Edge edge) const;

  syncmouse::InputCapture* capture_ = nullptr;
  syncmouse::Server* server_ = nullptr;
  bool enabled_ = false;
  bool remoteActive_ = false;
  syncmouse::PeerConnection* activePeer_ = nullptr;
  Edge activeEdge_ = Edge::None;
  QElapsedTimer edgeCooldown_;
  int edgeCooldownMs_ = 300;
  QPoint lastPos_;
  bool hasLastPos_ = false;
};

} // namespace syncmouse
