#include "comandiprincipali.h"
#include "ui_comandiprincipali.h"
#include "de10silicon.h"
#include <QObject>
#include <QtCharts>
#include <QChartView>
#include <QLineSeries>
#include <QString>

comandiPrincipali::comandiPrincipali(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::comandiPrincipali),
    de10("192.168.10.100", 5000)
{
    ui->setupUi(this);
    connect(&de10, SIGNAL(textChanged(QString)), ui->dati, SLOT(append(QString)));
    const bool connected = connect(&de10, SIGNAL(data(QVector<double>)), this, SLOT(grafico(QVector<double>)));
    series = new QLineSeries();
}

comandiPrincipali::~comandiPrincipali()
{
    delete ui;
}

void comandiPrincipali::on_init_clicked()
{
    de10.Init();
}


void comandiPrincipali::on_getevent_clicked()
{
    de10.GetEvent();
}


void comandiPrincipali::on_setdelay_clicked()
{
    de10.SetDelay();
}


void comandiPrincipali::on_setmode_clicked()
{
    de10.SetMode();
}


void comandiPrincipali::on_geteventnumber_clicked()
{
    de10.GetEventNumber();
}


void comandiPrincipali::on_printeventnumber_clicked()
{
    de10.PrintAllEventNumber();
}


void comandiPrincipali::on_eventreset_clicked()
{
    de10.EventReset();
}


void comandiPrincipali::on_overwritedelay_clicked()
{
    de10.OverWriteDelay();
}


void comandiPrincipali::on_calibrate_clicked()
{
    de10.Calibrate();
}


void comandiPrincipali::on_writecalibpar_clicked()
{
    de10.WriteCalibPar();
}


void comandiPrincipali::on_savecalibration_clicked()
{
    de10.SaveCalibrations();
}


void comandiPrincipali::on_avvia_clicked()
{
    de10.Init();
    sleep(1);
    int i = 0;
    while(i < 15){

        de10.GetEvent();
        i++;
        QCoreApplication::processEvents();
        sleep(1);
    }
}


void comandiPrincipali::on_stop_clicked()
{

}

void comandiPrincipali::grafico(QVector<double> event){

    /*QLineSeries *series = new QLineSeries();
    for(int i = 0; i < 4; i++){

        series->append(i, event[i]);
    }

    QChart *chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    chart->setTitle("slot");

    QChartView *view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    //view->setParent(ui->frame);
    view->repaint();
    ui->dati->append((char*)(event[0]));*/

    QVector<double> x(110);
    for(int i = 0; i < 110; i++){

        x[i] = i;
    }

    ui->dati->append("get event");
    ui->plot->addGraph();
    ui->plot->graph(0)->setData(x, event);
    ui->plot->xAxis->setLabel("x");
    ui->plot->yAxis->setLabel("y");
    ui->plot->xAxis->setRange(1,110);
    ui->plot->yAxis->setRange(1,999999999999999);
    ui->plot->replot();


}
