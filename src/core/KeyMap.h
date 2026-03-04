#pragma once

#include <QtGlobal>

namespace syncmouse {

quint16 usageFromPlatformKeycode(quint16 keycode);
quint16 platformKeycodeFromUsage(quint16 usage);

} // namespace syncmouse
