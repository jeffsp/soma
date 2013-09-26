#include <QApplication>

#include "usability.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    UsabilityDialog d;
    d.show();

    return app.exec();
}
