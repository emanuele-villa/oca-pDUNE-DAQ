#ifndef COMANDIPRINCIPALI_H
#define COMANDIPRINCIPALI_H

#include <QDialog>
#include "de10silicon.h"
#include <QLineSeries>

namespace Ui {
class comandiPrincipali;
}

class comandiPrincipali : public QDialog
{
    Q_OBJECT

public:
    explicit comandiPrincipali(QWidget *parent = nullptr);
    ~comandiPrincipali();
    de10_silicon de10;
private slots:
    void grafico(QVector<double> event);

    void on_init_clicked();

    void on_getevent_clicked();

    void on_setdelay_clicked();

    void on_setmode_clicked();

    void on_geteventnumber_clicked();

    void on_printeventnumber_clicked();

    void on_eventreset_clicked();

    void on_overwritedelay_clicked();

    void on_calibrate_clicked();

    void on_writecalibpar_clicked();

    void on_savecalibration_clicked();

    void on_avvia_clicked();

    void on_stop_clicked();

    void on_readReg_clicked();

private:
    Ui::comandiPrincipali *ui;
    QLineSeries *series;

};

#endif // COMANDIPRINCIPALI_H
