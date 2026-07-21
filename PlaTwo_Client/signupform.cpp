#include "signupform.h"
#include "ui_signupform.h"
#include "loginform.h"
#include <QRegularExpression>
#include <QMessageBox>
#include <QCryptographicHash>

SignUpForm::SignUpForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SignUpForm)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &SignUpForm::onReadyRead);
}

SignUpForm::~SignUpForm()
{
    delete ui;
}

void SignUpForm::on_signUpButton_clicked()
{
    if (!ui->nameEdit || !ui->usernameEdit || !ui->phoneEdit || !ui->emailEdit || !ui->passwordEdit) {
        QMessageBox::critical(this, "UI Error", "UI components missing!");
        return;
    }

    QString name = ui->nameEdit->text().trimmed();
    QString username = ui->usernameEdit->text().trimmed();
    QString phone = ui->phoneEdit->text().trimmed();
    QString email = ui->emailEdit->text().trimmed();
    QString password = ui->passwordEdit->text();

    if (name.isEmpty() || username.isEmpty() || phone.isEmpty() || email.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "All fields are required!");
        return;
    }

    QRegularExpression phoneRegex("^09\\d{9}$");
    if (!phoneRegex.match(phone).hasMatch()) {
        QMessageBox::warning(this, "Invalid Phone", "Please enter a valid phone number.");
        return;
    }

    QRegularExpression emailRegex("^[\\w\\.-]+@[\\w\\.-]+\\.[a-zA-Z]{2,6}$");
    if (!emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "Invalid Email", "Please enter a valid email address.");
        return;
    }

    if (password.length() < 8) {
        QMessageBox::warning(this, "Weak Password", "Password must be at least 8 characters long.");
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
    QString hashedPasswordStr = QString(hashedPassword);

    QString request = "SIGNUP:" + username + ":" + hashedPasswordStr + ":" + email + ":" + name + ":" + phone;
    socket->write(request.toUtf8());
}

void SignUpForm::onReadyRead()
{
    QByteArray response = socket->readAll();
    QString msg = QString::fromUtf8(response).trimmed();

    if (msg == "SIGNUP_SUCCESS") {
        QMessageBox::information(this, "Success", "Account created successfully! Please login.");

        LoginForm *loginPage = new LoginForm();
        loginPage->show();
        this->hide();
    } else if (msg == "SIGNUP_DUPLICATE") {
        QMessageBox::warning(this, "Duplicate User", "This username is already taken. Please choose another one.");
    } else {
        QMessageBox::critical(this, "Error", "Registration failed!");
    }
}

void SignUpForm::on_backToLoginButton_clicked()
{
    LoginForm *loginPage = new LoginForm();
    loginPage->setAttribute(Qt::WA_DeleteOnClose);
    loginPage->show();
    this->close();
}