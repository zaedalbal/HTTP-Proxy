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

    layout_->addWidget(logDisplayWidget);
}

void LogPageWidget::displayLog(QString log)
{
    logDisplayWidget->setPlainText(log);
}