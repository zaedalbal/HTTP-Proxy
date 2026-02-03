#pragma once
#include <QMainWindow>
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
#include <QListWidget>
#include <QStackedWidget>
#include "pages/ActiveConnectionsPage/ActiveConnectionsPageWidget.hpp"
#include "pages/ConfigPage/ConfigPageWidget.hpp"
#include "pages/LogPage/LogPageWidget.hpp"

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        MainWindow(QWidget* parent = nullptr); // constructor
        ~MainWindow(); // destructor

    private:
        // основное окно
        QWidget* Central; 
        QHBoxLayout* CentralLayout;

        // меню
        QListWidget* Menu; 

        // страницы
        QStackedWidget* Pages;
        ActiveConnectionsPageWidget* ActiveConnectionsPage;
        ConfigPageWidget* ConfigPage;
        LogPageWidget* LogPage;

        // сплитер (для изменения размера меню и страниц)
        QSplitter* Splitter;

    private:
        void setupUI();
        void pages_setup();
};