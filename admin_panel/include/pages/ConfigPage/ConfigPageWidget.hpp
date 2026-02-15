#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QTextEdit>

class ConfigPageWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit ConfigPageWidget(QWidget* parent = nullptr);

    public slots:
        void displayConfig(QString config);

    private:
        void setupUI();

    private:
        QVBoxLayout* layout_{nullptr};

        QTextEdit* configDisplayWidget_;
};