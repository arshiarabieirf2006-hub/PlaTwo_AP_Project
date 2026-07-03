#include "loginform.h"
#include "ui_loginform.h"
#include <QMessageBox>
#include "signupform.h"
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
    QString username = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    if (username.isEmpty() || password.isEmpty()){
        QMessageBox::warning(this,"Warning", "Please enter both username and password!");
        return;
    }
    if(username == "admin" && password == "1234"){
        QMessageBox::information(this,"Success","Login successful.Welcome.");
        //code
    }
    else{
        QMessageBox::critical(this,"Error", "Incorrect username or password!");
    }
}


void LoginForm::on_SignUp_clicked()
{

        //  Create a new sign up page
        SignUpForm *signUpPage = new SignUpForm();


        signUpPage->show();


        this->deleteLater();

}

