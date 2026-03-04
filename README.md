# SyncMouse (Prototype)

A simple C++/Qt prototype for a Barrier-like app with server/client modes, clipboard sync, and basic file transfers.

## Features Implemented
- Server mode: accepts multiple client connections.
- Client mode: connects to a server via IP + port.
- Clipboard sync: server -> clients and client -> server (toggleable).
- File transfer: both directions; incoming files are saved in the OS Downloads folder.
- Mouse/keyboard sharing: server captures global input and forwards to a client when the cursor hits a configured edge.
  Return hotkey: `Ctrl+Shift+Q`.

## Not Implemented Yet
- Multi-monitor edge mapping.
- Full keyboard layout mapping beyond US/ANSI.

These require OS-specific hooks (Windows: low-level hooks + SendInput, macOS: CGEventTap + CGEventCreate, Linux: X11/Wayland).

## Build
Prereqs: Qt 5.15+ or Qt 6.x, CMake 3.20+, C++17 compiler.

```bash
cmake -S . -B build
cmake --build build
```

## Usage
- Run the app, pick Server or Client tab.
- Server: choose port, start server, select a client, send a file.
- Client: enter server IP/port, connect, send a file.
- Clipboard sync toggles are per mode.
- Enable "Share Mouse/Keyboard" on the server to start input sharing.

## Permissions (macOS)
Global input capture/injection requires Accessibility permission.
System Settings > Privacy & Security > Accessibility > enable SyncMouse.
