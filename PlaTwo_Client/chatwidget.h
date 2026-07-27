#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QStringList>


class ChatWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChatWidget(QWidget *parent = nullptr);

    void addIncomingText(const QString &text);
    void addIncomingSticker(int stickerIndex);

    static int stickerCount();

signals:

    void sendTextRequested(const QString &text);
    void sendStickerRequested(int stickerIndex);

private slots:
    void onSendClicked();
    void onStickerButtonClicked();

private:
    QTextEdit *historyView;
    QLineEdit *inputEdit;
    QPushButton *sendButton;

    static const QStringList STICKERS;

    void appendLine(const QString &html);
};

#endif // CHATWIDGET_H