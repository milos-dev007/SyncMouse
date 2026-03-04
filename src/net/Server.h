#pragma once

#include <QObject>
#include <QVector>

#include "core/ClientPosition.h"

class QTcpServer;

namespace syncmouse {

class PeerConnection;

struct ClientInfo {
  PeerConnection* peer = nullptr;
  ClientPosition position = ClientPosition::Unknown;
};

class Server : public QObject {
  Q_OBJECT
public:
  explicit Server(QObject* parent = nullptr);
  ~Server() override;

  bool start(quint16 port, QString* error = nullptr);
  void stop();
  bool isRunning() const;

  QVector<ClientInfo> clients() const;
  void setClientPosition(PeerConnection* peer, ClientPosition position);

signals:
  void clientConnected(syncmouse::PeerConnection* peer);
  void clientDisconnected(syncmouse::PeerConnection* peer);
  void logMessage(const QString& message);

private slots:
  void onNewConnection();
  void onPeerDisconnected(syncmouse::PeerConnection* peer);

private:
  QTcpServer* server_ = nullptr;
  QVector<ClientInfo> clients_;
};

} // namespace syncmouse
