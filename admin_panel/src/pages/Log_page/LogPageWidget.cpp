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
    
    refreshButton = new QPushButton;
    refreshButton->setText("refresh");
    connect(refreshButton, &QPushButton::clicked, this, [this](){emit refreshButtonClicked();});

    layout_->addWidget(logDisplayWidget);
    layout_->addWidget(refreshButton);
}

void LogPageWidget::displayLog(QString log)
{
    logDisplayWidget->setPlainText(log);
}