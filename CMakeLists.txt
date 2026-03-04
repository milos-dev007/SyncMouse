cmake_minimum_required(VERSION 3.20)
project(SyncMouse LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (APPLE)
  enable_language(OBJCXX)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets Network Gui QUIET)
if (Qt6_FOUND)
  set(QT_LIBS Qt6::Widgets Qt6::Network Qt6::Gui)
else()
  find_package(Qt5 COMPONENTS Widgets Network Gui REQUIRED)
  set(QT_LIBS Qt5::Widgets Qt5::Network Qt5::Gui)
endif()

add_executable(SyncMouse
  src/main.cpp
  src/gui/MainWindow.h
  src/gui/MainWindow.cpp
  src/net/Protocol.h
  src/net/PeerConnection.h
  src/net/PeerConnection.cpp
  src/net/Server.h
  src/net/Server.cpp
  src/net/Client.h
  src/net/Client.cpp
  src/core/ClipboardSync.h
  src/core/ClipboardSync.cpp
  src/core/Downloads.h
  src/core/InputEvent.h
  src/core/InputCapture.h
  src/core/InputInjector.h
  src/core/InputShareController.h
  src/core/InputShareController.cpp
  src/core/KeyMap.h
  src/core/KeyMap.cpp
)

target_link_libraries(SyncMouse PRIVATE ${QT_LIBS})

target_include_directories(SyncMouse PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(SyncMouse PRIVATE
  SYNC_MOUSE_APP_NAME="SyncMouse"
)

if (APPLE)
  set_target_properties(SyncMouse PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.syncmouse.app"
    MACOSX_BUNDLE_BUNDLE_NAME "SyncMouse"
    MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0.0"
  )
  target_sources(SyncMouse PRIVATE
    src/core/InputCaptureMac.h
    src/core/InputCaptureMac.mm
    src/core/InputInjectorMac.h
    src/core/InputInjectorMac.mm
  )
  target_link_libraries(SyncMouse PRIVATE "-framework ApplicationServices")
endif()

if (WIN32)
  target_sources(SyncMouse PRIVATE
    src/core/InputCaptureWin.h
    src/core/InputCaptureWin.cpp
    src/core/InputInjectorWin.h
    src/core/InputInjectorWin.cpp
  )
  target_link_libraries(SyncMouse PRIVATE user32)
  set_target_properties(SyncMouse PROPERTIES
    WIN32_EXECUTABLE TRUE
  )
endif()
