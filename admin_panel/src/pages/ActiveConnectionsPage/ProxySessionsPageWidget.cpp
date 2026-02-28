#include "pages/ProxySessionsPage/ProxySessionsPageWidget.hpp"
#include <QLabel>

ProxySessionsPageWidget::ProxySessionsPageWidget(QWidget* parent) : QWidget(parent)
{
    setupUI();
}

void ProxySessionsPageWidget::setupUI()
{
    layout_ = new QVBoxLayout(this);

    SessionModel_ = new SessionModel(this);
    table_ = new QTableView(this);
    table_->setModel(SessionModel_);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    
    refreshButton_ = new QPushButton;
    refreshButton_->setText("refresh");
    connect(refreshButton_, &QPushButton::clicked, this, [this](){emit refreshButtonClicked();});

    layout_->addWidget(table_);
    layout_->addWidget(refreshButton_);
}

void ProxySessionsPageWidget::displaySessions(std::vector<SessionInfo> sessions)
{
    if(sessions.empty())
    {
        std::vector<SessionInfo> emptySessions;
        SessionInfo emptySession;
        emptySession.ip_ = "NONE SESSIONS";
        emptySessions.push_back(emptySession);
        SessionModel_->setSessions(emptySessions);
    }
    else
        SessionModel_->setSessions(sessions);
}