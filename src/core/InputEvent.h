#pragma once

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <QtGlobal>

namespace syncmouse {

enum class InputEventType : quint8 {
  MouseMove = 1,
  MouseButton = 2,
  MouseWheel = 3,
  Key = 4
};

enum class MouseButton : quint8 {
  Left = 1,
  Right = 2,
  Middle = 3,
  X1 = 4,
  X2 = 5
};

enum Modifiers : quint16 {
  ModShift = 1 << 0,
  ModCtrl = 1 << 1,
  ModAlt = 1 << 2,
  ModMeta = 1 << 3
};

struct InputEvent {
  InputEventType type = InputEventType::MouseMove;
  qint32 dx = 0;
  qint32 dy = 0;
  quint8 button = 0;   // MouseButton
  quint8 state = 0;    // 1 = down, 0 = up
  quint16 key = 0;     // USB HID usage (page 0x07)
  quint16 modifiers = 0;
};

inline void writeInputEvent(QDataStream& out, const InputEvent& e) {
  out << static_cast<quint8>(e.type);
  out << e.dx;
  out << e.dy;
  out << e.button;
  out << e.state;
  out << e.key;
  out << e.modifiers;
}

inline bool readInputEvent(QDataStream& in, InputEvent* e) {
  quint8 type = 0;
  in >> type;
  in >> e->dx;
  in >> e->dy;
  in >> e->button;
  in >> e->state;
  in >> e->key;
  in >> e->modifiers;
  e->type = static_cast<InputEventType>(type);
  return true;
}

inline int inputStreamVersion() {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  return QDataStream::Qt_6_0;
#else
  return QDataStream::Qt_5_15;
#endif
}

inline QByteArray serializeInputEvent(const InputEvent& e) {
  QByteArray payload;
  QDataStream out(&payload, QIODevice::WriteOnly);
  out.setVersion(inputStreamVersion());
  writeInputEvent(out, e);
  return payload;
}

inline bool deserializeInputEvent(const QByteArray& payload, InputEvent* e) {
  QByteArray buffer = payload;
  QDataStream in(&buffer, QIODevice::ReadOnly);
  in.setVersion(inputStreamVersion());
  return readInputEvent(in, e);
}

} // namespace syncmouse
