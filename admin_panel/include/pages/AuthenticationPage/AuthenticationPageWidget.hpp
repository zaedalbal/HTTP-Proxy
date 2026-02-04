#pragma once
#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>

class AuthenticationPageWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit AuthenticationPageWidget(QWidget* parent = nullptr);

        void startSetupUI();
    
        bool getAuthenticationStatus();
    
    signals:
        void loginSuccess();

    private:
        void setupUI();

    private:
        QPushButton* test_button; 

        bool isAuthenticated_ = false;

        QHBoxLayout* layout_{nullptr};
};