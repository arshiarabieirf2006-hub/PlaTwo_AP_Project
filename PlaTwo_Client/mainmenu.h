#ifndef MAINMENU_H
#define MAINMENU_H

#include <QWidget>
#include <QString>

namespace Ui {
class MainMenu;
}

class MainMenu : public QWidget
{
    Q_OBJECT

public:
    explicit MainMenu(QWidget *parent = nullptr);
    ~MainMenu();


    void setUsername(const QString &username);

private slots:
    void on_exitButton_clicked();


    void on_profileButton_clicked();

    void on_leaderboardButton_clicked();

    void on_storeButton_clicked();

    void on_startGameButton_clicked();

private:
    Ui::MainMenu *ui;


    QString loggedInUser;
};

#endif