#include "mainmenu.h"
#include "ui_mainmenu.h"
#include "profileform.h"
#include "leaderboard.h"
#include "store.h"
#include "gameform.h"
#include "morrisgameform.h"

MainMenu::MainMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainMenu)
{
    ui->setupUi(this);


    QStringList colors = {"Red", "Blue", "Green", "Yellow", "Purple", "Orange", "Pink", "Cyan", "Brown"};


    ui->comboColorP1->addItems(colors);
    ui->comboColorP2->addItems(colors);


    ui->comboColorP1->setCurrentText("Red");
    ui->comboColorP2->setCurrentText("Blue");

    socket = new QTcpSocket(this);

    socket->connectToHost("127.0.0.1", 8080);

    connect(socket, &QTcpSocket::connected, this, [](){
        qDebug() << "Successfully connected to the server!";
    });
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
    Store *storeWindow = new Store();
    storeWindow->setAttribute(Qt::WA_DeleteOnClose);
    storeWindow->show();
}


void MainMenu::on_startGameButton_clicked()
{
    QString c1 = ui->comboColorP1->currentText();
    QString c2 = ui->comboColorP2->currentText();

    GameForm *game = new GameForm(socket, QColor(c1), QColor(c2));
    game->setAttribute(Qt::WA_DeleteOnClose);
    game->show();

    this->close();
}

void MainMenu::on_startMorrisButton_clicked()
{
    QString c1 = ui->comboColorP1->currentText();
    QString c2 = ui->comboColorP2->currentText();

    MorrisGameForm *morrisGame = new MorrisGameForm(socket, QColor(c1), QColor(c2));
    morrisGame->setAttribute(Qt::WA_DeleteOnClose);
    morrisGame->show();

    this->close();
}

