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

        void setupUI();
    private:
        // основное окно
        QWidget* Central; 
        QHBoxLayout* CentralLayout;

        // меню
        QListWidget* Menu; 

        // страницы
        QStackedWidget* Pages; 
        QLabel* LogPage;
        QLabel* ConfigPage;
        QLabel* ActiveConnectionsPage;

        // сплитер (для изменения размера меню и страниц)
        QSplitter* Splitter;
};