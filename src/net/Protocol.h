#pragma once

#include <QtGlobal>

namespace syncmouse {

constexpr quint32 kProtocolMagic = 0x534D4F55; // "SMOU"
constexpr quint16 kProtocolVersion = 1;

enum class MessageType : quint16 {
  ClipboardText = 1,
  FileStart = 2,
  FileChunk = 3,
  FileEnd = 4,
  Heartbeat = 5,
  ClientHello = 6,
  ServerHello = 7,
  InputEvent = 8,
  ConfigUpdate = 9,
  ControlReturn = 10
};

struct FrameHeader {
  quint32 magic = kProtocolMagic;
  quint16 version = kProtocolVersion;
  quint16 type = 0;
  quint64 size = 0;
};

} // namespace syncmouse
