#include "pages/ProxySessionsPage/ProxySessionsPageWidget.hpp"
#include <QLabel>

ProxySessionsPageWidget::ProxySessionsPageWidget(QWidget* parent) : QWidget(parent)
{
    setupUI();
}

void ProxySessionsPageWidget::setupUI()
{
    layout_ = new QVBoxLayout(this);

    sessionsDisplayWidget_ = new QTextEdit(this);
    sessionsDisplayWidget_->setText("NONE");
    sessionsDisplayWidget_->setReadOnly(true);

    layout_->addWidget(sessionsDisplayWidget_);
}

void ProxySessionsPageWidget::displaySessions(QVector<SessionInfo> activeConnectionsVector)
{}