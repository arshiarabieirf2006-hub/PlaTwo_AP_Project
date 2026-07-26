#include "profileform.h"
#include "ui_profileform.h"
#include <QStringList>
#include <QMessageBox>

ProfileForm::ProfileForm(QTcpSocket *socket, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProfileForm),
    m_socket(socket)
{
    ui->setupUi(this);

    if (m_socket) {
        connect(m_socket, &QTcpSocket::readyRead, this, &ProfileForm::onSocketReadyRead);
    }
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

    currentUser = currentUsername;
    ui->usernameLineEdit->setText(currentUsername);

    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        QMessageBox::critical(this, "Error", "Not connected to server - cannot load profile.");
        return;
    }

    m_socket->write(QString("PROFILE_GET:%1\n").arg(currentUsername).toUtf8());
}

void ProfileForm::onSocketReadyRead()
{
    if (!m_socket) return;

    while (m_socket->canReadLine()) {
        QString line = QString::fromUtf8(m_socket->readLine()).trimmed();

        if (line.startsWith("PROFILE_DATA:")) {
            QString data = line.mid(QString("PROFILE_DATA:").length());
            QStringList details = data.split(",");
            if (details.size() >= 6) {
                currentUser = details[0];
                currentPasswordHash = details[1];

                ui->usernameLineEdit->setText(details[0]);
                ui->passwordLineEdit->clear();
                ui->emailLineEdit->setText(details[2]);
                ui->nameLineEdit->setText(details[3]);
                ui->phoneLineEdit->setText(details[4]);
            }
        } else if (line == "PROFILE_NOTFOUND") {
            QMessageBox::warning(this, "Error", "Could not find your profile on the server.");
        } else if (line == "PROFILE_UPDATE_SUCCESS") {
            currentUser = ui->usernameLineEdit->text().trimmed();
            ui->passwordLineEdit->clear();
            QMessageBox::information(this, "Success", "Profile updated successfully!");
        } else if (line == "PROFILE_UPDATE_FAILED") {
            QMessageBox::critical(this, "Error", "Could not save updates.");
        }
    }
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

    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        QMessageBox::critical(this, "Error", "Not connected to server - cannot save profile.");
        return;
    }

    QString newPasswordHash;
    if (!newPassword.isEmpty()) {
        newPasswordHash = QString(QCryptographicHash::hash(newPassword.toUtf8(), QCryptographicHash::Sha256).toHex());
    }

    QString request = QString("PROFILE_UPDATE:%1:%2:%3:%4:%5:%6\n")
                          .arg(currentUser, newUsername, newPasswordHash, newEmail, newName, newPhone);
    m_socket->write(request.toUtf8());
}
