#pragma once
#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        MainWindow(QWidget* parent = nullptr); // constructor
        ~MainWindow(); // destructor

        void setupUI();

};