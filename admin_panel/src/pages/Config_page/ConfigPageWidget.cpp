#include "pages/ConfigPage/ConfigPageWidget.hpp"
#include <QLabel>

ConfigPageWidget::ConfigPageWidget(QWidget* parent) : QWidget(parent)
{
    setupUI();
}

void ConfigPageWidget::setupUI()
{
    layout_ = new QVBoxLayout(this);
    
    auto* title = new QLabel("Config page");
    layout_->addWidget(title);
}