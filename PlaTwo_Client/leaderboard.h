#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <QWidget>

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

private:
    Ui::Leaderboard *ui;
    void loadLeaderboardData();
};

#endif // LEADERBOARD_H
