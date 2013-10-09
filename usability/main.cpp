/// @file main.cpp
/// @brief qt usability application
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-10-09
#include <QApplication>

#include "usability.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    UsabilityDialog d;
    d.show();

    return app.exec();
}
