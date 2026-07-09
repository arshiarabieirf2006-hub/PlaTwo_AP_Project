#ifndef STORE_H
#define STORE_H

#include <QWidget>

namespace Ui {
class Store;
}

class Store : public QWidget
{
    Q_OBJECT

public:
    explicit Store(QWidget *parent = nullptr);
    ~Store();

private slots:


    void on_backButton_clicked();

    void on_buyButton_clicked();

private:
    Ui::Store *ui;
    int userCoins;
    void loadStoreItems();
};

#endif // STORE_H
