#include "store.h"
#include "ui_store.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

Store::Store(QString username, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Store),
    currentUsername(username),
    userCoins(0)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &Store::onReadyRead);
    socket->connectToHost("127.0.0.1", 8080);

    if (socket->waitForConnected(2000)) {
        QString request = "GET_COINS:" + currentUsername;
        socket->write(request.toUtf8());
    } else {
        ui->coinsLabel->setText("Coins: Connection Error");
    }

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

        // ارسال درخواست خرید به سرور
        if (socket->state() == QAbstractSocket::ConnectedState) {
            QString request = "BUY_ITEM:" + currentUsername + ":" + QString::number(price);
            socket->write(request.toUtf8());
        } else {
            QMessageBox::critical(this, "Error", "Not connected to server!");
        }
    }
}

void Store::onReadyRead()
{
    QByteArray response = socket->readAll();
    QString msg = QString::fromUtf8(response).trimmed();

    QStringList parts = msg.split(":");

    if (parts[0] == "COINS_RESULT" && parts.size() >= 2) {
        userCoins = parts[1].toInt();
        ui->coinsLabel->setText("Your Coins: " + QString::number(userCoins));
    }
    else if (parts[0] == "BUY_SUCCESS" && parts.size() >= 2) {
        userCoins = parts[1].toInt();
        ui->coinsLabel->setText("Your Coins: " + QString::number(userCoins));
        QMessageBox::information(this, "Success", "Item purchased successfully!");
    }
    else if (msg == "BUY_FAILED_NO_COINS") {
        QMessageBox::critical(this, "Inadequate Funds", "You don't have enough coins!");
    }
}

void Store::on_backButton_clicked()
{
    this->close();
}