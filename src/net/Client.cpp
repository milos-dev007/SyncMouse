#include "net/Client.h"

#include "net/PeerConnection.h"

#include <QTcpSocket>

namespace syncmouse {

Client::Client(QObject* parent)
  : QObject(parent), socket_(new QTcpSocket(this)) {
  peer_ = new PeerConnection(socket_, this);

  connect(socket_, &QTcpSocket::connected, this, &Client::onConnected);
  connect(socket_, &QTcpSocket::disconnected, this, &Client::onDisconnected);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  connect(socket_, &QTcpSocket::errorOccurred, this, &Client::onSocketError);
#else
  connect(socket_, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
          this, &Client::onSocketError);
#endif
  connect(peer_, &PeerConnection::logMessage, this, &Client::logMessage);
}

Client::~Client() {
  disconnectFromHost();
}

void Client::connectToHost(const QString& host, quint16 port) {
  if (socket_->state() == QAbstractSocket::ConnectedState ||
      socket_->state() == QAbstractSocket::ConnectingState) {
    socket_->abort();
  }

  socket_->connectToHost(host, port);
  emit logMessage(QString("Connecting to %1:%2").arg(host).arg(port));
}

void Client::disconnectFromHost() {
  if (socket_->state() != QAbstractSocket::UnconnectedState) {
    socket_->disconnectFromHost();
  }
}

bool Client::isConnected() const {
  return socket_->state() == QAbstractSocket::ConnectedState;
}

PeerConnection* Client::peer() const {
  return peer_;
}

void Client::onConnected() {
  emit logMessage("Connected to server");
  emit connected();
}

void Client::onDisconnected() {
  emit logMessage("Disconnected from server");
  emit disconnected();
}

void Client::onSocketError() {
  emit logMessage(QString("Socket error: %1").arg(socket_->errorString()));
}

} // namespace syncmouse
