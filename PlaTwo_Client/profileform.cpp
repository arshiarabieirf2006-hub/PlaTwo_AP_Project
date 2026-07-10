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


        if (userDetails.size() >= 5 && userDetails[0] == currentUsername) {


            currentUser = userDetails[0];
            currentPasswordHash = userDetails[1];


            ui->usernameLineEdit->setText(userDetails[0]);

            ui->passwordLineEdit->clear();

            ui->emailLineEdit->setText(userDetails[2]);
            ui->nameLineEdit->setText(userDetails[3]);
            ui->phoneLineEdit->setText(userDetails[4]);

            break;
        }
    }
    file.close();
}
void ProfileForm::on_saveButton_clicked()
{
    QString newUsername = ui->usernameLineEdit->text().trimmed();
    QString newPassword = ui->passwordLineEdit->text();
    QString newEmail = ui->emailLineEdit->text();
    QString newName = ui->nameLineEdit->text();
    QString newPhone = ui->phoneLineEdit->text();

    if (newUsername.isEmpty()) {
        QMessageBox::warning(this, "Error", "Username cannot be empty!");
        return;
    }

    QString finalPasswordHash = currentPasswordHash;
    if (!newPassword.isEmpty()) {
        QByteArray hashedInput = QCryptographicHash::hash(newPassword.toUtf8(), QCryptographicHash::Sha256).toHex();
        finalPasswordHash = QString(hashedInput);
    }

    QFile file("users.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not open database.");
        return;
    }


    QStringList allLines;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList userDetails = line.split(",");


        if (userDetails.size() >= 5 && userDetails[0] == currentUser) {
            userDetails[0] = newUsername;
            userDetails[1] = finalPasswordHash;
            userDetails[2] = newEmail;

            userDetails[3] = newName;
            userDetails[4] = newPhone;

            line = userDetails.join(",");
        }
        allLines.append(line);
    }
    file.close();

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::critical(this, "Error", "Could not save updates.");
        return;
    }

    QTextStream out(&file);
    for (const QString &l : allLines) {
        out << l << "\n";
    }
    file.close();

    currentUser = newUsername;
    ui->passwordLineEdit->clear();
    QMessageBox::information(this, "Success", "Profile updated successfully!");
}

