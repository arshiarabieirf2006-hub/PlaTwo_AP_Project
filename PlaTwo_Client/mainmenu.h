#ifndef MAINMENU_H
#define MAINMENU_H
#include <QTcpSocket>

#include <QWidget>
#include <QString>
#include "fanorona.h"
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

    void on_startMorrisButton_clicked();

    void on_pushButton_clicked();
    void sendFanoronaMove(int startRow, int startCol, int endRow, int endCol);
    void sendFanoronaPass();
    void onReadyRead();


private:
    Ui::MainMenu *ui;
    QTcpSocket *socket;

    QString loggedInUser;
    Fanorona *fanoronaGame = nullptr;


};

#endif