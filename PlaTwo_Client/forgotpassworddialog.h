#ifndef FORGOTPASSWORDDIALOG_H
#define FORGOTPASSWORDDIALOG_H

#include <QDialog>
#include <QTcpSocket>

namespace Ui {
class ForgotPasswordDialog;
}

class ForgotPasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ForgotPasswordDialog(QTcpSocket *socket, QWidget *parent = nullptr);
    ~ForgotPasswordDialog();

private slots:
    void on_resetButton_clicked();
    void onReadyRead();

private:
    Ui::ForgotPasswordDialog *ui;
    QTcpSocket *m_socket;
};

#endif // FORGOTPASSWORDDIALOG_H