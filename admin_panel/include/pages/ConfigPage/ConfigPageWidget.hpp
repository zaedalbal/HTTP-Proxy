#pragma once
#include <QWidget>
#include <QVBoxLayout>

class ConfigPageWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit ConfigPageWidget(QWidget* parent = nullptr);
    
    private:
        QVBoxLayout* layout_{nullptr};

        void setupUI();
};