#include "loginform.h"
#include "ui_loginform.h"
#include <QMessageBox>
#include "signupform.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QCryptographicHash>
#include "mainwindow.h"
LoginForm::LoginForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginForm)
{
    ui->setupUi(this);
}

LoginForm::~LoginForm()
{
    delete ui;
}

void LoginForm::on_loginButton_clicked()
{
    //  مقادیر ورودی و حذف اسپیس از نام کاربری
    QString usernameInput = ui->usernameLineEdit->text().trimmed();
    QString passwordInput = ui->passwordLineEdit->text();

    //بررسی خالی نبودن
    if (usernameInput.isEmpty() || passwordInput.isEmpty()){
        QMessageBox::warning(this, "Warning", "Please enter both username and password!");
        return;
    }

    //  تبدیل رمزی که کاربر  تایپ کرده به کد هش
    QByteArray hashedInput = QCryptographicHash::hash(passwordInput.toUtf8(), QCryptographicHash::Sha256).toHex();
    QString hashedPasswordStr = QString(hashedInput);

    QFile file("users.txt");
    bool loginSuccessful = false;

    //  باز کردن فایل برای جستجو
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);

        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList userDetails = line.split(",");

            // در فایل ثبت نام ۵ آیتم ذخیره کردیم پس لیست  حداقل باید ۵ بخش داشته باشد
            if (userDetails.size() >= 5) {
                QString savedUsername = userDetails[0];
                QString savedPasswordHash = userDetails[1];

                //  مقایسه ورودی ها با اطلاعات فایل
                if (savedUsername == usernameInput && savedPasswordHash == hashedPasswordStr) {
                    loginSuccessful = true;
                    break;
                }
            }
        }
        file.close();
    } else {
        // اگر  کسی ثبت نام نکرده باشد
        QMessageBox::critical(this, "System Error", "Database not found. Please sign up first.");
        return;
    }


    if (loginSuccessful) {
        QMessageBox::information(this, "Success", "Login successful. Welcome.");

        MainWindow *mainPage = new MainWindow();
        mainPage->show();

        // پاک کردن صفحه لاگین
        this->deleteLater();

    } else {
        QMessageBox::critical(this, "Error", "Incorrect username or password!");
    }
}

void LoginForm::on_SignUp_clicked()
{
    // Create  sign up page
    SignUpForm *signUpPage = new SignUpForm();
    signUpPage->show();

    this->deleteLater();
}