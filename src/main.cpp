#include "gui/MainWindow.h"

#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  syncmouse::MainWindow window;
  window.show();

  return app.exec();
}
