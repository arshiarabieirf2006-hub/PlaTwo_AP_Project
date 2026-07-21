#ifndef STORE_H
#define STORE_H

#include <QWidget>
#include <QTcpSocket>
#include <QListWidgetItem>

namespace Ui {
class Store;
}

class Store : public QWidget
{
    Q_OBJECT

public:
explicit Store(QString username = "", QWidget *parent = nullptr);
    ~Store();

private slots:
    void on_backButton_clicked();
    void on_buyButton_clicked();
    void onReadyRead();

private:
    Ui::Store *ui;
    QTcpSocket *socket;
    QString currentUsername;
    int userCoins;

    void loadStoreItems();
};

#endif // STORE_H