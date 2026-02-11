#include "pages/AuthenticationPage/AuthenticationPageWidget.hpp"
#include <QLabel>
#include <QValidator>
#include <QDebug>

AuthenticationPageWidget::AuthenticationPageWidget(QWidget* parent) : QWidget(parent)
{
    qDebug() << "Auth widget constructor\n";
}

void AuthenticationPageWidget::startSetupUI()
{
    setupUI();
}

void AuthenticationPageWidget::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    formLayout = new QFormLayout();

    ipEdit_ = new QLineEdit(this);
    ipEdit_->setPlaceholderText("127.0.0.1"); // пример ввода
    
    portEdit_ = new QLineEdit(this);
    portEdit_->setPlaceholderText("65535"); // пример ввода
    portEdit_->setValidator(new QIntValidator(0, 65535, this)); // для ограничение ввода

    loginEdit_ = new QLineEdit(this);
    loginEdit_->setPlaceholderText("login");

    passwordEdit_ = new QLineEdit(this);
    passwordEdit_->setPlaceholderText("password");
    passwordEdit_->setEchoMode(QLineEdit::Password);

    formLayout->addRow("IP:", ipEdit_);
    formLayout->addRow("Port:", portEdit_);
    formLayout->addRow("Login:", loginEdit_);
    formLayout->addRow("Password:", passwordEdit_);

    mainLayout->addLayout(formLayout);

    connectButton_ = new QPushButton(this);
    
    mainLayout->addWidget(connectButton_);
}