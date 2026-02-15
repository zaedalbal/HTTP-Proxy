#include "pages/ConfigPage/ConfigPageWidget.hpp"
#include <QLabel>

ConfigPageWidget::ConfigPageWidget(QWidget* parent) : QWidget(parent)
{
    setupUI();
}

void ConfigPageWidget::setupUI()
{
    layout_ = new QVBoxLayout(this);
    
    //auto* title = new QLabel("Config page123");
    //layout_->addWidget(title);

    configDisplayWidget_ = new QTextEdit(this);
    configDisplayWidget_->setText("NONE");
    configDisplayWidget_->setReadOnly(true);
    QFont font = configDisplayWidget_->font();
    font.setPointSize(16);
    configDisplayWidget_->setFont(font);

    layout_->addWidget(configDisplayWidget_);
}

void ConfigPageWidget::displayConfig(QString config)
{
    configDisplayWidget_->setText(config);
}