#ifndef GAMEFORM_H
#define GAMEFORM_H

#include <QWidget>
#include <QGraphicsScene>
namespace Ui {
class GameForm;
}

class GameForm : public QWidget
{
    Q_OBJECT

public:
    explicit GameForm(QWidget *parent = nullptr);
    ~GameForm();

private:
    Ui::GameForm *ui;
    QGraphicsScene *scene;
};

#endif // GAMEFORM_H
