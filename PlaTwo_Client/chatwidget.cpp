#include "chatwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QFont>

const QStringList ChatWidget::STICKERS = {"😀", "😂", "😍", "👍", "😢", "🎉"};

ChatWidget::ChatWidget(QWidget *parent) : QWidget(parent)
{
    setStyleSheet("QWidget#chatRoot { background-color: rgba(25,25,25,225); border-radius: 8px; }");
    setObjectName("chatRoot");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(4);

    historyView = new QTextEdit(this);
    historyView->setReadOnly(true);
    historyView->setStyleSheet(
        "background-color: rgba(0,0,0,160); color: white; font-size: 12px; border: none; border-radius: 4px;");
    mainLayout->addWidget(historyView, 1);

    QHBoxLayout *stickerRow = new QHBoxLayout();
    stickerRow->setSpacing(2);

    QFont emojiFont;
    emojiFont.setFamilies({"Segoe UI Emoji", "Noto Color Emoji", "Apple Color Emoji"});
    emojiFont.setPointSize(14);

    for (int i = 0; i < STICKERS.size(); ++i) {
        QPushButton *btn = new QPushButton(STICKERS[i], this);
        btn->setFixedSize(28, 28);
        btn->setFont(emojiFont);
        btn->setProperty("stickerIndex", i);
        connect(btn, &QPushButton::clicked, this, &ChatWidget::onStickerButtonClicked);
        stickerRow->addWidget(btn);
    }
    mainLayout->addLayout(stickerRow);

    QHBoxLayout *inputRow = new QHBoxLayout();
    inputEdit = new QLineEdit(this);
    inputEdit->setPlaceholderText("Message...");
    sendButton = new QPushButton("Send", this);
    inputRow->addWidget(inputEdit, 1);
    inputRow->addWidget(sendButton);
    mainLayout->addLayout(inputRow);

    connect(sendButton, &QPushButton::clicked, this, &ChatWidget::onSendClicked);
    connect(inputEdit, &QLineEdit::returnPressed, this, &ChatWidget::onSendClicked);
}

int ChatWidget::stickerCount()
{
    return STICKERS.size();
}

void ChatWidget::onSendClicked()
{
    QString text = inputEdit->text().trimmed();
    if (text.isEmpty()) return;


    text.replace('\n', ' ').replace('\r', ' ');

    appendLine(QString("<b>You:</b> %1").arg(text.toHtmlEscaped()));
    emit sendTextRequested(text);
    inputEdit->clear();
}

void ChatWidget::onStickerButtonClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    int index = btn->property("stickerIndex").toInt();
    if (index < 0 || index >= STICKERS.size()) return;

    appendLine(QString("<b>You:</b> <span style=\"font-family:'Segoe UI Emoji','Noto Color Emoji','Apple Color Emoji'; font-size:22px\">%1</span>").arg(STICKERS[index]));
    emit sendStickerRequested(index);
}

void ChatWidget::addIncomingText(const QString &text)
{
    appendLine(QString("<b>Opponent:</b> %1").arg(text.toHtmlEscaped()));
}

void ChatWidget::addIncomingSticker(int stickerIndex)
{
    if (stickerIndex < 0 || stickerIndex >= STICKERS.size()) return;
    appendLine(QString("<b>Opponent:</b> <span style=\"font-family:'Segoe UI Emoji','Noto Color Emoji','Apple Color Emoji'; font-size:22px\">%1</span>").arg(STICKERS[stickerIndex]));
}

void ChatWidget::appendLine(const QString &html)
{
    historyView->append(html);
    QScrollBar *sb = historyView->verticalScrollBar();
    if (sb) sb->setValue(sb->maximum());
}