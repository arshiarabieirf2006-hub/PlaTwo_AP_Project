#include "loginform.h"
#include "ui_loginform.h"
#include <QMessageBox>
#include "signupform.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QCryptographicHash>
#include "mainwindow.h"
#include "mainmenu.h"
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

    QString usernameInput = ui->usernameLineEdit->text().trimmed();
    QString passwordInput = ui->passwordLineEdit->text();


    if (usernameInput.isEmpty() || passwordInput.isEmpty()){
        QMessageBox::warning(this, "Warning", "Please enter both username and password!");
        return;
    }


    QByteArray hashedInput = QCryptographicHash::hash(passwordInput.toUtf8(), QCryptographicHash::Sha256).toHex();
    QString hashedPasswordStr = QString(hashedInput);

    QFile file("users.txt");
    bool loginSuccessful = false;


    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);

        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList userDetails = line.split(",");


            if (userDetails.size() >= 5) {
                QString savedUsername = userDetails[0];
                QString savedPasswordHash = userDetails[1];


                if (savedUsername == usernameInput && savedPasswordHash == hashedPasswordStr) {
                    loginSuccessful = true;
                    break;
                }
            }
        }
        file.close();
    } else {

        QMessageBox::critical(this, "System Error", "Database not found. Please sign up first.");
        return;
    }


    if (loginSuccessful) {
        QMessageBox::information(this, "Success", "Login successful. Welcome.");

        MainMenu *mainPage = new MainMenu();
        mainPage->setUsername(usernameInput);
        mainPage->show();

        this->deleteLater();


    } else {
        QMessageBox::critical(this, "Error", "Incorrect username or password!");
    }
}


void LoginForm::on_SignUp_clicked()
{

    SignUpForm *signUpPage = new SignUpForm();
    signUpPage->show();

    this->deleteLater();
}