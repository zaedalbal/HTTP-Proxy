#pragma once
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

class AuthenticationPageWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit AuthenticationPageWidget(QWidget* parent = nullptr);

        void startSetupUI();
        
    signals:
        void loginSuccess();

        void connectButtonClicked(const QString& ip, const QString& port, const QString& login, const QString& password);
        
    private:
        void setupUI();

    private:
        QVBoxLayout* mainLayout;
        QFormLayout* formLayout;
        QLineEdit* ipEdit_;
        QLineEdit* portEdit_;
        QLineEdit* loginEdit_;
        QLineEdit* passwordEdit_;
        QPushButton* connectButton_;
};