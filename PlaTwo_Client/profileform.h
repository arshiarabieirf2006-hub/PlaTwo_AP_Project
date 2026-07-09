#ifndef PROFILEFORM_H
#define PROFILEFORM_H

#include <QWidget>

namespace Ui {
class ProfileForm;
}

class ProfileForm : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileForm(QWidget *parent = nullptr);
    ~ProfileForm();

private:
    Ui::ProfileForm *ui;
};

#endif // PROFILEFORM_H
