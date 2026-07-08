#include "signupform.h"
#include "ui_signupform.h"
#include "loginform.h"
#include <QRegularExpression>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QFile>
#include <QTextStream>
#include <QStringList>

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

    // اعتبارسنجی ورود
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

    //  رمزنگاری پسورد
    QByteArray passwordData = password.toUtf8();
    QByteArray hashedPassword = QCryptographicHash::hash(passwordData, QCryptographicHash::Sha256).toHex();
    QString hashedPasswordStr = QString(hashedPassword);

    // ذخیره در فایل
    QFile file("users.txt");
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);


        out << username << "," << hashedPasswordStr << "," << email << "," << name << "," << phone << "\n";

        file.close();

        QMessageBox::information(this, "Success", "Account created successfully!");


        ui->nameEdit->clear();
        ui->usernameEdit->clear();
        ui->phoneEdit->clear();
        ui->emailEdit->clear();
        ui->passwordEdit->clear();
    }
    else {
        QMessageBox::warning(this, "Error", "Could not create user record!");
    }
}


void SignUpForm::on_backToLoginButton_clicked()
{
    LoginForm *loginPage = new LoginForm();
    loginPage->show();

    this->deleteLater();
}

