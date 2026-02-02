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
        QWidget* LogPage;
        QWidget* ConfigPage;
        QWidget* ActiveConnectionsPage;
        QVBoxLayout* LogPage_Layout;
        QVBoxLayout* ConfigPage_Layout;
        QVBoxLayout* ActiveConnectionsPage_Layout;

        // сплитер (для изменения размера меню и страниц)
        QSplitter* Splitter;

    private:
        void setupUI();
        void pages_setup();
};