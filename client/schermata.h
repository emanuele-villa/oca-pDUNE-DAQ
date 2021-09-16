#ifndef SCHERMATA_H
#define SCHERMATA_H

#include <QDialog>
#include "de10silicon.h"
#include <QtCharts>
#include <QChartView>
#include <QLineSeries>


namespace Ui {
class Schermata;
}

class Schermata : public QDialog
{
    Q_OBJECT

public:
    explicit Schermata(QWidget *parent = nullptr);
    ~Schermata();
    de10_silicon de10;
private slots:
    void on_init_clicked();

    void on_getevent_clicked();

    void on_avvia_clicked();

    void grafico(QVector<double> event);



    void on_setdelay_clicked();

    void on_setmode_clicked();

    void on_geteventnumber_clicked();

    void on_printeventnumber_clicked();

    void on_eventreset_clicked();

    void on_overwritedelay_clicked();

    void on_calibrate_clicked();

    void on_writecalibpar_clicked();

    void on_savecalibration_clicked();

    void on_stop_clicked();

private:
    Ui::Schermata *ui;
    QLineSeries *series;
};

#endif // SCHERMATA_H
