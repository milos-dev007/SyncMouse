#pragma once

#include <QObject>
#include <QPointer>
#include <QString>

#include "core/InputEvent.h"

class QFile;
class QTcpSocket;
class QDataStream;

namespace syncmouse {

enum class MessageType : quint16;

class PeerConnection : public QObject {
  Q_OBJECT
public:
  explicit PeerConnection(QTcpSocket* socket, QObject* parent = nullptr);
  ~PeerConnection() override;

  QTcpSocket* socket() const;
  QString peerId() const;
  bool isConnected() const;

  void sendClipboardText(const QString& text);
  bool sendFile(const QString& path, QString* error = nullptr);
  void sendInputEvent(const syncmouse::InputEvent& event);

signals:
  void disconnected(syncmouse::PeerConnection* peer);
  void clipboardTextReceived(const QString& text);
  void fileReceived(const QString& path);
  void inputEventReceived(const syncmouse::InputEvent& event);
  void logMessage(const QString& message);

private slots:
  void onReadyRead();
  void onDisconnected();

private:
  void sendMessage(MessageType type, const QByteArray& payload);
  bool handleMessage(MessageType type, const QByteArray& payload);

  bool beginIncomingFile(const QByteArray& payload);
  bool appendIncomingFile(const QByteArray& payload);
  void finishIncomingFile();

  QPointer<QTcpSocket> socket_;
  QDataStream* in_ = nullptr;

  enum class ReadState { Header, Payload };
  ReadState state_ = ReadState::Header;
  quint16 currentType_ = 0;
  quint64 currentSize_ = 0;

  QFile* incomingFile_ = nullptr;
  quint64 incomingRemaining_ = 0;
};

} // namespace syncmouse
