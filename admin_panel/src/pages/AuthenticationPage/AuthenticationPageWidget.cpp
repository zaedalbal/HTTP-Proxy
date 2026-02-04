#include "pages/AuthenticationPage/AuthenticationPageWidget.hpp"
#include <QLabel>
#include <iostream>

AuthenticationPageWidget::AuthenticationPageWidget(QWidget* parent) : QWidget(parent)
{
    std::cout << "Auth widget constructor\n";
}

void AuthenticationPageWidget::startSetupUI()
{
    setupUI();
}

void AuthenticationPageWidget::setupUI()
{
    layout_ = new QHBoxLayout(this);
    test_button = new QPushButton("test button");
    layout_->addWidget(test_button);
    connect(test_button, &QPushButton::clicked, this, &AuthenticationPageWidget::loginSuccess);
}

bool AuthenticationPageWidget::getAuthenticationStatus()
{
    return isAuthenticated_;
}