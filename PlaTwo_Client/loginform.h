#ifndef LOGINFORM_H
#define LOGINFORM_H

#include <QWidget>
#include <QTcpSocket>

namespace Ui {
class LoginForm;
}

class LoginForm : public QWidget
{
    Q_OBJECT

public:
    explicit LoginForm(QWidget *parent = nullptr);
    ~LoginForm();

private slots:
    void on_loginButton_clicked();
void on_SignUp_clicked();
    void onReadyRead();

private:
    Ui::LoginForm *ui;
    QTcpSocket *socket;
};

#endif // LOGINFORM_H