#include "net/PeerConnection.h"

#include "core/Downloads.h"
#include "net/Protocol.h"

#include <QDataStream>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QHostAddress>
#include <QTcpSocket>

namespace syncmouse {

namespace {
constexpr qint64 kHeaderSize = 4 + 2 + 2 + 8;
constexpr qint64 kFileChunkSize = 64 * 1024;

enum class ConfigKey : quint8 {
  ClientPosition = 1
};

int streamVersion() {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  return QDataStream::Qt_6_0;
#else
  return QDataStream::Qt_5_15;
#endif
}
} // namespace

PeerConnection::PeerConnection(QTcpSocket* socket, QObject* parent)
  : QObject(parent), socket_(socket) {
  in_ = new QDataStream(socket_);
  in_->setVersion(streamVersion());

  connect(socket_, &QTcpSocket::readyRead, this, &PeerConnection::onReadyRead);
  connect(socket_, &QTcpSocket::disconnected, this, &PeerConnection::onDisconnected);
}

PeerConnection::~PeerConnection() {
  if (incomingFile_) {
    incomingFile_->close();
    delete incomingFile_;
    incomingFile_ = nullptr;
  }
  delete in_;
}

QTcpSocket* PeerConnection::socket() const {
  return socket_;
}

QString PeerConnection::peerId() const {
  if (!socket_) {
    return QStringLiteral("unknown");
  }
  return QString("%1:%2").arg(socket_->peerAddress().toString()).arg(socket_->peerPort());
}

bool PeerConnection::isConnected() const {
  return socket_ && socket_->state() == QAbstractSocket::ConnectedState;
}

void PeerConnection::sendClipboardText(const QString& text) {
  QByteArray payload;
  QDataStream out(&payload, QIODevice::WriteOnly);
  out.setVersion(streamVersion());
  out << text;
  sendMessage(MessageType::ClipboardText, payload);
}

bool PeerConnection::sendFile(const QString& path, QString* error) {
  if (!socket_ || socket_->state() != QAbstractSocket::ConnectedState) {
    if (error) {
      *error = QStringLiteral("No active connection");
    }
    return false;
  }

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    if (error) {
      *error = QStringLiteral("Failed to open file");
    }
    return false;
  }

  const QFileInfo info(file);
  const QString fileName = info.fileName();
  const quint64 fileSize = static_cast<quint64>(info.size());

  QByteArray startPayload;
  {
    QDataStream out(&startPayload, QIODevice::WriteOnly);
    out.setVersion(streamVersion());
    out << fileName;
    out << fileSize;
  }
  sendMessage(MessageType::FileStart, startPayload);

  while (!file.atEnd()) {
    QByteArray chunk = file.read(kFileChunkSize);
    if (chunk.isEmpty()) {
      break;
    }
    sendMessage(MessageType::FileChunk, chunk);
  }

  sendMessage(MessageType::FileEnd, QByteArray());
  emit logMessage(QString("Sent file %1 (%2 bytes)").arg(fileName).arg(fileSize));
  return true;
}

void PeerConnection::sendInputEvent(const InputEvent& event) {
  sendMessage(MessageType::InputEvent, serializeInputEvent(event));
}

void PeerConnection::sendClientPosition(ClientPosition position) {
  QByteArray payload;
  QDataStream out(&payload, QIODevice::WriteOnly);
  out.setVersion(streamVersion());
  out << static_cast<quint8>(ConfigKey::ClientPosition);
  out << static_cast<quint8>(position);
  sendMessage(MessageType::ConfigUpdate, payload);
}

void PeerConnection::sendReturnControl() {
  sendMessage(MessageType::ControlReturn, QByteArray());
}

void PeerConnection::onReadyRead() {
  if (!socket_ || !in_) {
    return;
  }

  while (true) {
    if (state_ == ReadState::Header) {
      if (socket_->bytesAvailable() < kHeaderSize) {
        return;
      }

      FrameHeader header;
      *in_ >> header.magic;
      *in_ >> header.version;
      *in_ >> header.type;
      *in_ >> header.size;

      if (header.magic != kProtocolMagic || header.version != kProtocolVersion) {
        emit logMessage("Protocol mismatch; disconnecting.");
        socket_->disconnectFromHost();
        return;
      }

      currentType_ = header.type;
      currentSize_ = header.size;
      state_ = ReadState::Payload;
    }

    if (state_ == ReadState::Payload) {
      if (socket_->bytesAvailable() < static_cast<qint64>(currentSize_)) {
        return;
      }

      QByteArray payload;
      if (currentSize_ > 0) {
        payload.resize(static_cast<int>(currentSize_));
        const int read = in_->readRawData(payload.data(), static_cast<int>(currentSize_));
        if (read != static_cast<int>(currentSize_)) {
          emit logMessage("Failed to read full payload; disconnecting.");
          socket_->disconnectFromHost();
          return;
        }
      }

      handleMessage(static_cast<MessageType>(currentType_), payload);
      state_ = ReadState::Header;
    }
  }
}

void PeerConnection::onDisconnected() {
  emit disconnected(this);
}

void PeerConnection::sendMessage(MessageType type, const QByteArray& payload) {
  if (!socket_) {
    return;
  }

  QByteArray frame;
  QDataStream out(&frame, QIODevice::WriteOnly);
  out.setVersion(streamVersion());

  FrameHeader header;
  header.type = static_cast<quint16>(type);
  header.size = static_cast<quint64>(payload.size());

  out << header.magic;
  out << header.version;
  out << header.type;
  out << header.size;

  if (!payload.isEmpty()) {
    out.writeRawData(payload.constData(), payload.size());
  }

  socket_->write(frame);
}

bool PeerConnection::handleMessage(MessageType type, const QByteArray& payload) {
  switch (type) {
    case MessageType::ClipboardText: {
      QByteArray buffer = payload;
      QDataStream in(&buffer, QIODevice::ReadOnly);
      in.setVersion(streamVersion());
      QString text;
      in >> text;
      emit clipboardTextReceived(text);
      return true;
    }
    case MessageType::FileStart:
      return beginIncomingFile(payload);
    case MessageType::FileChunk:
      return appendIncomingFile(payload);
    case MessageType::FileEnd:
      finishIncomingFile();
      return true;
    case MessageType::InputEvent: {
      InputEvent ev;
      if (deserializeInputEvent(payload, &ev)) {
        emit inputEventReceived(ev);
        return true;
      }
      return false;
    }
    case MessageType::ConfigUpdate: {
      if (payload.size() < 2) {
        return false;
      }
      QByteArray buffer = payload;
      QDataStream in(&buffer, QIODevice::ReadOnly);
      in.setVersion(streamVersion());
      quint8 key = 0;
      in >> key;
      if (static_cast<ConfigKey>(key) == ConfigKey::ClientPosition) {
        quint8 pos = 0;
        in >> pos;
        emit clientPositionReceived(static_cast<ClientPosition>(pos));
        return true;
      }
      emit logMessage(QString("Unknown config update %1").arg(key));
      return false;
    }
    case MessageType::ControlReturn:
      emit returnControlRequested(this);
      return true;
    default:
      emit logMessage(QString("Unhandled message type %1").arg(static_cast<int>(type)));
      return false;
  }
}

bool PeerConnection::beginIncomingFile(const QByteArray& payload) {
  QByteArray buffer = payload;
  QDataStream in(&buffer, QIODevice::ReadOnly);
  in.setVersion(streamVersion());

  QString fileName;
  quint64 fileSize = 0;
  in >> fileName;
  in >> fileSize;

  if (fileName.isEmpty()) {
    emit logMessage("Received file with empty name; ignoring.");
    return false;
  }

  QString targetPath = downloads::uniquePath(fileName);
  if (incomingFile_) {
    incomingFile_->close();
    delete incomingFile_;
    incomingFile_ = nullptr;
  }

  incomingFile_ = new QFile(targetPath, this);
  if (!incomingFile_->open(QIODevice::WriteOnly)) {
    emit logMessage("Failed to open downloads file for writing.");
    delete incomingFile_;
    incomingFile_ = nullptr;
    return false;
  }

  incomingRemaining_ = fileSize;
  emit logMessage(QString("Receiving file %1 (%2 bytes)").arg(fileName).arg(fileSize));

  if (incomingRemaining_ == 0) {
    finishIncomingFile();
  }

  return true;
}

bool PeerConnection::appendIncomingFile(const QByteArray& payload) {
  if (!incomingFile_) {
    emit logMessage("File chunk received without active file.");
    return false;
  }

  if (!payload.isEmpty()) {
    incomingFile_->write(payload);
  }

  if (incomingRemaining_ >= static_cast<quint64>(payload.size())) {
    incomingRemaining_ -= static_cast<quint64>(payload.size());
  } else {
    incomingRemaining_ = 0;
  }

  if (incomingRemaining_ == 0) {
    finishIncomingFile();
  }

  return true;
}

void PeerConnection::finishIncomingFile() {
  if (!incomingFile_) {
    return;
  }

  const QString path = incomingFile_->fileName();
  incomingFile_->close();
  delete incomingFile_;
  incomingFile_ = nullptr;
  incomingRemaining_ = 0;

  emit fileReceived(path);
  emit logMessage(QString("File saved to %1").arg(path));
}

} // namespace syncmouse
