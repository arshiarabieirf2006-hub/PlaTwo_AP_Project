#ifndef PROFILEFORM_H
#define PROFILEFORM_H

#include <QWidget>
#include <QCryptographicHash>
#include <QTcpSocket>
namespace Ui {
class ProfileForm;
}

class ProfileForm : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileForm(QTcpSocket *socket, QWidget *parent = nullptr);
    ~ProfileForm();

    void loadUserData(const QString &currentUsername);

private slots:

    void on_backButton_clicked();

    void on_saveButton_clicked();

    void onSocketReadyRead();

private:
    Ui::ProfileForm *ui;
    QTcpSocket *m_socket;
    QString currentUser;
    QString currentPasswordHash;
};

#endif
