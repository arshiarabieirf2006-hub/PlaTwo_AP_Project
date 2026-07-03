#include "signupform.h"
#include "ui_signupform.h"
#include <QRegularExpression>
#include <QMessageBox>
#include <QCryptographicHash>

SignUpForm::SignUpForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SignUpForm)
{
    ui->setupUi(this);
}

SignUpForm::~SignUpForm()
{
    delete ui;
}

void SignUpForm::on_signUpButton_clicked()
{

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
        QMessageBox::warning(this, "Invalid Phone", "Please enter a valid phone number (e.g., 09123456789).");
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

    QByteArray passwordData = password.toUtf8();
    QByteArray hashedPassword = QCryptographicHash::hash(passwordData, QCryptographicHash::Sha256).toHex();

    QString successMessage = QString("Validation passed successfully!\n\nUsername: %1\nHashed Password:\n%2")
                                 .arg(username).arg(QString(hashedPassword));

    QMessageBox::information(this, "Success", successMessage);

    ui->nameEdit->clear();
    ui->usernameEdit->clear();
    ui->phoneEdit->clear();
    ui->emailEdit->clear();
    ui->passwordEdit->clear();

}