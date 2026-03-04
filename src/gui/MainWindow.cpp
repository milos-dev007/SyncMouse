#include "gui/MainWindow.h"

#include "core/ClipboardSync.h"
#include "core/ClientPosition.h"
#include "core/InputInjector.h"
#include "core/InputShareController.h"
#include "net/Client.h"
#include "net/PeerConnection.h"
#include "net/Server.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QVariant>
#include <QVBoxLayout>

#if defined(Q_OS_MAC)
#include "core/InputInjectorMac.h"
#elif defined(Q_OS_WIN)
#include "core/InputInjectorWin.h"
#endif

namespace syncmouse {

namespace {
InputInjector::ReturnEdge returnEdgeForPosition(ClientPosition position) {
  switch (position) {
    case ClientPosition::Left:
      return InputInjector::ReturnEdge::Right;
    case ClientPosition::Right:
      return InputInjector::ReturnEdge::Left;
    case ClientPosition::Up:
      return InputInjector::ReturnEdge::Down;
    case ClientPosition::Down:
      return InputInjector::ReturnEdge::Up;
    default:
      return InputInjector::ReturnEdge::None;
  }
}
} // namespace

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent),
    server_(new Server(this)),
    client_(new Client(this)),
    clipboard_(new ClipboardSync(this)),
    inputShare_(new InputShareController(server_, this)) {
  setWindowTitle(QStringLiteral("SyncMouse"));
  resize(900, 600);

  tabs_ = new QTabWidget(this);
  serverTab_ = buildServerTab();
  clientTab_ = buildClientTab();

  tabs_->addTab(serverTab_, QStringLiteral("Server"));
  tabs_->addTab(clientTab_, QStringLiteral("Client"));

  log_ = new QPlainTextEdit(this);
  log_->setReadOnly(true);

  auto* central = new QWidget(this);
  auto* layout = new QVBoxLayout(central);
  layout->addWidget(tabs_);
  layout->addWidget(log_);
  setCentralWidget(central);

  connect(server_, &Server::clientConnected, this, &MainWindow::onClientConnected);
  connect(server_, &Server::clientDisconnected, this, &MainWindow::onClientDisconnected);
  connect(server_, &Server::logMessage, this, &MainWindow::onClientLogMessage);
  connect(inputShare_, &InputShareController::logMessage, this, &MainWindow::onClientLogMessage);

  connect(client_, &Client::connected, this, [this]() {
    clientToggleButton_->setText(QStringLiteral("Disconnect"));
    clientHost_->setEnabled(false);
    clientPort_->setEnabled(false);
  });
  connect(client_, &Client::disconnected, this, [this]() {
    clientToggleButton_->setText(QStringLiteral("Connect"));
    clientHost_->setEnabled(true);
    clientPort_->setEnabled(true);
    if (inputInjector_) {
      inputInjector_->setReturnEdge(InputInjector::ReturnEdge::None);
    }
  });
  connect(client_, &Client::logMessage, this, &MainWindow::onClientLogMessage);

  connect(clipboard_, &ClipboardSync::localClipboardChanged, this, &MainWindow::onLocalClipboardChanged);
  connect(client_->peer(), &PeerConnection::clipboardTextReceived, this, &MainWindow::onClipboardTextReceived);
  connect(client_->peer(), &PeerConnection::fileReceived, this, &MainWindow::onClientFileReceived);

#if defined(Q_OS_MAC)
  inputInjector_ = new InputInjectorMac(this);
#elif defined(Q_OS_WIN)
  inputInjector_ = new InputInjectorWin(this);
#else
  inputInjector_ = nullptr;
#endif

  if (inputInjector_) {
    connect(client_->peer(), &PeerConnection::inputEventReceived, this, [this](const InputEvent& ev) {
      inputInjector_->inject(ev);
    });
    connect(client_->peer(), &PeerConnection::clientPositionReceived, this, [this](ClientPosition position) {
      inputInjector_->setReturnEdge(returnEdgeForPosition(position));
    });
    connect(inputInjector_, &InputInjector::returnRequested, this, [this]() {
      if (client_->isConnected()) {
        client_->peer()->sendReturnControl();
      }
    });
    if (clientMouseSpeed_) {
      inputInjector_->setMouseScale(static_cast<double>(clientMouseSpeed_->value()) / 100.0);
    }
  }
}

MainWindow::~MainWindow() = default;

QWidget* MainWindow::buildServerTab() {
  auto* tab = new QWidget(this);
  auto* layout = new QVBoxLayout(tab);

  auto* topRow = new QHBoxLayout();
  topRow->addWidget(new QLabel(QStringLiteral("Port:"), tab));

  serverPort_ = new QSpinBox(tab);
  serverPort_->setRange(1, 65535);
  serverPort_->setValue(25000);
  topRow->addWidget(serverPort_);

  serverToggleButton_ = new QPushButton(QStringLiteral("Start Server"), tab);
  topRow->addWidget(serverToggleButton_);
  topRow->addStretch();

  layout->addLayout(topRow);

  clientTable_ = new QTableWidget(0, 2, tab);
  clientTable_->setHorizontalHeaderLabels({QStringLiteral("Client"), QStringLiteral("Position")});
  clientTable_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  clientTable_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  clientTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
  clientTable_->setSelectionMode(QAbstractItemView::SingleSelection);
  layout->addWidget(clientTable_);

  serverSendFileButton_ = new QPushButton(QStringLiteral("Send File to Selected Client"), tab);
  serverSendFileButton_->setEnabled(false);
  layout->addWidget(serverSendFileButton_);

  serverClipboardCheck_ = new QCheckBox(QStringLiteral("Sync Clipboard (Server -> Clients)"), tab);
  serverClipboardCheck_->setChecked(true);
  layout->addWidget(serverClipboardCheck_);

  serverInputCheck_ = new QCheckBox(QStringLiteral("Share Mouse/Keyboard (Server -> Client)"), tab);
  serverInputCheck_->setChecked(false);
  layout->addWidget(serverInputCheck_);

  auto* hotkeyLabel = new QLabel(QStringLiteral("Return: move to server edge or press Ctrl+Shift+Q"), tab);
  layout->addWidget(hotkeyLabel);

  connect(serverToggleButton_, &QPushButton::clicked, this, &MainWindow::onStartStopServer);
  connect(serverSendFileButton_, &QPushButton::clicked, this, &MainWindow::onSendFileFromServer);
  connect(clientTable_->selectionModel(), &QItemSelectionModel::selectionChanged,
          this, &MainWindow::onClientTableSelectionChanged);
  connect(serverInputCheck_, &QCheckBox::toggled, this, [this](bool enabled) {
    QString error;
    if (!inputShare_->setEnabled(enabled, &error)) {
      appendLog(error.isEmpty() ? QStringLiteral("Failed to enable input sharing.") : error);
      serverInputCheck_->blockSignals(true);
      serverInputCheck_->setChecked(false);
      serverInputCheck_->blockSignals(false);
    }
  });

  return tab;
}

QWidget* MainWindow::buildClientTab() {
  auto* tab = new QWidget(this);
  auto* layout = new QVBoxLayout(tab);

  auto* topRow = new QHBoxLayout();
  topRow->addWidget(new QLabel(QStringLiteral("Server IP:"), tab));

  clientHost_ = new QLineEdit(tab);
  clientHost_->setPlaceholderText(QStringLiteral("192.168.0.10"));
  topRow->addWidget(clientHost_);

  topRow->addWidget(new QLabel(QStringLiteral("Port:"), tab));
  clientPort_ = new QSpinBox(tab);
  clientPort_->setRange(1, 65535);
  clientPort_->setValue(25000);
  topRow->addWidget(clientPort_);

  clientToggleButton_ = new QPushButton(QStringLiteral("Connect"), tab);
  topRow->addWidget(clientToggleButton_);
  topRow->addStretch();

  layout->addLayout(topRow);

  clientSendFileButton_ = new QPushButton(QStringLiteral("Send File to Server"), tab);
  layout->addWidget(clientSendFileButton_);

  clientClipboardCheck_ = new QCheckBox(QStringLiteral("Sync Clipboard (Client -> Server)"), tab);
  clientClipboardCheck_->setChecked(true);
  layout->addWidget(clientClipboardCheck_);

  auto* speedRow = new QHBoxLayout();
  speedRow->addWidget(new QLabel(QStringLiteral("Mouse speed:"), tab));
  clientMouseSpeed_ = new QSlider(Qt::Horizontal, tab);
  clientMouseSpeed_->setRange(50, 200);
  clientMouseSpeed_->setValue(100);
  speedRow->addWidget(clientMouseSpeed_);
  clientMouseSpeedValue_ = new QLabel(QStringLiteral("100%"), tab);
  speedRow->addWidget(clientMouseSpeedValue_);
  layout->addLayout(speedRow);

  connect(clientToggleButton_, &QPushButton::clicked, this, &MainWindow::onConnectDisconnectClient);
  connect(clientSendFileButton_, &QPushButton::clicked, this, &MainWindow::onSendFileFromClient);
  connect(clientMouseSpeed_, &QSlider::valueChanged, this, [this](int value) {
    if (inputInjector_) {
      inputInjector_->setMouseScale(static_cast<double>(value) / 100.0);
    }
    if (clientMouseSpeedValue_) {
      clientMouseSpeedValue_->setText(QStringLiteral("%1%").arg(value));
    }
  });

  return tab;
}

void MainWindow::onStartStopServer() {
  if (server_->isRunning()) {
    server_->stop();
    serverToggleButton_->setText(QStringLiteral("Start Server"));
    serverPort_->setEnabled(true);
    if (serverInputCheck_ && serverInputCheck_->isChecked()) {
      serverInputCheck_->setChecked(false);
    }
    return;
  }

  QString error;
  if (!server_->start(static_cast<quint16>(serverPort_->value()), &error)) {
    appendLog(QString("Failed to start server: %1").arg(error));
    return;
  }

  serverToggleButton_->setText(QStringLiteral("Stop Server"));
  serverPort_->setEnabled(false);
}

void MainWindow::onConnectDisconnectClient() {
  if (client_->isConnected()) {
    client_->disconnectFromHost();
    return;
  }

  client_->connectToHost(clientHost_->text().trimmed(), static_cast<quint16>(clientPort_->value()));
}

void MainWindow::onSendFileFromServer() {
  PeerConnection* peer = selectedServerPeer();
  if (!peer) {
    appendLog("Select a client first.");
    return;
  }

  const QString filePath = QFileDialog::getOpenFileName(this, QStringLiteral("Select File"));
  if (filePath.isEmpty()) {
    return;
  }

  QString error;
  if (!peer->sendFile(filePath, &error)) {
    appendLog(QString("Failed to send file: %1").arg(error));
  }
}

void MainWindow::onSendFileFromClient() {
  if (!client_->isConnected()) {
    appendLog("Client is not connected.");
    return;
  }

  const QString filePath = QFileDialog::getOpenFileName(this, QStringLiteral("Select File"));
  if (filePath.isEmpty()) {
    return;
  }

  QString error;
  if (!client_->peer()->sendFile(filePath, &error)) {
    appendLog(QString("Failed to send file: %1").arg(error));
  }
}

void MainWindow::onLocalClipboardChanged(const QString& text) {
  if (server_->isRunning() && serverClipboardCheck_->isChecked()) {
    const auto clients = server_->clients();
    for (const ClientInfo& info : clients) {
      if (info.peer && info.peer->isConnected()) {
        info.peer->sendClipboardText(text);
      }
    }
  }

  if (client_->isConnected() && clientClipboardCheck_->isChecked()) {
    client_->peer()->sendClipboardText(text);
  }
}

void MainWindow::onClipboardTextReceived(const QString& text) {
  const bool allow = (server_->isRunning() && serverClipboardCheck_->isChecked()) ||
                     (client_->isConnected() && clientClipboardCheck_->isChecked());
  if (!allow) {
    return;
  }
  clipboard_->applyRemoteText(text);
}

void MainWindow::onClientConnected(PeerConnection* peer) {
  addClientRow(peer);
  connect(peer, &PeerConnection::clipboardTextReceived, this, &MainWindow::onClipboardTextReceived);
  connect(peer, &PeerConnection::fileReceived, this, &MainWindow::onClientFileReceived);
  connect(peer, &PeerConnection::logMessage, this, &MainWindow::onClientLogMessage);
  connect(peer, &PeerConnection::returnControlRequested, inputShare_, &InputShareController::requestReturn);
}

void MainWindow::onClientDisconnected(PeerConnection* peer) {
  removeClientRow(peer);
}

void MainWindow::onClientTableSelectionChanged() {
  serverSendFileButton_->setEnabled(selectedServerPeer() != nullptr);
}

void MainWindow::onClientFileReceived(const QString& path) {
  appendLog(QString("File received: %1").arg(path));
}

void MainWindow::onClientLogMessage(const QString& message) {
  appendLog(message);
}

void MainWindow::addClientRow(PeerConnection* peer) {
  const int row = clientTable_->rowCount();
  clientTable_->insertRow(row);

  auto* item = new QTableWidgetItem(peer->peerId());
  item->setData(Qt::UserRole, QVariant::fromValue<qulonglong>(reinterpret_cast<quintptr>(peer)));
  clientTable_->setItem(row, 0, item);

  auto* combo = new QComboBox(clientTable_);
  combo->addItems({QStringLiteral("Left"), QStringLiteral("Right"), QStringLiteral("Up"), QStringLiteral("Down"), QStringLiteral("Unknown")});
  combo->setCurrentText(QStringLiteral("Unknown"));
  clientTable_->setCellWidget(row, 1, combo);

  connect(combo, &QComboBox::currentTextChanged, this, [this, peer](const QString& text) {
    server_->setClientPosition(peer, parsePosition(text));
  });
}

void MainWindow::removeClientRow(PeerConnection* peer) {
  for (int row = 0; row < clientTable_->rowCount(); ++row) {
    auto* item = clientTable_->item(row, 0);
    if (!item) {
      continue;
    }
    const qulonglong stored = item->data(Qt::UserRole).toULongLong();
    if (stored == reinterpret_cast<quintptr>(peer)) {
      clientTable_->removeRow(row);
      return;
    }
  }
}

PeerConnection* MainWindow::selectedServerPeer() const {
  const int row = clientTable_->currentRow();
  if (row < 0) {
    return nullptr;
  }
  auto* item = clientTable_->item(row, 0);
  if (!item) {
    return nullptr;
  }
  const qulonglong stored = item->data(Qt::UserRole).toULongLong();
  return reinterpret_cast<PeerConnection*>(stored);
}

void MainWindow::appendLog(const QString& message) {
  log_->appendPlainText(message);
}

ClientPosition MainWindow::parsePosition(const QString& text) const {
  if (text == QStringLiteral("Left")) return ClientPosition::Left;
  if (text == QStringLiteral("Right")) return ClientPosition::Right;
  if (text == QStringLiteral("Up")) return ClientPosition::Up;
  if (text == QStringLiteral("Down")) return ClientPosition::Down;
  return ClientPosition::Unknown;
}

} // namespace syncmouse
