#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QStringList>
#include <QStringView>
#include "api.hpp"
#include "pages/ProxySessionsPage/structSessionInfo.hpp"

class ProxySessionsPageWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit ProxySessionsPageWidget(QWidget* parent = nullptr);
    
    public slots:
        void displaySessions(QVector<SessionInfo> activeConnectionsVector);
    
    private:
        void setupUI();
    
    private:
        QVBoxLayout* layout_{nullptr};

        QTextEdit* sessionsDisplayWidget_;
};