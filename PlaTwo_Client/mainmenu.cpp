#include "mainmenu.h"
#include "ui_mainmenu.h"
#include "profileform.h"
#include "leaderboard.h"
#include "store.h"
#include "gameform.h"
#include "morrisgameform.h"
<<<<<<< HEAD
#include "fanorona.h"
=======
#include <QMessageBox>
#include <QDebug>
>>>>>>> a420f1dfa0fb7761521f6f848030d4dea0cc1278

MainMenu::MainMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainMenu),
    myPlayerId(1)
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


<<<<<<< HEAD
    connect(socket, &QTcpSocket::readyRead, this, &MainMenu::onReadyRead);
=======
    m_playerIdConnection = connect(socket, &QTcpSocket::readyRead, this, &MainMenu::onSocketReadyRead);
>>>>>>> a420f1dfa0fb7761521f6f848030d4dea0cc1278
}

MainMenu::~MainMenu()
{
    delete ui;
}

void MainMenu::setUsername(const QString &username)
{
    loggedInUser = username;
}

void MainMenu::onSocketReadyRead()
{
    while (socket->canReadLine()) {
        QString line = QString::fromUtf8(socket->readLine()).trimmed();
        if (line.startsWith("PLAYERID:")) {
            bool ok = false;
            int id = line.section(':', 1, 1).toInt(&ok);
            if (ok && (id == 1 || id == 2)) {
                myPlayerId = id;
                qDebug() << "Assigned player id:" << myPlayerId;
            }
            disconnect(m_playerIdConnection);
            return;
        }
    }
}

void MainMenu::on_exitButton_clicked()
{
    QCoreApplication::quit();
}

void MainMenu::on_profileButton_clicked()
{
<<<<<<< HEAD
    ProfileForm *profileWindow = new ProfileForm();
=======

    ProfileForm *profileWindow = new ProfileForm(socket);
    profileWindow->setAttribute(Qt::WA_DeleteOnClose);

>>>>>>> a420f1dfa0fb7761521f6f848030d4dea0cc1278
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

    if (c1 == c2) {
        QMessageBox::warning(this, "خطا در انتخاب رنگ", "بازیکن ۱ و بازیکن ۲ نمی‌توانند رنگ یکسان انتخاب کنند!");
        return;
    }

    GameForm *game = new GameForm(socket, QColor(c1), QColor(c2), myPlayerId);
    game->setAttribute(Qt::WA_DeleteOnClose);
    game->show();


    this->hide();
}

void MainMenu::on_startMorrisButton_clicked()
{
    QString c1 = ui->comboColorP1->currentText();
    QString c2 = ui->comboColorP2->currentText();

    if (c1 == c2) {
        QMessageBox::warning(this, "خطا در انتخاب رنگ", "بازیکن ۱ و بازیکن ۲ نمی‌توانند رنگ یکسان انتخاب کنند!");
        return;
    }

    MorrisGameForm *morrisGame = new MorrisGameForm(socket, QColor(c1), QColor(c2), myPlayerId);
    morrisGame->setAttribute(Qt::WA_DeleteOnClose);
    morrisGame->show();

    this->hide();
}
<<<<<<< HEAD

void MainMenu::on_pushButton_clicked()
{

    QString msg = "REQUEST_FANORONA";
    socket->write(msg.toUtf8());
}

void MainMenu::sendFanoronaMove(int startRow, int startCol, int endRow, int endCol)
{
    QString msg = QString("FANORONA_MOVE|%1|%2|%3|%4").arg(startRow).arg(startCol).arg(endRow).arg(endCol);
    socket->write(msg.toUtf8());
}

void MainMenu::sendFanoronaPass()
{
    QString msg = "FANORONA_PASS";
    socket->write(msg.toUtf8());
}

void MainMenu::onReadyRead()
{
    QByteArray data = socket->readAll();
    QString message = QString::fromUtf8(data);


    if (message.startsWith("START_FANORONA")) {
        QStringList parts = message.split("|");
        if (parts.size() == 2) {
            int role = parts[1].toInt();

            if (fanoronaGame != nullptr) {
                delete fanoronaGame;
            }


            fanoronaGame = new Fanorona(role);
            fanoronaGame->setAttribute(Qt::WA_DeleteOnClose);


            connect(fanoronaGame, &Fanorona::movePlayed, this, &MainMenu::sendFanoronaMove);
            connect(fanoronaGame, &Fanorona::turnPassed, this, &MainMenu::sendFanoronaPass);

            fanoronaGame->show();
        }
    }

    else if (message.startsWith("FANORONA_MOVE")) {
        QStringList parts = message.split("|");
        if (parts.size() == 5 && fanoronaGame != nullptr) {
            int sr = parts[1].toInt();
            int sc = parts[2].toInt();
            int er = parts[3].toInt();
            int ec = parts[4].toInt();

            fanoronaGame->applyOpponentMove(sr, sc, er, ec);
        }
    }

    else if (message.startsWith("FANORONA_PASS")) {
        if (fanoronaGame != nullptr) {
            fanoronaGame->applyOpponentPass();
        }
    }

    else if (message.startsWith("FANORONA_PASS")) {
        if (fanoronaGame != nullptr) {
            fanoronaGame->applyOpponentPass();
        }
    }

    else if (message.startsWith("OPPONENT_DISCONNECTED")) {
        if (fanoronaGame != nullptr) {
            fanoronaGame->handleDisconnect();
        }
    }
}
=======
>>>>>>> a420f1dfa0fb7761521f6f848030d4dea0cc1278
