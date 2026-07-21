#include "leaderboard.h"
#include "ui_leaderboard.h"
#include <QMessageBox>
#include <QTableWidgetItem>

Leaderboard::Leaderboard(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Leaderboard)
{
    ui->setupUi(this);

    ui->leaderboardTable->setColumnCount(3);
    ui->leaderboardTable->setHorizontalHeaderLabels({"Rank", "Username", "Score"});

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &Leaderboard::onReadyRead);
    socket->connectToHost("127.0.0.1", 8080);

    requestLeaderboardData();
}

Leaderboard::~Leaderboard()
{
    delete ui;
}

void Leaderboard::requestLeaderboardData()
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->write("GET_LEADERBOARD\n");
    } else if (socket->waitForConnected(2000)) {
        socket->write("GET_LEADERBOARD\n");
    } else {
        QMessageBox::critical(this, "Connection Error", "Cannot connect to server!");
    }
}

void Leaderboard::onReadyRead()
{
    QByteArray response = socket->readAll();
    QString msg = QString::fromUtf8(response).trimmed();

    if (msg.startsWith("LEADERBOARD_RESULT:")) {
        ui->leaderboardTable->setRowCount(0);

        QString data = msg.mid(19); // حذف پیشوند "LEADERBOARD_RESULT:"
        if (data.isEmpty()) return;

        QStringList rows = data.split(";");
        ui->leaderboardTable->setRowCount(rows.size());

        for (int i = 0; i < rows.size(); ++i) {
            QStringList parts = rows[i].split(",");
            if (parts.size() >= 2) {
                QString username = parts[0];
                QString score = parts[1];

                ui->leaderboardTable->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
                ui->leaderboardTable->setItem(i, 1, new QTableWidgetItem(username));
                ui->leaderboardTable->setItem(i, 2, new QTableWidgetItem(score));
            }
        }
    }
}

void Leaderboard::on_refreshButton_clicked()
{
    requestLeaderboardData();
}

void Leaderboard::on_backButton_clicked()
{
    this->close();
}