#include "profileform.h"
#include "ui_profileform.h"

ProfileForm::ProfileForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ProfileForm)
{
    ui->setupUi(this);
}

ProfileForm::~ProfileForm()
{
    delete ui;
}
