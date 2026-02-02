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

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    std::cout << "constructor called\n";
    setupUI();
} 

void MainWindow::setupUI()
{
    // настройка главного окна
    Central = new QWidget(this);
    setCentralWidget(Central);
    setWindowTitle("Proxy control panel");
    CentralLayout = new QHBoxLayout(Central);
    
    // настройка меню
    Menu = new QListWidget;
    Menu->addItem("Log");
    Menu->addItem("Config");
    Menu->addItem("Active connections");

    // настройка страниц
    Pages = new QStackedWidget;
    LogPage = new QLabel("LogPage");
    ConfigPage = new QLabel("ConfigPage");
    ActiveConnectionsPage = new QLabel("ActiveConnectionsPage");
    Pages->addWidget(LogPage);
    Pages->addWidget(ConfigPage);
    Pages->addWidget(ActiveConnectionsPage);

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

MainWindow::~MainWindow()
{}