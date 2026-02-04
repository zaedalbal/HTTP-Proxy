#include "pages/ActiveConnectionsPage/ActiveConnectionsPageWidget.hpp"
#include <QLabel>

ActiveConnectionsPageWidget::ActiveConnectionsPageWidget(QWidget* parent) : QWidget(parent)
{
    setupUI();
}

void ActiveConnectionsPageWidget::setupUI()
{
    layout_ = new QVBoxLayout(this);
    
    auto* title = new QLabel("Active connections page");
    layout_->addWidget(title);
}