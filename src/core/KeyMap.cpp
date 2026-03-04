#include "core/KeyMap.h"

namespace syncmouse {

struct MapEntry {
  quint16 usage;
  quint16 key;
};

#if defined(Q_OS_MAC)

namespace {
constexpr quint16 kVK_ANSI_A = 0x00;
constexpr quint16 kVK_ANSI_B = 0x0B;
constexpr quint16 kVK_ANSI_C = 0x08;
constexpr quint16 kVK_ANSI_D = 0x02;
constexpr quint16 kVK_ANSI_E = 0x0E;
constexpr quint16 kVK_ANSI_F = 0x03;
constexpr quint16 kVK_ANSI_G = 0x05;
constexpr quint16 kVK_ANSI_H = 0x04;
constexpr quint16 kVK_ANSI_I = 0x22;
constexpr quint16 kVK_ANSI_J = 0x26;
constexpr quint16 kVK_ANSI_K = 0x28;
constexpr quint16 kVK_ANSI_L = 0x25;
constexpr quint16 kVK_ANSI_M = 0x2E;
constexpr quint16 kVK_ANSI_N = 0x2D;
constexpr quint16 kVK_ANSI_O = 0x1F;
constexpr quint16 kVK_ANSI_P = 0x23;
constexpr quint16 kVK_ANSI_Q = 0x0C;
constexpr quint16 kVK_ANSI_R = 0x0F;
constexpr quint16 kVK_ANSI_S = 0x01;
constexpr quint16 kVK_ANSI_T = 0x11;
constexpr quint16 kVK_ANSI_U = 0x20;
constexpr quint16 kVK_ANSI_V = 0x09;
constexpr quint16 kVK_ANSI_W = 0x0D;
constexpr quint16 kVK_ANSI_X = 0x07;
constexpr quint16 kVK_ANSI_Y = 0x10;
constexpr quint16 kVK_ANSI_Z = 0x06;

constexpr quint16 kVK_ANSI_0 = 0x1D;
constexpr quint16 kVK_ANSI_1 = 0x12;
constexpr quint16 kVK_ANSI_2 = 0x13;
constexpr quint16 kVK_ANSI_3 = 0x14;
constexpr quint16 kVK_ANSI_4 = 0x15;
constexpr quint16 kVK_ANSI_5 = 0x17;
constexpr quint16 kVK_ANSI_6 = 0x16;
constexpr quint16 kVK_ANSI_7 = 0x1A;
constexpr quint16 kVK_ANSI_8 = 0x1C;
constexpr quint16 kVK_ANSI_9 = 0x19;

constexpr quint16 kVK_Return = 0x24;
constexpr quint16 kVK_Escape = 0x35;
constexpr quint16 kVK_Delete = 0x33;
constexpr quint16 kVK_Tab = 0x30;
constexpr quint16 kVK_Space = 0x31;
constexpr quint16 kVK_ANSI_Minus = 0x1B;
constexpr quint16 kVK_ANSI_Equal = 0x18;
constexpr quint16 kVK_ANSI_LeftBracket = 0x21;
constexpr quint16 kVK_ANSI_RightBracket = 0x1E;
constexpr quint16 kVK_ANSI_Backslash = 0x2A;
constexpr quint16 kVK_ANSI_Semicolon = 0x29;
constexpr quint16 kVK_ANSI_Quote = 0x27;
constexpr quint16 kVK_ANSI_Grave = 0x32;
constexpr quint16 kVK_ANSI_Comma = 0x2B;
constexpr quint16 kVK_ANSI_Period = 0x2F;
constexpr quint16 kVK_ANSI_Slash = 0x2C;

constexpr quint16 kVK_CapsLock = 0x39;

constexpr quint16 kVK_F1 = 0x7A;
constexpr quint16 kVK_F2 = 0x78;
constexpr quint16 kVK_F3 = 0x63;
constexpr quint16 kVK_F4 = 0x76;
constexpr quint16 kVK_F5 = 0x60;
constexpr quint16 kVK_F6 = 0x61;
constexpr quint16 kVK_F7 = 0x62;
constexpr quint16 kVK_F8 = 0x64;
constexpr quint16 kVK_F9 = 0x65;
constexpr quint16 kVK_F10 = 0x6D;
constexpr quint16 kVK_F11 = 0x67;
constexpr quint16 kVK_F12 = 0x6F;
constexpr quint16 kVK_F13 = 0x69;
constexpr quint16 kVK_F14 = 0x6B;
constexpr quint16 kVK_F15 = 0x71;
constexpr quint16 kVK_F16 = 0x6A;
constexpr quint16 kVK_F17 = 0x40;
constexpr quint16 kVK_F18 = 0x4F;
constexpr quint16 kVK_F19 = 0x50;
constexpr quint16 kVK_F20 = 0x5A;
constexpr quint16 kVK_F21 = 0x5B;
constexpr quint16 kVK_F22 = 0x5C;
constexpr quint16 kVK_F23 = 0x5D;
constexpr quint16 kVK_F24 = 0x5E;

constexpr quint16 kVK_Help = 0x72;
constexpr quint16 kVK_Home = 0x73;
constexpr quint16 kVK_PageUp = 0x74;
constexpr quint16 kVK_ForwardDelete = 0x75;
constexpr quint16 kVK_End = 0x77;
constexpr quint16 kVK_PageDown = 0x79;
constexpr quint16 kVK_RightArrow = 0x7C;
constexpr quint16 kVK_LeftArrow = 0x7B;
constexpr quint16 kVK_DownArrow = 0x7D;
constexpr quint16 kVK_UpArrow = 0x7E;

constexpr quint16 kVK_Control = 0x3B;
constexpr quint16 kVK_Shift = 0x38;
constexpr quint16 kVK_Option = 0x3A;
constexpr quint16 kVK_Command = 0x37;
constexpr quint16 kVK_RightControl = 0x3E;
constexpr quint16 kVK_RightShift = 0x3C;
constexpr quint16 kVK_RightOption = 0x3D;
constexpr quint16 kVK_RightCommand = 0x36;
} // namespace

static const MapEntry kMap[] = {
  {0x04, kVK_ANSI_A}, {0x05, kVK_ANSI_B}, {0x06, kVK_ANSI_C}, {0x07, kVK_ANSI_D},
  {0x08, kVK_ANSI_E}, {0x09, kVK_ANSI_F}, {0x0A, kVK_ANSI_G}, {0x0B, kVK_ANSI_H},
  {0x0C, kVK_ANSI_I}, {0x0D, kVK_ANSI_J}, {0x0E, kVK_ANSI_K}, {0x0F, kVK_ANSI_L},
  {0x10, kVK_ANSI_M}, {0x11, kVK_ANSI_N}, {0x12, kVK_ANSI_O}, {0x13, kVK_ANSI_P},
  {0x14, kVK_ANSI_Q}, {0x15, kVK_ANSI_R}, {0x16, kVK_ANSI_S}, {0x17, kVK_ANSI_T},
  {0x18, kVK_ANSI_U}, {0x19, kVK_ANSI_V}, {0x1A, kVK_ANSI_W}, {0x1B, kVK_ANSI_X},
  {0x1C, kVK_ANSI_Y}, {0x1D, kVK_ANSI_Z},

  {0x1E, kVK_ANSI_1}, {0x1F, kVK_ANSI_2}, {0x20, kVK_ANSI_3}, {0x21, kVK_ANSI_4},
  {0x22, kVK_ANSI_5}, {0x23, kVK_ANSI_6}, {0x24, kVK_ANSI_7}, {0x25, kVK_ANSI_8},
  {0x26, kVK_ANSI_9}, {0x27, kVK_ANSI_0},

  {0x28, kVK_Return},
  {0x29, kVK_Escape},
  {0x2A, kVK_Delete},
  {0x2B, kVK_Tab},
  {0x2C, kVK_Space},
  {0x2D, kVK_ANSI_Minus},
  {0x2E, kVK_ANSI_Equal},
  {0x2F, kVK_ANSI_LeftBracket},
  {0x30, kVK_ANSI_RightBracket},
  {0x31, kVK_ANSI_Backslash},
  {0x33, kVK_ANSI_Semicolon},
  {0x34, kVK_ANSI_Quote},
  {0x35, kVK_ANSI_Grave},
  {0x36, kVK_ANSI_Comma},
  {0x37, kVK_ANSI_Period},
  {0x38, kVK_ANSI_Slash},

  {0x39, kVK_CapsLock},

  {0x3A, kVK_F1}, {0x3B, kVK_F2}, {0x3C, kVK_F3}, {0x3D, kVK_F4},
  {0x3E, kVK_F5}, {0x3F, kVK_F6}, {0x40, kVK_F7}, {0x41, kVK_F8},
  {0x42, kVK_F9}, {0x43, kVK_F10}, {0x44, kVK_F11}, {0x45, kVK_F12},
  {0x68, kVK_F13}, {0x69, kVK_F14}, {0x6A, kVK_F15}, {0x6B, kVK_F16},
  {0x6C, kVK_F17}, {0x6D, kVK_F18}, {0x6E, kVK_F19}, {0x6F, kVK_F20},
  {0x70, kVK_F21}, {0x71, kVK_F22}, {0x72, kVK_F23}, {0x73, kVK_F24},

  {0x49, kVK_Help},
  {0x4A, kVK_Home},
  {0x4B, kVK_PageUp},
  {0x4C, kVK_ForwardDelete},
  {0x4D, kVK_End},
  {0x4E, kVK_PageDown},
  {0x4F, kVK_RightArrow},
  {0x50, kVK_LeftArrow},
  {0x51, kVK_DownArrow},
  {0x52, kVK_UpArrow},

  {0xE0, kVK_Control},
  {0xE1, kVK_Shift},
  {0xE2, kVK_Option},
  {0xE3, kVK_Command},
  {0xE4, kVK_RightControl},
  {0xE5, kVK_RightShift},
  {0xE6, kVK_RightOption},
  {0xE7, kVK_RightCommand}
};

#elif defined(Q_OS_WIN)

#include <windows.h>

static const MapEntry kMap[] = {
  {0x04, 'A'}, {0x05, 'B'}, {0x06, 'C'}, {0x07, 'D'},
  {0x08, 'E'}, {0x09, 'F'}, {0x0A, 'G'}, {0x0B, 'H'},
  {0x0C, 'I'}, {0x0D, 'J'}, {0x0E, 'K'}, {0x0F, 'L'},
  {0x10, 'M'}, {0x11, 'N'}, {0x12, 'O'}, {0x13, 'P'},
  {0x14, 'Q'}, {0x15, 'R'}, {0x16, 'S'}, {0x17, 'T'},
  {0x18, 'U'}, {0x19, 'V'}, {0x1A, 'W'}, {0x1B, 'X'},
  {0x1C, 'Y'}, {0x1D, 'Z'},

  {0x1E, '1'}, {0x1F, '2'}, {0x20, '3'}, {0x21, '4'},
  {0x22, '5'}, {0x23, '6'}, {0x24, '7'}, {0x25, '8'},
  {0x26, '9'}, {0x27, '0'},

  {0x28, VK_RETURN},
  {0x29, VK_ESCAPE},
  {0x2A, VK_BACK},
  {0x2B, VK_TAB},
  {0x2C, VK_SPACE},
  {0x2D, VK_OEM_MINUS},
  {0x2E, VK_OEM_PLUS},
  {0x2F, VK_OEM_4},
  {0x30, VK_OEM_6},
  {0x31, VK_OEM_5},
  {0x33, VK_OEM_1},
  {0x34, VK_OEM_7},
  {0x35, VK_OEM_3},
  {0x36, VK_OEM_COMMA},
  {0x37, VK_OEM_PERIOD},
  {0x38, VK_OEM_2},

  {0x39, VK_CAPITAL},

  {0x3A, VK_F1}, {0x3B, VK_F2}, {0x3C, VK_F3}, {0x3D, VK_F4},
  {0x3E, VK_F5}, {0x3F, VK_F6}, {0x40, VK_F7}, {0x41, VK_F8},
  {0x42, VK_F9}, {0x43, VK_F10}, {0x44, VK_F11}, {0x45, VK_F12},
  {0x68, VK_F13}, {0x69, VK_F14}, {0x6A, VK_F15}, {0x6B, VK_F16},
  {0x6C, VK_F17}, {0x6D, VK_F18}, {0x6E, VK_F19}, {0x6F, VK_F20},
  {0x70, VK_F21}, {0x71, VK_F22}, {0x72, VK_F23}, {0x73, VK_F24},

  {0x49, VK_INSERT},
  {0x4A, VK_HOME},
  {0x4B, VK_PRIOR},
  {0x4C, VK_DELETE},
  {0x4D, VK_END},
  {0x4E, VK_NEXT},
  {0x4F, VK_RIGHT},
  {0x50, VK_LEFT},
  {0x51, VK_DOWN},
  {0x52, VK_UP},

  {0xE0, VK_LCONTROL},
  {0xE1, VK_LSHIFT},
  {0xE2, VK_LMENU},
  {0xE3, VK_LWIN},
  {0xE4, VK_RCONTROL},
  {0xE5, VK_RSHIFT},
  {0xE6, VK_RMENU},
  {0xE7, VK_RWIN}
};

#else

static const MapEntry kMap[] = {};

#endif

quint16 usageFromPlatformKeycode(quint16 keycode) {
  for (const auto& entry : kMap) {
    if (entry.key == keycode) {
      return entry.usage;
    }
  }
  return 0;
}

quint16 platformKeycodeFromUsage(quint16 usage) {
  for (const auto& entry : kMap) {
    if (entry.usage == usage) {
      return entry.key;
    }
  }
  return 0;
}

} // namespace syncmouse
