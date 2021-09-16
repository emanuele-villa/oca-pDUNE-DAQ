#include "schermata.h"
#include "ui_schermata.h"
#include "mainwindow.h"
#include "de10silicon.h"
#include <sys/time.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <QObject>
#include <QtCharts>
#include <QChartView>
#include <QLineSeries>

QT_CHARTS_USE_NAMESPACE

Schermata::Schermata(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Schermata),
    de10("127.0.0.1", 5000)
{
    ui->setupUi(this);
    ui->dati->append("connesso socket comandi principali");
    connect(&de10, SIGNAL(textChanged(QString)), ui->dati, SLOT(append(QString)));
    const bool connected = connect(&de10, SIGNAL(data(QVector<double>)), this, SLOT(grafico(QVector<double>)));
    qDebug() << "Connection established?" << connected;
    series = new QLineSeries();

}

Schermata::~Schermata()
{
    delete ui;


}


void Schermata::on_init_clicked()
{
    de10.Init();

}

void Schermata::on_getevent_clicked()
{
    de10.GetEvent();
}



void Schermata::on_avvia_clicked()
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

void Schermata::grafico(QVector<double> event){

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

    QVector<double> x(640);
    for(int i = 0; i < 640; i++){

        x[i] = i;
    }

    ui->dati->append("get event");
    ui->plot->addGraph();
    ui->plot->graph(0)->setData(x, event);
    ui->plot->xAxis->setLabel("x");
    ui->plot->yAxis->setLabel("y");
    ui->plot->xAxis->setRange(1,640);
    ui->plot->yAxis->setRange(1,100);
    ui->plot->replot();


}


void Schermata::on_setdelay_clicked()
{
    de10.SetDelay();
}

void Schermata::on_setmode_clicked()
{
    de10.SetMode();
}

void Schermata::on_geteventnumber_clicked()
{
    de10.GetEventNumber();
}

void Schermata::on_printeventnumber_clicked()
{
    de10.PrintAllEventNumber();
}

void Schermata::on_eventreset_clicked()
{
    de10.EventReset();
}

void Schermata::on_overwritedelay_clicked()
{
    de10.OverWriteDelay();
}

void Schermata::on_calibrate_clicked()
{
    de10.Calibrate();
}

void Schermata::on_writecalibpar_clicked()
{
    de10.WriteCalibPar();
}

void Schermata::on_savecalibration_clicked()
{

}

void Schermata::on_stop_clicked()
{

}
