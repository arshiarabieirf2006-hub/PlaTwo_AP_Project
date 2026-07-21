#ifndef SIGNUPFORM_H
#define SIGNUPFORM_H

#include <QWidget>
#include <QTcpSocket>

namespace Ui {
class SignUpForm;
}

class SignUpForm : public QWidget
{
    Q_OBJECT

public:
    explicit SignUpForm(QWidget *parent = nullptr);
    ~SignUpForm();

private slots:
    void on_signUpButton_clicked();
    void on_backToLoginButton_clicked();
    void onReadyRead();

private:
    Ui::SignUpForm *ui;
    QTcpSocket *socket;
};

#endif // SIGNUPFORM_H