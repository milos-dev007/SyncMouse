#include "net/Server.h"

#include "net/PeerConnection.h"

#include <QHostAddress>
#include <QTcpServer>
#include <QTcpSocket>

namespace syncmouse {

Server::Server(QObject* parent)
  : QObject(parent), server_(new QTcpServer(this)) {
  connect(server_, &QTcpServer::newConnection, this, &Server::onNewConnection);
}

Server::~Server() {
  stop();
}

bool Server::start(quint16 port, QString* error) {
  if (server_->isListening()) {
    server_->close();
  }

  if (!server_->listen(QHostAddress::Any, port)) {
    if (error) {
      *error = server_->errorString();
    }
    return false;
  }

  emit logMessage(QString("Server listening on port %1").arg(port));
  return true;
}

void Server::stop() {
  if (!server_->isListening()) {
    return;
  }

  server_->close();
  for (const ClientInfo& info : clients_) {
    if (info.peer && info.peer->socket()) {
      info.peer->socket()->disconnectFromHost();
    }
    if (info.peer) {
      info.peer->deleteLater();
    }
  }
  clients_.clear();
  emit logMessage("Server stopped");
}

bool Server::isRunning() const {
  return server_->isListening();
}

QVector<ClientInfo> Server::clients() const {
  return clients_;
}

void Server::setClientPosition(PeerConnection* peer, ClientPosition position) {
  for (ClientInfo& info : clients_) {
    if (info.peer == peer) {
      info.position = position;
      if (info.peer) {
        info.peer->sendClientPosition(position);
      }
      emit logMessage(QString("Client %1 position set to %2")
        .arg(peer->peerId())
        .arg(static_cast<int>(position)));
      return;
    }
  }
}

void Server::onNewConnection() {
  while (server_->hasPendingConnections()) {
    QTcpSocket* socket = server_->nextPendingConnection();
    auto* peer = new PeerConnection(socket, this);

    connect(peer, &PeerConnection::disconnected, this, &Server::onPeerDisconnected);
    connect(peer, &PeerConnection::logMessage, this, &Server::logMessage);

    clients_.push_back({peer, ClientPosition::Unknown});
    emit logMessage(QString("Client connected: %1").arg(peer->peerId()));
    emit clientConnected(peer);
  }
}

void Server::onPeerDisconnected(PeerConnection* peer) {
  for (int i = 0; i < clients_.size(); ++i) {
    if (clients_[i].peer == peer) {
      emit logMessage(QString("Client disconnected: %1").arg(peer->peerId()));
      clients_.removeAt(i);
      emit clientDisconnected(peer);
      peer->deleteLater();
      break;
    }
  }
}

} // namespace syncmouse
