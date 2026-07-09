#include "leaderboard.h"
#include "ui_leaderboard.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QVector>
#include <QPair>
#include <algorithm>

Leaderboard::Leaderboard(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Leaderboard)
{
    ui->setupUi(this);

    ui->leaderboardTable->setColumnCount(3);
    ui->leaderboardTable->setHorizontalHeaderLabels({"Rank", "Username", "Score"});

    loadLeaderboardData();
}

Leaderboard::~Leaderboard()
{
    delete ui;
}

void Leaderboard::loadLeaderboardData()
{

    ui->leaderboardTable->setRowCount(0);

    QVector<QPair<QString, int>> players;

    QFile file("users.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(",");

            if (parts.size() >= 6) {
                QString username = parts[0].trimmed();
                int score = parts[5].trimmed().toInt();
                players.append(qMakePair(username, score));
            }
        }
        file.close();
    }


    std::sort(players.begin(), players.end(), [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
        return a.second > b.second;
    });


    ui->leaderboardTable->setRowCount(players.size());


    for (int i = 0; i < players.size(); ++i) {
        ui->leaderboardTable->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
        ui->leaderboardTable->setItem(i, 1, new QTableWidgetItem(players[i].first));
        ui->leaderboardTable->setItem(i, 2, new QTableWidgetItem(QString::number(players[i].second)));
    }
}


void Leaderboard::on_refreshButton_clicked()
{
    loadLeaderboardData();
}


void Leaderboard::on_backButton_clicked()
{
     this->close();
}

