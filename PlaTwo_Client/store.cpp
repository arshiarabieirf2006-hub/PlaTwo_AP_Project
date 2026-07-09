#include "store.h"
#include "ui_store.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

Store::Store(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Store),
    userCoins(1000)
{
    ui->setupUi(this);


    ui->coinsLabel->setText("Your Coins: " + QString::number(userCoins));


    loadStoreItems();
}

Store::~Store()
{
    delete ui;
}

void Store::loadStoreItems()
{
    ui->itemsList->clear();


    QFile file("items.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(",");
            if (parts.size() >= 2) {
                QString itemName = parts[0].trimmed();
                QString price = parts[1].trimmed();


                ui->itemsList->addItem(itemName + " - Price: " + price + " Coins");
            }
        }
        file.close();
    } else {
        ui->itemsList->addItem("Double Jump - Price: 200 Coins");
        ui->itemsList->addItem("Shield Protection - Price: 450 Coins");
        ui->itemsList->addItem("Golden Skin - Price: 800 Coins");
    }
}

void Store::on_buyButton_clicked()
{
    QListWidgetItem *selectedItem = ui->itemsList->currentItem();

    if (!selectedItem) {
        QMessageBox::warning(this, "Selection Error", "Please select an item to buy!");
        return;
    }

    QString itemText = selectedItem->text();
    QStringList parts = itemText.split("Price: ");
    if (parts.size() >= 2) {
        int price = parts[1].split(" ").first().toInt();
        QString itemName = parts[0].split(" -").first();

        if (userCoins >= price) {
            userCoins -= price;
            ui->coinsLabel->setText("Your Coins: " + QString::number(userCoins));
            QMessageBox::information(this, "Success", "You successfully bought: " + itemName);
        } else {
            QMessageBox::critical(this, "Inadequate Funds", "You don't have enough coins!");
        }
    }
}

void Store::on_backButton_clicked()
{
    this->close();
}
