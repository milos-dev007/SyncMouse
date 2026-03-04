#pragma once

#include <QtGlobal>

namespace syncmouse {

enum class ClientPosition : quint8 {
  Left = 0,
  Right = 1,
  Up = 2,
  Down = 3,
  Unknown = 4
};

} // namespace syncmouse
