#include "loginform.h"
#include "ui_loginform.h"
#include "signupform.h"
#include "mainmenu.h"
#include <QMessageBox>
#include <QCryptographicHash>

LoginForm::LoginForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginForm)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &LoginForm::onReadyRead);
}

LoginForm::~LoginForm()
{
    delete ui;
}

void LoginForm::on_loginButton_clicked()
{
    if (!ui->usernameLineEdit || !ui->passwordLineEdit) {
        QMessageBox::critical(this, "UI Error", "UI elements are missing!");
        return;
    }

    QString username = ui->usernameLineEdit->text().trimmed();
    QString password = ui->passwordLineEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields!");
        return;
    }

    if (socket->state() != QAbstractSocket::ConnectedState) {
        socket->connectToHost("127.0.0.1", 8080);
        if (!socket->waitForConnected(3000)) {
            QMessageBox::critical(this, "Connection Error", "Cannot connect to server!");
            return;
        }
    }

    QByteArray passwordData = password.toUtf8();
    QByteArray hashedPassword = QCryptographicHash::hash(passwordData, QCryptographicHash::Sha256).toHex();

    QString request = "LOGIN:" + username + ":" + QString(hashedPassword);
    socket->write(request.toUtf8());
}

void LoginForm::onReadyRead()
{
    QByteArray response = socket->readAll();
    QString msg = QString::fromUtf8(response).trimmed();

    if (msg == "LOGIN_SUCCESS") {
        QMessageBox::information(this, "Success", "Login successful!");

        MainMenu *menu = new MainMenu();
        menu->setAttribute(Qt::WA_DeleteOnClose);
        menu->show();

        this->close();
    } else {
        QMessageBox::critical(this, "Login Failed", "Invalid username or password!");
    }
}

void LoginForm::on_SignUp_clicked()
{
    SignUpForm *signUpPage = new SignUpForm();
    signUpPage->setAttribute(Qt::WA_DeleteOnClose);
    signUpPage->show();
    this->close();
}