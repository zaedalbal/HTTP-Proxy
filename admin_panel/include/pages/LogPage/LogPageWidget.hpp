#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QPlainTextEdit>

class LogPageWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit LogPageWidget(QWidget* parent = nullptr);

    public slots:
        void displayLog(QString log);

    private:
        QVBoxLayout* layout_{nullptr};

        QPlainTextEdit* logDisplayWidget;

        void setupUI();
};