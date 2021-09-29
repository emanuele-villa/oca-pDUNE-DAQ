#ifndef SLOWCONTROL_H
#define SLOWCONTROL_H

#include <QDialog>
#include "de10silicon.h"

namespace Ui {
class slowControl;
}

class slowControl : public QDialog
{
    Q_OBJECT

public:
    explicit slowControl(QWidget *parent = nullptr);
    ~slowControl();

private:
    Ui::slowControl *ui;
};

#endif // SLOWCONTROL_H
