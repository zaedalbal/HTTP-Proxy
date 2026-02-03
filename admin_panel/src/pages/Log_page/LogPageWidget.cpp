#include "pages/LogPage/LogPageWidget.hpp"
#include <QLabel>

LogPageWidget::LogPageWidget(QWidget* parent) : QWidget(parent)
{
    setupUI();
}

void LogPageWidget::setupUI()
{
    layout_ = new QVBoxLayout(this);
    
    auto* title = new QLabel("Log page");
    layout_->addWidget(title);
}