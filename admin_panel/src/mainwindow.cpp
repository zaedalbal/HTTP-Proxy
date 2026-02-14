#include "mainwindow.hpp"
#include <iostream>
#include <QApplication>
#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QLabel>
#include <QSplitter>
#include <QGroupBox>
#include <QMessageBox>
#include <QCheckBox>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    RequestFacade_ = new RequestFacade(ServerInteraction_);
    std::cout << "constructor called\n";
    initialSetup();
} 

void MainWindow::initialSetup()
{
    connect(&ServerInteraction_, &ServerInteraction::successfulConnect, this, [this]()
    {
        RequestFacade_->sendAuthenticationRequest(lastLogin_, lastPassword_);
    });

    connect(&ServerInteraction_, &ServerInteraction::responseReceived, &ResponseHandler_, &ResponseHandler::handleResponse);
    connect(&ResponseHandler_, &ResponseHandler::Signal_AuthenticationSuccessful, this, &MainWindow::setupMainUI);

    // настройка главного окна
    Central = new QWidget(this);
    setCentralWidget(Central);
    setWindowTitle("Proxy control panel");
    resize(1200, 600);
    CentralLayout = new QHBoxLayout(Central);
    AuthenticationPage = new AuthenticationPageWidget(Central);
    CentralLayout->addWidget(AuthenticationPage);
    connect(AuthenticationPage, &AuthenticationPageWidget::loginSuccess, this, &MainWindow::onLoginSuccess);
    connect(AuthenticationPage, &AuthenticationPageWidget::connectButtonClicked, this, &MainWindow::onConnectButtonClicked);
    AuthenticationPage->startSetupUI();
}

void MainWindow::setupMainUI()
{
    std::cout << "Setup mainUI\n";
    // настройка меню
    AuthenticationPage->hide();
    Menu = new QListWidget;
    Menu->addItem("Active connections");
    Menu->addItem("Config");
    Menu->addItem("Log");

    // настройка страниц
    Pages = new QStackedWidget(Central);
    ActiveConnectionsPage = new ActiveConnectionsPageWidget;
    ConfigPage = new ConfigPageWidget;
    LogPage = new LogPageWidget;

    Pages->addWidget(ActiveConnectionsPage);
    Pages->addWidget(ConfigPage);
    Pages->addWidget(LogPage);

    // настройка сплиттера
    connect(Menu, &QListWidget::currentRowChanged, Pages, &QStackedWidget::setCurrentIndex);
    Splitter = new QSplitter(Qt::Horizontal, Central);
    Splitter->addWidget(Menu);
    Splitter->addWidget(Pages);
    Splitter->setStretchFactor(0, 0);
    Splitter->setStretchFactor(1, 1);
    Splitter->setSizes({200, 700});
    CentralLayout->addWidget(Splitter);
}

void MainWindow::onLoginSuccess()
{
    setupMainUI();
}

void MainWindow::onConnectButtonClicked(const QString& ip, const QString& port, const QString& login, const QString& password)
{
    qDebug() << "onConnectButtonClicked slot was called\n";
    lastLogin_ = login;
    lastPassword_ = password;
    bool check = false;
    quint16 port_to_arg = port.toUShort(&check);
    QHostAddress address;
    if(!address.setAddress(ip) || !check)
    {
        QMessageBox::warning(this, "Error", "Incorrect ip or port");
        return;
    }
    if(ServerInteraction_.state() == QAbstractSocket::SocketState::UnconnectedState)
        ServerInteraction_.connectToServer(ip, port_to_arg);
    //connect(&ServerInteraction_, &ServerInteraction::successfulConnect, this, [this](){ServerInteraction_.disconnect();});
}

MainWindow::~MainWindow()
{}