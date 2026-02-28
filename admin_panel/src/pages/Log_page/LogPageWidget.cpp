#include "pages/LogPage/LogPageWidget.hpp"
#include <QLabel>

LogPageWidget::LogPageWidget(QWidget* parent) : QWidget(parent)
{
    setupUI();
}

void LogPageWidget::setupUI()
{
    layout_ = new QVBoxLayout(this);
    
    logDisplayWidget = new QPlainTextEdit;
    logDisplayWidget->setPlainText("NONE");
    logDisplayWidget->setReadOnly(true);
    
    refreshButton_ = new QPushButton;
    refreshButton_->setText("refresh");
    connect(refreshButton_, &QPushButton::clicked, this, [this](){emit refreshButtonClicked();});

    layout_->addWidget(logDisplayWidget);
    layout_->addWidget(refreshButton_);
}

void LogPageWidget::displayLog(QString log)
{
    logDisplayWidget->setPlainText(log);
}