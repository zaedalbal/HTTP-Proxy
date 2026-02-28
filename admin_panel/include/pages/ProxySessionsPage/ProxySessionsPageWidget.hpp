#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QStringList>
#include <QStringView>
#include <QTableView>
#include <QPushButton>
#include "api.hpp"
#include "pages/ProxySessionsPage/SessionsModel/SessionsModel.hpp"

class ProxySessionsPageWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit ProxySessionsPageWidget(QWidget* parent = nullptr);
    
    public slots:
        void displaySessions(std::vector<SessionInfo> sessions);
    signals:
        void refreshButtonClicked();
    
    private:
        void setupUI();
    
    private:
        QVBoxLayout* layout_{nullptr};

        QTableView* table_;

        SessionModel* SessionModel_;

        QPushButton* refreshButton_;
};