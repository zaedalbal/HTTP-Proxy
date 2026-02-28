#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QPushButton>

class LogPageWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit LogPageWidget(QWidget* parent = nullptr);

    public slots:
        void displayLog(QString log);

    signals:
        void refreshButtonClicked();

    private:
        QVBoxLayout* layout_{nullptr};

        QPlainTextEdit* logDisplayWidget;

        QPushButton* refreshButton;

        void setupUI();
};