#pragma once

#include <QMainWindow>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QPlainTextEdit;
class QSpinBox;
class QTableWidget;
class QPushButton;
class QTabWidget;

namespace syncmouse {

class ClipboardSync;
class Client;
class PeerConnection;
class Server;

enum class ClientPosition;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow() override;

private slots:
  void onStartStopServer();
  void onConnectDisconnectClient();
  void onSendFileFromServer();
  void onSendFileFromClient();
  void onLocalClipboardChanged(const QString& text);
  void onClipboardTextReceived(const QString& text);
  void onClientConnected(syncmouse::PeerConnection* peer);
  void onClientDisconnected(syncmouse::PeerConnection* peer);
  void onClientTableSelectionChanged();
  void onClientFileReceived(const QString& path);
  void onClientLogMessage(const QString& message);

private:
  QWidget* buildServerTab();
  QWidget* buildClientTab();

  void addClientRow(syncmouse::PeerConnection* peer);
  void removeClientRow(syncmouse::PeerConnection* peer);
  syncmouse::PeerConnection* selectedServerPeer() const;
  void appendLog(const QString& message);

  ClientPosition parsePosition(const QString& text) const;

  QTabWidget* tabs_ = nullptr;

  QWidget* serverTab_ = nullptr;
  QSpinBox* serverPort_ = nullptr;
  QPushButton* serverToggleButton_ = nullptr;
  QTableWidget* clientTable_ = nullptr;
  QPushButton* serverSendFileButton_ = nullptr;
  QCheckBox* serverClipboardCheck_ = nullptr;

  QWidget* clientTab_ = nullptr;
  QLineEdit* clientHost_ = nullptr;
  QSpinBox* clientPort_ = nullptr;
  QPushButton* clientToggleButton_ = nullptr;
  QPushButton* clientSendFileButton_ = nullptr;
  QCheckBox* clientClipboardCheck_ = nullptr;

  QPlainTextEdit* log_ = nullptr;

  Server* server_ = nullptr;
  Client* client_ = nullptr;
  ClipboardSync* clipboard_ = nullptr;
};

} // namespace syncmouse
