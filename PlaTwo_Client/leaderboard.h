#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <QWidget>
#include <QTcpSocket>

namespace Ui {
class Leaderboard;
}

class Leaderboard : public QWidget
{
    Q_OBJECT

public:
    explicit Leaderboard(QWidget *parent = nullptr);
    ~Leaderboard();

private slots:
    void on_refreshButton_clicked();
    void on_backButton_clicked();
    void onReadyRead();

private:
    Ui::Leaderboard *ui;
    QTcpSocket *socket;
    void requestLeaderboardData();
};

#endif // LEADERBOARD_H