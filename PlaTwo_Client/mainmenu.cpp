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


    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainMenu::requestHistory);

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
        qDebug() << "Client received:" << message;

        if (message.startsWith("START_DOTS")) {
            QStringList parts = message.split("|");
            if (parts.size() == 2) {
                disconnect(socket, &QTcpSocket::readyRead, this, &MainMenu::onReadyRead);

                int role = parts[1].toInt();
                QString c1 = ui->comboColorP1->currentText();
                QString c2 = ui->comboColorP2->currentText();

                GameForm *game = new GameForm(socket, QColor(c1), QColor(c2), role);
                game->setUsername(loggedInUser);
                game->setAttribute(Qt::WA_DeleteOnClose);
                connect(game, &QObject::destroyed, this, [this]() {
                    connect(socket, &QTcpSocket::readyRead, this, &MainMenu::onReadyRead);
                });
                game->show();
                return;
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
                connect(fanoronaGame, &Fanorona::chatTextSent, this, [this](const QString &text) {
                    socket->write(("CHAT_TEXT:" + text + "\n").toUtf8());
                });
                connect(fanoronaGame, &Fanorona::chatStickerSent, this, [this](int index) {
                    socket->write(QString("CHAT_STICKER:%1\n").arg(index).toUtf8());
                });
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
        else if (message.startsWith("CHAT_TEXT:")) {
            if (fanoronaGame != nullptr) {
                fanoronaGame->receiveChatText(message.mid(QString("CHAT_TEXT:").length()));
            }
        }
        else if (message.startsWith("CHAT_STICKER:")) {
            if (fanoronaGame != nullptr) {
                fanoronaGame->receiveChatSticker(message.mid(QString("CHAT_STICKER:").length()).toInt());
            }
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
                connect(morrisGame, &QObject::destroyed, this, [this]() {
                    connect(socket, &QTcpSocket::readyRead, this, &MainMenu::onReadyRead);
                });
                morrisGame->show();
                return;
            }
        }
        else if (message.startsWith("HISTORY_RESULT:")) {
            QString data = message.mid(QString("HISTORY_RESULT:").length());
            showHistoryDialog(data);
        }
    }
}

void MainMenu::requestHistory()
{

    QMessageBox::information(this, "کلاینت", "دکمه تاریخچه کلیک شد! درخواست به سرور ارسال می‌شود...");

    if (socket && socket->state() == QAbstractSocket::ConnectedState) {
        QString msg = QString("GET_HISTORY:%1\n").arg(loggedInUser);
        socket->write(msg.toUtf8());
    } else {
        QMessageBox::warning(this, "Error", "Not connected to server!");
    }
}

void MainMenu::showHistoryDialog(const QString &data)
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Game History");
    dialog->resize(700, 400);

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    QTableWidget *table = new QTableWidget(dialog);

    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"Game", "Opponent", "Role", "Winner", "Score", "Date & Time"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setStyleSheet("QTableWidget { font-size: 14px; } QHeaderView::section { font-weight: bold; background-color: #2b2b2b; color: white; }");

    if (data == "EMPTY") {
        table->setRowCount(1);
        table->setItem(0, 0, new QTableWidgetItem("No games played yet."));
        table->setSpan(0, 0, 1, 6);
        table->item(0, 0)->setTextAlignment(Qt::AlignCenter);
    } else {
        QStringList games = data.split(";", Qt::SkipEmptyParts);
        table->setRowCount(games.size());
        for (int i = 0; i < games.size(); ++i) {
            QStringList details = games[i].split(",");
            if (details.size() >= 7) {
                table->setItem(i, 0, new QTableWidgetItem(details[1]));
                table->setItem(i, 1, new QTableWidgetItem(details[2]));
                table->setItem(i, 2, new QTableWidgetItem(details[3]));
                table->setItem(i, 3, new QTableWidgetItem(details[4]));
                table->setItem(i, 4, new QTableWidgetItem(details[5]));

                QString displayDate = details[6];
                displayDate.replace("-", ":");
                table->setItem(i, 5, new QTableWidgetItem(displayDate));
            }
        }
    }

    layout->addWidget(table);
    dialog->exec();
}