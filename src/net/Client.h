#pragma once

#include <QObject>
#include <QString>

class QTcpSocket;

namespace syncmouse {

class PeerConnection;

class Client : public QObject {
  Q_OBJECT
public:
  explicit Client(QObject* parent = nullptr);
  ~Client() override;

  void connectToHost(const QString& host, quint16 port);
  void disconnectFromHost();
  bool isConnected() const;

  PeerConnection* peer() const;

signals:
  void connected();
  void disconnected();
  void logMessage(const QString& message);

private slots:
  void onConnected();
  void onDisconnected();
  void onSocketError();

private:
  QTcpSocket* socket_ = nullptr;
  PeerConnection* peer_ = nullptr;
};

} // namespace syncmouse
