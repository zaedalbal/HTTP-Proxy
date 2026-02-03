#pragma once
#include <QWidget>
#include <QVBoxLayout>

class LogPageWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit LogPageWidget(QWidget* parent = nullptr);
    
    private:
        QVBoxLayout* layout_{nullptr};

        void setupUI();
};