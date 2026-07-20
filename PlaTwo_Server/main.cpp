#include <QCoreApplication>
#include "servercontroller.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ServerController server;

    return a.exec();
}
