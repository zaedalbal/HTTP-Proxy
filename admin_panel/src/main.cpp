#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Example Qt6 Widgets");
    window.setMinimumSize(200, 100);

    QLabel *label = new QLabel("always ok");
    label->setAlignment(Qt::AlignCenter);

    QVBoxLayout *layout = new QVBoxLayout(&window);
    layout->addWidget(label);

    window.show();

    return app.exec();
}
