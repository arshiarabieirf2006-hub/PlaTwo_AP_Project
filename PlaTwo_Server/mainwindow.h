#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNewConnection(); // وقتی کلاینت جدید وصل می‌شود
    void onReadyRead();      // وقتی کلاینتی پیام می‌فرستد

private:
    Ui::MainWindow *ui;
    QTcpServer *server;
    QVector<QTcpSocket*> clients;
};
#endif // MAINWINDOW_H