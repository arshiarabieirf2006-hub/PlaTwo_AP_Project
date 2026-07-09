#include "mainmenu.h"
#include "ui_mainmenu.h"
#include "profileform.h"
#include "leaderboard.h"
#include "store.h"

MainMenu::MainMenu(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainMenu)
{
    ui->setupUi(this);
}

MainMenu::~MainMenu()
{
    delete ui;
}

void MainMenu::setUsername(const QString &username)
{
    loggedInUser = username;
}

void MainMenu::on_exitButton_clicked()
{
    QCoreApplication::quit();
}

void MainMenu::on_profileButton_clicked()
{

    ProfileForm *profileWindow = new ProfileForm();


    profileWindow->loadUserData(loggedInUser);


    profileWindow->show();
}
void MainMenu::on_leaderboardButton_clicked()
{
    Leaderboard *lbPage = new Leaderboard();
    lbPage->setAttribute(Qt::WA_DeleteOnClose);
    lbPage->show();
}


void MainMenu::on_storeButton_clicked()
{
    Store *storePage = new Store();
    storePage->setAttribute(Qt::WA_DeleteOnClose);
    storePage->show();
}

