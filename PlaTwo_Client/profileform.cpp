#include "profileform.h"
#include "ui_profileform.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QMessageBox>

ProfileForm::ProfileForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProfileForm)
{
    ui->setupUi(this);
}

ProfileForm::~ProfileForm()
{
    delete ui;
}


void ProfileForm::on_backButton_clicked()
{
    this->close();
}


void ProfileForm::loadUserData(const QString &currentUsername)
{
    QFile file("users.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not open users.txt to read profile data.");
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList userDetails = line.split(",");

        // بررسی اینکه  کاربر فعلی است یا خیر
        if (userDetails.size() >= 5 && userDetails[0] == currentUsername) {


            ui->usernameLineEdit->setText(userDetails[0]);
            ui->emailLineEdit->setText(userDetails[2]);
            ui->nameLineEdit->setText(userDetails[3]);
            ui->phoneLineEdit->setText(userDetails[4]);

            break;
        }
    }
    file.close();
}