#include "loginform.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    LoginForm *w = new LoginForm();
    w->show();

    return QApplication::exec();
}