#ifndef PROFILEFORM_H
#define PROFILEFORM_H

#include <QWidget>
#include <QCryptographicHash>
namespace Ui {
class ProfileForm;
}

class ProfileForm : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileForm(QWidget *parent = nullptr);
    ~ProfileForm();

    void loadUserData(const QString &currentUsername);

private slots:

    void on_backButton_clicked();

    void on_saveButton_clicked();

private:
    Ui::ProfileForm *ui;
    QString currentUser;
    QString currentPasswordHash;
};

#endif