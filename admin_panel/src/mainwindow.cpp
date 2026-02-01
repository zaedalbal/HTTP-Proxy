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
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    setWindowTitle("Proxy control panel");
}

MainWindow::~MainWindow()
{}