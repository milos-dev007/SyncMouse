# SyncMouse (Prototype)

A simple C++/Qt prototype for a Barrier-like app with server/client modes, clipboard sync, and basic file transfers.

## Features Implemented
- Server mode: accepts multiple client connections.
- Client mode: connects to a server via IP + port.
- Clipboard sync: server -> clients and client -> server (toggleable).
- File transfer: both directions; incoming files are saved in the OS Downloads folder.

## Not Implemented Yet
- System-wide mouse/keyboard capture and injection.
- Screen-edge switching and client position logic.

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
