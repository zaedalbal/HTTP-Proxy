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
#include "pages/ProxySessionsPage/ProxySessionsPageWidget.hpp"
#include "pages/AuthenticationPage/AuthenticationPageWidget.hpp"
#include "pages/ConfigPage/ConfigPageWidget.hpp"
#include "pages/LogPage/LogPageWidget.hpp"
#include "server_interaction/ServerInteraction.hpp"
#include "server_interaction/ResponseHandler.hpp"
#include "server_interaction/RequestFacade.hpp"

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        MainWindow(QWidget* parent = nullptr); // constructor
        ~MainWindow(); // destructor

// приватное поле для объектов интерфейса
    private:
        // основное окно
        QWidget* Central; 
        QHBoxLayout* CentralLayout;

        // меню
        QListWidget* Menu; 

        // страницы
        QStackedWidget* Pages;
        ProxySessionsPageWidget* ActiveConnectionsPage;
        AuthenticationPageWidget* AuthenticationPage;
        ConfigPageWidget* ConfigPage;
        LogPageWidget* LogPage;

        // сплитер (для изменения размера меню и страниц)
        QSplitter* Splitter;

// приватное поле для взаимодействия с сервером
    private:
        ServerInteraction ServerInteraction_;

        ResponseHandler ResponseHandler_;

        RequestFacade* RequestFacade_;

        QString lastLogin_;
        QString lastPassword_;
        
    private:
        void initialSetup();

        void setupMainUI();
    
    private slots:
        void onLoginSuccess();

        void onConnectButtonClicked(const QString& ip, const QString& port, const QString& login, const QString& password);

};