#include "forgotpassworddialog.h"
#include <QCryptographicHash>
#include <QMessageBox>
#include "ui_forgotpassworddialog.h"

ForgotPasswordDialog::ForgotPasswordDialog(QTcpSocket *socket, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ForgotPasswordDialog)
    , m_socket(socket)
{
    ui->setupUi(this);

    ui->usernameLineEdit->setPlaceholderText("Enter Username...");
    ui->newPasswordLineEdit->setPlaceholderText("Enter New Password...");
    ui->newPasswordLineEdit->setEchoMode(QLineEdit::Password);

    connect(ui->resetButton, &QPushButton::clicked, this, &ForgotPasswordDialog::on_resetButton_clicked);

    if (m_socket) {
        connect(m_socket, &QTcpSocket::readyRead, this, &ForgotPasswordDialog::onReadyRead);
    }
}

ForgotPasswordDialog::~ForgotPasswordDialog()
{
    delete ui;
}

void ForgotPasswordDialog::on_resetButton_clicked()
{
    QString username = ui->usernameLineEdit->text().trimmed();
    QString newPassword = ui->newPasswordLineEdit->text();

    if (username.isEmpty() || newPassword.isEmpty()) {
        QMessageBox::warning(this, "خطا", "لطفاً تمامی فیلدها را پر کنید!");
        return;
    }


    if (!m_socket) {
        QMessageBox::critical(this, "خطای اتصال", "امکان اتصال به سرور وجود ندارد!");
        return;
    }

    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        m_socket->connectToHost("127.0.0.1", 8080);
        if (!m_socket->waitForConnected(3000)) {
            QMessageBox::critical(this, "خطای اتصال", "امکان اتصال به سرور وجود ندارد!");
            return;
        }
    }

    QByteArray passwordData = newPassword.toUtf8();
    QByteArray hashedPassword = QCryptographicHash::hash(passwordData, QCryptographicHash::Sha256).toHex();

    QString request = "FORGOT_PASS:" + username + ":" + QString(hashedPassword) + "\n";
    m_socket->write(request.toUtf8());
    m_socket->flush();
}

void ForgotPasswordDialog::onReadyRead()
{
    if (!m_socket) return;
    QByteArray response = m_socket->readAll();
    QString msg = QString::fromUtf8(response).trimmed();

    if (msg.contains("FORGOT_PASS_SUCCESS")) {
        QMessageBox::information(this, "موفقیت", "رمز عبور با موفقیت تغییر یافت!");
        this->accept();
    } else if (msg.contains("FORGOT_PASS_FAILED")) {
        QMessageBox::critical(this, "خطا", "نام کاربری یافت نشد یا عملیات با خطا مواجه شد!");
    }
}
