#include "mainmenu.h"
#include "ui_mainmenu.h"
#include "profileform.h"
#include "leaderboard.h"
#include "store.h"
#include "gameform.h"
#include "morrisgameform.h"
#include "fanorona.h"
#include <QMessageBox>
#include <QDebug>

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


    connect(socket, &QTcpSocket::readyRead, this, &MainMenu::onReadyRead);
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

    disconnect(socket, &QTcpSocket::readyRead, this, &MainMenu::onReadyRead);

    ProfileForm *profileWindow = new ProfileForm(socket);
    profileWindow->setAttribute(Qt::WA_DeleteOnClose);

    connect(profileWindow, &QObject::destroyed, this, [this]() {
        connect(socket, &QTcpSocket::readyRead, this, &MainMenu::onReadyRead);
    });

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


    socket->write("REQUEST_DOTS\n");
}

void MainMenu::on_startMorrisButton_clicked()
{
    QString c1 = ui->comboColorP1->currentText();
    QString c2 = ui->comboColorP2->currentText();

    if (c1 == c2) {
        QMessageBox::warning(this, "خطا در انتخاب رنگ", "بازیکن ۱ و بازیکن ۲ نمی‌توانند رنگ یکسان انتخاب کنند!");
        return;
    }

    QString msg = "REQUEST_MORRIS\n";
    socket->write(msg.toUtf8());
}

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
    while (socket->canReadLine()) {
        QString message = QString::fromUtf8(socket->readLine()).trimmed();


        if (message.startsWith("START_DOTS")) {

            QStringList parts = message.split("|");
            if (parts.size() == 2) {
                disconnect(socket, &QTcpSocket::readyRead, this, &MainMenu::onReadyRead);

                int role = parts[1].toInt();
                QString c1 = ui->comboColorP1->currentText();
                QString c2 = ui->comboColorP2->currentText();

                GameForm *game = new GameForm(socket, QColor(c1), QColor(c2), role);
                game->setAttribute(Qt::WA_DeleteOnClose);
                game->show();

                this->hide();
            }
        }
        else if (message.startsWith("START_FANORONA")) {
            QStringList parts = message.split("|");
            if (parts.size() == 2) {
                int role = parts[1].toInt();
                if (fanoronaGame != nullptr) delete fanoronaGame;
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
                fanoronaGame->applyOpponentMove(parts[1].toInt(), parts[2].toInt(), parts[3].toInt(), parts[4].toInt());
            }
        }
        else if (message.startsWith("FANORONA_PASS")) {
            if (fanoronaGame != nullptr) fanoronaGame->applyOpponentPass();
        }
        else if (message.startsWith("OPPONENT_DISCONNECTED")) {
            if (fanoronaGame != nullptr) fanoronaGame->handleDisconnect();
        }
        else if (message.startsWith("START_MORRIS")) {

            QStringList parts = message.split("|");
            if (parts.size() == 2) {
                disconnect(socket, &QTcpSocket::readyRead, this, &MainMenu::onReadyRead);

                int role = parts[1].toInt();
                QString c1 = ui->comboColorP1->currentText();
                QString c2 = ui->comboColorP2->currentText();

                MorrisGameForm *morrisGame = new MorrisGameForm(socket, QColor(c1), QColor(c2), role);
                morrisGame->setAttribute(Qt::WA_DeleteOnClose);
                morrisGame->show();

                this->hide();
            }
        }
    }
}