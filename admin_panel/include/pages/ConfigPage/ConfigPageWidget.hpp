#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>

class ConfigPageWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit ConfigPageWidget(QWidget* parent = nullptr);

    public slots:
        void displayConfig(QString config);

    signals:
        void refreshButtonClicked();

    private:
        void setupUI();

    private:
        QVBoxLayout* layout_{nullptr};

        QTextEdit* configDisplayWidget_;

        QPushButton* refreshButton_;
};