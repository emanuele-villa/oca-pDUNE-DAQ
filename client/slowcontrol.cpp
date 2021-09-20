#include "slowcontrol.h"
#include "ui_slowcontrol.h"

slowControl::slowControl(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::slowControl)
{
    ui->setupUi(this);
}

slowControl::~slowControl()
{
    delete ui;
}
