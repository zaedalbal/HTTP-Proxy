#pragma once
#include <QWidget>
#include <QVBoxLayout>

class ActiveConnectionsPageWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit ActiveConnectionsPageWidget(QWidget* parent = nullptr);
    
    private:
        QVBoxLayout* layout_{nullptr};

        void setupUI();
};